/******************************************************************************/
// DALI RGB-LED Demo using MSP430F2131 and TPS62260 from Texas Instruments   
// The MSP430F2131 IO does not support the DALI logic levels and a level     
// translation circuit is required.                                          
//                                                                            
// LED PWM Description:                                                      
// 8Mhz clock with a period of 5000 counts results in a frequency of 1.6Khz. 
// Minimum Physical Level setting for the TPS62260 EVM is 90                          
//                                                                            
// DALI:                                                                      
// TI_DALI_Init()                                                            
// This function restores the previous DALI settings stored in information
// memory and intializes the WDT+, RAM varaibles, and GPIO.  The WDT+ is used
// to update the actual_level based upon the DALI fade_rate or fade_time.                                    
//                                                                           
// TI_DALI_Transaction_Loop()                                                     
// This function continually receives, decodes, and then acts upon DALI      
// commands.                                                                 
//                                                                            
// TI_DALI_Update_Callback()                                                 
// The LED source continues without CPU intervention until the source needs  
// to be updated. In this case the source is the TIMER_A3, outputs A1 and A2. 
//                                                                           
// TI_DALI_Idle_Callback()                                                   
// The time between two transactions (forward-forward or forward-backward)   
// is limited in the standard to 9.17ms.  After the CPU has processed the    
// recevied information this leaves only about 7ms to do other processes     
// before the CPU must receive or transmit the next DALI transaction.         
//                                                                           
// TI_DALI_Flash_Update(FWKEY)                                               
// This takes all of the non-voltaile variables defined in the standard and  
// writes them into a free portion of one of the info memory segments.       
// This function is intended to be called only once before or during a power 
// loss event.  The write cycles takes approximately 2.5ms and Vcc must be   
// greater than 2.2V during the write.                                       
//                                                                            
// Chris Sterzik                                                               
// Texas Instruments                                                         
// Built with CCS Version: 4.0.1.01000                                       
//                                                                            
// Revision 1.0    Baseline                                         
/******************************************************************************/

// THIS PROGRAM IS PROVIDED "AS IS". TI MAKES NO WARRANTIES OR
// REPRESENTATIONS, EITHER EXPRESS, IMPLIED OR STATUTORY,
// INCLUDING ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
// FOR A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR
// COMPLETENESS OF RESPONSES, RESULTS AND LACK OF NEGLIGENCE.
// TI DISCLAIMS ANY WARRANTY OF TITLE, QUIET ENJOYMENT, QUIET
// POSSESSION, AND NON-INFRINGEMENT OF ANY THIRD PARTY
// INTELLECTUAL PROPERTY RIGHTS WITH REGARD TO THE PROGRAM OR
// YOUR USE OF THE PROGRAM.
//
// IN NO EVENT SHALL TI BE LIABLE FOR ANY SPECIAL, INCIDENTAL,
// CONSEQUENTIAL OR INDIRECT DAMAGES, HOWEVER CAUSED, ON ANY
// THEORY OF LIABILITY AND WHETHER OR NOT TI HAS BEEN ADVISED
// OF THE POSSIBILITY OF SUCH DAMAGES, ARISING IN ANY WAY OUT
// OF THIS AGREEMENT, THE PROGRAM, OR YOUR USE OF THE PROGRAM.
// EXCLUDED DAMAGES INCLUDE, BUT ARE NOT LIMITED TO, COST OF
// REMOVAL OR REINSTALLATION, COMPUTER TIME, LABOR COSTS, LOSS
// OF GOODWILL, LOSS OF PROFITS, LOSS OF SAVINGS, OR LOSS OF
// USE OR INTERRUPTION OF BUSINESS. IN NO EVENT WILL TI'S
// AGGREGATE LIABILITY UNDER THIS AGREEMENT OR ARISING OUT OF
// YOUR USE OF THE PROGRAM EXCEED FIVE HUNDRED DOLLARS
// (U.S.$500).
//
// Unless otherwise stated, the Program written and copyrighted
// by Texas Instruments is distributed as "freeware".  You may,
// only under TI's copyright in the Program, use and modify the
// Program without any charge or restriction.  You may
// distribute to third parties, provided that you transfer a
// copy of this license to the third party and the third party
// agrees to these terms by its first use of the Program. You
// must reproduce the copyright notice and any other legend of
// ownership on each copy or partial copy, of the Program.
//
// You acknowledge and agree that the Program contains
// copyrighted material, trade secrets and other TI proprietary
// information and is protected by copyright laws,
// international copyright treaties, and trade secret laws, as
// well as other intellectual property laws.  To protect TI's
// rights in the Program, you agree not to decompile, reverse
// engineer, disassemble or otherwise translate any object code
// versions of the Program to a human-readable form.  You agree
// that in no event will you alter, remove or destroy any
// copyright notice included in the Program.  TI reserves all
// rights not specifically granted under this license. Except
// as specifically provided herein, nothing in this agreement
// shall be construed as conferring by implication, estoppel,
// or otherwise, upon you, any license or other right under any
// TI patents, copyrights or trade secrets.
/******************************************************************************/
//#include "msp430x21x1.h"  //SJ3080812 - ORG. Compilation issue for g2553
#include "msp430g2553.h"  //SJ3080812 - 
#include "dali.h"
#include "global_var.h"  //SJ1130812 - 
//=== SJ1130812 - External global var =========================================
//main.c
extern STATE_STRUCT  gstate_var;

void Update_Callback(void);
void DALI_Idle_Callback(void);

/*********************LED Pointer Table ***************************************/
#if 1  //SJ2140812 - ORG
const unsigned int LED[255]={0,5,5,5,5,6,6,6,6,6,6,7,7,7,7,7,7,8,8,8,8,9,9,9,9,
10,10,10,10,11,11,11,12,12,12,13,13,13,14,14,14,15,15,16,16,17,17,17,18,18,19,
20,20,21,21,22,22,23,24,24,25,26,26,27,28,29,29,30,31,32,33,34,35,36,37,38,39,
40,41,42,43,44,46,47,48,49,51,52,54,55,57,58,60,61,63,65,67,69,70,72,74,76,79,
81,83,85,88,90,93,95,98,100,103,106,109,112,115,118,122,125,128,132,136,139,
143,147,151,156,160,164,169,173,178,183,188,193,199,204,210,216,222,228,234,
241,247,254,261,269,276,284,291,300,308,316,325,334,343,353,363,373,383,394,
404,416,427,439,451,464,476,490,503,517,531,546,561,577,593,609,626,643,661,
679,698,718,737,758,779,800,823,845,869,893,917,943,969,996,1023,1052,1081,
1111,1141,1173,1205,1239,1273,1308,1345,1382,1420,1459,1500,1541,1584,1628,
1673,1719,1767,1816,1866,1918,1971,2025,2081,2139,2198,2259,2321,2386,2452,
2519,2589,2661,2735,2810,2888,2968,3050,3135,3221,3311,3402,3496,3593,3693,
3795,3900,4008,4119,4233,4350,4470,4594,4721,4852,4986
}; //LED
#else  //SJ2140812 - Replaced.

//SJ3150812 - Represent 0=0%, 127=50%, 254=100% 
// 0     1     2     3     4     5     6     7     8     9    10    11    12    13    14    15    16    17    18    19
const unsigned int LED[255]=
{  0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0, 
   0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0, 
   0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0, 
   0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0, 
   0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0, 
   0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0, 
   0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0, 
   0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0, 
   0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0, 
   0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0, 
   0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0, 
   0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0, 
   0,    0,    0,    0,    0,    0, 3000, 4000, 4500, 4600, 4700, 4750, 4850, 4950, 5000
}; //LED

#endif
/******************************************************************************/

//SJ4050112 - MCU: MSP430F2131

// Main Program
//void main(void)  //SJ1060812 - ORG
void Dali_HPA338_main(void)
{
  unsigned int temp_flash_key;
 #if 0  //SJ1060812 - ORG
  WDTCTL=WDTPW+WDTHOLD;      // disable Watchdog
  DCOCTL = 0;
  BCSCTL1= CALBC1_8MHZ;      // System Clock Settings
  DCOCTL = CALDCO_8MHZ;      // use calibrated 8MHz settings
                             // This should be coordinated with the dali.h file
   
  /*** Port Initialization ****************************************************/
  P1OUT = 0x02;     // P1.0 is output (Enable for TPS62260), 
                    // P1.1 is high turning off LED3
  P1DIR = 0xFF;     // P1.4, P1.5, P1.6, P1.7 are not used => digital outputs
  P1SEL = 0x0C;     // P1.2 (TA1), P1.3 (TA2) are used as PWM Timer Outputs
  /****************************************************************************/
  temp_flash_key = FWKEY;         //
  TI_DALI_Init(temp_flash_key);   // Initialize WDT for DALI fade implementation
  temp_flash_key = 0;

  /*** PWM Timer Initialization ***********************************************/       
  P1OUT &= ~BIT0;                     // Turn off LEDs
  TACTL = TASSEL_2+ID_0+MC_0+TACLR;   // Timer clock = SMCLK = 8MHz
  TACCR0=5000;    // Define 1.6Khz frequency 
  TACCR1=LED[power_on_level];         // This power level will be overwritten
  TACCR2=LED[power_on_level];         // by the reception of a power command 
                                      // within 0.6 seconds.  After this time
                                      // the device will automatically go to
                                      // the power on level if no command is
                                      // received.
  TACCTL1 = CM_0+CCIS_2+OUTMOD_3;     // Set on TACCRx, Reset on TACCR0
  TACCTL2 = CM_0+CCIS_2+OUTMOD_3;
  /****************************************************************************/ 
  
  TACTL |= MC_1;  // start Timer_A (up mode)
  __enable_interrupt(); // enable maskable interrupts

  TI_DALI_Transaction_Loop();
  /* should never return */
  while(1);
 #else  //SJ1060812 - Replaced

  temp_flash_key = FWKEY;         //
  TI_DALI_Init(temp_flash_key);   // Initialize WDT for DALI fade implementation
  temp_flash_key = 0;

  TA1CCR0 = 5000;  //SJ3080812 - 5000 is defined as PWM_PERIOD macro in global_var.h 
  TA1CCR1 = LED[power_on_level];  //SJ1130812 - Official
//  TA1CCR1 = 500;  //SJ1130812 - Debugging
  TA1CTL |= MC_1;
 #endif  //SJ1060812 - #if 0 ... #else ... #endif

} //End Main

void TI_DALI_Update_Callback()
{
  //TACCTL1 |= CCIE;  //SJ3080812 - ORG. from TI DALI
  TA1CCTL1 |= CCIE;  //SJ3080812 - Replaced
}

void TI_DALI_Idle_Callback(void)
{
  __no_operation();
}

/******************************************************************************/
// Timer_A Interrupt Service Routine:       
/******************************************************************************/
#if 0  //SJ3080812 - ORG
#pragma vector=TIMERA1_VECTOR
#else  //SJ3080812 - Replaced
#pragma vector=TIMER1_A1_VECTOR
#endif
__interrupt void ISR_TimerA(void)
{
  if(actual_level==OFF)
  {
  	/* turn off LEDs */
  	//P1OUT &= ~BIT0;  //SJ3080812 - ORG
    //P2OUT &= ~BIT2;  //SJ3080812 - Replaced
    P2OUT |= BIT2;  //SJ3080812 - Replaced
    TA1CCR1 = 0;  //SJ2140812 - Added by SJ
    //SJ3080812 - PWM is 0, turn off AC P2.3
    //gstate_var.general_delay = 5;
    //while (gstate_var.general_delay) ;
    __no_operation();
    __no_operation();
    __no_operation();
    P2OUT &= 0xF7;  //SJ1130812 - AC_CNTRL off, P2.3 (11110111)
  }
  else 
  {
  	//--- PWM signal generation
  #if 0  //SJ3080812 - ORG. TI DALI
  	P1OUT |= BIT0;
  	TACCR1=LED[actual_level]; //   //SJ4160611 - Green LED
  	TACCR2=LED[actual_level]; //   //SJ4160611 - Red LED
  #else //SJ3080812 - Replaced

    //SJ3080812 - PWM is not 0, turn on AC P2.3
    P2OUT |= 0x08;//0xFF;  //SJ1130812 - AC_CNTRL on, P2.3 (xxxx1xxx)
    //gstate_var.general_delay = 5;
    //while (gstate_var.general_delay) ;
    __no_operation();
    __no_operation();
    __no_operation();

    //P2OUT |= BIT2;
    P2OUT &= ~BIT2;
    TA1CCR1 = LED[actual_level];  //SJ1130812 - Official
    //TA1CCR1 = 4500;  //SJ1130812 - Debuggin
  #endif
  }
  //TACCTL1 &= ~CCIE;  //SJ3080812 - ORG
  TA1CCTL1 &= ~CCIE;  //SJ3080812 - Replaced
}
//#endif
/******************************************************************************/
// Power loss event, save settings
/******************************************************************************/
//  if(flash_update_request)
//  {
//    TI_DALI_Flash_Update(FWKEY);
//    WDTCTL = 0x05;  // illegal write to watch dog causes reset
//  }
