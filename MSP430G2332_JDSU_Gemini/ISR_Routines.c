/******************************************************************************/
/*                                                                            */
/*  FILE        : ISR_Routines.c                                              */
/*  DATE        : Tue, July 02, 2013                                          */
/*  DESCRIPTION : Interrupt Service Routine                                   */
/*  CPU TYPE    : MSP430G2332                                                 */
/*  PROJECT     : JDSU Gemini                                                 */
/*  AUTHOR      : Suilarso Japit                                              */
/*  VERSION     : 100.00                                                      */
/*                                                                            */
/******************************************************************************/


//#include   <msp430x23x2.h>    //<msp430x21x2.h>
//#include   <msp430x54x.h>  //SJ2050213 - ???
#include   <msp430G2332.h>
#include   <string.h>  //SJ2131211 - for memmove() function prototype

//#include   "gp_io.h"
#include   "global_var.h"
//#include   "User_Input.h"



//=== SJ1031011 - External global var =========================================
//JDSU_Gemini_Main.c
//extern unsigned char ggeneral_buffer[50];
//extern int gvoltage_tab[VOLTAGE_SAMPLE];
//extern int gcurrent_tab[CURRENT_SAMPLE];
//extern unsigned char PTxData[5];  //SJ2050213 - I2C transmit buffer  //SJ2050213 - SJI2C

extern STATE_STRUCT  gstate_var;


//ISR_Routines.c
//=== SJ1031011 - END. External global var ====================================


//=== SJ1031011 - External function prototype =================================
//main.c
extern int RetrieveFromADC(int data_format);

//user_input.c
//=== SJ1031011 - END. External function prototype ============================


//__interrupt void Timer_A(void);  //SJ4120106 - Used in Code composer essentials
//__interrupt void ISR_Port1(void);
//__interrupt void ISR_ADC10(void);



#if 0  //--- SJ3190510 - Below codes for AC monitoring -------------
/*
FUNCTION: P2.2 IRQ - decoding interrupt service routine.
*/

#pragma vector=PORT2_VECTOR
__interrupt void ISR_Port2(void)
{
}  // end of ISR_Port2.
#endif //--- SJ1110411 - END ---------------------------------------


#if 0  //SJ3060213 - Don't use I2C first
//SJ1040213 - Copy from msp430g2xx3_uscib0_i2c_08.c
//SJ2050213 - SJI2C
//------------------------------------------------------------------------------
// The USCIAB0TX_ISR is structured such that it can be used to transmit any
// number of bytes by pre-loading gstate_var.TXByteCtr with the byte count. 
// Also, TXData points to the next byte to transmit.
//------------------------------------------------------------------------------
#pragma vector=USCIAB0TX_VECTOR
__interrupt void ISR_USCI_B0_I2C_Tx(void)
{
    if (gstate_var.TXByteCtr)                          // Check TX byte counter
    {
        //UCB0TXBUF = *PTxData++;               // Load TX buffer  //SJ2050213 - IAR
        UCB0TXBUF = PTxData[4-gstate_var.TXByteCtr];  //SJ2050213 - Take note of constant 4
        gstate_var.TXByteCtr--;               // Decrement TX byte counter
    }
    else
    {
        UCB0CTL1 |= UCTXSTP;                  // I2C stop condition
        UC0IFG &= ~UCB0TXIFG;  //SJ2050213 - MSP430G2553.h
        //__bic_SR_register_on_exit(LPM0_bits); // Exit LPM0

//#define UC0IFG                 IFG2
//#define UCA0RXIFG              (0x01)
//#define UCA0TXIFG              (0x02)
//#define UCB0RXIFG              (0x04)
//#define UCB0TXIFG              (0x08)
    }

}  //SJ2050213 - End of ISR_USCI_B0_I2C_Tx() function
#endif

#if 0
//SJ4120712 - UART interrupt service routine to read input from UART receive buffer.
#pragma vector=USCIAB0RX_VECTOR
__interrupt void ISR_UART_RX_A0(void)
{
    unsigned char receive_buf;
    unsigned int buf_ndx;//=MESSAGE_HEADER_LEN;  //SJ2170712 - equiv 8 bytes
    //unsigned int temp_LSB, temp_MSB;

}  //SJ4221112 - End of ISR_UART_RX_A0()
#endif


//SJ4120712 - With current timer setting, each second, this ISR will get in about 20 times.
//SJ4120712 - 50ms per interrupt.
#if 0  //SJ4130214 - ORG
#pragma vector=TIMER0_A0_VECTOR
__interrupt void ISR_Timer0_A0(void)
#else  //SJ4130214 - Replaced. Using watchdog timer interval
//#pragma vector=TIMER1_A0_VECTOR
//__interrupt void ISR_Timer1_A2(void)

#pragma vector=WDT_VECTOR
__interrupt void ISR_Watchdog_Timer(void)
#endif
{
//SJ1270812    unsigned char ndx=0;
    //SJ2091012 unsigned char temp_led;
    //unsigned int temp_signal=0;  //SJ3121212 - Not used in iSwitch project

    //SJ4140110 - When TimerA interrupt kicks in, first we need to stop the timer 
    //SJ4140110 - from counting. Follow by whatever actions we need to do.
    //SJ4140110 - Finally, re-activate timer counts.

    //SJ1110411 - Disable timer.
    //SJ1110411 TACCR0 = 0;      //SJ1110411 - Doing so will stop timer
    //SJ1110411 TACTL |= MC_0;   //Stop timer

    if (gstate_var.general_delay)
        gstate_var.general_delay--;

  #if 0  //SJ1280113 - Not relevant. DO NOT REMOVE first
    if (gstate_var.led_timer)
        gstate_var.led_timer--;
  #endif

    //SJ3191212 - Not used so far. KIV
    //if (gstate_var.flash_update_countdown)
    //    gstate_var.flash_update_countdown--;


  #if 0//def RED_N_GREEN_LED  // 1
    if (gstate_var.general_ctrl_flag & RED_LED_BLINK_ON)  //SJ0190812 - Replaced
    {
        //if (gstate_var.red_led_timer == 0)  //SJ3191212 - ORG
        if (gstate_var.led_blink_freq == 0)
        {
            if (P1OUT & ON_RED_LED)  //SJ2091012 - 0x08 (00001000)
            {
                P1OUT &= OFF_RED_LED;  //SJ1081012 - Turn off red LED. 0xF7 (11110111)
            }
            else
            {
                P1OUT |= ON_RED_LED;
            }
            //gstate_var.red_led_timer = BLINK_FREQ_250_MS;  //SJ3191212 - ORG
            gstate_var.led_blink_freq = LED_BLINK_FREQ;
        }
        else
            //gstate_var.red_led_timer--;  //SJ3191212 - ORG
            gstate_var.led_blink_freq--;
    }

    if (gstate_var.general_ctrl_flag & GRN_LED_BLINK_ON)  //SJ0190812 - Replaced
    {
        //if (gstate_var.green_led_timer == 0)  //SJ3191212 - ORG
        if (gstate_var.led_blink_freq == 0)
        {
            if (P1OUT & ON_GREEN_LED)   //SJ2091012 - 0x20 (00100000)
            {
                P1OUT &= OFF_GREEN_LED;  //SJ1081012 - Turn off green LED. 0xDF (11011111)
            }
            else
            {
                P1OUT |= ON_GREEN_LED;
            }
            //gstate_var.green_led_timer = BLINK_FREQ_250_MS;  //SJ3191212 - ORG
            gstate_var.led_blink_freq = LED_BLINK_FREQ;
        }
        else
            //gstate_var.green_led_timer--;  //SJ3191212 - ORG
            gstate_var.led_blink_freq--;
    }
  #endif

  #if 0
    if (gstate_var.UART_timer)
        gstate_var.UART_timer--;
  #endif

  #if 0  //SJ4130214 - Not used
    if (gstate_var.I2C_freq)
        gstate_var.I2C_freq--;
  #endif

//SJ1270812    gstate_var.general_ctrl_flag |= NEW_DATA_ON;

    //SJ1110411 - Re-active timer before leaving ISR.
    //TACCR0 = gstate_var.ISR_constant;  //SJ1110411 - Initialise compare register with value.  //SJ4130214 - Not used

}  //SJ4120712 - End of ISR_Timer0_A2()


#ifdef RESTRICTED_MODE  //SJ4101111 - For restricted mode
#pragma vector=WDT_VECTOR
__interrupt void ISR_Watchdog_Timer(void)
{

    _BIC_SR_IRQ(LPM3_bits);  //SJ4101111 - This put CPU back to on mode

}  //SJ4101111 - End of ISR_Watchdog_Timer()
#endif //SJ4101111 - End of restricted mode


/***  //SJ4140110 - DO NOT REMOVE. Resuse when needed.
//SJ3130110 - ADC10 interrupt service routine
#pragma vector=ADC10_VECTOR
__interrupt void ISR_ADC10(void)
{
	unsigned long temp_val;
	
	temp_val = ADC10MEM;
	
//	ADC10CTL0 |= ENC + ADC10SC;
  //__bic_SR_register_on_exit(CPUOFF);        // Clear CPUOFF bit from 0(SR)
}
***/



