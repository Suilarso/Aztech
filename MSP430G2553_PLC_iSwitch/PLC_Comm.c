/******************************************************************************/
/*                                                                            */
/*  FILE        : PLC_Comm.c                                                  */
/*  DATE        : Thur, July 12, 2012                                         */
/*  DESCRIPTION : PLC communication routines                                  */
/*  CPU TYPE    : MSP430G2553                                                 */
/*  PROJECT     : PLC Light                                                   */
/*  AUTHOR      : Suilarso Japit                                              */
/*  VERSION     : 100.00                                                      */
/*                                                                            */
/******************************************************************************/


//#include   <msp430x23x2.h>    //<msp430x21x2.h>
#include   <msp430G2553.h>
#include   <string.h>  //SJ2131211 - for memmove() function prototype

#include   "PLC_Comm.h"
#include   "global_var.h"
//#include   "User_Input.h"
//#include   "TAS3103_util.h"


//=== SJ1031011 - External global var =========================================
//main.c
extern unsigned char ggeneral_buffer[DATA_BUFFER_SZ];  //[50];
//extern int gvoltage_tab[VOLTAGE_SAMPLE];
//extern int gcurrent_tab[CURRENT_SAMPLE];

extern STATE_STRUCT  gstate_var;

//extern int gADC_data[10];
//extern short bit_counter;


//dali.c
extern unsigned char short_address;

//ISR_Routines.c
//=== SJ1031011 - END. External global var ====================================


//=== SJ1031011 - External function prototype =================================
//main.c
extern int RetrieveFromADC(int data_format);

//PLC_Lite.c
//extern void AZ_DALI_Response(int response_type);

//user_input.c
//=== SJ1031011 - END. External function prototype ============================


//__interrupt void Timer_A(void);  //SJ4120106 - Used in Code composer essentials
//__interrupt void ISR_Port1(void);
//__interrupt void ISR_ADC10(void);



//static unsigned short crctable[256] = {  //SJ4130912 - ORG
const unsigned short crctable[256] = {  //SJ4130912 - Replaced
  0x0000, 0x1021, 0x2042, 0x3063, 0x4084, 0x50a5, 0x60c6, 0x70e7, 
  0x8108, 0x9129, 0xa14a, 0xb16b, 0xc18c, 0xd1ad, 0xe1ce, 0xf1ef, 
  0x1231, 0x0210, 0x3273, 0x2252, 0x52b5, 0x4294, 0x72f7, 0x62d6, 
  0x9339, 0x8318, 0xb37b, 0xa35a, 0xd3bd, 0xc39c, 0xf3ff, 0xe3de, 
  0x2462, 0x3443, 0x0420, 0x1401, 0x64e6, 0x74c7, 0x44a4, 0x5485, 
  0xa56a, 0xb54b, 0x8528, 0x9509, 0xe5ee, 0xf5cf, 0xc5ac, 0xd58d, 
  0x3653, 0x2672, 0x1611, 0x0630, 0x76d7, 0x66f6, 0x5695, 0x46b4, 
  0xb75b, 0xa77a, 0x9719, 0x8738, 0xf7df, 0xe7fe, 0xd79d, 0xc7bc, 
  0x48c4, 0x58e5, 0x6886, 0x78a7, 0x0840, 0x1861, 0x2802, 0x3823, 
  0xc9cc, 0xd9ed, 0xe98e, 0xf9af, 0x8948, 0x9969, 0xa90a, 0xb92b, 
  0x5af5, 0x4ad4, 0x7ab7, 0x6a96, 0x1a71, 0x0a50, 0x3a33, 0x2a12, 
  0xdbfd, 0xcbdc, 0xfbbf, 0xeb9e, 0x9b79, 0x8b58, 0xbb3b, 0xab1a, 
  0x6ca6, 0x7c87, 0x4ce4, 0x5cc5, 0x2c22, 0x3c03, 0x0c60, 0x1c41, 
  0xedae, 0xfd8f, 0xcdec, 0xddcd, 0xad2a, 0xbd0b, 0x8d68, 0x9d49, 
  0x7e97, 0x6eb6, 0x5ed5, 0x4ef4, 0x3e13, 0x2e32, 0x1e51, 0x0e70, 
  0xff9f, 0xefbe, 0xdfdd, 0xcffc, 0xbf1b, 0xaf3a, 0x9f59, 0x8f78, 
  0x9188, 0x81a9, 0xb1ca, 0xa1eb, 0xd10c, 0xc12d, 0xf14e, 0xe16f, 
  0x1080, 0x00a1, 0x30c2, 0x20e3, 0x5004, 0x4025, 0x7046, 0x6067, 
  0x83b9, 0x9398, 0xa3fb, 0xb3da, 0xc33d, 0xd31c, 0xe37f, 0xf35e, 
  0x02b1, 0x1290, 0x22f3, 0x32d2, 0x4235, 0x5214, 0x6277, 0x7256, 
  0xb5ea, 0xa5cb, 0x95a8, 0x8589, 0xf56e, 0xe54f, 0xd52c, 0xc50d, 
  0x34e2, 0x24c3, 0x14a0, 0x0481, 0x7466, 0x6447, 0x5424, 0x4405, 
  0xa7db, 0xb7fa, 0x8799, 0x97b8, 0xe75f, 0xf77e, 0xc71d, 0xd73c, 
  0x26d3, 0x36f2, 0x0691, 0x16b0, 0x6657, 0x7676, 0x4615, 0x5634, 
  0xd94c, 0xc96d, 0xf90e, 0xe92f, 0x99c8, 0x89e9, 0xb98a, 0xa9ab, 
  0x5844, 0x4865, 0x7806, 0x6827, 0x18c0, 0x08e1, 0x3882, 0x28a3, 
  0xcb7d, 0xdb5c, 0xeb3f, 0xfb1e, 0x8bf9, 0x9bd8, 0xabbb, 0xbb9a, 
  0x4a75, 0x5a54, 0x6a37, 0x7a16, 0x0af1, 0x1ad0, 0x2ab3, 0x3a92, 
  0xfd2e, 0xed0f, 0xdd6c, 0xcd4d, 0xbdaa, 0xad8b, 0x9de8, 0x8dc9, 
  0x7c26, 0x6c07, 0x5c64, 0x4c45, 0x3ca2, 0x2c83, 0x1ce0, 0x0cc1, 
  0xef1f, 0xff3e, 0xcf5d, 0xdf7c, 0xaf9b, 0xbfba, 0x8fd9, 0x9ff8, 
  0x6e17, 0x7e36, 0x4e55, 0x5e74, 0x2e93, 0x3eb2, 0x0ed1, 0x1ef0 
};

//SJ5130712 - This function does system initialization for host communication.
unsigned char PLC_System_Init(int init_step)
{
    unsigned char ret_flag=SJ_FAIL;
    unsigned char i;
    unsigned char temp_msg_type;
    unsigned int ndx=0;
    unsigned int temp=0;


    //SJ5130712 - Construct message header for LOAD_SYSTEM_CONFIG
    ggeneral_buffer[ndx++] = MSG_TYPE__LOAD_SYSTEM_CONFIG;  //SJ5130712 - Message type (0x0C); ndx=0
    ggeneral_buffer[ndx++] = 0x80;  //SJ5130712 - ORG=1, RPY=0, RESV=0, SEQ=0 (10000000); ndx=1
    ggeneral_buffer[ndx++] = 0;  //SJ5130712 - Msg payload length LSB; ndx=2
    ggeneral_buffer[ndx++] = 0;  //SJ5130712 - Msg payload length MSB; ndx=3
    ggeneral_buffer[ndx++] = 0;  //SJ5130712 - Msg header CRC LSB; ndx=4
    ggeneral_buffer[ndx++] = 0;  //SJ5130712 - Msg header CRC MSB; ndx=5
    ggeneral_buffer[ndx++] = 0;  //SJ5130712 - Msg payload CRC LSB; ndx=6
    ggeneral_buffer[ndx++] = 0;  //SJ5130712 - Msg payload CRC MSB; ndx=7

//SJ2170712 - System_Init step 1 SYSTEM_INIT_PORT_DESIGNATION
    if (init_step == SYSTEM_INIT_STEP_1)
    {

    //SJ5130712 - Construct message body for LOAD_SYSTEM_CONFIG (Port designation)
    //ndx = 8;
    ggeneral_buffer[ndx++] = PORT_DESIGNATION;  //SJ5130712 - Config type LSB (0x0001 & 0x00FF); ndx=8
    ggeneral_buffer[ndx++] = (PORT_DESIGNATION >> 8);  //SJ5130712 - Config type MSB (0x0001 >> 8); ndx=9
    ggeneral_buffer[ndx++] = 0x01;  //SJ5130712 - Config length LSB (0x0001 & 0x00FF); ndx=10
    ggeneral_buffer[ndx++] = 0x00;  //SJ5130712 - Config length MSB (0x0001 >> 8); ndx=11
    ggeneral_buffer[ndx++] = 0x00;  //SJ5130712 - Diag Port=SCI-A, Data Port=SCI-A (00000000); ndx=12
    ggeneral_buffer[ndx++] = 0x00;  //SJ5130712 - Padding with zero; ndx=13.

    temp = ndx - 4 - 1;  //SJ2170712 - Substract the first four bytes and padding byte.
    ggeneral_buffer[2] = (temp & 0x00FF);  //SJ5130712 - Msg payload length LSB; Total payload length=10
    ggeneral_buffer[3] = (temp >> 8);      //SJ5130712 - Msg payload length MSB; Total payload length=10

    //SJ1160712 - Transmit PORT_DESIGNATION packet.
    //SJ1160712 - Acknowledgement from PLC device
    }
//SJ2170712 - System_Init step 2 SYSTEM_INIT_SYSTEM_CONFIG
    else if (init_step == SYSTEM_INIT_STEP_2)
    {
    //SJ1160712 - Construct message body for LOAD_SYSTEM_CONFIG (System Configuration)
    //ndx = 8;
    ggeneral_buffer[ndx++] = SYSTEM_CONFIGURATION;  //SJ1160712 - Config type LSB (0x0003 & 0x00FF); ndx=8
    ggeneral_buffer[ndx++] = (SYSTEM_CONFIGURATION >> 8);  //SJ1160712 - Config type MSB (0x0003 >> 8); ndx=9
    ggeneral_buffer[ndx++] = 0x1A;  //SJ1160712 - Config length LSB (0x001A & 0x00FF); ndx=10
    ggeneral_buffer[ndx++] = 0x00;  //SJ1160712 - Config length MSB (0x001A >> 8); ndx=11
    //SJ1160712 - Serial number length, Serial number, and EUI fields with 0x00; ndx=12 to ndx=35 (total 24 bytes of blank)
    for (i=ndx; i<36; i++)
        ggeneral_buffer[i] = 0x00;
    ndx = i;  //SJ1160712 = ndx=36
    //ggeneral_buffer[ndx++] = 0x00;  //SJ3211112 - Used in iLighting project //SJ1160712 - Device Mode=G3 normal mode; ndx=36
    ggeneral_buffer[ndx++] = 0x03;  //SJ3211112 - iLight micro project //SJ4260712 - Device Mode=P2P mode; ndx=36
    ggeneral_buffer[ndx++] = 0x00;  //SJ1160712 - Auto Mode; ndx=37.
    //SJ1160712 - Message is in 16-bits align so no need to pad with 0

    temp = ndx - 4;  //SJ2170712 - Substract the first four bytes.
    ggeneral_buffer[2] = (temp & 0x00FF);  //SJ1160712 - Msg payload length LSB; Total payload length=34
    ggeneral_buffer[3] = (temp >> 8);      //SJ1160712 - Msg payload length MSB; Total payload length=34

    //SJ1160712 - Transmit SYSTEM_CONFIGURATION packet.
    //SJ1160712 - Acknowledgement from PLC device
    }
//SJ2170712 - System_Init step 3 SYSTEM_INIT_G3_CONFIG
    else if (init_step == SYSTEM_INIT_STEP_3)
    {

    //SJ1160712 - Construct message body for LOAD_SYSTEM_CONFIG (G3 Configuration)
    //ndx = 8;
    ggeneral_buffer[ndx++] = G3_CONFIGURATION;  //SJ1160712 - Config type LSB (0x0008 & 0x00FF); ndx=8
    ggeneral_buffer[ndx++] = (G3_CONFIGURATION >> 8);  //SJ1160712 - Config type MSB (0x0008 >> 8); ndx=9
    ggeneral_buffer[ndx++] = 0x10;  //SJ1160712 - Config length LSB (0x0010 & 0x00FF); ndx=10
    ggeneral_buffer[ndx++] = 0x00;  //SJ1160712 - Config length MSB (0x0010 >> 8); ndx=11
    ggeneral_buffer[ndx++] = 0x00;  //SJ1160712 - TMR=0, COH=0; ndx=12
    ggeneral_buffer[ndx++] = 0x00;  //SJ1160712 - Reserved; ndx=13
    ggeneral_buffer[ndx++] = 0x08;  //SJ1160712 - G3 Long Address Byte 0 LSB; ndx=14
    ggeneral_buffer[ndx++] = 0x07;  //SJ1160712 - G3 Long Address Byte 1; ndx=15
    ggeneral_buffer[ndx++] = 0x06;  //SJ1160712 - G3 Long Address Byte 2; ndx=16
    ggeneral_buffer[ndx++] = 0x05;  //SJ1160712 - G3 Long Address Byte 3; ndx=17
    ggeneral_buffer[ndx++] = 0x04;  //SJ1160712 - G3 Long Address Byte 4; ndx=18
    ggeneral_buffer[ndx++] = 0x03;  //SJ1160712 - G3 Long Address Byte 5; ndx=19
    ggeneral_buffer[ndx++] = 0x00;  //SJ1160712 - G3 Long Address Byte 6; ndx=20  //SJ5240812 - Reserved for future 2-byte DALI address
    ggeneral_buffer[ndx++] = short_address;//0x01;  //SJ1160712 - G3 Long Address Byte 7 MSB; ndx=21   //short_address
    ggeneral_buffer[ndx++] = 0x00;  //SJ1160712 - Reserved Byte 0 LSB; ndx=22
    ggeneral_buffer[ndx++] = 0x00;  //SJ1160712 - Reserved Byte 1; ndx=23
    ggeneral_buffer[ndx++] = 0x00;  //SJ1160712 - Reserved Byte 2; ndx=24
    ggeneral_buffer[ndx++] = 0x00;  //SJ1160712 - Reserved Byte 3 MSB; ndx=25
    ggeneral_buffer[ndx++] = 0xEF;  //SJ1160712 - Always set to 239 LSB; ndx=26
    ggeneral_buffer[ndx++] = 0x00;  //SJ1160712 - Always set to 239 MSB; ndx=27
    //SJ1160712 - Message is in 16-bits align so no need to pad with 0

    temp = ndx - 4;  //SJ2170712 - Substract the first four bytes.
    ggeneral_buffer[2] = (temp & 0x00FF);  //SJ1160712 - Msg payload length LSB; Total payload length=24
    ggeneral_buffer[3] = (temp >> 8);      //SJ1160712 - Msg payload length MSB; Total payload length=24

    //SJ1160712 - Transmit G3_CONFIGURATION packet.
    //SJ1160712 - Acknowledgement from PLC device
    }
//SJ2170712 - System_Init step 4 SYSTEM_INIT_SHUTDOWN
    else if (init_step == SYSTEM_INIT_STEP_4)
    {

    //SJ1160712 - Construct message header for SHUTDOWN
    ndx = 0;
    ggeneral_buffer[ndx++] = MSG_TYPE__SHUTDOWN;  //SJ1160712 - Message type (0x05); ndx=0
    ggeneral_buffer[ndx++] = 0x80;  //SJ1160712 - ORG=1, RPY=0, RESV=0, SEQ=0 (10000000); ndx=1
    ggeneral_buffer[ndx++] = 6;  //SJ1160712 - Msg payload length LSB; ndx=2
    ggeneral_buffer[ndx++] = 0;  //SJ1160712 - Msg payload length MSB; ndx=3
    ggeneral_buffer[ndx++] = 0;  //SJ1160712 - Msg header CRC LSB; ndx=4
    ggeneral_buffer[ndx++] = 0;  //SJ1160712 - Msg header CRC MSB; ndx=5
    ggeneral_buffer[ndx++] = 0;  //SJ1160712 - Msg payload CRC LSB; ndx=6
    ggeneral_buffer[ndx++] = 0;  //SJ1160712 - Msg payload CRC MSB; ndx=7
    ggeneral_buffer[ndx++] = 0x00;  //SJ3180712 - Shutdown reset type LSB; ndx=8
    ggeneral_buffer[ndx++] = 0x00;  //SJ3180712 - Shutdown reset type MSB; ndx=9
    //SJ1160712 - Message is in 16-bits align so no need to pad with 0

    //SJ1160712 - Transmit SHUTDOWN packet.
    //SJ1160712 - Acknowledgement from PLC device
    }

    //gstate_var.UART_Rx_ndx = 0;
    //gstate_var.UART_Rx_len = MESSAGE_HEADER_LEN;
    temp_msg_type = ggeneral_buffer[MSG_HEADER_TYPE];

    //SJ3180712 - Transmit packet to PLC device
    for (i=0; i<ndx; i++)
    //while (1)
    {
        while (!(IFG2 & UCA0TXIFG));  // USCI_A0 TX buffer ready?
        UCA0TXBUF = ggeneral_buffer[i];  //0xA5
        ggeneral_buffer[i] = 0xFF;  //SJ4190712 - Empty buffer
    }

    //ggeneral_buffer[MSG_HEADER_TYPE] = 0x00;
    //ggeneral_buffer[MESSAGE_HEADER_LEN]  = 0x00;
    //ggeneral_buffer[MESSAGE_HEADER_LEN+1] = 0x00;

    //SJ1160712 - Acknowledgement from PLC device

    gstate_var.UART_Rx_State = UART_STANDBY;  //SJ1300712 - New method 
    gstate_var.UART_Rx_duration = TIMER_4_SECONDS;

    while (gstate_var.UART_Rx_duration)  //SJ3180712 - Wait for 2 seconds for UART_Rx to complete
    {
        if (gstate_var.UART_Rx_State == UART_PROCESSING)  //SJ1300712 - New method
        {
            gstate_var.UART_Rx_State = UART_INVALID;  //SJ1300712 - New method

            temp = (ggeneral_buffer[MESSAGE_HEADER_LEN+1] << 8) + ggeneral_buffer[MESSAGE_HEADER_LEN];

            //SJ3180712 - If msg type return from PLC device does not match the msg type sent by host, 
            //SJ3180712 - or reply status not equal 0x0000, raise error flag
            if ((ggeneral_buffer[MSG_HEADER_TYPE] == temp_msg_type) && (temp == MSG_REQUEST_OK))
                ret_flag = SJ_PASS;

            //gstate_var.UART_Rx_ndx = MESSAGE_HEADER_LEN+1;  //SJ3180712 - Prevent UART_Rx ISR from setting UART_Rx_COMPLETE
            gstate_var.UART_Rx_duration = 0;
        }
    }

    return (ret_flag);

}  //SJ5130712 - End of PLC_System_Init(init_step) { ... }


#if 0  //SJ3191212 - Not used in this project. DO NOT REMOVE
//SJ2170712 - This function does Network Configuration for host communication.
unsigned char PLC_Network_Config(int init_step)
{
    unsigned char ret_flag=SJ_FAIL;
    unsigned char i;
    unsigned char temp_msg_type;
    unsigned int ndx=0;
    unsigned int temp=0;


    //SJ2170712 - By pass SET_INFO which is optional.
    //MSG_TYPE__SET_INFO

    if (init_step == NETWORK_INIT_SET_INFO_1)
    {
        //SET_INFO.request
        ggeneral_buffer[ndx++] = MSG_TYPE__SET_INFO;  //SJ3220812 - Message type (0x04) ndx=0
        ggeneral_buffer[ndx++] = 0x80;  //SJ3220812 - ORG=1, RPY=0, RESV=0, SEQ=0 (10000000) ndx=1
        ggeneral_buffer[ndx++] = 0x1a;  //SJ3220812 - Msg payload length LSB  26 bytes of data ndx=2
        ggeneral_buffer[ndx++] = 0x00;  //SJ3220812 - Msg payload length MSB ndx=3
        ggeneral_buffer[ndx++] = 0x00;  //SJ3220812 - Msg header CRC LSB ndx=4
        ggeneral_buffer[ndx++] = 0x00;  //SJ3220812 - Msg header CRC MSB ndx=5
        ggeneral_buffer[ndx++] = 0x00;  //SJ3220812 - Msg payload CRC LSB ndx=6
        ggeneral_buffer[ndx++] = 0x00;  //SJ3220812 - Msg payload CRC MSB ndx=7

        ggeneral_buffer[ndx++] = G3_PHY_TX_PARAM_LSB;     //SJ3220812 - INFO_TYPE (0x02) ndx=8
        ggeneral_buffer[ndx++] = G3_PHY_TX_PARAM_MSB;     //SJ3220812 - INFO_TYPE (0x00) ndx=9
        ggeneral_buffer[ndx++] = 0x12;  //SJ3220812 - INFO Length LSB - 18 bytes  ndx=10
        ggeneral_buffer[ndx++] = 0x00;  //SJ3220812 - INFO Length MSB  ndx=11

        //SJ3220812 - Info value (G3 PHY Tx Parameters)
        ggeneral_buffer[ndx++] = 0x00;  //SJ3220812 - Band Sel=0, COH=0, TMR=0 ndx=12
        ggeneral_buffer[ndx++] = 0x00;  //SJ3220812 - Modulation=0x00 ndx=13
        ggeneral_buffer[ndx++] = 0x20;  //SJ3220812 - Tx Level - LSB ndx=14
        ggeneral_buffer[ndx++] = 0x00;  //SJ3220812 - Tx Level - MSB ndx=15

        //SJ3220812 - Tone mask value (14 bytes)
      #if 0  //SJ3220812 - Cenelec A 36
        ggeneral_buffer[ndx++] = 0x17;  //SJ3220812 - Tone mask, now on Cenelec A 36 setting ndx=16
        ggeneral_buffer[ndx++] = 0x24;  //SJ3220812 - ndx=17
        ggeneral_buffer[ndx++] = 0xff;  //SJ3220812 - ndx=18
        ggeneral_buffer[ndx++] = 0xff;  //SJ3220812 - ndx=19
        ggeneral_buffer[ndx++] = 0xff;  //SJ3220812 - ndx=20
        ggeneral_buffer[ndx++] = 0xff;  //SJ3220812 - ndx=21
        ggeneral_buffer[ndx++] = 0x0f;  //SJ3220812 - ndx=22
        ggeneral_buffer[ndx++] = 0x00;  //SJ3220812 - ndx=23
        ggeneral_buffer[ndx++] = 0x00;  //SJ3220812 - ndx=24
        ggeneral_buffer[ndx++] = 0x00;  //SJ3220812 - ndx=25
        ggeneral_buffer[ndx++] = 0x00;  //SJ3220812 - ndx=26
        ggeneral_buffer[ndx++] = 0x00;  //SJ3220812 - ndx=27
        ggeneral_buffer[ndx++] = 0x00;  //SJ3220812 - ndx=28
        ggeneral_buffer[ndx++] = 0x00;  //SJ3220812 - ndx=29
      #else  //SJ3220812 - Cenelec B
        ggeneral_buffer[ndx++] = 0x3F;  //SJ3220812 - Tone mask, now on Cenelec A 36 setting ndx=16
        ggeneral_buffer[ndx++] = 0x10;  //SJ3220812 - ndx=17
        ggeneral_buffer[ndx++] = 0xFF;  //SJ3220812 - ndx=18
        ggeneral_buffer[ndx++] = 0xFF;  //SJ3220812 - ndx=19
        ggeneral_buffer[ndx++] = 0x00;  //SJ3220812 - ndx=20
        ggeneral_buffer[ndx++] = 0x00;  //SJ3220812 - ndx=21
        ggeneral_buffer[ndx++] = 0x00;  //SJ3220812 - ndx=22
        ggeneral_buffer[ndx++] = 0x00;  //SJ3220812 - ndx=23
        ggeneral_buffer[ndx++] = 0x00;  //SJ3220812 - ndx=24
        ggeneral_buffer[ndx++] = 0x00;  //SJ3220812 - ndx=25
        ggeneral_buffer[ndx++] = 0x00;  //SJ3220812 - ndx=26
        ggeneral_buffer[ndx++] = 0x00;  //SJ3220812 - ndx=27
        ggeneral_buffer[ndx++] = 0x00;  //SJ3220812 - ndx=28
        ggeneral_buffer[ndx++] = 0x00;  //SJ3220812 - ndx=29
      #endif
        //SJ3220812 - No need padding
    }
    else if (init_step == NETWORK_INIT_SET_INFO_2)
    {
        //SET_INFO.request
        ggeneral_buffer[ndx++] = MSG_TYPE__SET_INFO;  //SJ3220812 - Message type (0x04) ndx=0
        ggeneral_buffer[ndx++] = 0x80;  //SJ3220812 - ORG=1, RPY=0, RESV=0, SEQ=0 (10000000) ndx=1
        ggeneral_buffer[ndx++] = 0x1a;  //SJ3220812 - Msg payload length LSB  26 bytes of data ndx=2
        ggeneral_buffer[ndx++] = 0x00;  //SJ3220812 - Msg payload length MSB ndx=3
        ggeneral_buffer[ndx++] = 0x00;  //SJ3220812 - Msg header CRC LSB ndx=4
        ggeneral_buffer[ndx++] = 0x00;  //SJ3220812 - Msg header CRC MSB ndx=5
        ggeneral_buffer[ndx++] = 0x00;  //SJ3220812 - Msg payload CRC LSB ndx=6
        ggeneral_buffer[ndx++] = 0x00;  //SJ3220812 - Msg payload CRC MSB ndx=7

        ggeneral_buffer[ndx++] = G3_PHY_RX_PARAM_LSB;     //SJ3220812 - INFO_TYPE (0x03) ndx=8
        ggeneral_buffer[ndx++] = G3_PHY_RX_PARAM_MSB;     //SJ3220812 - INFO_TYPE (0x00) ndx=9
        ggeneral_buffer[ndx++] = 0x12;  //SJ3220812 - INFO Length LSB - 18 bytes  ndx=10
        ggeneral_buffer[ndx++] = 0x00;  //SJ3220812 - INFO Length MSB  ndx=11

        //SJ3220812 - Info value (G3 PHY Rx Parameters)
        ggeneral_buffer[ndx++] = 0x00;  //SJ3220812 - Reserved ndx=12
        ggeneral_buffer[ndx++] = 0x03;  //SJ3220812 - Both BLI, and AGC on (0x05)) (.Ti config = 0x03) ndx=13  //SJ2280812 - ?????
        ggeneral_buffer[ndx++] = 0x00;  //SJ3220812 - Gain Value (.Ti config = 0x00) ndx=14
        ggeneral_buffer[ndx++] = 0x00;  //SJ3220812 - ndx=15

        //SJ3220812 - Tone mask value (14 bytes)
      #if 0  //SJ3220812 - Cenelec A 36
        ggeneral_buffer[ndx++] = 0x17;  //SJ3220812 - Tone mask, now on Cenelec A 36 setting ndx=16
        ggeneral_buffer[ndx++] = 0x24;  //SJ3220812 - ndx=17
        ggeneral_buffer[ndx++] = 0xff;  //SJ3220812 - ndx=18
        ggeneral_buffer[ndx++] = 0xff;  //SJ3220812 - ndx=19
        ggeneral_buffer[ndx++] = 0xff;  //SJ3220812 - ndx=20
        ggeneral_buffer[ndx++] = 0xff;  //SJ3220812 - ndx=21
        ggeneral_buffer[ndx++] = 0x0f;  //SJ3220812 - ndx=22
        ggeneral_buffer[ndx++] = 0x00;  //SJ3220812 - ndx=23
        ggeneral_buffer[ndx++] = 0x00;  //SJ3220812 - ndx=24
        ggeneral_buffer[ndx++] = 0x00;  //SJ3220812 - ndx=25
        ggeneral_buffer[ndx++] = 0x00;  //SJ3220812 - ndx=26
        ggeneral_buffer[ndx++] = 0x00;  //SJ3220812 - ndx=27
        ggeneral_buffer[ndx++] = 0x00;  //SJ3220812 - ndx=28
        ggeneral_buffer[ndx++] = 0x00;  //SJ3220812 - ndx=29
      #else  //SJ3220812 - Cenelec B
        ggeneral_buffer[ndx++] = 0x3F;  //SJ3220812 - Tone mask, now on Cenelec A 36 setting ndx=16
        ggeneral_buffer[ndx++] = 0x10;  //SJ3220812 - ndx=17
        ggeneral_buffer[ndx++] = 0xFF;  //SJ3220812 - ndx=18
        ggeneral_buffer[ndx++] = 0xFF;  //SJ3220812 - ndx=19
        ggeneral_buffer[ndx++] = 0x00;  //SJ3220812 - ndx=20
        ggeneral_buffer[ndx++] = 0x00;  //SJ3220812 - ndx=21
        ggeneral_buffer[ndx++] = 0x00;  //SJ3220812 - ndx=22
        ggeneral_buffer[ndx++] = 0x00;  //SJ3220812 - ndx=23
        ggeneral_buffer[ndx++] = 0x00;  //SJ3220812 - ndx=24
        ggeneral_buffer[ndx++] = 0x00;  //SJ3220812 - ndx=25
        ggeneral_buffer[ndx++] = 0x00;  //SJ3220812 - ndx=26
        ggeneral_buffer[ndx++] = 0x00;  //SJ3220812 - ndx=27
        ggeneral_buffer[ndx++] = 0x00;  //SJ3220812 - ndx=28
        ggeneral_buffer[ndx++] = 0x00;  //SJ3220812 - ndx=29
      #endif
        //SJ3220812 - No need padding
    }
    else if (init_step == NETWORK_INIT_SET_INFO_3)
    {
        //SET_INFO.request
        ggeneral_buffer[ndx++] = MSG_TYPE__SET_INFO;  //SJ3220812 - Message type (0x04) ndx=0
        ggeneral_buffer[ndx++] = 0x80;  //SJ3220812 - ORG=1, RPY=0, RESV=0, SEQ=0 (10000000) ndx=1
        ggeneral_buffer[ndx++] = 0x0A;  //SJ3220812 - Msg payload length LSB  26 bytes of data ndx=2
        ggeneral_buffer[ndx++] = 0x00;  //SJ3220812 - Msg payload length MSB ndx=3
        ggeneral_buffer[ndx++] = 0x00;  //SJ3220812 - Msg header CRC LSB ndx=4
        ggeneral_buffer[ndx++] = 0x00;  //SJ3220812 - Msg header CRC MSB ndx=5
        ggeneral_buffer[ndx++] = 0x00;  //SJ3220812 - Msg payload CRC LSB ndx=6
        ggeneral_buffer[ndx++] = 0x00;  //SJ3220812 - Msg payload CRC MSB ndx=7

        ggeneral_buffer[ndx++] = TX_PHY_ATTNT_GAIN_PARAM_LSB;     //SJ3220812 - INFO_TYPE (0x09) ndx=8
        ggeneral_buffer[ndx++] = TX_PHY_ATTNT_GAIN_PARAM_MSB;     //SJ3220812 - INFO_TYPE (0x00) ndx=9
        ggeneral_buffer[ndx++] = 0x02;  //SJ3220812 - INFO Length LSB - 18 bytes  ndx=10
        ggeneral_buffer[ndx++] = 0x00;  //SJ3220812 - INFO Length MSB  ndx=11

        //SJ3220812 - Info value (Tx PHY attnt gain Parameters)
        //ggeneral_buffer[ndx++] = 0x1E;  //SJ3220812 - Tx Gain/Attenuation ndx=12 (SJ4200912 - ORG)
        //ggeneral_buffer[ndx++] = 0x14;  //SJ3220812 - Tx Gain/Attenuation ndx=12  (SJ4200912 - DEBUGGING, 2db)
        //ggeneral_buffer[ndx++] = 0x00;  //SJ3220812 - Tx Gain/Attenuation ndx=12  (SJ4200912 - DEBUGGING, 0db)
        //ggeneral_buffer[ndx++] = 0xEC;  //SJ3220812 - Tx Gain/Attenuation ndx=12  (SJ4200912 - DEBUGGING, -2db)
		//ggeneral_buffer[ndx++] = 0xE2;  //SJ3171012 - Tx Gain/Attenuation ndx=12 (SJ3171012 - DEBUGGING,  -3dB)
		//ggeneral_buffer[ndx++] = 0xD8;  //SJ3171012 - Tx Gain/Attenuation ndx=12 (SJ3171012 - DEBUGGING,  -4dB)
		ggeneral_buffer[ndx++] = 0xCE;  //SJ3171012 - Tx Gain/Attenuation ndx=12 (SJ3171012 - DEBUGGING,  -5dB)
		//ggeneral_buffer[ndx++] = 0xC4;  //SJ3171012 - Tx Gain/Attenuation ndx=12 (SJ5191012 - DEBUGGING,  -6dB)
        ggeneral_buffer[ndx++] = 0x00;  //SJ3220812 - Tx PGA Attenuation ndx=13
//SJ1170912        ggeneral_buffer[ndx++] = 0xC4;  //SJ3220812 - Tx Gain/Attenuation ndx=12
//SJ1170912        ggeneral_buffer[ndx++] = 0x02;  //SJ3220812 - Tx PGA Attenuation ndx=13
    }
    //SJ2170712 - Construct message header for DISCOVER
    else if (init_step == NETWORK_INIT_DISCOVER)  //SJ5200712 - NETWORK_DISCOVER
    {
        ggeneral_buffer[ndx++] = MSG_TYPE__DISCOVER;  //SJ2170712 - Message type (0x12); ndx=0
        ggeneral_buffer[ndx++] = 0x80;  //SJ2170712 - ORG=1, RPY=0, RESV=0, SEQ=0 (10000000); ndx=1
        ggeneral_buffer[ndx++] = 6;  //SJ2170712 - Msg payload length LSB; ndx=2
        ggeneral_buffer[ndx++] = 0;  //SJ2170712 - Msg payload length MSB; ndx=3
        ggeneral_buffer[ndx++] = 0;  //SJ2170712 - Msg header CRC LSB; ndx=4
        ggeneral_buffer[ndx++] = 0;  //SJ2170712 - Msg header CRC MSB; ndx=5
        ggeneral_buffer[ndx++] = 0;  //SJ2170712 - Msg payload CRC LSB; ndx=6
        ggeneral_buffer[ndx++] = 0;  //SJ2170712 - Msg payload CRC MSB; ndx=7

        //SJ5130712 - Construct message body for DISCOVER (Network Discover)
        //ndx = 8;
        ggeneral_buffer[ndx++] = NETWORK_DISCOVER;  //SJ2170712 - Discover type (0x00); ndx=8
        ggeneral_buffer[ndx++] = 0x05;  //SJ2170712 - Duration (Recommended 0x05 secs); ndx=9
        //SJ3220812 - No need padding
    }
    else if (init_step == NETWORK_INIT_ATTACH)  //SJ5200712 - MSG_TYPE__ATTACH
    //if (ret_flag == SJ_PASS)
    {
        //SJ2170712 - Construct message header for ATTACH
        //ndx = 0;
        ggeneral_buffer[ndx++] = MSG_TYPE__ATTACH;  //SJ2170712 - Message type (0x10); ndx=0
        ggeneral_buffer[ndx++] = 0x80;  //SJ2170712 - ORG=1, RPY=0, RESV=0, SEQ=0 (10000000); ndx=1
        ggeneral_buffer[ndx++] = 8;  //SJ2170712 - Msg payload length LSB; ndx=2
        ggeneral_buffer[ndx++] = 0;  //SJ2170712 - Msg payload length MSB; ndx=3
        ggeneral_buffer[ndx++] = 0;  //SJ2170712 - Msg header CRC LSB; ndx=4
        ggeneral_buffer[ndx++] = 0;  //SJ2170712 - Msg header CRC MSB; ndx=5
        ggeneral_buffer[ndx++] = 0;  //SJ2170712 - Msg payload CRC LSB; ndx=6
        ggeneral_buffer[ndx++] = 0;  //SJ2170712 - Msg payload CRC MSB; ndx=7

        //SJ2170712 - Construct message body for ATTACH (PAN ID)
        //ndx = 8;
        ggeneral_buffer[ndx++] = (gstate_var.PAN_ID & 0x00FF);  //SJ2170712 - PAN ID LSB; ndx=8
        ggeneral_buffer[ndx++] = (gstate_var.PAN_ID >> 8);  //SJ2170712 - PAN ID MSB; ndx=9
        ggeneral_buffer[ndx++] = 0x00;  //SJ2170712 - Reserved byte LSB; ndx=10
        ggeneral_buffer[ndx++] = 0x00;  //SJ2170712 - Reserved byte MSB; ndx=11
        //SJ3220812 - No need padding
    }  //SJ4190712 - if (init_step ...) { ... } else if (init_step ...) { ... }


    temp_msg_type = ggeneral_buffer[MSG_HEADER_TYPE];

    //SJ2170712 - Transmit out packet.
    for (i=0; i<ndx; i++)
    {
        while (!(IFG2 & UCA0TXIFG));  // USCI_A0 TX buffer ready?
        UCA0TXBUF = ggeneral_buffer[i];
        ggeneral_buffer[i] = 0xFF;  //SJ2240712 - For debug purpose
    }

    //SJ2170712 - Get PAN ID from USCIAB0RX_VECTOR ISR
    gstate_var.UART_Rx_State = UART_STANDBY;  //SJ1300712 - New method
    //gstate_var.UART_Rx_duration = TIMER_2_MINUTES; //TIMER_8_SECONDS;//TIMER_4_SECONDS;
    gstate_var.UART_Rx_duration = 1800;  //SJ4230812 - Wait for 90 seconds

    while (gstate_var.UART_Rx_duration)  //SJ4190712 - Wait for 8 seconds for UART_Rx to complete
    {
        if (gstate_var.UART_Rx_State == UART_PROCESSING)  //SJ1300712 - New method
        {
            gstate_var.UART_Rx_State = UART_INVALID;  //SJ1300712 - New method

            temp = (ggeneral_buffer[MESSAGE_HEADER_LEN+1] << 8) + ggeneral_buffer[MESSAGE_HEADER_LEN];

            //SJ3180712 - If msg type return from PLC device does not match the msg type sent by host, 
            //SJ3180712 - or reply status not equal 0x0000, raise error flag
            if ((ggeneral_buffer[MSG_HEADER_TYPE] == temp_msg_type) && (temp == STAT_SUCCESS))
            {
                //init_step == NETWORK_INIT_SET_INFO_1
                if (init_step == NETWORK_INIT_DISCOVER)
                {
                    gstate_var.PAN_ID = (ggeneral_buffer[15] << 8) + ggeneral_buffer[14];  //SJ4260712 - PAN Id
                }
                else if (init_step == NETWORK_INIT_ATTACH)
                {
                    //SJ3191212 - network_addr is not used. Can be replaced with a local var
                    gstate_var.network_addr = (ggeneral_buffer[11] << 8) + ggeneral_buffer[10];
                    //AZ_DALI_Response(REVEAL_DALI_ADDRESS);
                }
//short_address
                ret_flag = SJ_PASS;
            }
            //gstate_var.UART_Rx_ndx = MESSAGE_HEADER_LEN+1;  //SJ3180712 - Prevent UART_Rx ISR from setting UART_Rx_COMPLETE
            gstate_var.UART_Rx_duration = 0;
            //break;
        }  //SJ3220812 - if (gstate_var.UART_Rx_State ...) { ... }

    }  //SJ3220812 - while (gstate_var.UART_Rx_duration) { ... }

    //SJ2170712 - Get ATTACH status from USCIAB0RX_VECTOR ISR

    return (ret_flag);

}  //SJ2170712 - End of PLC_Network_Config(init_step) { ... }
#endif  //SJ3191212 - #if 0 ... #endif


//SJ4130912 - CRC16 checksum. Copy from PLC Suite Host Message Protocol Specifications page 105.
#if 1

void CRC16_UpdateChecksum(unsigned short *pcrcvalue, const void *data, int length)
{
  unsigned short crc;
  const unsigned char *buf = (const unsigned char *) data;

  crc = *pcrcvalue;
  while (length--)
  {
    crc = (crc << 8) ^ crctable[(crc >> 8) ^ *buf++];
  }
  *pcrcvalue = crc;
}  //SJ4130912 - End of CRC16_UpdateChecksum() { ... }

#endif //SJ4130912 - 


//--- End of file (PLC_Comm.c) --------------------------------------------------------------------

