/******************************************************************************/
/*                                                                            */
/*  FILE        : ISR_Routines.c                                              */
/*  DATE        : Fri, July 06, 2012                                          */
/*  DESCRIPTION : Interrupt Service Routine                                   */
/*  CPU TYPE    : MSP430G2332                                                 */
/*  PROJECT     : PLC Light                                                   */
/*  AUTHOR      : Suilarso Japit                                              */
/*  VERSION     : 100.00                                                      */
/*                                                                            */
/******************************************************************************/


//#include   <msp430x23x2.h>    //<msp430x21x2.h>
#include   <msp430G2553.h>
#include   <string.h>  //SJ2131211 - for memmove() function prototype

//#include   "gp_io.h"
#include   "PLC_Comm.h"
#include   "global_var.h"
//#include   "User_Input.h"
//#include   "TAS3103_util.h"


//=== SJ1031011 - External global var =========================================
//PLC_Lite.c
extern unsigned char ggeneral_buffer[50];
//extern int gvoltage_tab[VOLTAGE_SAMPLE];
//extern int gcurrent_tab[CURRENT_SAMPLE];

extern STATE_STRUCT  gstate_var;

//extern int gADC_data[10];
//extern short bit_counter;


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


//SJ4120712 - UART interrupt service routine to read input from UART receive buffer.
#pragma vector=USCIAB0RX_VECTOR
__interrupt void ISR_UART_RX_A0(void)
{
    unsigned char receive_buf;
    unsigned int buf_ndx;//=MESSAGE_HEADER_LEN;  //SJ2170712 - equiv 8 bytes
    //unsigned int temp_LSB, temp_MSB;

//SJ2170712 - Maybe we should include UART_Rx_mode

    receive_buf = UCA0RXBUF;  //SJ4190712 - Once buffer is read, UCAxRXIFG is reset

    if (gstate_var.UART_Rx_State == UART_STANDBY)
    {
        gstate_var.UART_Rx_ndx = 0;
        gstate_var.UART_Rx_State = UART_RECEIVING;
    }

    //if (gstate_var.UART_Rx_State == UART_RECEIVING && gstate_var.UART_Rx_ndx < DATA_BUFFER_SZ)  //SJ3281112 - Official
    if (gstate_var.UART_Rx_State == UART_RECEIVING)  //SJ3281112 - Replaced
    {
      gstate_var.UART_Rx_threshold = TIMER_2_SECONDS;  //SJ4291112 - Solving out of sync state

      if (gstate_var.UART_Rx_ndx < DATA_BUFFER_SZ)
      {
        //ggeneral_buffer[gstate_var.UART_Rx_ndx] = UCA0RXBUF;
        ggeneral_buffer[gstate_var.UART_Rx_ndx] = receive_buf;
        //SJ4221112 - Using payload lenght to decide the termination point.
        if (gstate_var.UART_Rx_ndx == 3)  //SJ2210812 - It's time to calculate payload length
            gstate_var.Rx_payload_len = (ggeneral_buffer[gstate_var.UART_Rx_ndx] << 8) + ggeneral_buffer[gstate_var.UART_Rx_ndx-1];

        if (gstate_var.UART_Rx_ndx > 3)
        {
            buf_ndx = gstate_var.UART_Rx_ndx - 3;
            if (buf_ndx == gstate_var.Rx_payload_len)  //SJ2210812 - It's time to stop reading.
                gstate_var.UART_Rx_State = UART_PROCESSING;
        }

        gstate_var.UART_Rx_ndx++;
      }
      else //SJ3281112 - Buffer overflow
      {
        gstate_var.UART_Rx_State = UART_STANDBY;
      }
    }
}  //SJ4221112 - End of ISR_UART_RX_A0()



//SJ4120712 - With current timer setting, each second, this ISR will get in about 20 times.
//SJ4120712 - 50ms per interrupt.
#if 1
#pragma vector=TIMER0_A0_VECTOR
__interrupt void ISR_Timer0_A0(void)
#else
#pragma vector=TIMER1_A0_VECTOR
__interrupt void ISR_Timer1_A2(void)
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

    if (gstate_var.led_timer)
        gstate_var.led_timer--;

    if (gstate_var.error_delay)
        gstate_var.error_delay--;

    //SJ3191212 - Not used so far. KIV
    //if (gstate_var.flash_update_countdown)
    //    gstate_var.flash_update_countdown--;

  #if 0  //SJ2210812 - 
    //SJ4190712 - Time to indicate end of data
    if (gstate_var.UART_Rx_timeout)
        gstate_var.UART_Rx_timeout--;
    else
    {
        if (gstate_var.UART_Rx_State == UART_RECEIVING)
            gstate_var.UART_Rx_State = UART_PROCESSING;
    }
  #endif

    if (gstate_var.UART_Rx_duration)
        gstate_var.UART_Rx_duration--;

  #if 1  //SJ4291112 - Solving out of sync state
    if (gstate_var.UART_Rx_threshold)
        gstate_var.UART_Rx_threshold--;
    else if (gstate_var.UART_Rx_State == UART_RECEIVING)
        gstate_var.UART_Rx_State = UART_STANDBY;  //SJ4291112 - Taking too long to complete reading.
  #endif

  #ifdef RED_N_GREEN_LED  // 1
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

  #if 0  //SJ4041012 - Testing broadcast from service node
    if (gstate_var.broadcast_delay)
        gstate_var.broadcast_delay--;
  #endif


    //SJ4120712 - Shift away element 0 from voltage table
//SJ1270812    for (ndx=0; ndx<CURRENT_SAMPLE-1; ndx++)  //SJ2131211 - Can replace with memmove(*ptr, *ptr2, num); prototype in "string.h"
//SJ1270812        gcurrent_tab[ndx] = gcurrent_tab[ndx+1];

    //SJ2131211 - The above for loop can be replaced with below memmove function - "string.h"
    //memmove(&gvoltage_tab[0], &gvoltage_tab[1], (VOLTAGE_SAMPLE - 1));

    //SJ4120712 - Select channel A6 for LED current
    //ADC10CTL1 = INCH_6 + ADC10DIV_3;  //SJ4120712 - Use ADC10 internal clock
    //ADC10CTL1 = INCH_6 + ADC10DIV_3 + ADC10SSEL_3;  //SJ4120712 - SMCLK=8MHz, ADC10DIV_3=4; 8 / 4 = 2MHz
//SJ1270812    ADC10CTL1 = INCH_6 + ADC10DIV_7 + ADC10SSEL_3;  //SJ4120712 - SMCLK=8MHz, ADC10DIV_7=8; 8 / 8 = 1MHz
    //gstate_var.LED_current = RetrieveFromADC(ADC_BINARY);
//SJ1270812    gcurrent_tab[CURRENT_SAMPLE-1] = RetrieveFromADC(ADC_BINARY);  //SJ1031011 - New data is added to the 9th element of the table

    //SJ4120712 - Shift away element 0 from current table
//SJ1270812    for (ndx=0; ndx<VOLTAGE_SAMPLE-1; ndx++)
//SJ1270812        gvoltage_tab[ndx] = gvoltage_tab[ndx+1];
    //SJ2131211 - The above for loop can be replaced with below memmove function - "string.h"
    //memmove(&gcurrent_tab[0], &gcurrent_tab[1], (CURRENT_SAMPLE - 1));

    //SJ4120712 - Select  Channel A7  for LED voltage
    //ADC10CTL1 = INCH_7 + ADC10DIV_3;  //SJ4120712 - Use ADC10 internal clock
    //ADC10CTL1 = INCH_7 + ADC10DIV_3 + ADC10SSEL_3;  //SJ4120712 - SMCLK=8MHz, ADC10DIV_3=4; 8 / 4 = 2MHz
//SJ1270812    ADC10CTL1 = INCH_7 + ADC10DIV_7 + ADC10SSEL_3;  //SJ4120712 - SMCLK=8MHz, ADC10DIV_7=8; 8 / 8 = 1MHz
    //gstate_var.LED_voltage = RetrieveFromADC(ADC_BINARY);
//SJ1270812    gvoltage_tab[VOLTAGE_SAMPLE-1] = RetrieveFromADC(ADC_BINARY);  //SJ4120712 - New data is added to the 9th element of the table

//SJ3121212 - Not used in iSwitch project
#if 0  //SJ2180912 - Sensor signal

    //temp_signal = (P2IN & 0x20) ? 1 : 0;    //SJ2180912 - (00100000)  //SJ2271112 - iLight project
    temp_signal = (P1IN & 0x01);    //SJ2271112 - Use in Micro lite project, pin 1.0 (00000001)

    //SJ2180912 - sensor_signal is 4 bytes (32 bits), ISR frequency is 50msec, as such, it takes 1600msec to complete 32 bits.
    gstate_var.sensor_signal = (gstate_var.sensor_signal << 1) | (temp_signal);

    if (gstate_var.sensor_signal == 0xFFFFFFFF)
        gstate_var.sensor_state = ON_STATE;
    else
        gstate_var.sensor_state = OFF_STATE;

#endif

//SJ1270812    gstate_var.general_ctrl_flag |= NEW_DATA_ON;

    //SJ1110411 - Re-active timer before leaving ISR.
    //TA1CCR0 = TIMER_CONST;  //SJ4140110 - Initialise compare register with value.
    TACCR0 = TIMER_CONST;  //SJ1110411 - Initialise compare register with value.

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



