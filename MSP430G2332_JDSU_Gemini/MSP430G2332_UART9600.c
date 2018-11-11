//******************************************************************************
//  MSP430G2xx2 Demo - Timer_A, Ultra-Low Pwr UART 9600 Echo, 32kHz ACLK
//
//  Description: Use Timer_A CCR0 hardware output modes and SCCI data latch
//  to implement UART function @ 9600 baud. Software does not directly read and
//  write to RX and TX pins, instead proper use of output modes and SCCI data
//  latch are demonstrated. Use of these hardware features eliminates ISR
//  latency effects as hardware insures that output and input bit latching and
//  timing are perfectly synchronised with Timer_A regardless of other
//  software activity. In the Mainloop the UART function readies the UART to
//  receive one character and waits in LPM3 with all activity interrupt driven.
//  After a character has been received, the UART receive function forces exit
//  from LPM3 in the Mainloop which configures the port pins (P1 & P2) based
//  on the value of the received byte (i.e., if BIT0 is set, turn on P1.0).

//  ACLK = TACLK = LFXT1 = 32768Hz, MCLK = SMCLK = default DCO
//  //* An external watch crystal is required on XIN XOUT for ACLK *//  
//
//               MSP430G2xx2
//            -----------------
//        /|\|              XIN|-
//         | |                 | 32kHz
//         --|RST          XOUT|-
//           |                 |
//           |   CCI0B/TXD/P1.1|-------->
//           |                 | 9600 8N1
//           |   CCI0A/RXD/P1.2|<--------
//
//  D. Dang
//  Texas Instruments Inc.
//  December 2010
//  Built with CCS Version 4.2.0 and IAR Embedded Workbench Version: 5.10
//******************************************************************************

//SJ5090312 - SJUART
#if 1   //SJ5090312 - If this file is not needed, negate it

#include "msp430g2452.h"

#include   "global_var.h"  //SJ5090312 - Added

//------------------------------------------------------------------------------
// Hardware-related definitions
//------------------------------------------------------------------------------
#define UART_TXD   0x02                     // TXD on P1.1 (Timer0_A.OUT0)
#define UART_RXD   0x04                     // RXD on P1.2 (Timer0_A.CCI1A)

//------------------------------------------------------------------------------
// Conditions for 9600 Baud SW UART, SMCLK = 1MHz
//------------------------------------------------------------------------------
#define UART_TBIT_DIV_2     (1000000 / (9600 * 2))
#define UART_TBIT           (1000000 / 9600)

//#define END_OF_INPUT       0x40 //0xAA   //SJ3190214 - @ symbol is used to stop reading
//------------------------------------------------------------------------------
// Global variables used for full-duplex UART communication
//------------------------------------------------------------------------------
unsigned int txData;                        // UART internal variable for TX
//SJ unsigned char rxBuffer;                     // Received UART character

extern STATE_STRUCT  gstate_var;  //SJ5090312 - Added
//extern unsigned char gRC_signal1[RC_ARRAY_SZ];  //SJ4130214 - 
extern unsigned char gI2C_Buffer[EEPROM_BUFFER];   //SJ4130214 - equiv - [68]

//------------------------------------------------------------------------------
// Function prototypes
//------------------------------------------------------------------------------
void TimerA_UART_init(void);  //SJ5090312 - ORG
//void TimerA_UART_init(unsigned char UART_flag);  //SJ5090312 - Replaced
void TimerA_UART_tx(unsigned char byte);
void TimerA_UART_print(char *string);
void TimerA_UART_print_line(char *string, unsigned int size);

//------------------------------------------------------------------------------
// main()  //SJ4080312 - When port over to our project, this is not needed.
//------------------------------------------------------------------------------
/*****
void main(void)
{
    WDTCTL = WDTPW + WDTHOLD;               // Stop watchdog timer

    DCOCTL = 0x00;                          // Set DCOCLK to 1MHz
    BCSCTL1 = CALBC1_1MHZ;
    DCOCTL = CALDCO_1MHZ;

    P1OUT = 0x00;                           // Initialize all GPIO
    P1SEL = UART_TXD + UART_RXD;            // Timer function for TXD/RXD pins
    P1DIR = 0xFF & ~UART_RXD;               // Set all pins but RXD to output
    P2OUT = 0x00;
    P2SEL = 0x00;
    P2DIR = 0xFF;

    __enable_interrupt();
    
    TimerA_UART_init();                     // Start Timer_A UART
    TimerA_UART_print("G2xx2 TimerA UART\r\n");
    TimerA_UART_print("READY.\r\n");
    
    for (;;)
    {
        // Wait for incoming character
        __bis_SR_register(LPM0_bits);
        
        // Update board outputs according to received byte
        if (rxBuffer & 0x01) P1OUT |= 0x01; else P1OUT &= ~0x01;    // P1.0
        if (rxBuffer & 0x02) P1OUT |= 0x08; else P1OUT &= ~0x08;    // P1.3
        if (rxBuffer & 0x04) P1OUT |= 0x10; else P1OUT &= ~0x10;    // P1.4
        if (rxBuffer & 0x08) P1OUT |= 0x20; else P1OUT &= ~0x20;    // P1.5
        if (rxBuffer & 0x10) P1OUT |= 0x40; else P1OUT &= ~0x40;    // P1.6
        if (rxBuffer & 0x20) P1OUT |= 0x80; else P1OUT &= ~0x80;    // P1.7
        if (rxBuffer & 0x40) P2OUT |= 0x40; else P2OUT &= ~0x40;    // P2.6
        if (rxBuffer & 0x80) P2OUT |= 0x80; else P2OUT &= ~0x80;    // P2.7
        
        // Echo received character
        TimerA_UART_tx(rxBuffer);
    }
}
*****/



//------------------------------------------------------------------------------
// Function configures Timer_A for full-duplex UART operation
//------------------------------------------------------------------------------
void TimerA_UART_init(void)  //SJ5090312 - ORG
//void TimerA_UART_init(unsigned char UART_flag)  //SJ5090312 - Replaced
{
    //if (UART_flag)
    //{
    TACCTL0 = OUT;                          // Set TXD Idle as Mark = '1'
    TACCTL1 = SCS + CM1 + CAP + CCIE;       // Sync, Neg Edge, Capture, Int
    TACTL = TASSEL_2 + MC_2;                // SMCLK, start in continuous mode

    /*
    }
    else  //SJ5090312 - Added by SJ
    {
        TACCTL0 &= ~CCIE;
        TACCTL1 &= ~CCIE;

        //TACTL = TASSEL_2 + TACLR + MC_1;  //SJ1110411 - Set to 1MHz clock
        TACTL = TASSEL_2 + MC_1;  //SJ1110411 - Set to 1MHz clock, up mode.
        TACCR0 = RC_TIMER_CONST;  //SJ1110411 - Init compare reg with value.  //SJ1110411 - 0xC350 = 50000 base 10
    }*/
}
//------------------------------------------------------------------------------
// Outputs one byte using the Timer_A UART
//------------------------------------------------------------------------------
//SJ6170312 - At current timer setting of 1MHz, it will take 1040 micro seconds to complete sending 10 bits
void TimerA_UART_tx(unsigned char byte)
{
    while (TACCTL0 & CCIE);                 // Ensure last char got TX'd
    TACCR0 = TAR;                           // Current state of TA counter
    TACCR0 += UART_TBIT;                    // One bit time till first bit
    TACCTL0 = OUTMOD0 + CCIE;               // Set TXD on EQU0, Int
    txData = byte;                          // Load global variable
    txData |= 0x100;                        // Add mark stop bit to TXData
    txData <<= 1;                           // Add space start bit
}

//------------------------------------------------------------------------------
// Prints a string over using the Timer_A UART  //SJ4080312 - When port over to our project, this is not needed.
//------------------------------------------------------------------------------
//*****
void TimerA_UART_print(char *string)
{
    while (*string) {
        TimerA_UART_tx(*string++);
    }
}
//*****/


//------------------------------------------------------------------------------
// Prints a string over using the Timer_A UART  //SJ4080312 - When port over to our project, this is not needed.
//------------------------------------------------------------------------------
void TimerA_UART_print_line(char *string, unsigned int size)
{
    unsigned int ndx;
    
    ndx = 0;
    while (ndx < size) {
        TimerA_UART_tx(string[ndx]);
        ndx++;
    }
}



//------------------------------------------------------------------------------
// Timer_A UART - Transmit Interrupt Handler
//------------------------------------------------------------------------------
#pragma vector = TIMER0_A0_VECTOR
__interrupt void Timer_A0_ISR(void)
{
    static unsigned char txBitCnt = 10;

    //SJ5090312 - SJTODO: This has to be by pass when in SYSTEM_IDLE_STAGE
    //if (gstate_var.oper_stage != SYSTEM_IDLE_STAGE)
    //{
    TACCR0 += UART_TBIT;                    // Add Offset to CCRx
    if (txBitCnt == 0) {                    // All bits TXed?
        TACCTL0 &= ~CCIE;                   // All bits TXed, disable interrupt
        txBitCnt = 10;                      // Re-load bit counter
    }
    else {
        if (txData & 0x01) {
          TACCTL0 &= ~OUTMOD2;              // TX Mark '1'  //SJ2200312 - set/reset
        }
        else {
          TACCTL0 |= OUTMOD2;               // TX Space '0'  //SJ2200312 - toggle
        }
        txData >>= 1;
        txBitCnt--;
    }
    //}
}
//------------------------------------------------------------------------------
// Timer_A UART - Receive Interrupt Handler
//------------------------------------------------------------------------------
#pragma vector = TIMER0_A1_VECTOR
__interrupt void Timer_A1_ISR(void)
{
    unsigned char *infoC_ptr = (unsigned char *) 0x1040;
    unsigned char temp_Rx = 0;
    static unsigned char rxBitCnt = 8;
    static unsigned char rxData = 0;

    switch (__even_in_range(TA0IV, TA0IV_TAIFG)) { // Use calculated branching
        case TA0IV_TACCR1:                        // TACCR1 CCIFG - UART RX
            TACCR1 += UART_TBIT;                 // Add Offset to CCRx
            if (TACCTL1 & CAP) {                 // Capture mode = start bit edge
                TACCTL1 &= ~CAP;                 // Switch capture to compare mode
                TACCR1 += UART_TBIT_DIV_2;       // Point CCRx to middle of D0
//gstate_var.general_ctrl_flag |= UART_TIMER_ON;
            }
            else {
                rxData >>= 1;
                if (TACCTL1 & SCCI) {            // Get bit waiting in receive latch
                    rxData |= 0x80;
                }
                rxBitCnt--;
                if (rxBitCnt == 0) {             // All bits RXed?
                    //rxBuffer = rxData;           // Store in global variable  //SJ6170312 - Original. No longer needed.
                    rxBitCnt = 8;                // Re-load bit counter
                    TACCTL1 |= CAP;              // Switch compare to capture mode
    //                __bic_SR_register_on_exit(LPM0_bits);  // Clear LPM0 bits from 0(SR)

//SJ6170312 - Below codes are added by SJ --------------------------------
                #if 1
                    //SJ6170312 - It will take around (104 * 10 * 40 = 41600) micro seconds to fill up 40 data
                    //SJ1260312 - It will take aroung (104 * 10 * 9 = 9360) micro seconds to fill up 9 data
                    if ((rxData == END_OF_INPUT) || (rxData == END_OF_LINE) || 
                        (rxData == END_OF_PHASE))  //SJ4130214 - End of input detected. Stop reading.
                    {
//                        gI2C_Buffer[3]= rxData;
                        gI2C_Buffer[EEPROM_BUFFER-1] = rxData;  //END_OF_INPUT;  //SJ4200214 - Use rxData content
                        gstate_var.general_ctrl_flag |= NEW_DATA_ON;
//gstate_var.general_ctrl_flag &= UART_TIMER_OFF;
                    }
                    else
                    {

                  #if 1  //SJ4270214 - To allow for read EEPROM operation
                      //SJ4270214 - If received data is either 'R' or 'W', go back
                      if ((rxData == 0x52) && (gstate_var.oper_phase == IDLE_EEPROM))
                      {
                        gstate_var.oper_phase = READ_EEPROM;
                      }
                      else if ((rxData == 0x22) && (gstate_var.oper_phase == IDLE_EEPROM))  //SJ5210314 - Represent double quote to test line connectivity
                      {
                        gstate_var.oper_phase = LINE_HANDSHAKE;
                      }
                      else //if (rxData == 0x57)
                      {
                        gstate_var.oper_phase = WRITE_EEPROM;
                  #endif

                        temp_Rx = abs(rxData - 48);  //SJ2250214 - 48 being internal representation of 0
                        //temp_Rx = abs(rxData - 30);  //SJ2250214 - 48 being internal representation of 0

                        gstate_var.temp_input[gstate_var.byte_seq] = infoC_ptr[temp_Rx];
                        gstate_var.byte_seq++;
                        if (gstate_var.byte_seq == 2)
                        {
                          gstate_var.byte_seq = 0;
                          gI2C_Buffer[gstate_var.RC_index] = (gstate_var.temp_input[0] << 4);
                          gI2C_Buffer[gstate_var.RC_index] |= (gstate_var.temp_input[1] & 0x0F);
                          //temp_Rx = ((gstate_var.temp_input[0] & 0x0F) << 4);
                          //temp_Rx = temp_Rx | (gstate_var.temp_input[1] & 0x0F);
                          //gI2C_Buffer[gstate_var.RC_index] = temp_Rx;

                          gstate_var.RC_index++;
                          if (gstate_var.RC_index >= EEPROM_BUFFER)  //SJ4200214 - equiv 68
                          {
                            //SJ4200214 - If reaches here, something is terribly wrong!!!!!!!
                            gI2C_Buffer[EEPROM_BUFFER-1] = END_OF_LINE;
                            //gI2C_Buffer[EEPROM_BUFFER-1] = END_OF_PHASE;
                            gstate_var.RC_index = EEPROM_BUFFER-1;  //SJ4130214 - 
                            gstate_var.general_ctrl_flag |= NEW_DATA_ON;
//gstate_var.general_ctrl_flag &= UART_TIMER_OFF;
                          }
                        }  //SJ4270214 - if (gstate_var.byte_seq == 2) { ... }

                  #if 1  //SJ4270214 - To allow for read EEPROM operation
                      }  //SJ4270214 - if ((rxData == 0x52)) { ... } else { ... }
                  #endif

                    }  //SJ4130214 - if (rxData == END_OF_INPUT...) { ... } else { ... }
                #endif
//SJ6170312 - END --------------------------------------------------------
                }
            }
//gstate_var.UART_timer = 150;
            break;
    }
}
//------------------------------------------------------------------------------


#endif  //SJ5090312 - #if 1 ... #endif
