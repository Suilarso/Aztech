
/***********************************************************************/
/*                                                                     */
/*  FILE        : JDSU_Gemini_Main.                                    */
/*  DATE        : Tue, July 06, 2013                                   */
/*  DESCRIPTION : Main Program                                         */
/*  CPU TYPE    : MSP430G2332                                          */
/*  PROJECT     : JDSU Gemini                                          */
/*  AUTHOR      : Suilarso Japit                                       */
/*  VERSION     : 90?.001.1-000 (Should be 90?)                        */
/*  HISTORY     :                                                      */
/*    SJ2020713 - Start software development activity.                 */
/*                Completed ioExpander's initialisation function.      */
/*                Completed ioExpander's read function via I2C.        */
/*                Started main process loop (Process_Manager)          */
/*    SJ3030713 - Coding to read TCA6416A port via I2C, manipulate     */
/*                port's contents and response to MSP430G2332          */
/*                accordingly.                                         */
/*    SJ4040713 - Write codes to initialise TMP431A temperature sensor */
/*                IC; Read_From_TMP431A and also Write_To_TMP431A.     */
/*    SJ4110713 - Write codes to turn on respective relays.            */
/*                TCA6416A port 0 and port 1.                          */
/*    SJ5170114 - Test firmware with actual hardware. Test successful. */
/*                As @ today, code sz: 1060 bytes, data sz: 85 bytes.  */
/*    SJ4130214 - Introduce software UART code using P1.1 (pin 3) as   */
/*                Tx and P1.2 (pin 4) as Rx. As a result of this       */
/*                implementation, ISR is now based on watchdog timer.  */
/*                As @ today, code sz: 1284 bytes, data sz: 110 bytes. */
/*    SJ4200214 - Modified Write_To_24LC256I() to allow writing two    */
/*                times to EEPROM. One start at location 0x0000, and   */
/*                another start at 0x4000 which serve as backup copy.  */
/*                As @ today, code sz: 1354 bytes, data sz: 90 bytes.  */
/*    SJ5210214 - Add code to inform GUI end of task by transmitting   */
/*                ESC (0x1B) via UART to GUI.                          */
/*                As of now, code sz: 1426 bytes, data sz: 90 bytes.   */
/*                Add codes to fill up the balance of EEPROM first     */
/*                16KB with 0x00.                                      */
/*                As @ today, code sz: 1536 bytes, data sz: 90 bytes.  */
/*    SJ2250214 - GUI convert each byte into two bytes in ascii        */
/*                format, my firmware will merge the two bytes into    */
/*                one byte again.                                      */
/*    SJ3260214 - Incorporate I2C read module to read from EEPROM.     */
/*    SJ4200314 - Incorporate USI I2C module. Codes are provided by    */
/*                TI: i2c_usi_mst.c. Enable WITH_USI_I2C macro when    */
/*                using USI module. With USI module, code size after   */
/*                compilation is 1854 for code and 130 for data.       */
/*    SJ42203?? - Backup 906.001.1-000.                                */
/*                906.001.1-001 introduces new pin for battery cutoff. */
/*    SJ42704?? - Release to factory for production use.               */
/*                906.001.1-001                                        */
/*    SJ10406?? - In mode 1_3 change CHARGER_EN_PIN to 1 instead of    */
/*                0. For Andrew to do testing only.                    */
/*                                                                     */
/*  U1 - L68-C20001-L00 - LF Control Board                             */
/*  H55-E30011-A42 / L68-C20001-L00 / L70-C20001-000                   */
/*  Version: 90?.001.1-000                                             */
/***********************************************************************/

//#include   <msp430x16x.h>
//#include   "msp430x21x1.h"  //<msp430x23x2.h>
#include   <msp430G2332.h>
#include   <stdlib.h>  //SJ5301112 - Need rand() and srand(seed) prototype.
//#include   <stdio.h>
#include   <string.h>

//#include   "gp_io.h"
#include   "global_var.h"
#ifdef FLASH_COMPONENT
#include   "flash_module.h"
#endif

#ifndef WITH_USI_I2C  //SJ4200314 - New
#include   "Soft_I2C.h"
#else
#include   "i2c_usi_mst.h"
#endif

#define PRODUCT_CODE  (90?)
#define CUST_ID_CODE  (001)
#define RELEASE_CODE  (1)
#define VERSION_CODE  (000)


#define DATA_BUFFER_LEN     10


//=== SJ5060712 - External variables ==========================================
//I2C_routine.c


//TAS3103_util.c
//=== SJ5060712 - END. External variables =====================================

//=== SJ5060712 - Functions prototype =========================================
int Init_Global_Vars(int *val);
//int Init_TCA6416A(int *param);
int Process_Manager(int *val);
//int Read_From_TCA6416A(int *num_of_byte);
//int Turn_On_Relevant_LEDs(int *num_of_byte);
int Read_From_24LC256I(int *reg_addr);
int Write_To_24LC256I(int *num_of_byte);
int System_Idle_Mode(int *none);
int Clear_EEPROM_Flag(int *none);
//int Init_TMP431A(int *param);
//int Read_From_TMP431A(int *reg_addr);
//int Write_To_TMP431A(int *num_of_byte);
int RetrieveFromADC(int *data_format);
//void AZ_Transmit_Routine(void);  //SJ4221112 - Testing transmit

//__interrupt void ISR_Port1(void);  //SJ3150812 - Selftest



//void (* Function_Pointer_Void) (void);
//typedef int (* Function_Pointer_Int) (int);
//Function_Pointer_Int Fn_array_int[15];

//=== SJ5060712 - END. Functions prototype ====================================


//=== SJ5060712 - External function prototype =================================
#ifdef FLASH_COMPONENT
//flash_module.c
extern void Flash_Write(unsigned int start_addr, short amount_byte, unsigned char *buffer);
#endif  //#ifdef FLASH_COMPONENT ... #endif

//ISR_Routines.c
//void QuickDelay(unsigned int interval);

//Soft_I2C.c
extern void SW_I2CStart(void);
extern void SW_I2CStop(void);
extern void SW_I2CReset(void);  //SJ5140314 - Added for Microchip EEPROM
//extern unsigned char I2CCheckAcknowledgement(void);
extern unsigned char SW_I2CWrite(char NumOfByte);
//extern  unsigned char SW_I2CReadByte(void);
//extern unsigned char SW_I2CRead(int NumOfByte);

//MSP430G2332_UART9600.c
extern void TimerA_UART_print(char *string);
extern void TimerA_UART_print_line(char *string, unsigned int size);

//=== SJ5060712 - END. External functions prototype ===========================


//=== SJ5060712 - Global variable and constant declaration ====================
//SJ4110713 - SJNOTE: If RELAY_1_ENABLE, all column 0 entries will be ENABLE,
//SJ4110713           If RELAY_2_ENABLE, all column 1 entries will be ENABLE.
//#define RELAY_1_ENABLE   5
//#define RELAY_2_ENABLE   6
//const unsigned char gRelay_Tab[5][3] = {
//   {DISABLE, NOT_USED, ENABLE},
//   {DISABLE, NOT_USED, DISABLE},
//   {DISABLE, DISABLE, NOT_USED},
//   {DISABLE, DISABLE, NOT_USED},
//   {DISABLE, DISABLE, NOT_USED}};

//unsigned char ggeneral_buffer[DATA_BUFFER_SZ];  //[50];

//SJ2020713 - SJI2C
//SJ2020713 - Data storage for I2C transmit
unsigned char gI2C_Buffer[EEPROM_BUFFER];   //SJ2140114 - equiv - [68]


//SJ3211112 - ???


unsigned char gFunc_Seq;//=SYS_INITIALISE_VARS;

STATE_STRUCT  gstate_var;
CALLBACK_STRUCT gcallback_list[SYS_LAST_TASK] = 
{ {0, Init_Global_Vars}, 
  {0, Process_Manager},  /* Process Manager not used here */
  {0, NULL},  /* Reserved for init EEPROM */
  {0, Read_From_24LC256I},
  {0, Write_To_24LC256I},
  {0, System_Idle_Mode},
  {0, Clear_EEPROM_Flag} };
//  {0, NULL},
//  {0, NULL},
//  {0, Read_From_24LC256I},
//  {0, Write_To_TCA6416A},
//  {0, Init_TMP431A},
//  {0, Read_From_TMP431A},
//  {0, Write_To_TMP431A},
//  {0, Clear_EEPROM_Flag}, 
//  {0, NULL}, 
//  {0, NULL}, 
//  {0, NULL}, 
//  {0, NULL}, 
//  {0, NULL} };

//=== SJ5060712 - END. Global variables declaration ===========================


//SJ2020713 - MCU detail:
//MSP430G2332 (RISC 16 bits)
//Flash memory size: 4KB + 256B
//Flash memory consists of main memory (8 segments of 512B), and information memory (4 segments of 64B)
//Segment Info_D = 0x1000 - 0x1039
//Segment Info_C = 0x1040 - 0x1079
//Segment Info_B = 0x1080 - 0x10BF
//Segment Info_A = 0x10C0 - 0x10FF  //SJ5141212 - Care must be exercised for using this segment
//RAM size: 256B
//Internal Frequencies up to 16 MHz
//Default master clock and sub-main clock is at 1.1 Mhz upon system power up.
//Two timer: Timer A with three capture /compare registers.

//SJ1110110 - Short is 2 bytes long
//SJ1110110 - int is 2 bytes long
//SJ1110110 - long is 4 bytes long

//SJ2020713 Main routine
void main(void)
{
    unsigned char *DummyValue=(unsigned char *) 0x1080;
//unsigned char ndx, ndx2, temp, temp2;
//unsigned char test_arr[15];
    //int i, j, k;
    int param;

    WDTCTL = WDTPW + WDTHOLD;       // disable Watchdog

    //SJ4131005 - Below two settings are used to manipulate with timer clock frequency.  
    //SJ5060712 - If timer frequency is changed here, remember to change the ADC10 as well.
    //SJ4130214 - Set clock to 1MHz.
    //DCOCTL = CALDCO_8MHZ;   //SJ5300911 - CALDCO_1MHZ;
    //BCSCTL1 = CALBC1_8MHZ;  //SJ5300911 - CALBC1_1MHZ;
    DCOCTL = CALDCO_1MHZ;   //SJ1141111 - Set to 1 MHz  //CALDCO_8MHZ;
    BCSCTL1 = CALBC1_1MHZ;  //SJ1141111 - Set to 1 MHz  //CALBC1_8MHZ;

  #ifdef  WITH_WDT
    //SJ5060712 - When use DCO, ACLK is 12KHz.
    //BCSCTL3 |= LFXT1S1;  //SJ5060712 - Used as ACLK for WDT+. Same effect as below sttmt
    BCSCTL3 |= LFXT1S_2;  //SJ5060712 - ACLK = VLO
  #endif

//SJ3110712 - Initialise MSP430 Registers ---------------------------------------------------------
    //SJ3110712 I/O pins initialisation. --------------------------------------
    //SJ3110712 - NOTE: Unused pins should be configured as I/O in SEL reg and out in DIR reg.
    //SJ3110712 - SELect register. 0 indicates pin is configured as gpio whereas 1 indicates peripheral.
    //SJ3110712 - DIRection register. 0 indicates pin configured as input whereas 1 indicates output.    
    //SJ3110712 - Interrupt Enable register. 0 indicates pin interrupt is disabled whereas 1 indicates enabled
    //SJ3110712 - Interrupt Edge Select register. 0 indicates raising edge whereas 1 indicates falling edge    

    //SJ2020713 - Port 1 initialisation.
  #ifndef WITH_USI_I2C  //SJ4200314 - Use SW_I2C
    P1SEL = DummyValue[2];   //SJ4130214 - (00000110) All pin set as GPIO  //SJ4200314 - SW_I2C
    //SJ3190214 - If using UART module, below sttmt is a must. Though I don't know WHY???
    //SJ3190214 - If using software UART, don't need below sttmt. Again I don't know WHY???
    //P1SEL2 = DummyValue[2];  //SJ4130214 - (00000110) All pin set as GPIO
  #else  //SJ4200314 - Use I2C_Module (USI)
    P1SEL = 0xC6;  //SJ4200314 - (11000110) I2C_Module
    P1SEL2 &= ~(BIT6 + BIT7);
  #endif
    P1DIR = DummyValue[3];   //SJ4130214 - (11011011) Pin 6 & 7 for I2C, pin 1&2 for UART
    //P1IN = 0x00;
    //P1REN = 0x01;  //SJ2020713 - Not used in this project
    P1OUT = 0x00;  //SJ2020713 - 
    //P1IE  = 0x80;  //SJ2120411 - Pin 7 is interrupt enabled (10000000)
    //P1IES = 0x80;  //SJ2120411 - Pin 7 is set to 1 to set falling edge (10000000)

    //SJ5051012 - Port 2 initialisation
    P2SEL = DummyValue[4];  //SJ2020713 (00000000) All of the pins as GPIO.
    //P2SEL2 = DummyValue[4];  //SJ5071212 - DO NOT USE this statement
    P2DIR = DummyValue[5];  //SJ2020713 - (00000000) Pin 6 and 7 serve as bit 1 and 2 of board type combination
    //P2IN = 0xF0;  //SJ5051012 - (11110000) PCB with DIP switch  //SJ1261112 - SJTODO
    //P2IE = 0x04;
    //P2IES = 0x04;
    //P2REN = 0xF0;  //SJ1081012 - Enable internal pullup for P2.4, P2.5, P2.6, and P2.7 (11110000)
    //P2OUT = 0xF7;  //SJ3160113 - (11110111). Required because of internal pullup enabled.

  #if 0  //SJ2020713 - This MCU does not have port 3.
    //SJ5051012 - Port 3 initialisation
    P3SEL = DummyValue[6];//0x00;  //SJ5051012 - (00000000) All pins as GPIO.
    //P3SEL2 = DummyValue[6];//0x00;
    P3DIR = DummyValue[7];//0xFF;  //SJ5051012 - (00000000) All set as input
    //P3IN = 0xFF;   //SJ5051012 - (11111111) All set to high
    //P3OUT = 0x07;  //SJ5051012 - (00000111)
    //P3IE = 0x04;
    //P3IES = 0x04;
    P3REN = 0xFF;  //SJ1081012 - Enable all internal pullup for P3 (11111111)
    P3OUT = 0xFF;  //SJ3160113 - Required because of internal pullup enabled.
  #endif

    //SJ3110712 Timers initialisation. ----------------------------------------
    //SJ3110712 - NOTE: TACTL MC_1 should not be set here. Once set timer start to tick.
    //SJ3110712 - MC_0 - Stop mode, MC_1 - Up mode, MC_2 - Continuous mode, MC_3 - Up/Down mode

    //SJ2020713 - Timer0_A3 control register initialisation.
    //SJ2020713 - Timer0_A3 is used as timer interval for ISR general routine.
    //SJ3220712 - I think this can be achieve by using Timer1_A2; share with PWM timer.
  #if 0  //SJ4130214 - Without UART.
    //TACTL = TASSEL_2 + TACLR;  //SJ3110712 - SMCLK = 1MHz; ID_0 ==> 1MHz / 1 = 1MHz
    TACTL = TASSEL_2 + ID_3 + TACLR;  //SJ3110712 - SMCLK = 8MHz; ID_3 ==> 8MHz / 8 = 1MHz
    //SJ3110712 - Initialise Capture/Compare control register.
    //SJ3110712 - Capture/Compare block 0 is used as compare mode for timer.
    TACCTL0 = CCIE;
  #else  //SJ4130214 - Using UART to read input.

    //SJ4220312 - Below three sttmts transfered from TimerA_UART_init() function
    TACCTL0 = OUT;                          // Set TXD Idle as Mark = '1'
    TACCTL1 = SCS + CM1 + CAP + CCIE;       // Sync, Neg Edge, Capture, Int
    TACTL = TASSEL_2 + MC_2;                // SMCLK, start in continuous mode

    //SJ4080312 - Used Watchdog timer as interval interrupt
    WDTCTL = WDTPW + WDTTMSEL + WDTCNTCL;  //SJ4080312 - Clock is 1MHz, counter is 32768, so 1000000/32768 = 31 times
    //WDTCTL = WDTPW + WDTTMSEL + WDTCNTCL + WDTIS0;  //SJ4080312 - Clock is 1MHz, counter is 8192, so 1000000/8192 = 122 times

  #endif

    //SJ3110712 - Timer1_A3 control register initialisation.
    //SJ3110712 - Timer1_A1 (Capture/Compare block 1) is used to generate PWM.
    //TA1CTL = TASSEL_2 + TACLR;  //SJ3110712 - SMCLK = 1MHz; ID_0 ==> 1Mhz / 1 = 1MHz
    //TA1CTL = TASSEL_2 + ID_3 + TACLR;  //SJ3110712 - SMCLK = 8MHz; ID_3 ==> 8MHz / 8 = 1MHz
    //SJ3110712 - Initialise Capture/Compare control register.
    //SJ3110712 - Possible setting for outmode:
    //SJ3110712 - OUTMOD_1;  //SJ3110712 - Set
    //SJ3110712 - OUTMOD_2;  //SJ3110712 - Toggle/reset
    //SJ3110712 - OUTMOD_3;  //SJ3110712 - Set/reset
    //SJ3110712 - OUTMOD_4;  //SJ3110712 - Toggle
    //SJ3110712 - OUTMOD_5;  //SJ3110712 - Reset
    //SJ3110712 - OUTMOD_6;  //SJ3110712 - Toggle/set
    //SJ3110712 - OUTMOD_7;  //SJ3110712 - Reset/set
    //TA1CCTL1 |= OUTMOD_3;//OUTMOD_6;  //SJ3110712 - Toggle/set

  #ifdef WITH_USI_I2C  //SJ4200314 - Using USI module
    //SJ4200314 USI (I2C) initialisation. -----------------------------------
    //SJ4200314 - Below codes copied from TI sample (i2c_usi_mst.c)
    USICTL0 = USIPE6 + USIPE7 + USIMST + USISWRST;  // Port & USI mode setup  //SJ4200314 - Use as is
    USICTL1 = USII2C + USISTTIE + USIIE;            // Enable I2C mode & USI interrupt  //SJ4200314 - Use as is
    USICKCTL = USIDIV_7 + USISSEL_2 + USICKPL;      // USI clks: SCL = SMCLK/128  //SJ4200314 - Use as is, SCL = 8KHz
    USICNT |= USIIFGCC ;                            // Disable automatic clear control  //SJ4200314 -Use as is
    USICTL0 &= ~USISWRST;                           // Enable USI  //SJ4200314 - Release I2C module for operation
    USICTL1 &= ~(USIIFG + USISTTIFG);               // Clear pending flag
  #endif

    //SJ3110712 UART initialisation. ----------------------------------------
    //SJ3110712 - NOTE: - Below notes extract from TI document
    //SJ3110712 - Note: Initializing or Re-Configuring the USCI Module
    //SJ3110712 - The recommended USCI initialization/re-configuration process is:
    //SJ3110712 - 1) Set UCSWRST (BIS.B #UCSWRST,&UCAxCTL1)
    //SJ3110712 - 2) Initialize all USCI registers with UCSWRST = 1 (including UCAxCTL1)
    //SJ3110712 - 3) Configure ports.
    //SJ3110712 - 4) Clear UCSWRST via software (BIC.B #UCSWRST,&UCAxCTL1)
    //SJ3110712 - 5) Enable interrupts (optional) via UCAxRXIE and/or UCAxTXIE

  #if 0  //SJ2020713 - No UART is needed
    //UCA0CTL0;  //SJ2100712 - No need to set; all default value meet our requirement. For future use, may consider UCMODE2.
    UCA0CTL1 = UCSWRST;  //SJ2100712 - Step 1
    UCA0CTL1 |= UCSSEL1; //SJ2100712 - Step 2 
    //SJ3060213 - Baud rate is 57600
    //UCA0BR0 = 0x08;        //SJ2100712 - Refer to Tab 15-5 of MSP430 user manual.
    //UCA0BR1 = 0x00;        //SJ2100712 - Refer to Tab 15-5 of MSP430 user manual.
    //UCA0MCTL = 0xB0+UCOS16;//SJ2100712 - Refer to Tab 15-5 of MSP430 user manual. UCBRFx=11+UCBRSx=0+UCOS16
    //SJ3060213 - Baud rate is 38400
    UCA0BR0 = 13;          //SJ2100712 - Refer to Tab 15-5 of MSP430 user manual.
    UCA0BR1 = 0x00;        //SJ2100712 - Refer to Tab 15-5 of MSP430 user manual.
    UCA0MCTL = 0x00+UCOS16;//SJ2100712 - Refer to Tab 15-5 of MSP430 user manual. UCBRFx=1+UCBRSx=0+UCOS16
    //SJ2010610 - Step 3 was done above in the port initialisation
    UCA0CTL1 &= ~UCSWRST;  //SJ2100712 - Step 4
    UC0IE |= UCA0RXIE;  //SJ2100712 - Step 5 (Set on rx interrupt in IE2 register)  //SJ3110712 - Check if this sttmt can be here??
  #endif

//SJ3080513 - User to select whether to use I2C or ADC. Both modules share the same pins.
  #if 0  //SJ2290113 - Reserved for I2C module initialisation
    //SJ4160513 I2C  initialisation. ----------------------------------------
    //SJ2290113 - NOTE: Below comments are extracted from TI's MSP application notes. (Pg 464 of chapter 17)
    //SJ2290113 - NOTE: Initializing or Reconfiguring the USCI Module
    //SJ2290113 - The recommended USCI initialization or reconfiguration process is:
    //SJ2290113 - 1. Set UCSWRST (BIS.B #UCSWRST,&UCxCTL1)
    //SJ2290113 - 2. Initialize all USCI registers with UCSWRST=1 (including UCxCTL1)
    //SJ2290113 - 3. Configure ports.
    //SJ2290113 - 4. Clear UCSWRST via software (BIC.B #UCSWRST,&UCxCTL1)
    //SJ2290113 - 5. Enable interrupts (optional) via UCxRXIE and/or UCxTXIE

//SJ1040213 - Copy from msp430g2xx3_uscib0_i2c_08.c
    //SJDONE WDTCTL = WDTPW + WDTHOLD;                 // Stop WDT
    //SJDONE P3SEL |= 0x06;                            // Assign I2C pins to USCI_B0
    UCB0CTL1 |= UCSWRST;                    //SJ1040213 - Step 1. Enable SW reset
    UCB0CTL0 = UCMST + UCMODE_3 + UCSYNC;   //SJ1040213 - Step 2. Master mode, I2C mode, Synchronous mode
    UCB0CTL1 = UCSSEL_2 + UCSWRST;          //SJ1040213 - Step 2. Use SMCLK, keep SW reset
    UCB0BR0 = 0x1B;                         //SJ1040213 - Step 2. fSCL = SMCLK/27 ==> 8MHz/27 = ~300kHz
    UCB0BR1 = 0;
    UCB0I2CSA = TCA6424A_ADDR;              //SJ1040213 - Step 2. Slave Address is 0x22 (34)
    UCB0CTL1 &= ~UCSWRST;                   //SJ1040213 - Step 4. Clear SW reset, resume operation
    //UCB0IE |= UCTXIE;                       //SJ1040213 - Step 5. Enable TX interrupt. (msp430x54x.h)
    UC0IE |= UCB0TXIE;  //SJ2050213 - Step 5. Enable TX interrupt. (msp430g2553.h)
  //#else  //SJ2290113 - End of I2C initialisation

    //SJ3110712 ADC initialisation. -----------------------------------------
    //SJ3110712 - ADC10 control register initialisation.
    ADC10AE0 |= DummyValue[8];//0xC0;  //SJ3110712 - Assign P1.6 (A6), and P1.7 (A7) as analog input to ADC. (11000000)
    //ADC10SA = 0x200;   //SJ3130110 - This reg contains ram addrs for data transfer.
    //ADC10DTC1 = 0x20;  //32 conversions  //SJ3130110 - I believe this reg is used for multiple conversion.

  #endif  //SJ2290113 - If I2C is used, ADC will not be fefined

    _BIS_SR(GIE);  //SJ1130812 - Enable interrupt. equiv __enable_interrupt();?????
    IE1 |= WDTIE;  //SJ4130214 - Use watchdog timer as interval timer. (Software UART)
//SJ3110712 - End of MSP430 Registers Initialisation. ---------------------------------------------

#if 0  //SJ2020713 - Everything within it #if ... #endif has been moved to Init_Global_Vars()
//SJ3110712 - Initialize global var. --------------------------------------------------------------
//SJ3110712 - End of global var initialisation ----------------------------------------------------

//SJ3110712 - Access to flash memory --------------------------------------------------------------
//SJ3110712 - End of Access to flash memory -------------------------------------------------------

//SJ4090812 - Start timer -------------------------------------------------------------------------

    //--- SJ3110712 - Timer is active beyond this point ---------------------------------

    //--- SJ4290710 - This code is to introduce delay to allow MCU2 to stabalise ---
#endif


//SJ4090812 - DALI Initialisation -----------------------------------------------------------------
//SJ5071212 - DALI initialisation is moved up before modem configuration

    //--- SJ1031011 - At this point, both LED_current and LED_voltage would have -----
    //--- SJ1031011 - cum 40 data each. 1 sec 20 ISR, 2 sec 40 ISR entries. -------------
    //--- SJ1031011 - Potential issue: the first 10 data is not so accurate -------------

    gFunc_Seq = SYS_INITIALISE_VARS;

//SJ3110712 - Main loop ---------------------------------------------------------------------------
    while(1)   //SJ5060712 - Loop forever
    {
      //SJ3040810 - Use watchdog timer to reset in the event of infinite loop
      //SJ4130214 - Overwrite above sttmt. Can't use watchdog as it is being used as timer interval
      #if 0 //def  WITH_WDT
        //SJ3040810 - Reset watchdog timer ~ 2.5 sec. 
        WDTCTL = WDT_ARST_1000;  //SJ4050810 - equiv  (WDTPW + WDTCNTCL + WDTSSEL)
//SJ2020713 - SJNOTE: With callback, the placement of this sttmt need to be reviewed.
      #endif

        if (gcallback_list[gFunc_Seq].Callback_fn_Int)
        {
            param = gstate_var.param;
            //gcallback_list[gFunc_Seq].Callback_fn_Int(&param);
            gFunc_Seq = gcallback_list[gFunc_Seq].Callback_fn_Int(&param);  //SJ2020713 - Maybe can consider this option.
        }
    }   //SJ5300911 while(1) { ... }
//SJ3110712 - End of Main loop --------------------------------------------------------------------

}  //SJ5060712 - End of main(void) { ... }


//SJ2020713 - This is the place where system and global variables are initialised
//SYS_INITIALISE_VARS
int Init_Global_Vars(int *val)
{
//SJ3120314    unsigned char *data_ptr = (unsigned char *) 0x1000;  //SJ2020713 - DON'T REMOVE first
    unsigned char *dummy_ptr = (unsigned char *) 0x1080;  //SJ2020713 - Replaced
    unsigned char temp_bit;

    gstate_var.general_ctrl_flag = 0;
    gstate_var.general_delay = 5;  //SJ2020713 - equiv to 250milisecond
    gstate_var.board_type = 0;

    //SJ4110713 - P2.6|P2.7|P1.5 represent bit2|bit1|bit0
    gstate_var.board_type = (P1IN & 0x20) ? 1 : 0;  //SJ4110713 - bit 5 and Constructing bit 0 of gstate_var.board_type
    temp_bit = P2IN & 0x80;  //SJ3070813 - bit 7
    gstate_var.board_type |= (temp_bit >> 6);  //SJ3070813 - Constructing bit 1 of gstate_var.board_type
    temp_bit = P2IN & 0x40;  //SJ4110713 - bit 6
    gstate_var.board_type |= (temp_bit >> 4);  //SJ3070813 - Constructing bit 2 of gstate_var.board_type

    gstate_var.RC_index = 3;  //SJ4130214 - Used as index to input buffer
    //gstate_var.UART_timer = 0;
    gstate_var.byte_seq = 0;
    gstate_var.temp_input[0] = 0;
    gstate_var.temp_input[1] = 0;
    gstate_var.write_once = 0;
    //gstate_var.oper_phase = WRITE_EEPROM;  //SJ4270214 - Official  IDLE_EEPROM
    gstate_var.oper_phase = IDLE_EEPROM;  //SJ4270214 - Official
//    gstate_var.oper_phase = LINE_HANDSHAKE;//READ_EEPROM;  //SJ4270214 - For testing
    //gstate_var.led_blink_freq = 0;  //SJ3191212 - Replaced above two sttmts
    //gstate_var.I2C_freq = 0;  //SJ2020713 - Don't initialise here
	gstate_var.ISR_constant = (dummy_ptr[0] << 8) + dummy_ptr[1];
    gstate_var.EEPROM_offset = 0x00;  //SJ4200214 - Start from loc 00

    gstate_var.param = 0;
    //gstate_var.device_state_flag.com_field.BYTE = 0;  //SJ3200411 - Added
    //gstate_var.GEN_ctrl_flag.com_field.BYTE = 0;

//SJ4090812 - Start timer -------------------------------------------------------------------------
  #if 0  //SJ4130214 - Not used, used watchdog timer interval instead. (Software UART)
    TACCR0 = gstate_var.ISR_constant;  //SJ1110411 - Init compare reg with value.
    TACTL = TACTL | MC_1;  //SJ1110411 - Start timer in up mode.
  #endif
//--- SJ3110712 - Timer is active beyond this point ---------------------------------

//SJ3120314    if (data_ptr[0] == 0x00)  //SJ4130214 - Indicate EEPROM is written before, so go to idle mode
//SJ3120314    {
//SJ3120314        P1OUT |= 0x01;  //SJ4130214 - Since there is nothing to do, turn on LED to indicate end of operation.
        //IE1 &= ~WDTIE;
//SJ3120314        return (SYS_IDLE_MODE);
//SJ3120314    }
//SJ3120314    else
//SJ3120314    {
//gstate_var.general_delay = 62;//1 secs
        //--- SJ4290710 - This code is to introduce delay to allow MCU to stabalise ---
        while (gstate_var.general_delay) ;
        //gFunc_Seq = SYS_INIT_TCA6416A;

        gI2C_Buffer[0] = 'R';  //SJ4200214 - 
        gI2C_Buffer[1] = 'e';  //SJ4200214 - 
        gI2C_Buffer[2] = 'a';  //SJ4200214 - 
        gI2C_Buffer[3] = 'd';  //SJ4200214 - 
        gI2C_Buffer[4] = 'y';  //SJ4200214 - 
        gI2C_Buffer[5] = '.';  //SJ4200214 - 
        gI2C_Buffer[6] = '.';  //SJ4200214 - 
        gI2C_Buffer[7] = '.';  //SJ4200214 -
        gI2C_Buffer[8] = '\n';  //SJ4200214 -
        gI2C_Buffer[9] = '\0'; 
        TimerA_UART_print((char *) gI2C_Buffer);

        return (SYS_PROCESS_MANAGER);  //SJ4130214 - From here goes to Process_Manager
        //return (SYS_CLEAR_EEPROM_FLAG);  //SJ4130214 - For testing purpose only

        //return (SYS_WRITE_24LC256I);  //SJ2140114 - Goes to write to EEPROM
//SJ3120314    }  //SJ4130214 - if (dummy...) { ... } else { ... }

}  //SJ2300413 - End of Init_Global_Vars(int *val);


//SJ2020713 - This is the main process controlling the entire system
//SYS_PROCESS_MANAGER
int Process_Manager(int *val)
{
    int seq=SYS_PROCESS_MANAGER;
    //unsigned char in_val=0;
    //unsigned char out_mask=0;

    //WDTCTL = WDT_ARST_1000;

    //SJ4270214 - Kode dibawah ini hanya untuk percobaan. Hapus sesudah di pakai
    //gstate_var.oper_phase = READ_EEPROM;  //SJ4270214 - Paksa ke baca mode
    if (gstate_var.oper_phase == READ_EEPROM)
    {
        seq = SYS_READ_24LC256I;
    }
#if 1
    else if (gstate_var.oper_phase == LINE_HANDSHAKE)
    {
        gstate_var.general_delay = 5;
        while (gstate_var.general_delay) ;

        gI2C_Buffer[0] = 0x22;  //SJ5210314 - Represent double quote to test line connectivity
        gI2C_Buffer[1] = '\0';
        TimerA_UART_print((char *) gI2C_Buffer);

        gstate_var.oper_phase = IDLE_EEPROM;
    }
#endif
    else if (gstate_var.oper_phase == WRITE_EEPROM)
    {
        if (gstate_var.general_ctrl_flag & NEW_DATA_ON)  //SJ2020713 - Input data is ready for manipulation
        {
            gstate_var.general_ctrl_flag &= NEW_DATA_OFF;

            seq = SYS_WRITE_24LC256I;  //SJ2140114 - Goes to write to EEPROM
        }
    }

    //gI2C_Buffer[0] = 'E';  //SJ4200214 - 
    //gI2C_Buffer[1] = 'n';  //SJ4200214 - 
    //gI2C_Buffer[2] = 'd';  //SJ4200214 - 
    //gI2C_Buffer[3] = '.';  //SJ4200214 - 
    //gI2C_Buffer[4] = '\0'; 
    //TimerA_UART_print((char *) gI2C_Buffer);

/*
    if ((gstate_var.general_ctrl_flag & UART_TIMER_ON) && (gstate_var.UART_timer == 0))
    {
        gstate_var.general_ctrl_flag &= UART_TIMER_OFF;
        seq = SYS_WRITE_24LC256I;  //SJ2250214 - Goes to write to EEPROM
    }
*/

    return (seq);//(0);

}  //SJ2020713 - End of Process_Manager() function


#if 1  //SJ3231013 - 
//SJ3231013 - Write to EEPROM MC-24LC256I
//SYS_WRITE_24LC256I
int Write_To_24LC256I(int *num_of_byte)
{
    int seq=SYS_PROCESS_MANAGER;
    unsigned int EEPROM_loc;
//SJ3120314    unsigned char ndx;
//SJ3120314    unsigned char *dummy_ptr = (unsigned char *) 0x1000;
    //unsigned char *dummy_ptr = (unsigned char *) 0x1080;

    //SJ4200214 - 0 to 2 contain control byte and EEPROM address
    //SJ4200214 - 3 to 66 contain data to write to EEPROM
    //SJ4200214 - 67 contain indicator
    EEPROM_loc = gstate_var.EEPROM_offset;  //SJ4200214 - EEPROM location of the first set of data.
    gI2C_Buffer[0]  = DEV_WriteCommand;     //SJ3030713 - Transmit 4 bytes
    gI2C_Buffer[1]  = (EEPROM_loc >> 8); //0x00;  //SJ3231013 - 24LC256I Most Significant Byte Address
    gI2C_Buffer[2]  = (EEPROM_loc & 0x00FF); //0x00;  //SJ3231013 - 24LC256I Least Significant Byte Address
    //gI2C_Buffer[3]  = 0x73;  //SJ3231013 - MSB of EEPROM magic number 0x739D
    //gI2C_Buffer[4]  = 0x9D;  //SJ3231013 - LSB of EEPROM magic number 0x739D

  #if 1  //SJ5210214 - Codes added to fill the balance of EEPROM 16K with 0
    //if (gI2C_Buffer[EEPROM_BUFFER-1] == END_OF_PHASE)  //SJ5210214 - Indicate end of phase
    //{
        while (gstate_var.RC_index < (EEPROM_BUFFER-1))
        {
            gI2C_Buffer[gstate_var.RC_index] = 0x00;  //SJ5210214 - Fill the balance of buffer with 0
            gstate_var.RC_index++;
        }
    //}
  #endif

  #ifndef WITH_USI_I2C
    SW_I2CStart();
    SW_I2CWrite(67);//(gstate_var.RC_index);     //SJ4130214 - Max if 64 bytes
    SW_I2CStop();
  #else  //SJ4200314 - Use USI_I2C

    i2c_usi_mst_gen_start();  //SJ4200314 - Start I2C
    i2c_usi_mst_send_address(0x51, 0);  //SJ4200314 - Write operation
    gstate_var.RC_index = 1;
    while (gstate_var.RC_index < (EEPROM_BUFFER-1))
    {
        i2c_usi_mst_send_byte(gI2C_Buffer[gstate_var.RC_index]);  //SJ4200314 - Sent out byte by byte
        gstate_var.RC_index++;
    }
    i2c_usi_mst_gen_stop();  //SJ4200314 - Stop I2C

  #endif


  #if 1  //SJ5210214 - Codes added to fill the balance of EEPROM 16K with 0
    if (gI2C_Buffer[EEPROM_BUFFER-1] == END_OF_PHASE)  //SJ5210214 - Fill the rest of 16K with 0
    {
#if 0
/*****
        gI2C_Buffer[0] = 'B';  //SJ4200214 - 
        gI2C_Buffer[1] = 'u';  //SJ4200214 - 
        gI2C_Buffer[2] = 's';  //SJ4200214 - 
        gI2C_Buffer[3] = 'y';  //SJ4200214 - 
        gI2C_Buffer[4] = '\0'; 
        TimerA_UART_print((char *) gI2C_Buffer);

        gstate_var.EEPROM_offset += PAGE_SIZE;  //SJ4200214 - equiv 64;
        for (ndx=3; ndx<(EEPROM_BUFFER-1); ndx++)
            gI2C_Buffer[ndx] = 0x00;

        while (gstate_var.EEPROM_offset < 16384)  //SJ5210214 - 16K (0x4000)
        {
            EEPROM_loc = gstate_var.EEPROM_offset;  //SJ4200214 - EEPROM location of the first set of data.
            gI2C_Buffer[0]  = DEV_WriteCommand;     //SJ3030713 - Transmit 4 bytes
            gI2C_Buffer[1]  = (EEPROM_loc >> 8); //0x00;  //SJ3231013 - 24LC256I Most Significant Byte Address
            gI2C_Buffer[2]  = (EEPROM_loc & 0x00FF); //0x00;  //SJ3231013 - 24LC256I Least Significant Byte Address

            SW_I2CStart();
            SW_I2CWrite(67);//(gstate_var.RC_index);     //SJ4130214 - Max if 64 bytes
            SW_I2CStop();

            gstate_var.general_delay = 5;
            while (gstate_var.general_delay) ;

            gstate_var.EEPROM_offset += PAGE_SIZE;
        }
*****/
#endif
        gstate_var.EEPROM_offset = 0x4000;
        gstate_var.RC_index = 3;

        //SJ5210214 - Ready to accept next input
        gI2C_Buffer[0] = ACK_GUI; //0x3E;  //SJ4200214 - equiv > sign
        gI2C_Buffer[1] = '\0';  //SJ5210214 - EOL
        TimerA_UART_print((char *) gI2C_Buffer);
    }
    else
  #endif
    {
        gstate_var.EEPROM_offset += 64;//PAGE_SIZE;  //SJ4200214 - equiv 64;
        gstate_var.RC_index = 3;
        //gstate_var.general_delay = 4;
        //while (gstate_var.general_delay) ;
    }

    if (gI2C_Buffer[67] == END_OF_INPUT)
        seq = SYS_CLEAR_EEPROM_FLAG;
    //else
    //{
        //SJ4200214 - Ready to accept next input
    //    gI2C_Buffer[0] = ACK_GUI; //0x3E;  //SJ4200214 - equiv > sign
    //    gI2C_Buffer[1] = '\0';  //SJ5210214 - EOL
    //    TimerA_UART_print((char *) gI2C_Buffer);
    //}

    //return (SYS_CLEAR_EEPROM_FLAG);  //SJ4130214 - From here goes to clear EEPROM flag
    //return (SYS_PROCESS_MANAGER);  //SJ2140114 - From here goes to process manager.
    //return (SYS_IDLE_MODE);  //SJ2140114 - Goes to system idle mode.

    return (seq);

}  //SJ3231013 - End of Write_To_24LC256I() function
#endif


#if 1
//SJ3260214 - Read 24LC256I's address
//SYS_READ_24LC256I
int Read_From_24LC256I(int *reg_addr)
{
    //int seq=SYS_PROCESS_MANAGER;
    //unsigned int EEPROM_loc=0x0000;
    unsigned char proceed_flag=0xFF;
    //unsigned char register_addr=(unsigned char) *reg_addr;
    unsigned char num_read, bal_to_read, byte_read;
    unsigned char in_arr[65];
    unsigned char temp1, temp2;
    unsigned char ndx, i;
//    unsigned char *dummy_ptr = (unsigned char *) 0x1000;

    P1OUT |= 0x01;  //SJ3190314 - Turn on LED
#if 0  //SJ2110314 - For debug purpose
    gI2C_Buffer[0] = 'R';  //SJ4200214 - 
    gI2C_Buffer[1] = 'e';  //SJ4200214 - 
    gI2C_Buffer[2] = 'a';  //SJ4200214 - 
    gI2C_Buffer[3] = 'd';  //SJ4200214 - 
    gI2C_Buffer[4] = 'i';  //SJ4200214 - 
    gI2C_Buffer[5] = 'n';  //SJ4200214 - 
    gI2C_Buffer[6] = 'g';  //SJ4200214 - 
    gI2C_Buffer[7] = '>';  //SJ4200214 -
    gI2C_Buffer[8] = '\n';  //SJ4200214 -
    gI2C_Buffer[9] = '\0'; 
    TimerA_UART_print((char *) gI2C_Buffer);
#endif

    //TACCTL0 &= ~CCIE;  //SJ1240314 - Disable software UART interrupt (Transmit)
    TACCTL1 &= ~CCIE;  //SJ3120314 - Disable software UART interrupt (Read)

//    SW_I2CReset();
//    gstate_var.general_delay = 10;
//    while (gstate_var.general_delay) ;

  #ifndef WITH_USI_I2C
    gI2C_Buffer[0] = DEV_WriteCommand;     //SJ2020713 - Transmit 2 bytes
    gI2C_Buffer[1] = 0x00;//(EEPROM_loc >> 8);  //SJ3260214 - 24LC256I Most Significant Byte Address
    gI2C_Buffer[2] = 0x00;//(EEPROM_loc & 0x00FF);  //SJ3260214 - 24LC256I Most Significant Byte Address
    SW_I2CStart();
    SW_I2CWrite(3);     //SJ3260214 - TX 3 bytes
//    SW_I2CStop();
  #else  //SJ4200314 - Use USI_I2C

    //gstate_var.RC_index = 1;
    i2c_usi_mst_gen_start();  //SJ4200314 - Start I2C
    i2c_usi_mst_send_address(0x51, 0);  //SJ4200314 - Write operation
    i2c_usi_mst_send_byte(0x40);  //SJ4200314 - EEPROM address - MSB
    i2c_usi_mst_send_byte(0x00);  //SJ4200314 - EEPROM address - LSB
    //i2c_usi_mst_gen_stop();  //SJ4200314 - Stop I2C
    i2c_usi_mst_gen_start();  //SJ4200314 - Start I2C
    //i2c_usi_mst_gen_repeated_start();  // generate RESTART

  #endif


#if 1
    //SJ3260214 - Total bytes to be read is 169. Each EEPROM page is 64 bytes.
    //SJ3260214 - First loop num_read = 0
    //SJ3260214 - Second loop num_read = 64
    //SJ3260214 - Third loop num_read = 126
    //SJ3260214 - Fourth loop num_read = 190
    num_read = 0;
    bal_to_read = 170;
    while (num_read < 170)
    {
        //byte_read = (bal_to_read >= 64) ? 64 : bal_to_read;
        byte_read = (bal_to_read >= PAGE_SIZE) ? PAGE_SIZE : bal_to_read;  //SJ3120314 - PAGE_SIZE = 32
      #ifndef WITH_USI_I2C
        gI2C_Buffer[0] = DEV_ReadCommand;  //SJ3260214 - 
        proceed_flag = SW_I2CRead(byte_read);  //SJ2020713 -
        //proceed_flag = SW_I2CRead(64);  //SJ2020713 -
      #else  //SJ4200314 - Use USI_I2C

        proceed_flag = 1;
        if(i2c_usi_mst_send_address(0x51, 1) == 1)
        {
            for (ndx=0; ndx<byte_read; ndx++)
            {
                gI2C_Buffer[ndx] = i2c_usi_mst_read_byte();  //SJ4200314 - Read one byte
                i2c_usi_mst_send_n_ack(1);  //SJ4200314 - ACK slave
            }
            
            i2c_usi_mst_send_n_ack(0);  //SJ4200314 - NACK slave
            i2c_usi_mst_gen_stop();  //SJ4200314 - Stop I2C
            proceed_flag = 0;
        }

      #endif

        if (proceed_flag == 0)  //SJ3260214 - Berarti baca dari EEPROM berhasil
        {
            gI2C_Buffer[64] = '\0';
            //TimerA_UART_print((char *) gI2C_Buffer);
            //TimerA_UART_print_line((char *) gI2C_Buffer, byte_read);//64);

            //SJ3120314 - Need to add a function to edit buffer before transmit out.
            //SJ3120314 - Use gI2C_Buffer loc 65 and 66 for bit manipulation
            i = 0;
            for (ndx=0; ndx<byte_read; ndx++)
            {
                //gI2C_Buffer[65] = gI2C_Buffer[ndx] >> 4;  //SJ3120314 - bit 4 to 7
                //gI2C_Buffer[66] = gI2C_Buffer[ndx] & 0x0F;  //SJ3120314 - bit 0 to 3
                temp1 = gI2C_Buffer[ndx] >> 4;  //SJ3120314 - bit 4 to 7
                temp2 = gI2C_Buffer[ndx] & 0x0F;  //SJ3120314 - bit 0 to 3

                switch (temp1)
                {
                    case 0:
                    case 1:
                    case 2:
                    case 3:
                    case 4:
                    case 5:
                    case 6:
                    case 7:
                    case 8:
                    case 9:
                        in_arr[i] = 48 + temp1;
                        break;

                    case 10:
                    case 11:
                    case 12:
                    case 13:
                    case 14:
                    case 15:
                        in_arr[i] = 55 + temp1;
                        break;
                }
                switch (temp2)
                {
                    case 0:
                    case 1:
                    case 2:
                    case 3:
                    case 4:
                    case 5:
                    case 6:
                    case 7:
                    case 8:
                    case 9:
                        in_arr[i+1] = 48 + temp2;
                        break;

                    case 10:
                    case 11:
                    case 12:
                    case 13:
                    case 14:
                    case 15:
                        in_arr[i+1] = 55 + temp2;
                        break;
                }

                i += 2;
            }  //SJ3120314 - for(...) { ... }

            ndx = byte_read * 2;
            TimerA_UART_print_line((char *) in_arr, ndx);//64);

            gstate_var.general_delay = 20;
            while (gstate_var.general_delay) ;

        }  //SJ4130314 - End of if (proceed_flag == 0) { ... }

        bal_to_read -= PAGE_SIZE;
        num_read += PAGE_SIZE;  //SJ3260214 - equiv 64;
    }
#endif
//    gstate_var.general_delay = 50;
//    while (gstate_var.general_delay) ;
    //TACCTL1 |= CCIE;  //SJ3120314 - Enable software UART

//    return (SYS_PROCESS_MANAGER);  //SJ4270214 - Kode percobaan. Kode dibawah yg harus di pake
    return (SYS_IDLE_MODE);  //SJ2140114 - Goes to system idle mode

}  //SJ3260214 - End of Read_From_24LC256I() function
#endif


#if 1  //SJ2140114 -
//SJ2140114 - Put system to idle. For the moment just loop infinitely; for future expansion, may consider
//SJ2140114 - putting MCU to low power mode.
//SYS_IDLE_MODE
int System_Idle_Mode(int *none)
{
    //while (1) ;

    return (SYS_IDLE_MODE);  //SJ2140114 - Goes to system idle mode

}  //SJ2140114 - End of System_Idle_Mode() function
#endif


#if 1  //SJ4130214 - This routine mark EEPROM had been written.
//SYS_CLEAR_EEPROM_FLAG
int Clear_EEPROM_Flag(int *none)
{
  #if 0  //SJ1240214 - Mark as processed in flash
    unsigned char *dummy_ptr = (unsigned char *) 0x1000;
    //unsigned char i;
    unsigned int key = FWKEY;

    IE1 &= ~WDTIE;
    WDTCTL = WDTPW+WDTHOLD;           // stop the WDT

    //FCTL2 = key+FSSEL0+FLASH_DIVIDER; // 333kHz Flash Timing Generator
    FCTL2 = key + FSSEL0 + FN1;  //SJ4130214 - To achieve 333kHz, need to use FN1 ==> 1000000 / (2+1)
    FCTL3 = key;

    //--- SJ1171212 - Start of flash write -----------------------------------------
    FCTL1 = key+ERASE;
    dummy_ptr[0] = 0;  //SJ4130214 - Dummy write
    FCTL1 = key+WRT;

    //SJ4130214 - Write 0x00DEAD
    dummy_ptr[0] = 0x00;  //SJ4130214 - 
    dummy_ptr[1] = 0xDE;  //SJ4130214 - 
    dummy_ptr[2] = 0xAD;  //SJ4130214 - 

    /*****
    i = 3;
    while (i < gstate_var.RC_index)
    {
        dummy_ptr[i] = gI2C_Buffer[i];
        i++;
    }
    *****/

    __no_operation();
    __no_operation();
    __no_operation();

    FCTL1 = key;  /* lock flash */
    FCTL3 = key+LOCK;

    //SJ4130214 - Re-activate watchdog timer
    WDTCTL = WDTPW + WDTTMSEL + WDTCNTCL;  //SJ4080312 - Clock is 1MHz, counter is 32768, so 1000000/32768 = 31 times
    IE1 |= WDTIE;
  #endif

    P1OUT |= 0x01;  //SJ4130214 - Turn on LED to indicate end of operation.
    gI2C_Buffer[0] = END_OF_TASK;  //SJ4200214 - equiv ESC
    gI2C_Buffer[1] = '\0';  //SJ5210214 - EOL
    TimerA_UART_print((char *) gI2C_Buffer);  //SJ5210214 - End of process.

  #if 1  //SJ4240414 - ORG
    return (SYS_IDLE_MODE);  //SJ2140114 - Goes to system idle mode
  #else  //SJ4240414 - To allow read after write, used below code to test
    gstate_var.oper_phase = READ_EEPROM;  //SJ4240414 - Paksa ke baca mode

    gstate_var.general_delay = 60;  //SJ4240414 - Around 2 seconds delay
    while (gstate_var.general_delay) ;

    return (SYS_PROCESS_MANAGER);  //SJ4240414 - Goes to progress manager mode
  #endif

}  //SJ4130214 - End of Clear_EEPROM_Flag() function
#endif

//SJ5210214 - END OF CODE ----------------------------

#if 0  //SJ3231013 - Used for ioExpander
//SJ2020713 - Read Port 1 of TCA6416A ioExpander
//SYS_READ_TCA6416A
//int Read_From_TCA6416A(int *num_of_byte)
int Turn_On_Relevant_LEDs(int *num_of_byte)
{
    unsigned char proceed_flag=0xFF;
    unsigned char in_val=0;
    unsigned char out_mask=0;

    gI2C_Buffer[0] = DEV_WriteCommand;     //SJ2020713 - Transmit 2 bytes
    gI2C_Buffer[1] = 0x01;  //SJ2020713 - TCA6416A Input port1 command
    SW_I2CStart();
    SW_I2CWrite(2);     //SJ2020713 - TX 2 bytes
    SW_I2CStop();
    gI2C_Buffer[0] = DEV_ReadCommand;  //SJ4040713 - Transfered from SW_I2CRead() function
    proceed_flag = SW_I2CRead(1);  //SJ2020713 -

    //SJ2020713 - Below two sttmts may be needed depending on the hardware design
    //gI2C_Buffer[0] |= 0xF0;  //SJ4040413 - Force bit 4 to 7 to high
    //gI2C_Buffer[0] |= BUTTON_MASK;  //SJ4100413 - Mask off those un-used bits
    //gstate_var.I2C_freq = I2C_FREQ;   //SJ4100413 - 200 milli seconds;

    if (proceed_flag == 0)
    {
        //SJ3030613 - Need to map pin 2, 3, 4, 5, 6 of TCA6416A
        //SJ3030613                to 4, 3, 2, 1, 0 of MSP430G2332.
        in_val = (gI2C_Buffer[0] >> 2) & 0x01;  //SJ3030613 - Pin 2 is in LSB
        out_mask |= (in_val << 4);  //SJ3030613 - P1OUT pin 4

        in_val = (gI2C_Buffer[0] >> 3) & 0x01;  //SJ3030613 - Pin 3 is in LSB
        out_mask |= (in_val << 3);  //SJ3030613 - P1OUT pin 3

        in_val = (gI2C_Buffer[0] >> 4) & 0x01;  //SJ3030613 - Pin 4 is in LSB
        out_mask |= (in_val << 2);  //SJ3030613 - P1OUT pin 2

        in_val = (gI2C_Buffer[0] >> 5) & 0x01;  //SJ3030613 - Pin 5 is in LSB
        out_mask |= (in_val << 1);  //SJ3030613 - P1OUT pin 1

        in_val = (gI2C_Buffer[0] >> 6) & 0x01;  //SJ3030613 - Pin 6 is in LSB
        out_mask |= (in_val);  //SJ3030613 - P1OUT pin 0

        //SJ3030613 - Retain the value of pin 7, 6, and 5 of P1OUT.
        //SJ3030613 - It is crucial to do this step because pin 7, and 6 are SDA and SCL of I2C.
        //SJ3030613 - They should not be changed to prevent unnecessary false trigger of I2C.
        in_val = P1OUT & 0xE0;
        in_val |= out_mask;
        P1OUT = in_val;  //SJ3030613 - This will turn on respective LED.
    }
    else  //SJ4040713 - Decide what to do when reading to ioExpander fails
    {
        ;
    }

    gstate_var.I2C_freq = 20;  //SJ2020713 - equiv to 150miliseconds
    //gstate_var.general_ctrl_flag |= NEW_DATA_ON;
    //gFunc_Seq = SYS_PROCESS_MANAGER;

    return (SYS_WRITE_TCA6416A);//(SYS_PROCESS_MANAGER);

}  //SJ2020713 - End of Read_From_TCA6416A() function
#endif


#if 0
//SJ2020713 - This function is to initialise TCA6416A ioexpander.
//SJ2020713 - In other word, the device acts as a master.
//SYS_INIT_TCA6416A
int Init_TCA6416A(int *param)
{
    //int i, j;//, k;

    /*****  //SJ2020713 - Skip below reset first
    P2OUT &= 0xF3;  //SJ2020713 - Put pin ??? low. (11110011)
    __no_operation();
    __no_operation();
    __no_operation();
    __no_operation();
    P2OUT |= 0x08;  //SJ2020713 - Put pin ??? to high. (00001000)
    __no_operation();
    __no_operation();
    *****/

/*****
    //SJ2020713 - Reverse polarity. If needed.
    gI2C_Buffer[0] = DEV_WriteCommand;  //SJ2020713 - Slave address
    gI2C_Buffer[1] = 0x04;  //SJ2020713 - TCA6424A command for polarity inversion
    gI2C_Buffer[2] = 0xFF;  //SJ2020713 - Set Port 0 polarity invert
    gI2C_Buffer[3] = 0xFF;  //SJ2020713 - Set Port 1 polarity invert
*****/

    //SJ2020713 - Initialise TCA6416A
    //gI2C_Buffer[TCA6416A_NDX_9] = 0xFF;  //SJ2190313 - Indicate no repeat start is needed
    gI2C_Buffer[0] = DEV_WriteCommand;     //SJ3200313 - Number of data
    gI2C_Buffer[1] = 0x06;  //SJ2050213 - TCA6424A command for port configuration
//SJ3030713 - SJTODO: Review if port 0 pin 0 to 3 need to be set as output!!!
//SJ3030713 -         At the moment set as input (0x0F). If need to set as output change value to 0x00
    gI2C_Buffer[2] = 0x0F;//0x00;  //SJ2020713 - Set Port 0 as output. (1 means port pin is set as input, 0 means set as output)
    gI2C_Buffer[3] = 0xFC;  //SJ2020713 - Set Port 1 Pin 0, and 1 as output; 2 to 7 as input (11111100 )
    //gI2C_Buffer[4] = 0x00;  //SJ2050213 - Set Port 2 as output
    SW_I2CStart();
    SW_I2CWrite(4);     //SJ2020713 - TX 4 bytes
    SW_I2CStop();

    //gFunc_Seq = SYS_PROCESS_MANAGER;
    //gstate_var.I2C_freq = 3;  //SJ2020713 - equiv to 150miliseconds  //SJ4040713 - Can be removed. Confirm later
    //return (SYS_PROCESS_MANAGER);//(0);  //SJ4040713 - ORG
    return (SYS_INIT_TMP431A);  //SJ4110713 - From here goes to init temerature sensor IC (TMP431A)

}  //SJ2020713 - End of Init_TCA6416A() function
#endif


#if 0  //SJ3231013 - Used for ioExpander
//SJ3030713 - Write to Port 0 of TCA6416A ioExpander
//SYS_WRITE_TCA6416A
int Write_To_TCA6416A(int *num_of_byte)
{
    unsigned char port_0, port_1;

//#define ANNEX_A_SM       0
//#define ANNEX_B_SM       1
//#define ANNEX_A_30A      2
//#define ANNEX_B_30A      3
//#define ANNEX_A_BONDED   4
//#define RELAY_1_ENABLE   5
//#define RELAY_2_ENABLE   6

    port_0 = 0x00;  //SJ4110713 - xxxxxx00
    port_1 = 0x70;  //SJ4110713 - 0111xxxx
    switch (gstate_var.board_type)
    {
        //SJ4110713 - Relay 1 disable, Relay 3 enable
        //SJ4110713 - Port 0: 1100xxxx, Port 1: xxxxxx00
        case ANNEX_A_SM:       //SJ4110713 - 0
            port_0 = 0xC0;  //SJ4110713 - 1100xxxx
            port_1 = 0x00;  //SJ4110713 - xxxxxx00
            break;

        //SJ4110713 - Relay 1 disable, Relay 3 disable
        //SJ4110713 - Port 0: 1000xxxx, Port 1: xxxxxx10
        case ANNEX_B_SM:       //SJ4110713 - 1
            port_0 = 0x80;  //SJ4110713 - 1000xxxx
            port_1 = 0x02;  //SJ4110713 - xxxxxx10
            break;

        //SJ4110713 - Board type 2, 3, and 4 share same behaviour for relay 1 and relay2
        //SJ4110713 - Relay 1 disable, Relay 2 disable
        //SJ4110713 - Port 0: 1000xxxx, Port 1: xxxxxx01
        case ANNEX_A_30A:      //SJ4110713 - 2
            //break;
        case ANNEX_B_30A:      //SJ4110713 - 3
            //break;
        case ANNEX_A_BONDED:   //SJ4110713 - 4
            port_0 = 0x80;  //SJ4110713 - 1000xxxx
            port_1 = 0x01;  //SJ4110713 - xxxxxx01
            break;

        //SJ4110713 - Board type 5, relay 1 becomes enalbe
        //SJ4110713 - Port 0: 0101xxxx, Port 1: xxxxxx00
        //SJ4110713 - Port 0: 0001xxxx, Port 1: xxxxxx10  (Also possible for Annex B single mode)
        case RELAY_1_ENABLE:   //SJ4110713 - 5
            port_0 = 0x50;  //SJ4110713 - 0101xxxx
            port_1 = 0x00;  //SJ4110713 - xxxxxx00
            //SJ4110713 - Below also possible for Annex B single mode
            //port_0 = 0x10;  //SJ4110713 - 0001xxxx
            //port_1 = 0x02;  //SJ4110713 - xxxxxx10
            break;

        //SJ4110714 - Board type 6, relay 2 becomes enable
        //SJ4110713 - Port 0: 1010xxxx, Port 1: xxxxxx00
        case RELAY_2_ENABLE:   //SJ4110713 - 6
            port_0 = 0xA0;  //SJ4110713 - 1010xxxx
            port_1 = 0x00;  //SJ4110713 - xxxxxx00
            break;

    }  //SJ4110713 - End of switch(gstate_var.board_type) { ... }

    gI2C_Buffer[0] = DEV_WriteCommand;     //SJ3030713 - Transmit 4 bytes
    gI2C_Buffer[1] = 0x02;  //SJ3030713 - TCA6416A command output port
    //out_val = 0xF0;  //SJ3030713 - equiv 11110000
    gI2C_Buffer[2] = port_0;//0xF0;  //SJ3030713 - Write to output port 0 (pin 4, 5, 6, 7)
    gI2C_Buffer[3] = port_1;//0x03;  //SJ3030713 - Write to output port 1

    SW_I2CStart();
    SW_I2CWrite(4);     //SJ3030713 - TX 4 bytes
    SW_I2CStop();

    return (SYS_PROCESS_MANAGER);  //SJ4110713 - From here goes to process manager.

}  //SJ3030713 - End of int Write_To_TCA6416A() function
#endif


//SJ4040713 - Below #if ... #endif block contains routine meant accessing TMP431A device
#if 0
//SJ4040713 - This function is to initialise TMP431A temperature sensor device.
//SYS_INIT_TMP431A
int Init_TMP431A(int *param)
{
    gI2C_Buffer[0] = TMP431A_WriteCmd;     //SJ4040713 - Transmit 3 bytes
    gI2C_Buffer[1] = 0xFC;  //SJ4040713 - TMP431A software reset register
    gI2C_Buffer[2] = 0xAA;  //SJ4040713 - Write any value to 0xFC, will activate software reset
    SW_I2CStart();
    SW_I2CWrite(3);     //SJ4040713 - TX 3 bytes
    SW_I2CStop();

    __no_operation();
    __no_operation();
    __no_operation();
    __no_operation();

    //SJ4040713 - At the moment that's all we need to do to reset TMP431A, should there be a need
    //SJ4040713 - to do other settings, add those commands below here below return sttmt.

    return (SYS_READ_TCA6416A);//(SYS_PROCESS_MANAGER);

}  //SJ4040713 - End of Init_TMP431A() function


//#define TMP431A_SlaveAddr  0x4C  //SJ3030713 - Bit 7-1 1001100x
//#define TMP431A_WriteCmd   0x98  //SJ3030713 - Bit 7-0 10011000
//#define TMP431A_ReadCmd    0x99  //SJ3030713 - Bit 7-0 10011001


//SJ4040713 - Read TMP431A's register
//SYS_READ_TMP431A
int Read_From_TMP431A(int *reg_addr)
{
    unsigned char proceed_flag=0xFF;
    unsigned char register_addr=(unsigned char) *reg_addr;

    gI2C_Buffer[0] = TMP431A_WriteCmd;     //SJ2020713 - Transmit 2 bytes
    gI2C_Buffer[1] = register_addr;//0x00  //SJ4040713 - Set TMP431A pointer address to register 0
    SW_I2CStart();
    SW_I2CWrite(2);     //SJ2020713 - TX 2 bytes
    SW_I2CStop();
    gI2C_Buffer[0] = TMP431A_ReadCmd;  //SJ4040713 - Transfered from SW_I2CRead() function
    proceed_flag = SW_I2CRead(2);  //SJ2020713 -

    //if (proceed_flag == 0)
    //    gstate_var.general_ctrl_flag |= NEW_DATA_ON;

    //gFunc_Seq = SYS_PROCESS_MANAGER;

    return (SYS_PROCESS_MANAGER);

}  //SJ4040713 - End of Read_From_TMP431A() function


//SJ4040713 - Write to TMP431A's register
//SYS_WRITE_TMP431A
int Write_To_TMP431A(int *num_of_byte)
{
    //SJ4040713 - Don't think there is anything to write to TMP431A as of now.

    return (SYS_PROCESS_MANAGER);

}  //SJ4040713 - End of Write_To_TMP431A() function

#endif

//SJ4040310 - Retrieve from ADC and convert its output to signed integer
int RetrieveFromADC(int *data_format)
{
    int temp_adc;

    //ADC10CTL0 &= ~ENC;  //SJ1010310 - Moved up from below

/*****
SREF_0: (VR+ = AVCC   and VR- = AVSS)
SREF_1: (VR+ = VREF+  and VR- = AVSS)
SREF_2: (VR+ = VEREF+ and VR- = AVSS)
SREF_4: (VR+ = AVCC   and VR- = VREF-/VEREF-)
SREF_5: (VR+ = VREF+  and VR- = VREF-/VEREF-)
SREF_6: (VR+ = VEREF+ and VR- = VREF-/VEREF-)
*****/

    //SJ1150310 - ADC10SHT_2 Sample 16 while ADC10SHT_3 Sample 64. 
    //ADC10CTL0 = SREF_1 + ADC10SHT_2 + ADC10SR + REF2_5V + REFON + ADC10ON + ENC + ADC10SC;  //SJ2110510 - Using 2.5 volts  //SJ1031011 -
    //ADC10CTL0 = SREF_6 + ADC10SHT_2 + ADC10SR + ADC10ON + ENC + ADC10SC;  //SJ1031011 - (SREF_6: VR+ = VEREF+ and VR- = VREF-/VEREF-)
    ADC10CTL0 = SREF_2 + ADC10SHT_2 + ADC10SR + ADC10ON + ENC + ADC10SC;  //SJ1031011 - (SREF_2: VR+ = VEREF+ and VR- = AVSS)

    while (ADC10CTL1 & ADC10BUSY);  //SJ4050810 - NOTE: Do we need to reset WDT+ counter here???
    temp_adc = ADC10MEM;
    ADC10CTL0 &= ~ENC;

    if (*data_format == ADC_TWO_COMPLEMENT)
    {
        if (temp_adc & 0x8000)  //SJ5260210 - If true, negative value
        {
            temp_adc = ~temp_adc;
            temp_adc = ((temp_adc >> 6) + 1) * (-1);
        }
        else
        {
            temp_adc = (temp_adc >> 6);
        }
    }

    return (temp_adc);

}  //SJ4040310 - End of RetrieveFromADC()



//SJ4160513 - Codes above this point are to be considered as official codes for this project.
//==============================================================================
//SJ4160513 - DO NOT put any code beyond this point. If any of below functions is needed, cut and move up.


//SJ4130314 - PARKING LOT
#if 0  //SJ5060712
//if (gstate_var.GEN_ctrl_flag.com_field.BYTE & NEW_DATA_ON)
//gstate_var.GEN_ctrl_flag.com_field.BYTE &= NEW_DATA_OFF;
//gstate_var.GEN_ctrl_flag.com_field.BYTE &= BLINK_LED_OFF;  //SJ5300911 - At the moment just this is enough. If more flags are used, need to revise
//switch (oper_mode)
//{
//}  //SJ5300911 - switch (oper_mode) { ... }
//_BIS_SR(LPM3_bits + GIE);     //SJ4101111 - Enter LPM3 mode. CPU, MCLK, SMCLK all set to off
//IE1 &= ~WDTIE;  //SJ4101111 - Disable WDT interrupt
//WDTCTL = WDT_ARST_1000;  //SJ4101111 - Put back to watchdog mode - (WDTPW + WDTCNTCL + WDTSSEL)
//while (1) ;  //SJ4101111 - Auto reset
//(gstate_var.GEN_ctrl_flag.com_field.BYTE & NEW_DATA_ON)  //SJ5281011 - For reference only
//gstate_var.GEN_ctrl_flag.com_field.BYTE &= NEW_DATA_OFF; //SJ5281011 - For reference only


/*****
    //SJ3120314 - Processing bit 4 to 7
    //if ((gI2C_Buffer[65] >= 0) && (gI2C_Buffer[65] <= 9))
    if ((temp1 <= 9))
    {
        in_arr[i] = 48 + temp1;  //SJ3120314 - 48 being ASCII code for 0
    }
    else if ((temp1 >= 10) && (temp1 <= 15))
    {
        in_arr[i] = 55 + temp1;  //SJ3120314 - ASCII code representation for A is 65.
    }
//    else
//        in_arr[i] = 186;

    //SJ3120314 - Processing bit 0 to 3
    //if ((gI2C_Buffer[66] >= 0) && (gI2C_Buffer[66] <= 9))
    if ((temp2 <= 9))
    {
        in_arr[i+1] = 48 + temp2;  //SJ3120314 - 48 being ASCII code for 0
    }
    else if ((temp2 >= 10) && (temp2 <= 15))
    {
        in_arr[i+1] = 55 + temp2;  //SJ3120314 - ASCII code representation for A is 65.
    }
//    else
//        in_arr[i+1] = 186;
*****/


/*****
        if (gstate_var.write_once == 0)
        {
            IE1 &= ~WDTIE;
            WDTCTL = WDTPW+WDTHOLD;           // stop the WDT

            //FCTL2 = FWKEY+FSSEL0+FLASH_DIVIDER; // 333kHz Flash Timing Generator
            FCTL2 = FWKEY + FSSEL0 + FN1;  //SJ4130214 - To achieve 333kHz, need to use FN1 ==> 1000000 / (2+1)
            FCTL3 = FWKEY;

            //--- SJ1171212 - Start of flash write -----------------------------------------
            FCTL1 = FWKEY+ERASE;
            dummy_ptr[0] = 0;  //SJ4130214 - Dummy write
            FCTL1 = FWKEY+WRT;

            ndx = 3;
            while (ndx < (EEPROM_BUFFER-1))  //gstate_var.RC_index)
            {
                dummy_ptr[ndx] = gI2C_Buffer[ndx-3];
                ndx++;
            }

            __no_operation();
            __no_operation();
            __no_operation();

            FCTL1 = FWKEY;  // lock flash
            FCTL3 = FWKEY+LOCK;

            //SJ4130214 - Re-activate watchdog timer
            WDTCTL = WDTPW + WDTTMSEL + WDTCNTCL;  //SJ4080312 - Clock is 1MHz, counter is 32768, so 1000000/32768 = 31 times
            IE1 |= WDTIE;
            gstate_var.write_once = 1;
        }
*****/

#endif

//--- End of file (JDSU_Gemini_Main.c) --------------------------------------------------------------------

