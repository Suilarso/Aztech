
/***********************************************************************/
/*                                                                     */
/*  FILE        : PLC_iSwitch.c                                        */
/*  DATE        : Mon, December 03, 2012                               */
/*  DESCRIPTION : Main Program                                         */
/*  CPU TYPE    : MSP430G2553                                          */
/*  PROJECT     : PLC Switch                                           */
/*  AUTHOR      : Suilarso Japit                                       */
/*  VERSION     : 90?.001.1-000 (Should be 909)                        */
/*  HISTORY     :                                                      */
/*    SJ1031212 - Start software development activity.                 */
/*    SJ4061212 - Modify code; Remove network initialisation;          */
/*                DIP switch is working and able to control light.     */
/*    SJ5071212 - Experiment random generator using current or voltage */
/*                data input from ADC channel.                         */
/*    SJ2181212 - Make a backup under scarlet prototype folder.        */
/*    SJ3191212 - Implement codes to allow toggle function of a same   */
/*                button being pressed twice.                          */
/*    SJ5211212 - Write codes to implement master on/off key.          */
/*    SJ5040113 - Implement system reset when command 0x20 is          */
/*                received. short_address=0, Room_ID=0, scene[]=0xFF,  */
/*                and gswitch_open[]=0xFF before writing to flash.     */
/*                Then issue watchdog to initiate system reset.        */
/*    SJ40112?? - Change solar monitoring timer to 10 sec from 1 sec.  */
/*    SJ11212?? - Change 0.5V value representation to 165 from 365.    */
/*    SJ11203?? - Change CONST_0_24_VOLT value from 75 to 200.         */
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
//#include   <msp430x23x2.h>    //"msp430x21x1.h"
#include   <msp430G2553.h>
#include   <stdlib.h>  //SJ5301112 - Need rand() and srand(seed) prototype.
//#include   <stdio.h>
#include   <string.h>

//#include   "gp_io.h"
#include   "PLC_Comm.h"
#include   "global_var.h"
#ifdef FLASH_COMPONENT
#include   "flash_module.h"
#endif


#define PRODUCT_CODE  (90?)
#define CUST_ID_CODE  (001)
#define RELEASE_CODE  (1)
#define VERSION_CODE  (000)


#define DATA_BUFFER_LEN     10


//=== SJ5060712 - External variables ==========================================
//I2C_routine.c

//dali.c
extern const unsigned char version_number;
extern const unsigned char device_type;
extern const unsigned char phys_min_lvl;

extern unsigned char power_on_level;
extern unsigned char actual_level;
extern unsigned char random_address_byte_h;
extern unsigned char random_address_byte_m;
extern unsigned char random_address_byte_l;
extern unsigned char system_failure_level;
extern unsigned char min_level;
extern unsigned char max_level;
extern unsigned char fade_rate;
extern unsigned char fade_time;
extern unsigned char short_address;
extern unsigned char group_0_7;
extern unsigned char group_8_f;
extern unsigned char scene[16];
extern unsigned char status_information;
extern unsigned char data_transfer_register;
extern unsigned char data_transfer_register1;
extern unsigned char data_transfer_register2;


extern unsigned char flash_update_request;


//TAS3103_util.c
//=== SJ5060712 - END. External variables =====================================

//=== SJ5060712 - Functions prototype =========================================
int RetrieveFromADC(int data_format);//(void);
void AZ_Transmit_Routine(int data_type);  //SJ4221112 - Testing transmit
void AZ_Randomised_Long_Address(void);  //SJ5071212 - Generate random long address
//void UART_Rx_Acknowledgement(int validity_flag);  //SJ3010812 - 
//SJ1171212 - DO NOT REMOVE below prototype; maybe needed in non-iSwitch project
//unsigned char AZ_Acknowledge_Basenode_Request(unsigned char Message_Type);  //SJ4300812 - Response to basenode request
//void AZ_DALI_Response(int validity_flag);  //SJ3220812 - ORG. Point-to-Point
void AZ_DALI_Response(int response_type);  //SJ3220812 - Replaced. Network type
//void PWM_Selftest(void);  //SJ3080812 - 
//void PWM_Selftest_2(void);  //SJ5100812 - 
//__interrupt void ISR_Port1(void);  //SJ3150812 - Selftest
//=== SJ5060712 - END. Functions prototype ====================================


//=== SJ5060712 - External function prototype =================================
#ifdef FLASH_COMPONENT
//flash_module.c
extern void Flash_Write(unsigned int start_addr, short amount_byte, unsigned char *buffer);
#endif  //#ifdef FLASH_COMPONENT ... #endif

//PLC_Comm.c
extern unsigned char PLC_System_Init(int init_step);
extern unsigned char PLC_Network_Config(int init_step);
extern void CRC16_UpdateChecksum(unsigned short *pcrcvalue, const void *data, int length);  //SJ4130912 - 

//ISR_Routines.c
//void QuickDelay(unsigned int interval);

//Dali_HPA338.c
extern void Dali_HPA338_main(void);

//dali.c
extern void TI_DALI_Transaction_Loop(void);  //SJ4090812 - 
extern unsigned char TI_DALI_Match_Address(unsigned char address);
//extern void TI_DALI_Flash_Update(unsigned int key);
extern void TI_DALI_Flash_Update(unsigned int key, unsigned char wrt_area);  //SJ1171212 - Replaced
//=== SJ5060712 - END. External functions prototype ===========================


//=== SJ5060712 - Global variable and constant declaration ====================
unsigned char ggeneral_buffer[DATA_BUFFER_SZ];  //[50];
#ifdef SWITCH_CONTROLLER
unsigned char gswitch_oper[SWITCH_OPER_SZ];  //SJ5141212 - equiv 64
#endif

//SJ3211112 - These two variables are defined for DIP switch
unsigned int DIP_switch_val;
//unsigned char sensor_mode;
//unsigned char PWM_value;

#if 0  //SJ1270812
//SJ1031011 - This table contains 10 running data for battary voltage
int gvoltage_tab[VOLTAGE_SAMPLE] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

//SJ1031011 - This table contains 10 running data for solar voltage
int gcurrent_tab[CURRENT_SAMPLE] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
#endif

STATE_STRUCT  gstate_var;
//=== SJ5060712 - END. Global variables declaration ===========================



//SJ5060712 - MCU detail:
//MSP430G2553 (RISC 16 bits)
//Flash memory size: 16KB + 256B
//Flash memory consists of main memory (8 segments of 512B), and information memory (4 segments of 64B)
//Segment Info_D = 0x1000 - 0x1039
//Segment Info_C = 0x1040 - 0x1079
//Segment Info_B = 0x1080 - 0x10BF
//Segment Info_A = 0x10C0 - 0x10FF  //SJ5141212 - Care must be exercised for using this segment
//RAM size: 512B
//Internal Frequencies up to 16 MHz
//Default master clock and sub-main clock is at 1.1 Mhz upon system power up.
//One timer: Timer A with three capture /compare registers.

//SJ1110110 - Short is 2 bytes long
//SJ1110110 - int is 2 bytes long
//SJ1110110 - long is 4 bytes long

//SJ5060712 Main routine
void main(void)
{
    //unsigned char data_buffer[DATA_BUFFER_LEN+1];  //SJ4061011 - equiv 10 + 1 = 11
    //unsigned char ndx;
    unsigned char proceed_flag;

//SJ1270812    unsigned int data_len=0;
    //unsigned short temp_bit;  //SJ4221112 - Used for bit manipulation
    int init_step=SYSTEM_INIT_STEP_1;  //SJ3180712 - May skip the initialisation here
    int trial=0;  //SJ3180712 - May skip the initialisation here
    //int current_trial, current_count;

    //int *PtrIntFlash;
    //int timer_const;
    //int i, j, k;

  #ifndef SWITCH_CONTROLLER
    unsigned int current_trial, current_count;
  #endif

    WDTCTL = WDTPW + WDTHOLD;       // disable Watchdog

    //SJ4131005 - Below two settings are used to manipulate with timer clock frequency.  
    //SJ5060712 - If timer frequency is changed here, remember to change the ADC10 as well.
    //SJ5060712 - Set clock to 8MHz.
    DCOCTL = CALDCO_8MHZ;   //SJ5300911 - CALDCO_1MHZ;
    BCSCTL1 = CALBC1_8MHZ;  //SJ5300911 - CALBC1_1MHZ;
    //DCOCTL = CALDCO_1MHZ;   //SJ1141111 - Set to 1 MHz  //CALDCO_8MHZ;
    //BCSCTL1 = CALBC1_1MHZ;  //SJ1141111 - Set to 1 MHz  //CALBC1_8MHZ;

  //SJ1060812 - Below setting is irrelevant when use TI DALI code
  #if 0//def  WITH_WDT
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

    //SJ3110712 - Port 1 initialisation.
    P1SEL = 0x06;  //SJ3110712 - (00000110) Pin 1(Rx), and 2(Tx) - UART Peripheral
    P1SEL2 = 0x06;  //SJ3110712 - (00000110) Pin 1, and 2 - UART Peripheral  //SJ1171212 - Without this sttmt UART will not work. WHY???
    P1DIR = 0x28;  //SJ3110712 - (00101000)
    //P1IN = 0x00;
  #ifdef RED_N_GREEN_LED
    P1OUT = 0x08;  //SJ1081012 - Turn on red LED (00001000)
  #endif
    //P1IE  = 0x80;  //SJ2120411 - Pin 7 is interrupt enabled (10000000)
    //P1IES = 0x80;  //SJ2120411 - Pin 7 is set to 1 to set falling edge (10000000)

  #if 0  //SJ5051012 - ORG.
    //SJ3110712 - Port 2 initialisation
    P2SEL = 0x04;  //SJ3110712 (00000100) Pin 2 - PWM, The rest of the pins as GPIO.
    //P2SEL2 = 0x00;
    P2DIR = 0xDF;//0xFF;//0xFB;  //SJ3110712 - (11011111) Set according to spec recommendation.
    P2OUT = 0x07;//0x0F;  //SJ3110712 - (00000111)
    //P2IE = 0x04;
    //P2IES = 0x04;
  #else  //SJ5051012 - For new version.
    //SJ5051012 - Port 2 initialisation
    P2SEL = 0x04;  //SJ5051012 (00000100) Pin 2 - PWM, The rest of the pins as GPIO.
    //P2SEL2 = 0x00;  //SJ5071212 - DO NOT USE this statement
    P2DIR = 0x0F;  //SJ5051012 - (00001111)
    //P2IN = 0xF0;  //SJ5051012 - (11110000) PCB with DIP switch  //SJ1261112 - SJTODO
//    P2OUT = 0x07;//0x0F;  //SJ5051012 - (00000111)
    //P2IE = 0x04;
    //P2IES = 0x04;
    P2REN = 0xF0;  //SJ1081012 - Enable internal pullup for P2.4, P2.5, P2.6, and P2.7 (11110000)
//SJTEST    P2OUT = 0x07;//0x0F;  //SJ5051012 - (00000111)  //SJ5071212 - Why place here?????
    P2OUT = 0xF7;  //SJ3160113 - (11110111). Required because of internal pullup enabled.

    //SJ5051012 - Port 3 initialisation
    P3SEL = 0x00;  //SJ5051012 - (00000000) All pins as GPIO.
    //P3SEL2 = 0x00;
    P3DIR = 0x00;  //SJ5051012 - (00000000) All set as input
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

    //SJ3110712 - Timer0_A2 control register initialisation.
    //SJ3110712 - Timer0_A2 is used as timer interval for ISR general routine.
    //SJ3110712 - I think this can be achieve by using Timer1_A2; share with PWM timer.
    //TACTL = TASSEL_2 + TACLR;  //SJ3110712 - SMCLK = 1MHz; ID_0 ==> 1MHz / 1 = 1MHz
    TACTL = TASSEL_2 + ID_3 + TACLR;  //SJ3110712 - SMCLK = 8MHz; ID_3 ==> 8MHz / 8 = 1MHz
    //SJ3110712 - Initialise Capture/Compare control register.
    //SJ3110712 - Capture/Compare block 0 is used as compare mode for timer.
    TACCTL0 = CCIE;

    //SJ3110712 - Timer1_A3 control register initialisation.
    //SJ3110712 - Timer1_A1 (Capture/Compare block 1) is used to generate PWM.
    //TACTL = TASSEL_2+ID_0+MC_0+TACLR;   // Timer clock = SMCLK = 8MHz  //SJ1130812 - Reference from TI DALI code
    //TA1CTL = TASSEL_2 + TACLR;  //SJ3110712 - SMCLK = 1MHz; ID_0 ==> 1Mhz / 1 = 1MHz
    TA1CTL = TASSEL_2 + ID_3 + TACLR;  //SJ3110712 - SMCLK = 8MHz; ID_3 ==> 8MHz / 8 = 1MHz
    //SJ3110712 - Initialise Capture/Compare control register.
    //SJ3110712 - Possible setting for outmode:
    //SJ3110712 - OUTMOD_1;  //SJ3110712 - Set
    //SJ3110712 - OUTMOD_2;  //SJ3110712 - Toggle/reset
    //SJ3110712 - OUTMOD_3;  //SJ3110712 - Set/reset
    //SJ3110712 - OUTMOD_4;  //SJ3110712 - Toggle
    //SJ3110712 - OUTMOD_5;  //SJ3110712 - Reset
    //SJ3110712 - OUTMOD_6;  //SJ3110712 - Toggle/set
    //SJ3110712 - OUTMOD_7;  //SJ3110712 - Reset/set
    TA1CCTL1 |= OUTMOD_3;//OUTMOD_6;  //SJ3110712 - Toggle/set
  #if 0  //SJ3110712 - DO NOT REMOVE. If below works, we can save 1 timer
    //SJ3110712 - Timer1 block 2 as timer interval for ISR
    TA1CCTL2 = CCIE;
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

    //UCA0CTL0;  //SJ2100712 - No need to set; all default value meet our requirement. For future use, may consider UCMODE2.
    UCA0CTL1 = UCSWRST;  //SJ2100712 - Step 1
    UCA0CTL1 |= UCSSEL1; //SJ2100712 - Step 2 
    UCA0BR0 = 0x08;        //SJ2100712 - Refer to Tab 15-5 of MSP430 user manual.
    UCA0BR1 = 0x00;        //SJ2100712 - Refer to Tab 15-5 of MSP430 user manual.
    UCA0MCTL = 0xB0+UCOS16;//SJ2100712 - Refer to Tab 15-5 of MSP430 user manual. UCBRFx=11+UCBRSx=0+UCOS16
    //SJ2010610 - Step 3 was done above in the port initialisation
    UCA0CTL1 &= ~UCSWRST;  //SJ2100712 - Step 4
//SJ4221112 - Below sttmt is commentted out for debugging purpose. Uncomment it after use
    UC0IE |= UCA0RXIE;  //SJ2100712 - Step 5 (Set on rx interrupt in IE2 register)  //SJ3110712 - Check if this sttmt can be here??

    //SJ3110712 ADC initialisation. -----------------------------------------

    //SJ3110712 - ADC10 control register initialisation.
    ADC10AE0 |= 0xC0;  //SJ3110712 - Assign P1.6 (A6), and P1.7 (A7) as analog input to ADC. (11000000)
    //ADC10CTL0 = SREF_1 + ADC10SHT_3 + REF2_5V + REFON + ADC10ON;  //SJ4210110 - For testing temperature.
    //ADC10CTL1 = INCH_0 + ADC10DIV_3;            // Repeat single channel, A0
    //ADC10SA = 0x200;   //SJ3130110 - This reg contains ram addrs for data transfer.
    //ADC10DTC1 = 0x20;  //32 conversions  //SJ3130110 - I believe this reg is used for multiple conversion.

    _BIS_SR(GIE);  //SJ1130812 - Enable interrupt. equiv __enable_interrupt();?????
//SJ3110712 - End of MSP430 Registers Initialisation. ---------------------------------------------

//SJ3110712 - Initialize global var. --------------------------------------------------------------
    //gstate_var.PLC_status = SJ_FAIL;
    gstate_var.general_ctrl_flag = 0;
    gstate_var.general_delay = TIMER_2_SECONDS;  //SJ1040411 - Renamed
    gstate_var.error_delay = 0;  //SJ2110912 - 
    gstate_var.UART_Rx_ndx = 0;
    gstate_var.UART_Rx_State = UART_INVALID;
    //gstate_var.UART_Rx_timeout = 0; //MESSAGE_HEADER_LEN;  //SJ3180712 - Min. bytes read are 8 bytes of header
    gstate_var.DALI_response = 0;  //SJ1170912 - 
    //gstate_var.sensor_state = OFF_STATE;  //SJ3121212 - Not used in iSwitch project
  #ifdef RED_N_GREEN_LED
    //gstate_var.red_led_timer = 0;
    //gstate_var.green_led_timer = 0;
    gstate_var.led_blink_freq = 0;  //SJ3191212 - Replaced above two sttmts
  #endif
  #if 0  //SJ4041012 - Testing broadcast from service node
    gstate_var.broadcast_mode = 0x00;
    gstate_var.broadcast_delay = TIMER_20_SECONDS;
  #endif
    gstate_var.led_timer = 0;
    //gstate_var.flash_update_countdown = 0;  //SJ2110912 - Countdown to flash update  //SJ3191212 - KIV
    gstate_var.UART_Rx_threshold = 0;  //SJ4291112 - Solving out of sync state
    gstate_var.UART_Rx_duration = 0;
    gstate_var.Rx_payload_len = 0;  //SJ2170712 - SJNOTE: May not be needed
    //gstate_var.PAN_ID = 0x0000;   //SJ3191212 - Not used so far. DO NOT REMOVE
    //gstate_var.coord_addr = 0x0000;  //SJ4260712 - DO NOT REMOVE
    //gstate_var.network_addr = 0x0000;  //SJ3191212 - Can be removed and replaced with local var.
    gstate_var.Room_ID = 0x0000;  //SJ1101212 - Room ID
    gstate_var.prev_key = 0;

    //gstate_var.cum_current = 0;//CONST_1_44_VOLT * CURRENT_SAMPLE;
    gstate_var.LED_current = 0;
    gstate_var.LED_voltage = 0;
    //gstate_var.sensor_signal = 0;  //SJ3121212 - Not used in iSwitch project
    //gstate_var.device_state_flag.com_field.BYTE = 0;  //SJ3200411 - Added
    //gstate_var.GEN_ctrl_flag.com_field.BYTE = 0;
//SJ3110712 - End of global var initialisation ----------------------------------------------------

//SJ3110712 - Access to flash memory --------------------------------------------------------------
//SJ3110712 - End of Access to flash memory -------------------------------------------------------

    //PtrIntFlash = (int *) 0x10C0;
    //timer_const = (int) (*PtrIntFlash);

//SJ4090812 - Start timer -------------------------------------------------------------------------
    TACCR0 = TIMER_CONST;  //SJ1110411 - Init compare reg with value.  //SJ1110411 - 0xC350 = 50000 base 10
    TACTL = TACTL | MC_1;  //SJ1110411 - Start timer in up mode.
    //TA1CCR2 = TIMER_CONST;  //SJ3110712 - DO NOT REMOVE.
    //--- SJ3110712 - Timer is active beyond this point ---------------------------------

#if 0  //SJ1191112 - Re-instate //SJ5310812 - Skip this as they are done in DALI initialisation.
    P2OUT |= 0x08;  //SJ3281112 - Purely for debugging only
    __no_operation();
    __no_operation();
    __no_operation();
    PWM_PERIOD_REG = PWM_PERIOD;  //SJ1191112 - PWM_PERIOD = 5000  //SJ3281112 - TA1CCR0
    DUTY_CYCLE_REG = 5000;//PWM_0_PERCENT;  //SJ1191112 - Maybe need to refer to array. For reference see Dali_HPA338_main() - TA1CCR1
    //SJ2020210 - MC_1 - Up mode, MC_2 - Continuous mode, MC_3 - Up/Down mode
    TA1CTL |= MC_1;  //SJ3110712 - Up
    //--- SJ3110712 - PWM are active beyond this point ----------------------------------
#endif

    //--- SJ4290710 - This code is to introduce delay to allow MCU2 to stabalise ---
    while (gstate_var.general_delay) ;

    //SJ1081012 - Turn on red LED here
    //P1OUT |= ON_RED_LED;  //SJ1081012 - Done during port initialisation

    Dali_HPA338_main();

#if 0
while (1)
{
    if (P3IN != 0xFF && gstate_var.led_timer == 0 && device_type >= 11)  //SJ4061212 - means one of the switch is pressed
    {
          //DIP_switch_val = 0xFF - (P3IN & 0xFF);  //SJ5071212
          DIP_switch_val = ~(P3IN & 0xFF);  //SJ5071212 - Should achieve the same result as above. (2 bytes saving)
          DIP_switch_val = DIP_switch_val & 0x00FF;

          switch (DIP_switch_val)
          {
            case 1:
            break;
            
            case 2:
            break;
            
            case 6:
            break;
          }
    }
}
#endif

//gstate_var.Room_ID = ~(P3IN & 0xFF);  //SJ3121212 - For testing.
//SJ3190912 - For testing CRC checksum update
//TI_DALI_Flash_Update(FWKEY);

#ifdef WITH_PLC
  #if 1
    status_information |= BIT2;  //SJ4300812 - Turn on arc power - #define LAMP_ARC_POWER_ON  BIT2
  #endif

//SJ3180712 - PLC system initialization. ----------------------------------------------------------

  #ifdef RED_N_GREEN_LED  //SJ1081012 - Start blinking red LED
    //gstate_var.red_led_timer = BLINK_FREQ_250_MS;  //SJ3191212 - ORG
    gstate_var.led_blink_freq = LED_BLINK_FREQ;
    gstate_var.general_ctrl_flag |= RED_LED_BLINK_ON;
  #endif

    init_step = SYSTEM_INIT_STEP_1;
    trial = 0;
    while (init_step < SYSTEM_INIT_END && trial < MAX_RETRIAL)  //SJ3180712 - 1 trial only
    {
        proceed_flag = PLC_System_Init(init_step);
        //gstate_var.PLC_status = PLC_System_Init(init_step);

        if (proceed_flag == SJ_PASS)
        //if (gstate_var.PLC_status == SJ_PASS)
        {
            //gstate_var.PLC_status = init_step;
            init_step++;
            trial = 0;

            //gstate_var.general_delay = TIMER_2_SECONDS;  //SJ4230812 - Delay 2 seconds before retrial
            gstate_var.general_delay = 5;  //SJ1270812 - Delay 250 micro seconds before going to next phase
            while (gstate_var.general_delay) ;
        }
        else
        {//SJ1270812 - Temporary solution, once there are LEDs indicator in the hardware, use LED to indicate initialisation status.
            //gstate_var.general_delay = TIMER_2_SECONDS;  //SJ4230812 - Delay 2 seconds before retrial
            //while (gstate_var.general_delay) ;
            trial++;
        }
    }

    //SJ1191112 - No need this delay in this project.
    //SJ1191112 gstate_var.general_delay = TIMER_2_SECONDS;
    //SJ1191112 while (gstate_var.general_delay) ;  //SJ1160712 - Delay two seconds before returning.

    //init_step = NETWORK_INIT_SET_INFO_1;
    gstate_var.PLC_status = 0;
    if (proceed_flag == SJ_FAIL)
    //if (gstate_var.PLC_status == SJ_PASS)
    {
      #if 1  //SJ5071212 - This will reset system
        //SJ1191112 - System initialization fail, can't proceed to next stage.
        //SJ3250712 - SJTODO: What does the system do next? Reset and retry, or proceed?
        //P2OUT = 0x07;//0x0F;
        BCSCTL3 |= LFXT1S_2;  //SJ5060712 - ACLK = VLO
        WDTCTL = WDT_ARST_1000;  //SJ4230812 - For development use only. Remove after that.
        while (1);
      #else  //SJ5071212 - This will by-pass system reset
        ;
      #endif
    }
  #ifdef RED_N_GREEN_LED
    else
    {
        //SJ2091012 - Stop blinking
        gstate_var.general_ctrl_flag &= RED_LED_BLINK_OFF;
        //gstate_var.red_led_timer = 0;  //SJ3191212 - ORG
        gstate_var.led_blink_freq = 0;

        //SJ4061212 - Turn off red and on green LED
        P1OUT &= OFF_RED_LED;  //SJ1081012 - Turn off red LED
        P1OUT |= ON_GREEN_LED;  //SJ4291112 - Turn on green LED
    }
  #endif

#endif  //SJ2310712 - #ifdef WITH_PLC ... #endif

//ON_RED_LED   0x08   //SJ1081012 - Red LED on  (00001000)
//OFF_RED_LED  0xF7   //SJ1081012 - Red LED off (11110111)
//ON_GREEN_LED  0x20  //SJ1081012 - Green LED on  (00100000)
//OFF_GREEN_LED 0xDF  //SJ1081012 - Green LED off (11011111)


//SJ4090812 - DALI Initialisation -----------------------------------------------------------------
//SJ5071212 - DALI initialisation is moved up before modem configuration

    //gstate_var.general_delay = TIMER_1_SECOND;
    //while (gstate_var.general_delay) ;

    //--- SJ1031011 - At this point, both LED_current and LED_voltage would have -----
    //--- SJ1031011 - cum 40 data each. 1 sec 20 ISR, 2 sec 40 ISR entries. -------------
    //--- SJ1031011 - Potential issue: the first 10 data is not so accurate -------------

    gstate_var.UART_Rx_State = UART_STANDBY;  //SJ2310712 - New method

  #ifndef SWITCH_CONTROLLER
    current_trial = 0;
    current_count = 0;
  #endif

    //SJ3211112 - Retrieve device address from DIP switch
    //short_address = 0;  //SJ3211112 - This may not be needed.
    //DIP_switch_val = P3IN & 0x3F;  //SJ3211112 - Bit 0 to 5 contain dev addr (00111111)
    //DIP_switch_val = 0x06;  //SJ2271112 - For debugging purpose. Set the device to be 6

    //SJ3121212 - SJTODO: At the moment just below sttmt is sufficient to return Room_ID, 
    //SJ3121212 - in future hardware, may need to change.
    //DIP_switch_val = ~(P3IN & 0xFF);
    gstate_var.Room_ID = ~(P3IN & 0xFF);
    gstate_var.Room_ID = gstate_var.Room_ID & 0x00FF;  //SJ3020113 - Remember to mask MSB to zero
    if (device_type >= 11)  //SJ4131212 - This is temporay solution. REMOVE WHEN NO LONGER NEEDED
        gstate_var.Room_ID = 0x001D;

//SJ3110712 - Main loop ---------------------------------------------------------------------------
    while(1)   //SJ5060712 - Loop forever
    {
      //SJ1060812 - TI DALI code require the use of watchdog as interval, as such below watchdog will be skipped.
      //SJ3040810 - Use watchdog timer to reset in the event of infinite loop
      #if 0//def  WITH_WDT
        //WDTCTL = WDTPW + WDTHOLD;
        //SJ3040810 - Reset watchdog timer ~ 2.5 sec. 
        WDTCTL = WDT_ARST_1000;  //SJ4050810 - equiv  (WDTPW + WDTCNTCL + WDTSSEL)
      #endif

      #if 0  //SJ1191112 - This project uses point to point connection. DO NOT REMOVE, just in case it is needed
      /*****
        //SJ4230812 - If attachment not successfull, re-do
        if (gstate_var.PLC_status < NETWORK_INIT_ATTACH)
        {
        #ifdef RED_N_GREEN_LED
            gstate_var.led_blink_freq = LED_BLINK_FREQ;
            gstate_var.general_ctrl_flag |= RED_LED_BLINK_ON;
        #endif
            gstate_var.UART_Rx_duration = short_address * NETWORK_DELAY_CONST;  //SJ4300812 - Beware of byte overflow
            while (gstate_var.UART_Rx_duration) ;

            init_step = gstate_var.PLC_status + 1;
            trial = 0;

            while (init_step < NETWORK_INIT_END && trial < MAX_RETRIAL)  //SJ4190712 - 1 trial
            {
                proceed_flag = PLC_Network_Config(init_step);

                if (proceed_flag == SJ_PASS)
                {
                    gstate_var.PLC_status = init_step;
                    init_step++;
                    trial = 0;
                }
                else
                    trial++;
            }

          #ifdef RED_N_GREEN_LED
            gstate_var.general_ctrl_flag &= RED_LED_BLINK_OFF;
            gstate_var.led_blink_freq = 0;
            if (gstate_var.PLC_status == NETWORK_INIT_ATTACH)
            {
                //SJ2091012 - Device is successfully attached to network. Time to turn off red LED and on green LED
                P1OUT &= OFF_RED_LED;  //SJ1081012 - Turn off red LED. 0xF7 (11110111)
                P1OUT |= ON_GREEN_LED;  //SJ2091012 - Turn on green LED. 0x20 (00100000)
            }
          #endif

            gstate_var.UART_Rx_State = UART_STANDBY;  //SJ2310712 - New method
        }
      *****/
      #endif  //SJ1191112 - #if 0 ... #endif

//SJ4061212 - Processing input switches
      #ifdef SWITCH_CONTROLLER  //SJ4061212 - Processing switch
        if (P3IN != 0xFF && gstate_var.led_timer == 0 && device_type >= 11)  //SJ4061212 - means one of the switch is pressed
        {
            if (short_address == 0)
                //SJ3020113 - Happens only during commissioning of devices
                AZ_DALI_Response(0x0D);  //#define AZ_SWITCH_IDENTITY 0x0D in command.h
            else
            {
              //DIP_switch_val = 0xFF - (P3IN & 0xFF);  //SJ5071212
              DIP_switch_val = ~(P3IN & 0xFF);  //SJ5071212 - Should achieve the same result as above. (2 bytes saving)
              DIP_switch_val = DIP_switch_val & 0x00FF;  //SJ3020113 - Mask MSB to zero

              switch (DIP_switch_val)
              {
                case 1:  //SJ4061212 - Bit 0
                    //AZ_Transmit_Routine(0xFF05);
                    DIP_switch_val = 1;
                    break;

                case 2:  //SJ4061212 - Bit 1
                    //AZ_Transmit_Routine(0xFF00);
                    DIP_switch_val = 2;
                    break;

                case 4:  //SJ4061212 - Bit 2
                    //AZ_Transmit_Routine(0xFE7F);
                    DIP_switch_val = 3;
                    break;

                case 8:  //SJ4061212 - Bit 3
                    //AZ_Transmit_Routine(0xFF02);
                    DIP_switch_val = 4;
                    break;

                case 16:  //SJ4061212 - Bit 4
                    //AZ_Transmit_Routine(0xFF01);
                    DIP_switch_val = 5;
                    break;

                case 32:  //SJ4061212 - Bit 5
                    //AZ_Transmit_Routine(5);
                    DIP_switch_val = 6;
                    break;

                case 64:  //SJ4061212 - Bit 6
                    //AZ_Transmit_Routine(6);
                    DIP_switch_val = 7;
                    break;

                case 128:  //SJ4061212 - Bit 7
                    //AZ_Transmit_Routine(7);
                    DIP_switch_val = 8;
                    break;

                default:
                    break;
              }  //SJ4061212 - switch (DIP_switch_val) { ... }

            #if 0  //SJ3191212 - ORG
              if ((DIP_switch_val > 0) && (DIP_switch_val <= (device_type-10)))
                  AZ_DALI_Response(0x0A);
            #else  //SJ3191212 - Replaced
              if ((DIP_switch_val > 0) && (DIP_switch_val <= (device_type-10)))
              {
                  if (DIP_switch_val != gstate_var.prev_key)  //SJ3191212 - First pressed
                  {
                    gstate_var.prev_key = DIP_switch_val;
                    //AZ_DALI_Response(0x0A);
                  }
                  else
                  {
                    gstate_var.prev_key = 0;  //SJ4201212 - If comes here, need to send proprietary command to light
                  }

                  AZ_DALI_Response(0x0A);
              }
            #endif

            }  //SJ4131212 - if (short_address == 0) { ... } else { ... }

            gstate_var.led_timer = 4;  //SJ4061212 - SJTODO: Use a proper variable.
        }  //SJ4061212 - if (P3IN != 0xFF) { ... }
      #endif

//SJ4061212 - Gateway to DALI transaction loop
        if (gstate_var.UART_Rx_State == UART_PROCESSING)
        {
            if (ggeneral_buffer[MSG_HEADER_TYPE] == MSG_TYPE__DATA_TRANSFER)
            {
                TI_DALI_Transaction_Loop();   //SJ1191112 - DALI protocol is not used in this project
                //UART_Rx_Acknowledgement(1);

                //SJ2110912 - Update to flash is requested, so start countdown.
                //*****  //SJ3211112 - If there is need for this, need to modify.
                if (flash_update_request)
                {
				  #if 0  //SJ3171012 - ORG. With delay before writing to flash
                    gstate_var.flash_update_countdown = TIMER_30_SECONDS;  //SJ2110912 - In real implemtation, change to TIMER_5_MINUTES
				  #else //SJ3171012 - Replaced. No delay, write to flash immediately
		            flash_update_request = 0;
		            //TI_DALI_Flash_Update(FWKEY);//FWKEY
		            TI_DALI_Flash_Update(FWKEY, 0);  //SJ1171212 - Replaced
#if 1  //SJ5071212 - This will reset system
//if (ggeneral_buffer[66] == 0x20)  //SJ5040113 #define  AZ_CMD_LOC  66 defined in dali.h
if (gstate_var.general_ctrl_flag & SYS_RESET_ON)
{
    //gstate_var.general_ctrl_flag &= SYS_RESET_OFF;  //SJ5040113 - Since the system is going to reset, don't think we need this
    BCSCTL3 |= LFXT1S_2;  //SJ5060712 - ACLK = VLO
    WDTCTL = WDT_ARST_1000;  //SJ4230812 - For development use only. Remove after that.
    while (1);
}
#endif
		            WDTCTL = (WDT_MDLY_8);  //SJ4200912 - FADE_INTERVAL equiv (WDT_MDLY_8 - SMCLK/8192), 1 second will goto ISR 977 times.
		            IE1 |= WDTIE;
				  #endif
                }/*****/

            }  //SJ5270712 - if (ggeneral_buffer[MSG_HEADER_TYPE] == MSG_TYPE__DATA_TRANSFER) { ... }

            gstate_var.UART_Rx_State = UART_STANDBY;  //SJ2310712 - Move down from above
        }

//SJ1171212 - Don't need this if device is switch controller
#ifndef SWITCH_CONTROLLER
//SJ4061212 - Reading ADC channels for current and voltage value
      #if 0   //SJ1270812
        if (gstate_var.general_ctrl_flag & NEW_DATA_ON)
        {
            //gstate_var.cum_current = 0;
            trial = 0;
            for (data_len=0; data_len<CURRENT_SAMPLE; data_len++)
                //gstate_var.cum_current += gcurrent_tab[data_len];
                trial  += gcurrent_tab[data_len];

            trial = trial / CURRENT_SAMPLE;
            gstate_var.LED_current = (unsigned int) ((float) trial * 3.62);   //0.000362;

            //gstate_var.cum_voltage = 0;
            trial = 0;
            for (data_len=0; data_len<VOLTAGE_SAMPLE; data_len++)
                //gstate_var.cum_voltage += gvoltage_tab[data_len];
                trial  += gvoltage_tab[data_len];

            trial = trial / VOLTAGE_SAMPLE;
            gstate_var.LED_voltage = (unsigned int) ((float) trial * 3.0938);   //0.030938;

            gstate_var.general_ctrl_flag &= NEW_DATA_OFF;
        }
      #else  //SJ1270812 - Reading ADC

        //SJ1270812 - Reading channel 6 for LED current
        if (current_count == CURRENT_SAMPLE)  //SJ2271112 - CURRENT_SAMPLE equiv 50
        {
            current_trial = current_trial / CURRENT_SAMPLE;//10;
            gstate_var.LED_current = (unsigned int) ((float) current_trial * 3.62);   //0.000362;
            current_trial = 0;
            current_count = 0;
            //SJ1100912 - New LED_current is registered here.
            //SJ1100912 - 1) If prev_level not equal to actual_level, assign actual_level to prev_level and LED_current to prev_current.
            //SJ1100912 - 2) If prev_current not equal LED_current and actual_level equal prev_level, send error packet to DC.

            //SJ2110912 - SJTODO: Don't think below if sttmt is required at the moment; if it does in the future, it can be 
            //SJ2110912 -         placed within ISR_TimerA() ISR routine.
            //SJ1170912 if (gstate_var.prev_level != actual_level)
            //SJ1170912     gstate_var.prev_level = actual_level;
                //gstate_var.prev_current = gstate_var.LED_current;

            //if (gstate_var.LED_current < gstate_var.prev_current)
            //    current_trial = gstate_var.prev_current - gstate_var.LED_current;

            if (gstate_var.error_delay == 0)
            {
              if ((gstate_var.LED_current != 0) && (gstate_var.general_ctrl_flag & BATT_MONITOR_ON))
              {
                gstate_var.general_ctrl_flag &= BATT_MONITOR_OFF;
                //AZ_DALI_Response(DEVICE_QUERY);  //SJ1270812 #define DEVICE_QUERY  2 in PLC_Comm.h  //SJ2271112 - Not used

                gstate_var.error_delay = 10;
              }
              else
              {
                //SJ1100912 - Check for possible device malfunction
                if ((gstate_var.LED_current == 0) && (actual_level != 0x00))
                {
                    //if ((current_trial > 1000) && (gstate_var.prev_level == actual_level))
                    //if ((gstate_var.LED_current == 0) && (gstate_var.prev_level == actual_level))
                    //if ((gstate_var.prev_level == actual_level) && !(gstate_var.general_ctrl_flag & BATT_MONITOR_ON))  //SJ1100912 - Alert DC
                    if (!(gstate_var.general_ctrl_flag & BATT_MONITOR_ON))  //SJ1100912 - Alert DC
                    {
                        gstate_var.general_ctrl_flag |= BATT_MONITOR_ON;
                        //AZ_DALI_Response(DEVICE_MALFUNCTION);  //SJ1100912 - #define DEVICE_MALFUNCTION 4 in PLC_Comm.h  //SJ2271112 - Not used

                        gstate_var.error_delay = 10;
                    }
                }
              }
            }  //SJ2110912 - if (error_delay == 0) { ... }
            //current_trial = 0;
            //current_count = 0;
        }
        else
        {
            ADC10CTL1 = INCH_6 + ADC10DIV_7 + ADC10SSEL_3;  //SJ4120712 - SMCLK=8MHz, ADC10DIV_7=8; 8 / 8 = 1MHz
            current_trial += RetrieveFromADC(ADC_BINARY);
            current_count++;
        }

        //SJ1270812 - Reading channel 7 for LED voltage
        trial = 0;
        ADC10CTL1 = INCH_7 + ADC10DIV_7 + ADC10SSEL_3;  //SJ4120712 - SMCLK=8MHz, ADC10DIV_7=8; 8 / 8 = 1MHz
        trial = RetrieveFromADC(ADC_BINARY);
        gstate_var.LED_voltage = (unsigned int) ((float) trial * 3.0938);
      #endif
#endif  //SJ1171212 - #ifndef SWITCH_CONTROLLER ... #endif
//SJ1171212 - Don't need this if device is switch controller

      #if 0  //SJ2180912 - Alert GUI of sensor being detected
        //SJ2271112 - In Micro lite project, those devices equipped with sensor will execute this
        if (gstate_var.sensor_state == ON_STATE)
        {
            if (!(gstate_var.general_ctrl_flag & SENSOR_ON))
            {
                gstate_var.general_ctrl_flag |= SENSOR_ON;
                AZ_DALI_Response(SENSOR_DETECTED);  //SJ2180912 #define SENSOR_DETECTED 7 in PLC_Comm.h
                gstate_var.led_timer = TIMER_5_SECONDS;
            }
            else
                gstate_var.led_timer = TIMER_5_SECONDS;
        }
        else
        {
            if ((gstate_var.general_ctrl_flag & SENSOR_ON) && (gstate_var.led_timer == 0))
            {
                gstate_var.general_ctrl_flag &= SENSOR_OFF;
                AZ_DALI_Response(SENSOR_DETECTED);  //SJ2180912 #define SENSOR_DETECTED 7 in PLC_Comm.h
            }
        }
      #endif

	  #if 0  //SJ3171012 - DO NOT REMOVE. Move up to right after dali transaction
        if (flash_update_request && gstate_var.flash_update_countdown <= 5)
        {
            flash_update_request = 0;
            //TI_DALI_Flash_Update(FWKEY);//FWKEY
            TI_DALI_Flash_Update(FWKEY, 0);  //SJ1171212 - Replaced
            WDTCTL = (WDT_MDLY_8);  //SJ4200912 - FADE_INTERVAL equiv (WDT_MDLY_8 - SMCLK/8192), 1 second will goto ISR 977 times.
            IE1 |= WDTIE;
        }
	  #endif

      #if 0  //SJ4041012 - Test broadcast packet from service node
        if (gstate_var.broadcast_delay == 0)
        {
            if (gstate_var.broadcast_mode == 0x00)
                gstate_var.broadcast_mode = 0x05;
            else
                gstate_var.broadcast_mode = 0x00;

            AZ_DALI_Response(BROADCAST_FROM_SN);
            gstate_var.broadcast_delay = TIMER_20_SECONDS;
        }
      #endif

//AZ_Randomised_Long_Address();  //SJ5071212 - Purely for testing. REMOVE AFTER USE
    }   //SJ5300911 while(1) { ... }
//SJ3110712 - End of Main loop --------------------------------------------------------------------

}  //SJ5060712 - End of main(void) { ... }



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
#endif


//SJ4040310 - Retrieve from ADC and convert its output to signed integer
int RetrieveFromADC(int data_format)//(void)
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
    //ADC10CTL0 = SREF_0 + ADC10SHT_2 + ADC10SR + ADC10ON + ENC + ADC10SC;  //SJ1110411 - (SREF_0: VR+ = AVCC and VR- = AVSS)
    //ADC10CTL0 = SREF_1 + ADC10SHT_2 + ADC10SR + REF2_5V + REFON + ADC10ON + ENC + ADC10SC;  //SJ2110510 - Using 2.5 volts  //SJ1031011 - 
  #if 0  //SJ5170812 - ORG
    ADC10CTL0 = SREF_6 + ADC10SHT_2 + ADC10SR + ADC10ON + ENC + ADC10SC;  //SJ1031011 - (SREF_6: VR+ = VEREF+ and VR- = VREF-/VEREF-)
  #else  //SJ5170812 - Replaced
    ADC10CTL0 = SREF_2 + ADC10SHT_2 + ADC10SR + ADC10ON + ENC + ADC10SC;  //SJ1031011 - (SREF_2: VR+ = VEREF+ and VR- = AVSS)
  #endif

    while (ADC10CTL1 & ADC10BUSY);  //SJ4050810 - NOTE: Do we need to reset WDT+ counter here???
    temp_adc = ADC10MEM;
    ADC10CTL0 &= ~ENC;

    if (data_format == ADC_TWO_COMPLEMENT)
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


//SJ2181212 - SJNOTE: Below function may be omitted.
#if 0//def SWITCH_CONTROLLER  //SJ4221112 - Point to point transmit

//SJ2271112 - This function is likely used by devices equipped with sensor.
//SJ2271112 - In other word, the device acts as a master.
//SJ1171212 - SJNOTE: This function maybe omitted and use AZ_DALI_Response() instead
void AZ_Transmit_Routine(int data_type)
{
    unsigned char temp_msg_type;
    unsigned int temp=0;
    int i, j, k;
    //int temp_ADC;
    //unsigned int ndx=10;

#ifdef RED_N_GREEN_LED  //SJ2091012 - Start blinking green LED
    //gstate_var.green_led_timer = BLINK_FREQ_250_MS;  //SJ3191212 - ORG
    gstate_var.led_blink_freq = LED_BLINK_FREQ;
    gstate_var.general_ctrl_flag |= GRN_LED_BLINK_ON;
#endif

    ggeneral_buffer[0] = MSG_TYPE__DATA_TRANSFER;  //SJ3220812 - Data transfer request type 0x00 i=0
    ggeneral_buffer[1] = 0x80;  //SJ4160812 - ORG=1, RPY=0, RESV=0, SEQ=0 (10000000) i=1
    ggeneral_buffer[4] = 0x00;  //SJ4160812 - Msg header CRC LSB; i=4
    ggeneral_buffer[5] = 0x00;  //SJ4160812 - Msg header CRC MSB; i=5
    ggeneral_buffer[6] = 0x00;  //SJ4160812 - Msg payload CRC LSB; i=6
    ggeneral_buffer[7] = 0x00;  //SJ4160812 - Msg payload CRC MSB; i=7
    ggeneral_buffer[8] = 0xDD;  //SJ4160812 - NSDU handle i=8
    ggeneral_buffer[9] = 0x01;  //SJ4160812 - QoS=0, SEC=0 D-route=1 i=9

    //L_SDU DATA
    ggeneral_buffer[10] = 0x60;  //IPv Version & Prority
    ggeneral_buffer[11] = 0x00;  //flow label
    ggeneral_buffer[12] = 0x00;
    ggeneral_buffer[13] = 0x00;
    ggeneral_buffer[14] = 0x00;  //payload length
    ggeneral_buffer[15] = 0x14;  //IPv6 payload Length, match with byte 55
    ggeneral_buffer[16] = 0x11;  //next header
    ggeneral_buffer[17] = 0x08;  //Hop limit

    //Source IPv6 address
    ggeneral_buffer[18] = 0xfe;
    ggeneral_buffer[19] = 0x80;
    ggeneral_buffer[20] = 0x00;
    ggeneral_buffer[21] = 0x00;
    ggeneral_buffer[22] = 0x00;
    ggeneral_buffer[23] = 0x00;
    ggeneral_buffer[24] = 0x00;
    ggeneral_buffer[25] = 0x00;
    ggeneral_buffer[26] = 0x01;  //Room ID //PANID[1];
    ggeneral_buffer[27] = 0x01;  //Room ID //PANID[0];
    ggeneral_buffer[28] = 0x00;
    ggeneral_buffer[29] = 0xff;
    ggeneral_buffer[30] = 0xfe;
    ggeneral_buffer[31] = 0x00;
    ggeneral_buffer[32] = 0x00;  //own short address
    ggeneral_buffer[33] = 0x00;

    //Destination IPv6 address
    ggeneral_buffer[34] = 0xfe;
    ggeneral_buffer[35] = 0x80;
    ggeneral_buffer[36] = 0x00;
    ggeneral_buffer[37] = 0x00;
    ggeneral_buffer[38] = 0x00;
    ggeneral_buffer[39] = 0x00;
    ggeneral_buffer[40] = 0x00;
    ggeneral_buffer[41] = 0x00;
    ggeneral_buffer[42] = 0x01;  //Room ID
    ggeneral_buffer[43] = 0x01;  //Room ID
    ggeneral_buffer[44] = 0x00;
    ggeneral_buffer[45] = 0xff;
    ggeneral_buffer[46] = 0xfe;
    ggeneral_buffer[47] = 0x00;
    ggeneral_buffer[48] = 0x00; 
    ggeneral_buffer[49] = 0x00;

    //UDP
    ggeneral_buffer[50] = 0x00;  //source port
    ggeneral_buffer[51] = 0x00;
    ggeneral_buffer[52] = 0x00;  //destination port
    ggeneral_buffer[53] = 0x00;

    //length in hex, 2 bytes [54][55]
    ggeneral_buffer[54] = 0x00;  //length in hex
    ggeneral_buffer[55] = 0x14;  //start from byte 50 (after ipv6 address)

    //CRC16
    ggeneral_buffer[56] = 0x00;
    ggeneral_buffer[57] = 0x00;

    // *********** Data design start here
    ggeneral_buffer[58] = 0x00;  //Reserved 58-61
    ggeneral_buffer[59] = 0x00;   
    ggeneral_buffer[60] = 0x00;
    ggeneral_buffer[61] = 0x00;

    ggeneral_buffer[62] = (gstate_var.Room_ID & 0xFF00) >> 8;  //SJ1101212 - Room ID (MSB) i=62
    ggeneral_buffer[63] = (gstate_var.Room_ID & 0x00FF);  //SJ1101212 - Room ID (LSB) i=63

    ggeneral_buffer[64] = 0x00;  //short address
    ggeneral_buffer[65] = (data_type & 0xFF00) >> 8;
    ggeneral_buffer[66] = (data_type & 0x00FF);  //DALI command

    ggeneral_buffer[67] = 0x00;  //Aztech propietary 3 bytes
    ggeneral_buffer[68] = 0x00;
    ggeneral_buffer[69] = 0x00;

	//SJ4221112 - Beginning of payload to be transmitted
//    ggeneral_buffer[10] = 0x00;	//SJ4061212 - Reserved;  i=10
//    ggeneral_buffer[11] = 0x00;	//SJ3281112 - Reserved;  i=11
//    ggeneral_buffer[12] = 0x00; //SJ4061212 - Reserved;  i=12
//    ggeneral_buffer[13] = 0x00; //SJ4061212 - Reserved;  i=13
//    ggeneral_buffer[14] = data_type;  //SJ4061212 - Room ID MSB;  i=14
//    ggeneral_buffer[15] = data_type;  //SJ4061212 - Room ID LSB;  i=15
//    ggeneral_buffer[16] = 0x00;	//SJ4061212 - Short address MSB;  i=16
//    ggeneral_buffer[17] = (data_type & 0xFF00) >> 8;//0x01;	//SJ4061212 - Short address LSB;  i=17
//    ggeneral_buffer[18] = (data_type & 0x00FF);	//SJ4061212 - Special control code;  i=18
//    ggeneral_buffer[19] = 0x00; //SJ4061212 - Aztech proprietary code; i=19
//    ggeneral_buffer[20] = 0x00; //SJ4061212 - Aztech proprietary code; i=20

    i = 70;

    if ((i % 2))  //SJ3010812 - Padding required
    {
        ggeneral_buffer[i++] = 0x00;
        k = i - 5;  //SJ3010812 - Substract the first four bytes and padding byte.
    }
    else
        k = i - 4;  //SJ3010812 - Substract the first four bytes.

    ggeneral_buffer[2] = (k & 0x00FF);  //SJ3010812 - Msg payload length LSB; Total payload length=64
    ggeneral_buffer[3] = (k >> 8);

  #if 0  //SJ2110912 - Re-open if delay is needed.
    gstate_var.general_delay = 16;  //SJ4160812 - 800 msec
    while (gstate_var.general_delay) ;
  #endif

    temp_msg_type = ggeneral_buffer[MSG_HEADER_TYPE];

    //SJ3010812 - Transmit packet to PLC device
    for (j=0; j<i; j++)
    {
        while (!(IFG2 & UCA0TXIFG));  // USCI_A0 TX buffer ready?
        UCA0TXBUF = ggeneral_buffer[j];  //0xA5
//        ggeneral_buffer[i] = 0xFF;  //SJ4190712 - Empty buffer
    }

    gstate_var.UART_Rx_State = UART_STANDBY;  //SJ1300712 - New method
    gstate_var.UART_Rx_duration = TIMER_2_SECONDS;

    while (gstate_var.UART_Rx_duration)  //SJ3180712 - Wait for 2 seconds for UART_Rx to complete
    {
        if (gstate_var.UART_Rx_State == UART_PROCESSING)  //SJ1300712 - New method
        {
            temp = (ggeneral_buffer[MESSAGE_HEADER_LEN+1] << 8) + ggeneral_buffer[MESSAGE_HEADER_LEN];

            //SJ3180712 - If msg type return from PLC device does not match the msg type sent by host,
            //SJ3180712 - or reply status not equal 0x0000, raise error flag
            if ((ggeneral_buffer[MSG_HEADER_TYPE] == temp_msg_type) && (temp == MSG_REQUEST_OK))
            {
                ;
                //ret_flag = SJ_PASS;
                //if (response_type == REVEAL_DALI_ADDRESS)  //SJ4130912 - Not used at the moment
                //    gstate_var.general_ctrl_flag |= ATTACH_ON;
            }

            //gstate_var.UART_Rx_ndx = MESSAGE_HEADER_LEN+1;  //SJ3180712 - Prevent UART_Rx ISR from setting UART_Rx_COMPLETE
            gstate_var.UART_Rx_duration = 0;
            gstate_var.UART_Rx_State = UART_STANDBY;  //SJ1100912 -
        }
    }

  #ifdef RED_N_GREEN_LED
    //SJ2091012 - Stop blinking
    gstate_var.general_ctrl_flag &= GRN_LED_BLINK_OFF;
    //gstate_var.green_led_timer = 0;  //SJ3191212 - ORG
    gstate_var.led_blink_freq = 0;
    P1OUT |= ON_GREEN_LED;  //SJ4061212 - Ensure green LED is on in the event of out of sync
  #endif

}  //SJ3191212 - End of AZ_Transmit_Routine() function
#endif


//================================================
#if 1
//SJ5071212 - This function is used to generate randomised long address. (24 bits)
void AZ_Randomised_Long_Address(void)
{
    //unsigned char ret_flag=SJ_FAIL;
    //unsigned char temp_byte=0;
    int seed=0;
    //int trial;
    int *PtrIntFlash;

    if (short_address == 0x00)  //SJ5071212 - This indicated device has not been assigned with short addr yet
    {

      #if 1  //SJ5071212 - Using ADC value
        //ADC10CTL1 = INCH_7 + ADC10DIV_7 + ADC10SSEL_3;  //SJ4120712 - SMCLK=8MHz, ADC10DIV_7=8; 8 / 8 = 1MHz
        //seed = RetrieveFromADC(ADC_BINARY);        

        seed = TAR;  //SJ5071212 - Use timer counter, above method use ADC data

        if (seed == 0)
      #else  //SJ5071212 - Using global variable
        if (gstate_var.LED_voltage != 0)
        {
            seed = gstate_var.LED_voltage / 100;
        }
        else
      #endif
        {
            PtrIntFlash = (int *) 0x10C0;
            seed = (int) (*PtrIntFlash);
            seed = (seed & 0xFF00) >> 8;
        }

        srand(seed);
        random_address_byte_h = rand();
        random_address_byte_m = rand();
        random_address_byte_l = rand();

        //temp_byte = seed & 0x00FF;
        gstate_var.DALI_response = seed & 0x00FF;

        flash_update_request = 1;

    }  //SJ5071212 - if (short_address == 0x00) { ... }

    //return (ret_flag);

}  //SJ5071212 - End of AZ_Randomised_Long_Address()
#endif
//================================================

//SJ1171212 - Don't think it'll be used in iSwitch project; keep it in case needed
#if 0
unsigned char AZ_Acknowledge_Basenode_Request(unsigned char Message_Type)
//void UART_Rx_Acknowledgement(int validity_flag)
{
    unsigned char ret_flag=SJ_FAIL;
    unsigned char temp_msg_type;
    int i, j, k;
    unsigned int temp=0;

  #ifdef RED_N_GREEN_LED   //SJ2091012 - Start blinking green LED
    //gstate_var.green_led_timer = BLINK_FREQ_250_MS;  //SJ3191212 - ORG
    gstate_var.led_blink_freq = LED_BLINK_FREQ;
    gstate_var.general_ctrl_flag |= GRN_LED_BLINK_ON;
  #endif

    //SJ4300812 - Construct message header
    //ggeneral_buffer[0] = MSG_TYPE__DETACH;  //SJ4300812 - Message type (0x11) i=0
    ggeneral_buffer[1] = 0x80;  //SJ4300812 - ORG=1, RPY=0, RESV=0, SEQ=0 (10000000); i=1
    ggeneral_buffer[2] = 0x00;  //SJ4300812 - Msg payload length LSB; i=2
    ggeneral_buffer[3] = 0x00;  //SJ4300812 - Msg payload length MSB; i=3
    ggeneral_buffer[4] = 0x00;  //SJ4300812 - Msg header CRC LSB; i=4
    ggeneral_buffer[5] = 0x00;  //SJ4300812 - Msg header CRC MSB; i=5
    ggeneral_buffer[6] = 0x00;  //SJ4300812 - Msg payload CRC LSB; i=6
    ggeneral_buffer[7] = 0x00;  //SJ4300812 - Msg payload CRC MSB; i=7


    if (Message_Type == MSG_TYPE__DETACH)
    {
        ggeneral_buffer[0] = MSG_TYPE__DETACH;  //SJ4300812 - Message type (0x11) i=0

        //SJ4300812 - Construct message payload
        ggeneral_buffer[8] = 0x00;//0x08;  //SJ4300812 - G3 Long Address Byte 0 LSB; i=8
        ggeneral_buffer[9] = 0x00;//0x07;  //SJ4300812 - G3 Long Address Byte 1; i=9
        ggeneral_buffer[10] = 0x00;//0x06;  //SJ4300812 - G3 Long Address Byte 2; i=10
        ggeneral_buffer[11] = 0x00;//0x05;  //SJ4300812 - G3 Long Address Byte 3; i=11
        ggeneral_buffer[12] = 0x00;//0x04;  //SJ4300812 - G3 Long Address Byte 4; i=12
        ggeneral_buffer[13] = 0x00;//0x03;  //SJ4300812 - G3 Long Address Byte 5; i=13
        ggeneral_buffer[14] = 0x00;  //SJ4300812 - G3 Long Address Byte 6; i=14  //SJ5240812 - Reserved for future 2-byte DALI address
        ggeneral_buffer[15] = 0x00;//short_address;  //SJ4300812 - G3 Long Address Byte 7 MSB; i=15   //short_address
        i = 16;
    }

    //i = (ggeneral_buffer[MSG_PAYLOAD_LEN_MSB] << 8) + ggeneral_buffer[MSG_PAYLOAD_LEN_LSB];
    //i += 4;

    if ((i % 2) == 1)  //SJ4300812 - Padding required
    {
        ggeneral_buffer[i++] = 0x00;
        k = i - 5;  //SJ4300812 - Substract the first four bytes and padding byte.
    }
    else
        k = i - 4;  //SJ4300812 - Substract the first four bytes.

    ggeneral_buffer[2] = (k & 0x00FF);  //SJ4300812 - Msg payload length LSB; Total payload length=24
    ggeneral_buffer[3] = (k >> 8);  

    temp_msg_type = Message_Type;
    //SJ4300812 - Transmit packet to PLC device
    for (j=0; j<i; j++)
    {
        while (!(IFG2 & UCA0TXIFG));  // USCI_A0 TX buffer ready?
        UCA0TXBUF = ggeneral_buffer[j];  //0xA5
        //ggeneral_buffer[i] = 0xFF;  //SJ4300812 - Empty buffer
    }

    gstate_var.UART_Rx_State = UART_STANDBY;  //SJ4300812 -  
    gstate_var.UART_Rx_duration = TIMER_2_SECONDS;

    while (gstate_var.UART_Rx_duration)  //SJ4300812 - Wait for 2 seconds for UART_Rx to complete
    {
        if (gstate_var.UART_Rx_State == UART_PROCESSING)  //SJ4300812 - 
        {
//SJ1100912            gstate_var.UART_Rx_State = UART_INVALID;  //SJ4300812 - 

            temp = (ggeneral_buffer[MESSAGE_HEADER_LEN+1] << 8) + ggeneral_buffer[MESSAGE_HEADER_LEN];

            //SJ4300812 - If msg type return from PLC device does not match the msg type sent by host, 
            //SJ4300812 - or reply status not equal 0x0000, raise error flag
            if ((ggeneral_buffer[MSG_HEADER_TYPE] == temp_msg_type) && (temp == MSG_REQUEST_OK))
                ret_flag = SJ_PASS;

            //gstate_var.UART_Rx_ndx = MESSAGE_HEADER_LEN+1;  //SJ4300812 - Prevent UART_Rx ISR from setting UART_Rx_COMPLETE
            gstate_var.UART_Rx_duration = 0;
            gstate_var.UART_Rx_State = UART_STANDBY;  //SJ1100912 - 
        }
    }

  #ifdef RED_N_GREEN_LED
    //SJ2091012 - Stop blinking
    gstate_var.general_ctrl_flag &= GRN_LED_BLINK_OFF;
    //gstate_var.green_led_timer = 0;  //SJ3191212 - ORG
    gstate_var.led_blink_freq = 0;
    P1OUT |= ON_GREEN_LED;  //SJ4061212 - Ensure green LED is on in the event of out of sync
  #endif

    return (ret_flag);

}  //SJ3010812 - End of UART_Rx_Acknowledgement()
#endif  //SJ4230812 - #if 0 { ... } #else { ... }


//SJ4160812 - This function is used to send device current and voltage to GUI
void AZ_DALI_Response(int response_type)
{
    unsigned char temp_msg_type;
    unsigned int temp=0;
    int i, j, k;
    //int temp_ADC;
    //unsigned int ndx=10;

#ifdef RED_N_GREEN_LED  //SJ2091012 - Start blinking green LED
    //gstate_var.green_led_timer = BLINK_FREQ_250_MS;  //SJ3191212 - ORG
    gstate_var.led_blink_freq = LED_BLINK_FREQ;
    gstate_var.general_ctrl_flag |= GRN_LED_BLINK_ON;
#endif

    ggeneral_buffer[0] = MSG_TYPE__DATA_TRANSFER;  //SJ3220812 - Data transfer request type 0x00 i=0
    ggeneral_buffer[1] = 0x80;  //SJ4160812 - ORG=1, RPY=0, RESV=0, SEQ=0 (10000000) i=1
    ggeneral_buffer[4] = 0x00;  //SJ4160812 - Msg header CRC LSB; i=4
    ggeneral_buffer[5] = 0x00;  //SJ4160812 - Msg header CRC MSB; i=5
    ggeneral_buffer[6] = 0x00;  //SJ4160812 - Msg payload CRC LSB; i=6
    ggeneral_buffer[7] = 0x00;  //SJ4160812 - Msg payload CRC MSB; i=7
    ggeneral_buffer[8] = 0xDD;  //SJ4160812 - NSDU handle i=8
    ggeneral_buffer[9] = 0x01;  //SJ4160812 - QoS=0, SEC=0 D-route=1 i=9
    //i = 10;

	//L_SDU DATA
    ggeneral_buffer[10] = 0x60;	//IPv Version & Prority  i=10

    //flow label
    ggeneral_buffer[11] = 0x00;	//i=11
    ggeneral_buffer[12] = 0x00;	//i=12
    ggeneral_buffer[13] = 0x00; //i=13

//SJ2040912    ggeneral_buffer[14] = 0x00;	//payload length i=14
//SJ2040912    ggeneral_buffer[15] = 0x22;	//SJ3220812 - equiv 34 i=15

    ggeneral_buffer[16] = 0x11;	//next header i=16
    ggeneral_buffer[17] = 0x08;	//Hop limit i=17

    //SJ3220812 - Source IPv6 address
    ggeneral_buffer[18] = 0xfe;	//i=18
    ggeneral_buffer[19] = 0x80;	//i=19
    ggeneral_buffer[20] = 0x00;	//i=20
    ggeneral_buffer[21] = 0x00;	//i=21
    ggeneral_buffer[22] = 0x00;	//i=22
    ggeneral_buffer[23] = 0x00;	//i=23
    ggeneral_buffer[24] = 0x00;	//i=24
    ggeneral_buffer[25] = 0x00;	//i=25
    ggeneral_buffer[26] = 0x01;//(gstate_var.PAN_ID >> 8);	//SJ1101212 - Need to change to Room ID (MSB) i=26
    ggeneral_buffer[27] = 0x01;//(gstate_var.PAN_ID & 0x00FF); //SJ1101212 - Need to change to Room D(LSB) i=27
    ggeneral_buffer[28] = 0x00;	//i=28
    ggeneral_buffer[29] = 0xff;	//i=29
    ggeneral_buffer[30] = 0xfe;	//i=30
    ggeneral_buffer[31] = 0x00;	//i=31
    ggeneral_buffer[32] = 0x00;//(gstate_var.network_addr >> 8);	//SJ1101212 - TAKE NOTE short address (MSB) i=32
    ggeneral_buffer[33] = 0x00;//(gstate_var.network_addr & 0x00FF);	//SJ1101212 - TAKE NOTE short address (LSB) i=33

	//SJ3220812 - Destination IPv6 address
	//the order is swapped based on 2-byte (word)
    ggeneral_buffer[34] = 0xfe;	//i=34
    ggeneral_buffer[35] = 0x80;	//i=35
    ggeneral_buffer[36] = 0x00;	//i=36
    ggeneral_buffer[37] = 0x00;	//i=37
    ggeneral_buffer[38] = 0x00;	//i=38
    ggeneral_buffer[39] = 0x00;	//i=39
    ggeneral_buffer[40] = 0x00;	//i=40
    ggeneral_buffer[41] = 0x00;	//i=41
    ggeneral_buffer[42] = 0x01; //(gstate_var.PAN_ID >> 8);	//SJ1101212 - Need to change to Room ID (MSB) i=42
    ggeneral_buffer[43] = 0x01; //(gstate_var.PAN_ID & 0x00FF); //SJ1101212 - Need to change to Room ID(LSB) i=43
    ggeneral_buffer[44] = 0x00;	//i=44
    ggeneral_buffer[45] = 0xff;	//i=45
    ggeneral_buffer[46] = 0xfe;	//i=46
    ggeneral_buffer[47] = 0x00;	//i=47
    ggeneral_buffer[48] = 0x00;	//SJ3220812 - source short address (MSB) i=48
    ggeneral_buffer[49] = 0x00;	//SJ3220812 - source short address (LSB) i=49

	//SJ3220812 - UDP
    ggeneral_buffer[50] = 0x00;	//SJ3220812 - source port (MSB) i=50
    ggeneral_buffer[51] = 0x00;	//SJ3220812 - source port (LSB) i=51

    ggeneral_buffer[52] = 0x00;	//destination port i=52
    ggeneral_buffer[53] = 0x00;	//i=53

//SJ1030912    ggeneral_buffer[54] = 0x00;  //length in hex i=54
//SJ1030912    ggeneral_buffer[55] = 0x0E;  //0x10;  //SJ3220812 - THIS NEED TO CHANGE. Reflect the lenght of UDP packet i=55

    ggeneral_buffer[56] = 0x00;	//CRC16 i=56
    ggeneral_buffer[57] = 0x00;	//i=57

	//data length
    ggeneral_buffer[58] = 0x00;	//SJ2040912 - RESERVED. Reflect the lenght of data to send i=58
    ggeneral_buffer[59] = 0x00;	//i=59
    ggeneral_buffer[60] = 0x00; //i=60
    ggeneral_buffer[61] = 0x00; //i=61

    ggeneral_buffer[62] = (gstate_var.Room_ID >> 8); //& 0xFF00) >> 8;  //SJ1101212 - Room ID (MSB) i=62
    ggeneral_buffer[63] = (gstate_var.Room_ID);// & 0x00FF);  //SJ1101212 - Room ID (LSB) i=63

//==================================================
//SJ!!!! ggeneral_buffer[14] = 0x00;  //payload length
//SJ!!!! ggeneral_buffer[15] = 0x14;  //IPv6 payload Length, match with byte 55
    //length in hex, 2 bytes [54][55]
//SJ!!!! ggeneral_buffer[54] = 0x00;  //length in hex
//SJ!!!! ggeneral_buffer[55] = 0x14;  //start from byte 50 (after ipv6 address)
//==================================================

//--- SJ4131212 - Start of switch ---
    switch (response_type)
    {
/*****
  #if 0  //SJ4201212 -
	//if (response_type == REVEAL_DALI_ADDRESS)  //SJ2040912 - Not used at the moment
      case REVEAL_DALI_ADDRESS:
	  {
        ggeneral_buffer[14] = 0x00;	//payload length i=14
        ggeneral_buffer[15] = 0x0E;	//SJ3220812 - equiv 14 i=15

	    ggeneral_buffer[54] = 0x00;  //length in hex i=54
        ggeneral_buffer[55] = 0x0E;  //0x10;  //SJ3220812 - THIS NEED TO CHANGE. Reflect the lenght of UDP packet i=55

        //ggeneral_buffer[58] = 0x02;	//SJ3220812 - THIS NEED TO CHANGE. Reflect the lenght of data to send (1 for dali addr, 1 for command) i=58

        //SJ3220812 - short_address
        ggeneral_buffer[62] = short_address;  //SJ3220812 - DALI short addressi=62
        ggeneral_buffer[63] = 0x0A;  //SJ3220812 - i=63
        i = 64;
        break;
      }
  #endif

  #if 0
    //else if (response_type == DEVICE_QUERY)
      case DEVICE_QUERY:
      {
        ggeneral_buffer[14] = 0x00;	//payload length i=14
        ggeneral_buffer[15] = 0x14;	//SJ3220812 - equiv 20 i=15

	    ggeneral_buffer[54] = 0x00;  //length in hex i=54
        ggeneral_buffer[55] = 0x14;  //SJ3220812 - THIS NEED TO CHANGE. Reflect the lenght of UDP packet i=55

        //ggeneral_buffer[58] = 0x08;	//SJ3220812 - THIS NEED TO CHANGE. Reflect the lenght of data to send 
                                    //SJ1030912 - (1 for dali addr, 1 for command, 2 for current, 2 for voltage, 1 for info, and 1 for padding) i=58

        ggeneral_buffer[62] = short_address;  //SJ2280812 - Dali short address i=62
        ggeneral_buffer[63] = 0x90;           //SJ2280812 - #define QUERY_STATUS 144 defined in command.h i=63

        ggeneral_buffer[64] = (gstate_var.LED_current & 0x00FF);  //SJ4160812 - i=64
        ggeneral_buffer[65] = (gstate_var.LED_current >> 8);      //SJ4160812 - i=65

        ggeneral_buffer[66] = (gstate_var.LED_voltage & 0x00FF);  //SJ4160812 - i=66
        ggeneral_buffer[67] = (gstate_var.LED_voltage >> 8);      //SJ4160812 - i=67
        ggeneral_buffer[68] = status_information;                 //SJ1030912 - i=68
        ggeneral_buffer[69] = actual_level;                       //SJ2110912 - i=69
        i = 70;
        break;
      }
  #endif

  #if 0  //SJ5070912 -
    //else if (response_type == GENERIC_QUERY)  //#define GENERIC_QUERY  3
      case GENERIC_QUERY:  //#define GENERIC_QUERY  3
      {
        ggeneral_buffer[14] = 0x00;	//payload length i=14
        ggeneral_buffer[15] = 0x1C;	//SJ1240912 - equiv 28 i=15

	    ggeneral_buffer[54] = 0x00;  //length in hex i=54
        ggeneral_buffer[55] = 0x1C;  //SJ1240912 - THIS NEED TO CHANGE. Reflect the lenght of UDP packet i=55

        //ggeneral_buffer[58] = 0x08;	//SJ3220812 - THIS NEED TO CHANGE. Reflect the lenght of data to send 

        ggeneral_buffer[62] = short_address;  //SJ2280812 - Dali short address i=62
        //ggeneral_buffer[63] = 0xA0;           //SJ5070912 - #define QUERY_ACTUAL_LEVEL 160 defined in command.h i=63

        ggeneral_buffer[64] = status_information;  //SJ1240912 - i=64
        ggeneral_buffer[65] = version_number;  //SJ1240912 - i=65
        ggeneral_buffer[66] = device_type;  //SJ1240912 - i=66
        ggeneral_buffer[67] = phys_min_lvl;  //SJ1240912 - i=67
        ggeneral_buffer[68] = actual_level;  //SJ1240912 - i=68
        ggeneral_buffer[69] = max_level;  //SJ1240912 - i=69
        ggeneral_buffer[70] = min_level;  //SJ1240912 - i=70
        ggeneral_buffer[71] = power_on_level;  //SJ1240912 - i=71
        ggeneral_buffer[72] = system_failure_level;  //SJ1240912 - i=72
        ggeneral_buffer[73] = fade_rate;  //SJ1240912 - i=73
        ggeneral_buffer[74] = fade_time;  //SJ1240912 - i=74
        ggeneral_buffer[75] = data_transfer_register;  //SJ1240912 - i=75
        ggeneral_buffer[76] = data_transfer_register1;  //SJ1240912 - i=76
        ggeneral_buffer[77] = data_transfer_register2;  //SJ1240912 - i=77
        i = 78;  //SJ1170912 - 
        break;
      }
  #endif

  #if 0  //SJ5070912 -
    //else if (response_type == DEVICE_MALFUNCTION)  //#define DEVICE_MALFUNCTION  4
      case DEVICE_MALFUNCTION:  //#define DEVICE_MALFUNCTION  4
      {
        ggeneral_buffer[14] = 0x00;	//payload length i=14
        ggeneral_buffer[15] = 0x0E;	//SJ1100912 - equiv 14 i=15

	    ggeneral_buffer[54] = 0x00;  //length in hex i=54
        ggeneral_buffer[55] = 0x0E;  //SJ1100912 - THIS NEED TO CHANGE. Reflect the lenght of UDP packet i=55

        //ggeneral_buffer[58] = 0x08;	//SJ3220812 - THIS NEED TO CHANGE. Reflect the lenght of data to send 

        ggeneral_buffer[62] = short_address;  //SJ2280812 - Dali short address i=62
        ggeneral_buffer[63] = 0x0E;           //SJ1100912 - #define AZ_DEVICE_ERROR 0x0E defined in command.h i=63
        i = 64;
        break;
      }
  #endif
*****/
  #ifdef SWITCH_CONTROLLER  //SJ2181212 - If device type is switch
      case  10:  //SJ2181212 - #define AZ_PROPRIETARY_CMD  0x0A in command.h
        //SJ2181212 - UDP data at location 50 up to 63 already define above. (Total 14 bytes excluding subsequent data)
        ggeneral_buffer[14] = 0x00;	//payload length i=14
        ggeneral_buffer[15] = 0x14;	//SJ4130912 - equiv 18 i=15

	    ggeneral_buffer[54] = 0x00;  //length in hex i=54
        ggeneral_buffer[55] = 0x14;  //SJ4130912 - THIS NEED TO CHANGE. Reflect the lenght of UDP packet i=55

        //ggeneral_buffer[62] = (gstate_var.Room_ID & 0xFF00) >> 8;//0x00;  //SJ1101212 - Room ID (MSB) i=62
        //ggeneral_buffer[63] = (gstate_var.Room_ID & 0x00FF);//0x00;  //SJ1101212 - Room ID (LSB) i=63

        ggeneral_buffer[64] = 0x00;  //SJ1101212 - short address (MSB) i=64
        ggeneral_buffer[65] = 0xFF;//short_address;  //SJ4130912 - Dali short address i=65

        ggeneral_buffer[66] = gswitch_oper[DIP_switch_val-1];  //SJ4130912 - #define QUERY_RANDOM_ADDR_H 194 defined in command.h i=66
        ggeneral_buffer[67] = 0x00;  //SJ4130912 - i=67
        ggeneral_buffer[68] = 0x00;  //SJ4130912 - i=68
        if (gstate_var.prev_key == 0)
        {
          //SJ5211212 - If button is programmed to turn on, off it here
          if (gswitch_oper[DIP_switch_val-1] == 0x05)  //SJ5211212 - #define RECALL_MAX_LEVEL 0x05 in command.h
          {
            ggeneral_buffer[66] = 0x00;
          }
          else
          {
            ggeneral_buffer[66] = 0x0A;  //SJ3191212 - Introduce to allow toggle function of the same key pressed twice
            ggeneral_buffer[67] = 0x10;  //SJ4130912 - i=67  //SJ4201212 - #define AZ_EXT_CMD_REVERSE 0x10 defined in command.h
            ggeneral_buffer[68] = gswitch_oper[DIP_switch_val-1];  //SJ4130912 - i=68  //SJ4201212 - Decide if we need to substract 1.
          }  //SJ5211212 - if (gswitch_oper[DIP_switch_val-1] == 0x05) { ... } else { ... }
        }
        ggeneral_buffer[69] = 0x00;  //SJ4130912 - i=69
        //ggeneral_buffer[70] = 0x00;  //SJ1101212 - SJTODO: used device type global var
        i = 70;  //SJ4130912 -
        break;
  #endif

  #if 1
      case 128: //SJ4201212 - #define STORE_THE_DTR_AS_SHORT_ADDRESS 0x80 in command.h
      case 183: //SJ4201212 - #define PROGRAM_SHORT_ADDRESS	0xB7 in special_command.h
        ggeneral_buffer[14] = 0x00;	//payload length i=14
        ggeneral_buffer[15] = 0x14;	//SJ4201212 - equiv 20 i=15

	    ggeneral_buffer[54] = 0x00;  //length in hex i=54
        ggeneral_buffer[55] = 0x14;  //SJ4130912 - THIS NEED TO CHANGE. Reflect the lenght of UDP packet i=55

        //ggeneral_buffer[62] = (gstate_var.Room_ID & 0xFF00) >> 8;//0x00;  //SJ1101212 - Room ID (MSB) i=62
        //ggeneral_buffer[63] = (gstate_var.Room_ID & 0x00FF);//0x00;  //SJ1101212 - Room ID (LSB) i=63

        ggeneral_buffer[64] = 0x00;  //SJ1101212 - short address (MSB) i=64
        ggeneral_buffer[65] = short_address;//0x00;  //SJ4130912 - Dali short address i=65

        //ggeneral_buffer[66] = 0xB7;  //SJ4130912 - #define PROGRAM_SHORT_ADDRESS 0xB7 in special_command.h i=66
        ggeneral_buffer[66] = response_type;  //SJ4130912 - #define PROGRAM_SHORT_ADDRESS 0xB7 in special_command.h i=66
        ggeneral_buffer[67] = short_address;  //SJ4130912 - i=67
        ggeneral_buffer[68] = 0x00;  //SJ4130912 - i=68
        ggeneral_buffer[69] = 0x00;  //SJ4130912 - i=69
        i = 70;  //SJ4130912 -
        break;
  #endif

  #if 1  //SJ4130912 - Used to return random address to user
    //else if (response_type == 194)  //SJ4130912 - #define QUERY_RANDOM_ADDR_H 194 in command.h
      case  13:  //SJ4131212 - #define AZ_SWITCH_IDENTITY 0x0D in command.h
      case 194:  //SJ4130912 - #define QUERY_RANDOM_ADDR_H 194 in command.h
      //{
        ggeneral_buffer[14] = 0x00;	//payload length i=14
        ggeneral_buffer[15] = 0x16;	//SJ4130912 - equiv 18 i=15

	    ggeneral_buffer[54] = 0x00;  //length in hex i=54
        ggeneral_buffer[55] = 0x16;  //SJ4130912 - THIS NEED TO CHANGE. Reflect the lenght of UDP packet i=55

        //ggeneral_buffer[58] = 0x08;	//SJ4130912 - THIS NEED TO CHANGE. Reflect the lenght of data to send 

        //ggeneral_buffer[62] = (gstate_var.Room_ID & 0xFF00) >> 8;//0x00;  //SJ1101212 - Room ID (MSB) i=62
        //ggeneral_buffer[63] = (gstate_var.Room_ID & 0x00FF);//0x00;  //SJ1101212 - Room ID (LSB) i=63

        ggeneral_buffer[64] = 0x00;  //SJ1101212 - short address (MSB) i=64
        ggeneral_buffer[65] = 0x01;//short_address;  //SJ4130912 - Dali short address i=65
        ggeneral_buffer[66] = 0x0D;
        if (response_type == 194)
            ggeneral_buffer[66] = 0x0C;//0xC2;           //SJ4130912 - #define QUERY_RANDOM_ADDR_H 194 defined in command.h i=66

        ggeneral_buffer[67] = random_address_byte_h;  //SJ4130912 - i=67
        ggeneral_buffer[68] = random_address_byte_m;  //SJ4130912 - i=68
        ggeneral_buffer[69] = random_address_byte_l;  //SJ4130912 - i=69
        ggeneral_buffer[70] = device_type;  //SJ1101212 - SJTODO: used device type global var
        i = 71;  //SJ4130912 - Padding
        break;
      //}
  #endif

  #if 0  //SJ1170912 - Used to return grouping to user
//#define GROUPING_QUERY          6   //SJ1170912 - use the macro defined in command.h
    //else if (response_type == 192)  //SJ1170912 - #define QUERY_GROUPS_0_7 192 in command.h
      case 192:  //SJ1170912 - #define QUERY_GROUPS_0_7 192 in command.h
      {
        ggeneral_buffer[14] = 0x00;	//payload length i=14
        ggeneral_buffer[15] = 0x10;	//SJ1170912 - equiv 16 i=15

	    ggeneral_buffer[54] = 0x00;  //length in hex i=54
        ggeneral_buffer[55] = 0x10;  //SJ1170912 - THIS NEED TO CHANGE. Reflect the lenght of UDP packet i=55

        //ggeneral_buffer[58] = 0x08;	//SJ1170912 - THIS NEED TO CHANGE. Reflect the lenght of data to send 

        ggeneral_buffer[62] = short_address;  //SJ1170912 - Dali short address i=62
        ggeneral_buffer[63] = 0xC0;           //SJ1170912 - #define QUERY_GROUPS_0_7 192 defined in command.h i=63

        ggeneral_buffer[64] = group_0_7;  //SJ1170912 - i=64
        ggeneral_buffer[65] = group_8_f;  //SJ1170912 - i=65
        i = 66;
        break;
      }
  #endif

  #if 0  //SJ1170912 - Used to send sensor packet to GUI
    //else if (response_type == SENSOR_DETECTED)  //#define SENSOR_DETECTED 7
      case SENSOR_DETECTED:  //#define SENSOR_DETECTED 7
      {
        ggeneral_buffer[14] = 0x00;	//payload length i=14
        ggeneral_buffer[15] = 0x10;	//SJ1170912 - equiv 14 i=15

	    ggeneral_buffer[54] = 0x00;  //length in hex i=54
        ggeneral_buffer[55] = 0x10;  //SJ1170912 - THIS NEED TO CHANGE. Reflect the lenght of UDP packet i=55

        //ggeneral_buffer[58] = 0x08;	//SJ1170912 - THIS NEED TO CHANGE. Reflect the lenght of data to send 

        ggeneral_buffer[62] = short_address;  //SJ1170912 - Dali short address i=62
        ggeneral_buffer[63] = 0x0D;           //SJ1170912 - #define AZ_SENSOR_DETECT 0x0D defined in command.h i=63
        ggeneral_buffer[64] = (gstate_var.general_ctrl_flag & SENSOR_ON) ? 1 : 0;
        i = 65;  //SJ2180912 - Padding
        break;
      }
  #endif

  #if 0  //SJ3190912 - Use to send scene data for scene query
    //else if (response_type == 176)  //#define QUERY_SCENE_LEVEL_0 176 in command.h
      case 176:  //#define QUERY_SCENE_LEVEL_0 176 in command.h
      {
        ggeneral_buffer[14] = 0x00;	//payload length i=14
        ggeneral_buffer[15] = 0x1E;	//SJ3190912 - equiv 30 i=15

	    ggeneral_buffer[54] = 0x00;  //length in hex i=54
        ggeneral_buffer[55] = 0x1E;  //SJ3190912 - THIS NEED TO CHANGE. Reflect the lenght of UDP packet i=55

        //ggeneral_buffer[58] = 0x08;	//SJ3190912 - THIS NEED TO CHANGE. Reflect the lenght of data to send 

        ggeneral_buffer[62] = short_address;  //SJ3190912 - Dali short address i=62
        //ggeneral_buffer[63] = 0xB0;           //SJ3190912 - #define QUERY_SCENE_LEVEL_0 176 defined in command.h i=63

        ggeneral_buffer[64] = scene[0];  //SJ3190912 - Scene 1 - //unsigned char scene[16]; define in dali.c
        ggeneral_buffer[65] = scene[1];  //SJ3190912 - Scene 2
        ggeneral_buffer[66] = scene[2];  //SJ3190912 - Scene 3
        ggeneral_buffer[67] = scene[3];  //SJ3190912 - Scene 4
        ggeneral_buffer[68] = scene[4];  //SJ3190912 - Scene 5
        ggeneral_buffer[69] = scene[5];  //SJ3190912 - Scene 6
        ggeneral_buffer[70] = scene[6];  //SJ3190912 - Scene 7
        ggeneral_buffer[71] = scene[7];  //SJ3190912 - Scene 8
        ggeneral_buffer[72] = scene[8];  //SJ3190912 - Scene 9
        ggeneral_buffer[73] = scene[9];  //SJ3190912 - Scene 10
        ggeneral_buffer[74] = scene[10];  //SJ3190912 - Scene 11
        ggeneral_buffer[75] = scene[11];  //SJ3190912 - Scene 12
        ggeneral_buffer[76] = scene[12];  //SJ3190912 - Scene 13
        ggeneral_buffer[77] = scene[13];  //SJ3190912 - Scene 14
        ggeneral_buffer[78] = scene[14];  //SJ3190912 - Scene 15
        ggeneral_buffer[79] = scene[15];  //SJ3190912 - Scene 16
        i = 80;  //SJ2180912 - Padding
        break;
      }
  #endif

  #if 0  //SJ4041012 - Test broadcast from service node
    //else if (response_type == BROADCAST_FROM_SN)  //#define BROADCAST_FROM_SN 8 in command.h
      case BROADCAST_FROM_SN:  //#define BROADCAST_FROM_SN 8 in command.h
      {
        ggeneral_buffer[8] = 0xEE;  //SJ4160812 - NSDU handle i=8
        ggeneral_buffer[14] = 0x00;	//payload length i=14
        ggeneral_buffer[15] = 0x0E;	//SJ3190912 - equiv 30 i=15

	    ggeneral_buffer[54] = 0x00;  //length in hex i=54
        ggeneral_buffer[55] = 0x0E;  //SJ3190912 - THIS NEED TO CHANGE. Reflect the lenght of UDP packet i=55

        //ggeneral_buffer[58] = 0x08;	//SJ3190912 - THIS NEED TO CHANGE. Reflect the lenght of data to send 
        ggeneral_buffer[62] = 0xff;  //SJ3190912 - Dali short address i=62
        ggeneral_buffer[63] = gstate_var.broadcast_mode;           //SJ3190912 - #define QUERY_SCENE_LEVEL_0 176 defined in command.h i=63
        i = 64;  //SJ2180912 - Padding
        break;
      }
  #endif
    }  //SJ4131212 - switch (response_type) { ... }
//--- SJ4131212 - End of switch ---

    //i = 64;
    //k = 60;  //i - 4;

    //if ((i % 2) == 1)  //SJ3010812 - Padding required
    if ((i % 2))  //SJ3010812 - Padding required
    {
        ggeneral_buffer[i++] = 0x00;
        k = i - 5;  //SJ3010812 - Substract the first four bytes and padding byte.
    }
    else
        k = i - 4;  //SJ3010812 - Substract the first four bytes.

    ggeneral_buffer[2] = (k & 0x00FF);  //SJ3010812 - Msg payload length LSB; Total payload length=64
    ggeneral_buffer[3] = (k >> 8);  

  #if 1  //SJ1101212 - If command if 94, then generate delay.
    if (response_type == 194)
    {
    //gstate_var.general_delay = 16;  //SJ4160812 - 800 msec
    gstate_var.general_delay = gstate_var.DALI_response;
    while (gstate_var.general_delay) ;
    }
  #endif

    temp_msg_type = ggeneral_buffer[MSG_HEADER_TYPE];

    //SJ3010812 - Transmit packet to PLC device
    for (j=0; j<i; j++)
    {
        while (!(IFG2 & UCA0TXIFG));  // USCI_A0 TX buffer ready?
        UCA0TXBUF = ggeneral_buffer[j];  //0xA5
//        ggeneral_buffer[i] = 0xFF;  //SJ4190712 - Empty buffer
    }

    gstate_var.UART_Rx_State = UART_STANDBY;  //SJ1300712 - New method 
    gstate_var.UART_Rx_duration = TIMER_2_SECONDS;

    while (gstate_var.UART_Rx_duration)  //SJ3180712 - Wait for 2 seconds for UART_Rx to complete
    {
        if (gstate_var.UART_Rx_State == UART_PROCESSING)  //SJ1300712 - New method
        {
//SJ1100912            gstate_var.UART_Rx_State = UART_INVALID;  //SJ1300712 - New method

            temp = (ggeneral_buffer[MESSAGE_HEADER_LEN+1] << 8) + ggeneral_buffer[MESSAGE_HEADER_LEN];

            //SJ3180712 - If msg type return from PLC device does not match the msg type sent by host, 
            //SJ3180712 - or reply status not equal 0x0000, raise error flag
            if ((ggeneral_buffer[MSG_HEADER_TYPE] == temp_msg_type) && (temp == MSG_REQUEST_OK))
            {
                //ret_flag = SJ_PASS;
//SJ4201212                if (response_type == REVEAL_DALI_ADDRESS)  //SJ4130912 - Not used at the moment
//SJ4201212                    gstate_var.general_ctrl_flag |= ATTACH_ON;
                ;
            }

            //gstate_var.UART_Rx_ndx = MESSAGE_HEADER_LEN+1;  //SJ3180712 - Prevent UART_Rx ISR from setting UART_Rx_COMPLETE
            gstate_var.UART_Rx_duration = 0;
            gstate_var.UART_Rx_State = UART_STANDBY;  //SJ1100912 - 
        }
    }

  #ifdef RED_N_GREEN_LED
    //SJ2091012 - Stop blinking
    gstate_var.general_ctrl_flag &= GRN_LED_BLINK_OFF;
    //gstate_var.green_led_timer = 0;  //SJ3191212 - ORG
    gstate_var.led_blink_freq = 0;
    P1OUT |= ON_GREEN_LED;  //SJ4061212 - Ensure green LED is on in the event of out of sync
  #endif

}  //SJ3010812 - End of AZ_DALI_Response()


#if 0  //SJ3010812 - Don't delete first
//SJ4120712 - This routine pumps data out via UART transmit channel.
char UART_TX_A0_Routine()
{
    //while (UC0IFG & UCA0TXIFG)  //SJ2240810 - ORG
    while (!(IFG2 & UCA0TXIFG));  // USCI_A0 TX buffer ready?
    UCA0TXBUF = 0xAA;//gstate_var.timer_ndx;

    return (0xFF);

}  //SJ4120712 - End of UART_TX_A0_Routine() routine
#endif


//--- End of file (PLC_Lite.c) --------------------------------------------------------------------

