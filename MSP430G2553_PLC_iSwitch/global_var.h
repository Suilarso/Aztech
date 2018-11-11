
/***********************************************************************/
/*                                                                     */
/*  FILE        : global_var.h                                         */
/*  DATE        : Fri, July 06, 2012                                   */
/*  DESCRIPTION : Global variables definition                          */
/*  CPU TYPE    : MSP430G2553                                          */
/*  PROJECT     : PLC Light                                            */
/*  AUTHOR      : Suilarso Japit                                       */
/*  VERSION     : 100.00                                               */
/*                                                                     */
/***********************************************************************/

#ifndef __GLB_VAR_H
#define __GLB_VAR_H


//#define  WORK_IN_PROGRESS
#define  WITH_WDT
//#define  FLASH_COMPONENT  //SJ5300911 - Disabled
#define  WITH_PLC   //SJ5070912 - The purpose of this macro definition is to facilitate ease of running DALI code. Just remark this to run dali.
#define  SWITCH_CONTROLLER  //SJ1171212 - SJNOTE: !!!!! REMEMBER TYPE_INDICATOR !!!!!

//Macro definitions for miscellaneous
//#define  REMOTE_CONTROL_ENABLE 1
#ifdef REMOTE_CONTROL_ENABLE
//    #define  LONG_PRESSED_BUILD  1  //SJ1040411 - Skip this feature first. DO NOT REMOVE
#endif

//SJ4190712 - General buffer size
#define DATA_BUFFER_SZ     100
#define SWITCH_OPER_SZ      64

#define ISR_RISING_EDGE    0
#define ISR_FALLING_EDGE   1


//SJ5210510 - General representation signal
#define LOW_SIGNAL_CAP     2//3   //SJ320710 - Around 100msec
#define LOW_SIGNAL         0
#define HIGH_SIGNAL        1
#define OFF_STATE          0
#define ON_STATE           1
#define SJ_FAIL            0
#define SJ_PASS            1


//Macro definitions for ???


#define OPER_MODE_NONE          0
#define OPER_MODE_1_1          11
#define OPER_MODE_1_2          12
#define OPER_MODE_1_3          13
#define OPER_MODE_1_4          14
#define OPER_MODE_2_1          21
#define OPER_MODE_2_2          22
#define OPER_MODE_2_3          23
#define OPER_MODE_2_4          24
#define OPER_MODE_2_9          25   //SJ5281011 - This should be the last mode.
#define OPER_MODE_INVALID    0xFF


#define UART_STANDBY       0
#define UART_RECEIVING     1
#define UART_PROCESSING    2
#define UART_INVALID    0xFF


//SJ5130712 - Macroes definition for Flash. NOTE: If possible move it to flash.h file

#define INFO_MEM_SEG_D_ADDR    0x1000
#define INFO_MEM_SEG_C_ADDR    0x1040
#define INFO_MEM_SEG_B_ADDR    0x1080
#define INFO_MEM_SEG_A_ADDR    0x10C0

//SJ3190912 - Info C flash location offset to PLC long address and CRC checksum.
#define PLC_LONG_ADDR_8_LOC   0   //SJ3190912 - PLC G3 long address byte 0
#define PLC_LONG_ADDR_7_LOC   1
#define PLC_LONG_ADDR_6_LOC   2
#define PLC_LONG_ADDR_5_LOC   3
#define PLC_LONG_ADDR_4_LOC   4
#define PLC_LONG_ADDR_3_LOC   5   //SJ3190912 - PLC G3 long address byte 5
#define PLC_LONG_ADDR_2_LOC   6   //SJ3190912 - Future use for expansion DALI short address from 1 byte to 2 bytes
#define PLC_LONG_ADDR_1_LOC   7   //SJ3190912 - Contains DALI short address
#define CRC_CHECKSUM_MSB      8   //SJ3190912 - CRC checksum MSB
#define CRC_CHECKSUM_LSB      9   //SJ3190912 - CRC checksum LSB
#if 0  //SJ1171212 - ORG. Used in lighting project
#define INFO_C_LAST_LOC      10
#else  //SJ2111212 - Replaced. Additional info needed for i_Switch project
#define DEVICE_TYPE_LOC      10
#define ROOM_ID_MSB_LOC      11
#define ROOM_ID_LSB_LOC      12
#define PRODUCT_ID_LOC       13
#define RELEASE_CODE_LOC     14
#define VERSION_CODE_LOC     15
#define INFO_C_LAST_LOC      16
#endif

//SJ1081012 - GPIO pins mapping
#define RED_N_GREEN_LED
#define ON_RED_LED   0x08   //SJ1081012 - Red LED on, P1.3 (00001000)
#define OFF_RED_LED  0xF7   //SJ1081012 - Red LED off, P1.3 (11110111)
#define ON_GREEN_LED  0x20  //SJ1081012 - Green LED on, P1.5 (00100000)
#define OFF_GREEN_LED 0xDF  //SJ1081012 - Green LED off, P1.5 (11011111)

//#define BLINK_FREQ

//SJ2160310 - Below macroes control LED on duration; the values need to be adjusted accordingly.
#define  TIMER_SZ                6//7
#define  TIMER_DEFAULT_VAL       4  //SJ4300611 3  //SJ3220611 1

//SJ1050710 - TAR register increment or decrement at every rising edge of the clock signal.
//SJ3110712 - With SMCLK = 1 MHz and TIMER_CONST = 50000, 1000000 / 50000 = 20 occurences each second.
//SJ3110712 - As such the ISR will be serviced 20 times each second.
//SJ3110712 - Which is also equivalent to 1000000 (1 sec) / 20 times = 50000 (50 millisecond)

//#define  TIMER_CONST             0x4E20  //SJ3110712 - 20000 in base 10. Will get 20ms (50 cyclse per sec)
//#define  TIMER_CONST             0x61A8  //SJ3110712 - 25000 in base 10. Will get 25ms (40 cycles per sec)
#define  TIMER_CONST             0xC350  //SJ3110712 - 50000 in base 10. Will get 50ms (20 cycles per sec)
//#define  SUB_TIMER_CONST         0xA028  //SJ2040111 - 2000 in base 10.
//#define  RC_TIMER_CONST          0xC350  //SJ3040511 - 50000 in base 10. Will get 400ms @ 125KHz


#define  LOOP_PER_SECOND         20

#define  TIMER_1_SECOND         (LOOP_PER_SECOND)
#define  TIMER_2_SECONDS        (LOOP_PER_SECOND * 2)     //SJ2030810 - 40
#define  TIMER_3_SECONDS        (LOOP_PER_SECOND * 3)     //SJ5300710 - 60
#define  TIMER_4_SECONDS        (LOOP_PER_SECOND * 4)     //SJ5300710 - 80
#define  TIMER_5_SECONDS        (LOOP_PER_SECOND * 5)     //SJ5300710 - 100
#define  TIMER_6_SECONDS        (LOOP_PER_SECOND * 6)     //SJ4250712 - 120
#define  TIMER_8_SECONDS        (LOOP_PER_SECOND * 8)     //SJ4190712 - 160
#define  TIMER_10_SECONDS       (LOOP_PER_SECOND * 10)    //SJ5300710 - 200
#define  TIMER_20_SECONDS       (LOOP_PER_SECOND * 20)    //SJ4041012 - 400
#define  TIMER_30_SECONDS       (LOOP_PER_SECOND * 30)    //SJ5300710 - 600
#define  TIMER_45_SECONDS       (LOOP_PER_SECOND * 45)    //SJ5300710 - 900
#define  TIMER_1_MINUTE         (LOOP_PER_SECOND * 60)    //SJ5300710 - 1200
#define  TIMER_2_MINUTES        (LOOP_PER_SECOND * 120)   //SJ5270712 - 2400
#define  TIMER_5_MINUTES        (LOOP_PER_SECOND * 300)   //SJ5300710 - 6000
#define  TIMER_10_MINUTES       (LOOP_PER_SECOND * 600)   //SJ5300710 - 12000
#define  TIMER_15_MINUTES       (LOOP_PER_SECOND * 900)   //SJ5300710 - 18000
#define  TIMER_30_MINUTES       (LOOP_PER_SECOND * 1800)  //SJ5300710 - 36000
#define  TIMER_1_HOUR           (LOOP_PER_SECOND * 3600)  //SJ5300710 - 72000
//#define  LED_ON_2_HOURS          (LOOP_PER_SECOND * 7200)
//#define  LED_ON_2_HOURS          0x23280   //SJ4050810 - equiv 144000
//#define  LED_ON_4_HOURS          0x46500   //SJ5090710 - equiv 288000
//#define  LED_ON_6_HOURS          0x69780   //SJ4050810 - equiv 432000
//#define  LED_ON_8_HOURS          0x8CA00   //SJ3210711 - equiv 576000

#define  TIMER_1000_MSECONDS    (20)  //(LOOP_PER_SECOND)       //SJ5281011 - 1 Second
#define  TIMER_1500_MSECONDS    (30)  //(LOOP_PER_SECOND+10)       //SJ5281011 - 1.5 Seconds
#define  TIMER_2000_MSECONDS    (40)  //(LOOP_PER_SECOND * 2)   //SJ5281011 - 2 Seconds


//SJ3290812 - Macro for delay during Network initialisation.
#define  NETWORK_DELAY_CONST      16  //SJ3290812 - 16 is equivalent to 16 * 50 = 800milliseconds

#define  LED_POWER_OFF           120


//SJ5300911 - Below macroes are for Battery and Solar voltage
#define CONST_0_70_VOLT      477  //SJ5300312 - (0.70 / 1.50) * 1023 = 477.4
#define CONST_0_24_VOLT      164  //SJ5300312 - (0.24 / 1.50) * 1023 = 163.68
#define CONST_0_50_VOLT      341  //SJ5300312 - (0.50 / 1.50) * 1023 = 341
#define CONST_1_20_VOLT      818  //SJ5300312 - (1.20 / 1.50) * 1023 = 818.4
#define CONST_0_84_VOLT      573  //SJ5300312 - (0.84 / 1.50) * 1023 = 572.88

#define VOLTAGE_SAMPLE    10
//#define CURRENT_SAMPLE    10  //SJ1030912 - ORG
#define CURRENT_SAMPLE    50  //30


//SJ5160710 - Below macroes are for various LED blinking frequency
#define BLINK_FREQ_50_MS      1
#define BLINK_FREQ_100_MS     2
#define BLINK_FREQ_150_MS     3
#define BLINK_FREQ_250_MS     5
#define BLINK_FREQ_500_MS    10
#define BLINK_FREQ_750_MS    15
#define BLINK_FREQ_1000_MS   (LOOP_PER_SECOND)  //20
#define BLINK_FREQ_2000_MS   (LOOP_PER_SECOND * 2)  //40

#define LED_BLINK_FREQ       BLINK_FREQ_50_MS

//SJ5160410 - Below macroes for ambience

//SJ2160310 - Below macroes control twilight value; value need to be adjusted accordingly


//Macro definition for various ADC allotted read time.
//#define  ADC_POLLING_CONST       80
#define  ADC_BINARY                  0
#define  ADC_TWO_COMPLEMENT          1

//SJ3110712 - Macroes definition for PWM.
#define  PWM_SZ               4  //6  //SJ3180810 - Change to four options
//SJ4220710 - Timer0 clock speed = 8MHz / 8 ==> 1MHz.
//SJ4220710 - To set timer register to achieve 5KHz: 1000000 / 5000 ==> 200
//SJ4220710 - 1000000 / 1000 ==> 1000
//SJ4220710 - To set timer register to achieve 100Hz: 1000000 / 100 ==> 10000
//#defein  PWM_PERIOD               12500  //SJ2140910 - To achieve 80Hz.
//#define  PWM_PERIOD               10000  //SJ3080910 - To achieve 100Hz.
//#define  PWM_PERIOD                8000  //SJ2140910 - To achieve 125Hz.
//#define  PWM_PERIOD                6250  //SJ2140910 - To achieve 160Hz.
//#define  PWM_PERIOD                5000  //SJ1130910 - To achieve 200Hz.
//#define  PWM_PERIOD                4000  //SJ4220710 - To achieve 250Hz.
//#define  PWM_PERIOD                2000  //SJ4220710 - To achieve 500Hz.
//#define  PWM_PERIOD                1000  //SJ4220710 - To achieve 1KHz.
//#define  PWM_PERIOD                 200  //SJ3210710 - To achieve 5KHz.
//#define  PWM_PERIOD                 100  //SJ3210710 - To achieve 10KHz.

/*****  //SJ5240910 - 50Hz
#define  PWM_PERIOD               20000  //SJ5240910 - To achieve 50Hz.
  //SJ3290910 - For 0 percentage it is curcial to place 5 instead of 0. This is to allow 
  //SJ3290910 - the port ISR to be able to capture the rising edge.
  #define  PWM_0_PERCENT                 5   //SJ5240910 - Off
  #define  PWM_10_PERCENT            (2000)
  #define  PWM_25_PERCENT            (5000)
  #define  PWM_30_PERCENT            (6000)
  #define  PWM_40_PERCENT            (8000)
  #define  PWM_45_PERCENT            (9000)
  #define  PWM_50_PERCENT           (10000)
  #define  PWM_60_PERCENT           (12000)
  #define  PWM_75_PERCENT           (15000)
  #define  PWM_80_PERCENT           (16000)
  #define  PWM_100_PERCENT          (20000)  //SJ5240910 - On
*****/
/*****  //SJ2140910 - 80Hz
#define  PWM_PERIOD               12500  //SJ2140910 - To achieve 80Hz.
  #define  PWM_0_PERCENT                 0   //SJ3080910 - Off
  #define  PWM_10_PERCENT            (1250)
  #define  PWM_25_PERCENT            (3125)
  #define  PWM_30_PERCENT            (3750)
  #define  PWM_40_PERCENT            (5000)
  #define  PWM_45_PERCENT            (5625)
  #define  PWM_50_PERCENT            (6250)
  #define  PWM_60_PERCENT            (7500)
  #define  PWM_75_PERCENT            (9375)
  #define  PWM_80_PERCENT           (10000)
  #define  PWM_100_PERCENT          (12500)  //SJ3080910 - On
*****/
/*****  //SJ3130411 - 100Hz
#define  PWM_PERIOD               10000  //SJ3130411 - To achieve 100Hz.
  #define  PWM_0_PERCENT                0   //SJ3130411 - Off
  #define  PWM_10_PERCENT            1000   //SJ3130411 - 1ms on, 9ms off
  #define  PWM_25_PERCENT            2500   //SJ3130411 - 2.5ms on, 7.5ms off
  #define  PWM_30_PERCENT            3000   //SJ3130411 - 3ms on, 7ms off
  #define  PWM_40_PERCENT            4000   //SJ3130411 - 4ms on, 6ms off
  #define  PWM_45_PERCENT            4500   //SJ3130411 - 4.5ms on, 5.5ms off
  #define  PWM_50_PERCENT            5000   //SJ3130411 - 5ms on, 5ms off
  #define  PWM_60_PERCENT            6000   //SJ3130411 - 6ms on, 4ms off
  #define  PWM_70_PERCENT            7000   //SJ3130411 - 7ms on, 3ms off
  #define  PWM_75_PERCENT            7500   //SJ3130411 - 7.5ms on, 2.5ms off
  #define  PWM_80_PERCENT            8000   //SJ3130411 - 8ms on, 2ms off
  #define  PWM_85_PERCENT            8500   //SJ3130411 - 8.5ms on, 1.5ms off
  #define  PWM_90_PERCENT            9000   //SJ3130411 - 9ms on, 1ms off
  #define  PWM_100_PERCENT          10000   //SJ3130411 - On      //SJ3220910 - 10ms on, 0ms off
*****/
/*****  //SJ2140910 - 125Hz
#define  PWM_PERIOD               8000  //SJ2140910 - To achieve 125Hz.
  #define  PWM_0_PERCENT                 0   //SJ3080910 - Off
  #define  PWM_10_PERCENT             (800)
  #define  PWM_25_PERCENT            (2000)
  #define  PWM_30_PERCENT            (2400)
  #define  PWM_40_PERCENT            (3200)
  #define  PWM_45_PERCENT            (3600)
  #define  PWM_50_PERCENT            (4000)
  #define  PWM_60_PERCENT            (4800)
  #define  PWM_75_PERCENT            (6000)
  #define  PWM_80_PERCENT            (6400)
  #define  PWM_100_PERCENT           (8000)  //SJ3080910 - On
*****/
/*****  //SJ2140910 - 160Hz
#define  PWM_PERIOD               6250  //SJ2140910 - To achieve 160Hz.
  #define  PWM_0_PERCENT                 0   //SJ3080910 - Off
  #define  PWM_10_PERCENT             (625)
  #define  PWM_25_PERCENT            (1562)//?
  #define  PWM_30_PERCENT            (1875)
  #define  PWM_40_PERCENT            (2500)
  #define  PWM_45_PERCENT            (2812)//?
  #define  PWM_50_PERCENT            (3125)
  #define  PWM_60_PERCENT            (3750)
  #define  PWM_75_PERCENT            (4687)//?
  #define  PWM_80_PERCENT            (5000)
  #define  PWM_100_PERCENT           (6250)  //SJ3080910 - On
*****/
//*****  //SJ3080812 - 200Hz
#define  PWM_PERIOD               5000  //SJ3080812 - To achieve 200Hz.
  #define  PWM_0_PERCENT                0   //SJ3130411 - Off
  #define  PWM_10_PERCENT             500   //SJ3130411 - 1ms on, 9ms off
  #define  PWM_20_PERCENT            1000   //SJ3130411 - 1ms on, 9ms off
  #define  PWM_25_PERCENT            1250   //SJ3130411 - 2.5ms on, 7.5ms off
  #define  PWM_30_PERCENT            1500   //SJ3130411 - 3ms on, 7ms off
  #define  PWM_40_PERCENT            2000   //SJ3130411 - 4ms on, 6ms off
  #define  PWM_45_PERCENT            2250   //SJ3130411 - 4.5ms on, 5.5ms off
  #define  PWM_50_PERCENT            2500   //SJ3130411 - 5ms on, 5ms off
  #define  PWM_60_PERCENT            3000   //SJ3130411 - 6ms on, 4ms off
  #define  PWM_70_PERCENT            3500   //SJ3130411 - 7ms on, 3ms off
  #define  PWM_75_PERCENT            3750   //SJ3130411 - 7.5ms on, 2.5ms off
  #define  PWM_80_PERCENT            4000   //SJ3130411 - 8ms on, 2ms off
  #define  PWM_85_PERCENT            4250   //SJ3130411 - 8.5ms on, 1.5ms off
  #define  PWM_90_PERCENT            4500   //SJ3130411 - 9ms on, 1ms off
  #define  PWM_91_PERCENT            4550
  #define  PWM_92_PERCENT            4600
  #define  PWM_93_PERCENT            4650
  #define  PWM_94_PERCENT            4700
  #define  PWM_95_PERCENT            4750
  #define  PWM_96_PERCENT            4800
  #define  PWM_97_PERCENT            4850
  #define  PWM_98_PERCENT            4900
  #define  PWM_99_PERCENT            4950
  #define  PWM_100_PERCENT           5000   //SJ3130411 - On      //SJ3220910 - 10ms on, 0ms off
/*****/


#define PWM_PERIOD_REG  TA1CCR0
#define DUTY_CYCLE_REG  TA1CCR1 

//(TA1CCR0);  //Timer1_A3 Capture/Compare 0
//(TA1CCR1);  //Timer1_A3 Capture/Compare 1
//(TA1CCR2);  //Timer1_A3 Capture/Compare 0

//#define STEP_CONSTANT    2000  //SJ5240910 - ORG. Used with 100Hz
//#define STEP_CONSTANT    3000  //SJ3080910 - Replaced. Use with 50Hz
#define STEP_CONSTANT    5000  //SJ3080910 - Replaced. Use with 50Hz
#define UP_DIRECTION     0
#define DOWN_DIRECTION   1

//SJ5210510 - Macroes for AC
#define  AC_CONSTANT     50
#define  AC_DEFAULT     400

//System default setting


//SJ1040411 - System operational mode


//State
#define  SWITCH_OFF       0  //SJ3030310 - 
#define  SWITCH_STANDBY   1  //SJ3030310 - 
#define  SWITCH_ON        2  //SJ3030310 - Used to indicate that the LED light can be switched on

//SJ4140110 - Macro for motion.



//SJ4140110 - General Controller flag
#define  LED_BIT       0x00   //SJ2271112 - Bit 0 is used for controlling LED on / off
#define  LED_ON        0x01   //SJ2271112 - 00000001
#define  LED_OFF       0xFE   //SJ2271112 - 11111110
#define  RED_LED_BLINK_BIT         0x02   //SJ1081012 - Bit 1 is used to control red LED's blinking
#define  RED_LED_BLINK_ON          0x02   //SJ1081012 - 00000010
#define  RED_LED_BLINK_OFF         0xFD   //SJ1081012 - 11111101
#define  BATT_MONITOR_BIT   0x04   //SJ5281011 - Bit 2 is used for battery monitoring
#define  BATT_MONITOR_ON    0x04   //SJ5281011 - 00000100
#define  BATT_MONITOR_OFF   0xFB   //SJ5281011 - 11111011
#define  NEW_DATA_BIT    0x08   //SJ4140110 - Bit 3 is used to indicate new ADC data read
#define  NEW_DATA_ON     0x08   //SJ4140110 - 00001000
#define  NEW_DATA_OFF    0xF7   //SJ4140110 - 11110111
#define  ATTACH_BIT      0x10   //SJ3220812 - Bit 4 is used as device attachment bit
#define  ATTACH_ON       0x10   //SJ3220812 - 00010000
#define  ATTACH_OFF      0xEF   //SJ3220812 - 11101111
#define  GRN_LED_BLINK_BIT   0x20   //SJ1081012 - Bit 5 is used to control green LED's blinking
#define  GRN_LED_BLINK_ON    0x20   //SJ1081012 - 00100000
#define  GRN_LED_BLINK_OFF   0xDF   //SJ1081012 - 11011111
#define  SENSOR_BIT      0x40   //SJ2180912 - Bit 6 is used to indicate the high state of the sensor
#define  SENSOR_ON       0x40   //SJ2180912 - 01000000
#define  SENSOR_OFF      0xBF   //SJ2180912 - 10111111
#define  SYS_RESET_BIT     0x80   //SJ5040113 - Bit 7 is used to indicate sytem reset is activated
#define  SYS_RESET_ON      0x80   //SJ5040113 - 10000000
#define  SYS_RESET_OFF     0x7F   //SJ5040113 - 01111111


//Direction

//Misc
#define   AMB_LIGHT_REQUEST      (0xAA)
#define   PIR_DATA_REQUEST       (0xBB)
#define   DIAL_DATA_REQUEST      (0xCC)  //SJ5011010 - BEWARE of Inter-dependency

//Macro definition for LED controller


typedef struct field_template
{
    union
    {
        unsigned char BYTE;               /*  Byte Access */
        struct
        {                                 /*  Bit  Access */
            unsigned char B0:1;           /*    Bit 7     */
            unsigned char B1:1;           /*    Bit 6     */
            unsigned char B2:1;           /*    Bit 5     */
            unsigned char B3:1;           /*    Bit 4     */
            unsigned char B4:1;           /*    Bit 3     */
            unsigned char B5:1;           /*    Bit 2     */
            unsigned char B6:1;           /*    Bit 1     */
            unsigned char B7:1;           /*    Bit 0     */
        }  BIT;
    } com_field;

} FIELD_STRUCT;

typedef struct tagREMOTE_CTRL
{
	unsigned int header;
	unsigned int CustomCode;
	unsigned int DataCode;
	
} REMOTE_CTRL_STRUCT;


typedef struct tagIR_CODE
{
    union
    {
        unsigned long ir_bitStream;   // ir bitstream
        struct
        {
          #if 1
            unsigned short custCode;    // custom code
            unsigned char  d_Code;      // data code
            unsigned char  d_Code_n;    // inverted data code
          #else
            unsigned char  d_Code_n;    // inverted data code
            unsigned char  d_Code;      // data code
            unsigned short custCode;    // custom code
          #endif
        }decompData;
    }un;

    unsigned char irValid;
    unsigned char irRepeat;
    //BYTE irSensor;  // indicate signal from Main or Sub sensor.
} IR_CODE_STRUCT;

//irCode, temp_irCode;


typedef struct tagSTATE_STRUCT
{
    //unsigned char oper_mode;   //SJ5080411 - May not use in LF2xx
    unsigned char PLC_status;
    unsigned char general_ctrl_flag;  //SJ4090812 - Don't think is used

    unsigned char general_delay;  //SJ1040411 - Rename below var.
    unsigned char error_delay;  //SJ2110912 - Gap between on off error simulation
    unsigned char UART_Rx_ndx;
    unsigned char UART_Rx_State;  //SJ1300712 - Standby, Receiving, Processing, and Invalid
    //unsigned char UART_Rx_timeout;  //SJ2280812 - DO NOT REMOVE

    unsigned char DALI_response;  //SJ3121212 - In iSwitch project it is used as delay in sending randomised address.
    //unsigned char sensor_state;  //SJ3121212 - Not used in iSwitch project
  #ifdef RED_N_GREEN_LED
    //unsigned char red_led_timer;
    //unsigned char green_led_timer;
    unsigned char led_blink_freq;  //SJ3191212 - Introduce to replace above two members.
  #endif
  #if 0  //SJ4041012 - Testing broadcast from service node
    unsigned char broadcast_mode;
    unsigned int broadcast_delay;
  #endif
    //SJ5210510 - Use below for 250 msec
    unsigned int  led_timer;    //SJ3121212 - In iSwitch project this var is used as key debounce timing
    //unsigned int  led_off_time;
    //SJ5210510 - Use below for 125 msec
    //SJ1031011 unsigned long led_timer;            //SJ3161105 Possible to use char if timer constant increased
    //SJ1031011 unsigned long led_off_time;  //SJ1130910 - Will be used in debug mode.
    //unsigned int flash_update_countdown;  //SJ2110912 - Keep track of flash update time. //SJ3191212 - KIV

    unsigned int UART_Rx_threshold;  //SJ4291112 - Solving out of sync state
    unsigned int UART_Rx_duration;
    unsigned int Rx_payload_len;
    //unsigned int PAN_ID;  //SJ3191212 - Not used so far. DO NOT REMOVE
    //unsigned int coord_addr;  //SJ4260712 - DO NOT REMOVE
    //unsigned int network_addr;  //SJ3191212 - Not used. Can be replaced with local var in PLC_Network_Config() function
    unsigned int Room_ID;
    unsigned int prev_key;  //SJ3191212 - Introduce to allow toggle function of the same key

    unsigned int LED_current;  //SJ5300911
    unsigned int LED_voltage;   //SJ5300911
    //int cum_current;
    //int cum_voltage;

    //int blink_freq;  //SJ5060810 - LED blinking for slave mode

    //SJ3121212 - Below member is not used in iSwitch project
    //unsigned long sensor_signal;  //SJ2180912 - Monitoring signal at P2.5. If continously high for 1600msec (1.6 sec), to be considered high

    //FIELD_STRUCT device_state_flag;
    //FIELD_STRUCT GEN_ctrl_flag;  //SJ2170712 - Replaced with general_ctrl_flag

}  STATE_STRUCT;


#endif // __GLB_VAR_H
