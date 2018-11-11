/******************************************************************************/
// dali.c
// defines functions called by application as well as functions for the
// Manchester encoding phy and the address decoding
//
// Functions called by main application
//    void TI_DALI_Init(void)
//    void TI_DALI_Transaction_Loop(void)
//    void TI_DALI_Flash_Update(unsigned int)
//
// Functions called by TI_Dali_Transaction_Loop
//    void TI_DALI_Reset_Check(void)
//    void TI_DALI_Tx(unsigned char)
//    unsigned char TI_DALI_Rx(unsigned char *rx_msg)
//    unsigned char TI_DALI_Match_Address(unsigned char)
//    unsigned char TI_DALI_Command(unsigned char*)
//    unsigned char TI_DALI_Special_Command(unsigned char*)
//    void TI_DALI_Update_Callback(void);
//    void TI_DALI_Idle_Callback(void);
//
// ISR
// #pragma vector=WDT_VECTOR
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
#include "dali.h"
#include "msp430x21x1.h"
#include "global_var.h"  //SJ5100812 - Added by SJ
#include "command.h"  //SJ5070912 - Added by SJ


//SJ5070912 - External function prototype ---------------------------
//PLC_Lite.c
extern void AZ_DALI_Response(int response_type);

//PLC_Comm.c
extern void CRC16_UpdateChecksum(unsigned short *pcrcvalue, const void *data, int length);

//SJ5100812 - External global variables declaration ------------------
//PLC_iSwitch.c
extern unsigned char ggeneral_buffer[DATA_BUFFER_SZ];
#ifdef SWITCH_CONTROLLER
extern unsigned char gswitch_oper[SWITCH_OPER_SZ];
#else
extern unsigned char gextended_scene[EXT_SCENE_SZ];  //SJ1140113 - equiv 64
#endif
extern STATE_STRUCT  gstate_var;

/*** DALI Constants ***********************************************************/
const unsigned char min_fast_fade_time = 11;
const unsigned char gear_type = 0;
const unsigned char possible_operating_modes = 0;
const unsigned char features = 0;
const unsigned char version_number = 0;
const unsigned char extended_version_number=1;
#if 0  //SJ2111212 - ORG
const unsigned char device_type = 6;       /* Device type 6, device for LEDs */
#else  //SJ2111212 - Replaced. To allow sharing of code between Switch controller and lighting controller
const unsigned char device_type = DEVICE_TYPE;       /* Device type 6, device for LEDs */
#endif
const unsigned char phys_min_lvl = PHYS_MIN_LEVEL;  //SJ5140912  #define PHYS_MIN_LEVEL 90 defined in dali.h
/*** Memory Bank 0 *************************************************************/
#if 0  //SJ5080313 - Not used
const unsigned char memory_bank_0[15] = 
{ 
  LAST_BNK0_ADDR,
  CHKSUM_BNK0,
  LAST_BNK,
  GTIN_0,
  GTIN_1,
  GTIN_2,
  GTIN_3,
  GTIN_4,
  GTIN_5,
  FRMWR_VER_0,
  FRMWR_VER_1,
  SERIAL_1,
  SERIAL_2,
  SERIAL_3,
  SERIAL_4
};
#endif
/*** DALI Global RAM Variables ************************************************/
unsigned char power_on_level;
unsigned char actual_level;
/* SEARCH_ADDRESS and RANDOM_ADDRESS are defined in the DALI standard.  While */
// the strict definintion is not followed the funtionality is provided with
// three other variables (low, middle, and high) for each.
//unsigned long int search_address = SEARCH_ADDRES_DEFAULT;
//unsigned long int random_address = RANDOM_ADDRES_DEFAULT;
unsigned char search_address_byte_h;
//unsigned char dummy1;
unsigned char search_address_byte_m;
//unsigned char dummy2;
unsigned char search_address_byte_l;
//unsigned char dummy3;
unsigned char random_address_byte_h;
//unsigned char dummy4;
unsigned char random_address_byte_m;
//unsigned char dummy5;
unsigned char random_address_byte_l;
unsigned char system_failure_level;
unsigned char min_level;
unsigned char max_level;
unsigned char fade_rate;
unsigned char fade_time;
unsigned char fast_fade_time;
//SJ2220113 - SJEXT_ADDR: Use below declaration for extended DALI address
#ifdef SJEXT_ADDR
  unsigned int  short_address;
#else  //SJ3200213 - ORG
  //unsigned char short_address;
#endif
unsigned char group_0_7;
unsigned char group_8_f;
unsigned char scene[16];
unsigned char status_information;
unsigned char data_transfer_register;
unsigned char data_transfer_register1;
unsigned char data_transfer_register2;
unsigned char failure_status;
unsigned char operating_mode;
unsigned char dimming_curve;
/******************************************************************************/
// Additional Arc fade control global variables 
unsigned char target_level;
unsigned long fade_count;
unsigned int config_count;
unsigned long dapc_count;       /*Direct Arc Power Control */
unsigned long start_time;
unsigned long fade_step_size;
unsigned char *flash_ptr;
unsigned char flash_update_request;
unsigned long initialise_count;
unsigned char initialise_minute;
unsigned char idle_time;
unsigned char physical_selection=0;
unsigned char enable_device_type=0;
unsigned char *infoC_ptr;  //SJ3190912 - Added for PLC long address and CRC checksum
unsigned char *infoB_ptr;  //SJ5141212 - Added for switch operation
/******************************************************************************/
// TI_Dali_Init(unsinged char)
//
/******************************************************************************/
void TI_DALI_Init(unsigned int key){
  volatile unsigned char i;
  //unsigned char *tmp_ptr;  //SJ3190912 - ORG. Not used in our project

  /*** Setup Memory and non-volatile variables ********************************/
  // Data exists in either INFO memory B,C, or D depending on what was last 
  // written to the information memory space. After a factory program of flash
  // only INFOD will be programmed with the factory defaults.  The in system 
  // flash routine in addition to this intitialization routine will provide a 
  // cyclical buffer in the information memory. The valid range of the 
  // MAX_LEVEL is min_level to 254 if MAX_LEVEL = 255 then that portion of the
  // segment is empty.

  unsigned short crcvalue=0;
  unsigned short stored_crc=0;
  //const void *data

  flash_ptr = (unsigned char *)INFO_D1;  //SJ5140912 - equiv 0x1000 - DALI's default
//  infoC_ptr = (unsigned char *)INFO_C1;  //SJ3190912 - equiv 0x1040 - //SJ2190213 - Remark but DO NOT REMOVE
  infoB_ptr = (unsigned char *)INFO_B1;  //SJ5141212 - equiv 0x1080 - Extended scene

  //CRC16_UpdateChecksum(&crcvalue, (const void *) 0x1000, DATA_SIZE);
  CRC16_UpdateChecksum(&crcvalue, (const void *) flash_ptr, DATA_SIZE);  //SJ2190213 - DATA_SIZE = 32, defined in dali.h

//0x1048
//#define EXTENDED_INFO_D      32
//#define CRC_CHECKSUM_MSB      8
//#define CRC_CHECKSUM_LSB      9
//  stored_crc = (unsigned short) ((infoC_ptr[CRC_CHECKSUM_MSB] << 8) + infoC_ptr[CRC_CHECKSUM_LSB]);  //SJ3190912 - Retrieve CRC checksum from InfoC (0x1040)
  stored_crc = (unsigned short) ((flash_ptr[EXTENDED_INFO_D+CRC_CHECKSUM_MSB] << 8) + flash_ptr[EXTENDED_INFO_D+CRC_CHECKSUM_LSB]);  //SJ3190912 - Retrieve CRC checksum from InfoD (0x1028)
  if (stored_crc == crcvalue)
  {	//power_on_level = 0;

    power_on_level = flash_ptr[POWER_ON_LEVEL];              //SJ1060611 - equiv 0

    /* Initialize RAM variables */
    system_failure_level = flash_ptr[SYSTEM_FAILURE_LEVEL];  //SJ1060611 - equiv 1
    min_level = flash_ptr[MIN_LEVEL];                        //SJ1060611 - equiv 2
    max_level = flash_ptr[MAX_LEVEL];                        //SJ1060611 - equiv 3
    fade_rate = flash_ptr[FADE_RATE];                        //SJ1060611 - equiv 4
    fade_time = flash_ptr[FADE_TIME];                        //SJ1060611 - equiv 5
    if (fade_time == 0)   fade_time = 1;                        //SJ3170413 - For Aztech use.
   #if 0  //SJ3190912 - ORG. DALI
    short_address = flash_ptr[SHORT_ADDRESS];                //SJ1060611 - equiv 6
   #else  //SJ3190912 - Replaced. Aztech
//SJ2220113 - SJEXT_ADDR: Un-remark below sttmt for extended DALI address
//  short_address = (infoC_ptr[PLC_LONG_ADDR_2_LOC] << 8) | infoC_ptr[PLC_LONG_ADDR_1_LOC];
//    short_address = infoC_ptr[PLC_LONG_ADDR_1_LOC];  //SJ3190912 - Retrieve DALI short address from Info C.
//SJ2190213 - SJTODO: To merge infoC with infoD.
    #ifdef SJEXT_ADDR
    short_address = (flash_ptr[EXTENDED_INFO_D+PLC_LONG_ADDR_2_LOC] << 8) | flash_ptr[EXTENDED_INFO_D+PLC_LONG_ADDR_1_LOC];
    #else  //SJ3200213 - ORG
    short_address = flash_ptr[EXTENDED_INFO_D+PLC_LONG_ADDR_1_LOC];  //SJ3190912 - Retrieve DALI short address from Info D.
    #endif
   #endif
    group_0_7 = flash_ptr[GROUP_0_7];                        //SJ1060611 - equiv 7
    group_8_f = flash_ptr[GROUP_8_F];                        //SJ1060611 - equiv 8
    for(i=0;i<NUMBER_OF_SCENES;i++)
    {
      scene[i] = flash_ptr[i+SCENE_0];                       //SJ1060611 - equiv 9 - 24
    }
    random_address_byte_h = flash_ptr[RANDOM_ADDRESS_H];     //SJ1060611 - equiv 25
    random_address_byte_m = flash_ptr[RANDOM_ADDRESS_M];     //SJ1060611 - equiv 26
    //random_address_byte_h = flash_ptr[RANDOM_ADDRESS_L];     //SJ1060611 - equiv 27  //SJ2120612 - Why random_address_byte_h???? Is it typo error???
    random_address_byte_l = flash_ptr[RANDOM_ADDRESS_L];     //SJ1060611 - equiv 27  //SJ1130812 -
    fast_fade_time = flash_ptr[FAST_FADE_TIME];              //SJ1060611 - equiv 28
    failure_status = (flash_ptr[FAILURE_STATUS] & 0x80);     //SJ1060611 - equiv 29
    operating_mode = (flash_ptr[OPERATING_MODE] & 0x10);     //SJ1060611 - equiv 30
    dimming_curve = flash_ptr[DIMMING_CURVE];                //SJ1060611 - equiv 31

   #if 1  //SJ2260213 - Retrieve protocol and data structure version number from flash
    gstate_var.protocol_ver = flash_ptr[EXTENDED_INFO_D+PROTOCOL_VER_LOC];     //SJ2260213 - Protocol version
    gstate_var.data_struct_ver = flash_ptr[EXTENDED_INFO_D+DATA_STRUCT_VER_LOC];  //SJ2260213 - Data structure version

    //for (i=0; i<EXT_SCENE_SZ; i++)
    //    gextended_scene[i] = infoB_ptr[i];
   #endif

 #if 1  //SJ4200912 - Restore default value in case flash is corrupted
    //flash_update_request = 0;
  }
  else
  {
    power_on_level = POWER_ON_LEVEL_DEFAULT;              //SJ4200912 - equiv 254
    system_failure_level = SYSTEM_FAILURE_LEVEL_DEFAULT;  //SJ4200912 - equiv 254
    min_level = MIN_LEVEL_DEFAULT;                        //SJ4200912 - equiv 90
    max_level = MAX_LEVEL_DEFAULT;                        //SJ4200912 - equiv 254
    fade_rate = FADE_RATE_DEFAULT;                        //SJ4200912 - equiv 7
    fade_time = FADE_TIME_DEFAULT;                        //SJ4200912 - equiv 0
    short_address = 0;  //SJ2220113 - SJEXT_ADDR: no change       //SJ4200912 - Retrieve DALI short address from Info C.
    group_0_7 = GROUP_0_7_DEFAULT;                        //SJ4200912 - equiv 0x00
    group_8_f = GROUP_8_F_DEFAULT;                        //SJ4200912 - equiv 0x00
    for(i=0;i<NUMBER_OF_SCENES;i++)
    {
      scene[i] = SCENE_0_F_DEFAULT;                       //SJ4200912 - equiv 254
    }
    random_address_byte_h = RANDOM_ADDR_BYTE_DEFAULT;     //SJ4200912 - equiv 0xFF
    random_address_byte_m = RANDOM_ADDR_BYTE_DEFAULT;     //SJ4200912 - equiv 0xFF
    random_address_byte_l = RANDOM_ADDR_BYTE_DEFAULT;     //SJ4200912 - equiv 0xFF 
    fast_fade_time = FAST_FADE_TIME_DEFAULT;              //SJ4200912 - equiv 0
    failure_status = FAILURE_STATUS_DEFAULT;              //SJ4200912 - equiv 0
    operating_mode = OPERATING_MODE_DEFAULT;              //SJ4200912 - equiv 0
    dimming_curve = DIMMING_CURVE_DEFAULT;                //SJ4200912 - equiv 0

    //SJ2111212 - iSwitch ----------
    #if 0//def I_SWITCH_PROJECT  //SJ1171212 - No longer needed as room ID is determined by DIP switch
      gstate_var.Room_ID = 0;//(infoC_ptr[ROOM_ID_MSB_LOC] << 8) + infoC_ptr[ROOM_ID_LSB_LOC];
    #endif
    //SJ2111212 - End of iSwitch ---

    //flash_update_request = 1;
    //gstate_var.flash_update_countdown = 10;//TIMER_30_SECONDS;
    //TI_DALI_Flash_Update(FWKEY);  //SJ4200912 - For debugging purpose
    TI_DALI_Flash_Update(FWKEY, 0);  //SJ1171212 - Write to segment 0x1000 and 0x1040, unsigned char wrt_area
  }
 #endif

  search_address_byte_h = MASK;  //SJ1060611 - equiv 0xFF
  search_address_byte_m = MASK;
  search_address_byte_l = MASK;
  fade_count=0;
  flash_update_request = 0;  //SJ4200912 - ORG. DO NOT REMOVE
  config_count=CONFIG_TIMEOUT;  //SJ1060611 - equiv 98
  dapc_count=DAPC_TIMEOUT;      //SJ1060611 - equiv 195
  initialise_count=INITIALISE_SEC+1;  //SJ1060611 - equiv 58594
  status_information = STATUS_INFO_DEFAULT;  //SJ2220512 - equiv 0x80 (STATUS_POWER_FAILURE is the default)

  //SJ3060313 - Lighting device
  for (i=0; i<EXT_SCENE_SZ; i++)
      gextended_scene[i] = infoB_ptr[i];

  /*** Setup WDT **************************************************************/
  WDTCTL = FADE_INTERVAL;  //SJ4300812 - equiv (WDT_MDLY_8 - SMCLK/8192), 1 second will goto ISR 977 times.
  IE1 |= WDTIE;

  actual_level = power_on_level;
  if(power_on_level<min_level)
  {
    actual_level = min_level;
  }
  if(power_on_level>max_level)
  {
    actual_level = max_level;
  }
  start_time = 0;  // Initialize timer.  If expires go to power on level.
} // END TI_Dali_Init()

/******************************************************************************/
// TI_Dali_Transaction_Loop
// This function gets the DALI received data, rx_msg[], calls the appropriate 
// decode function, execute_command or execute_special_command, and then calls
// back to the main application. After returning from the main application a  
// response transmission is sent if needed and then repeats.
/******************************************************************************/
void TI_DALI_Transaction_Loop(void)
{
  /* the information pushed onto the stack for this call is not needed */
  /* possibly adjust stact pointer?                                    */
  unsigned char bytes_received, command_type;
  static unsigned char rx_msg[VALID_BYTES];  //SJ2070611 - equiv 2
  static unsigned char response;
  //while (1)  //SJ4090812 - ORG. No need to loop forever here as this function is call from main loop
  {
      response=0;
      bytes_received=0;
      command_type = 0;
      bytes_received = TI_DALI_Rx(rx_msg);  //SJ4090812 - ORG. TI DALI
      //bytes_received = TI_DALI_Rx(&ggeneral_buffer[82]);  //SJ5100812 - Replaced. Using PLC DataTransfer packet
      if(bytes_received == SYSTEM_FAILURE)
      {
      #if 0  //SJ4280213 - Don't compile since it is not used. Modify the logic if we need to implement system failure.
        status_information |= STATUS_POWER_FAILURE;
        status_information &= ~FADE_RUNNING;
        //arc_power_level(system_failure_level);
        /* check system_failure_level, if 255 (mask) then no change
        *  if a different value then go to that value without fading
        */
        if (system_failure_level != MASK)
        {
          /* Update both status and actual_level before WDT call*/
          IE1 &= ~WDTIE;   //SJ3080611 - Disable watchdog interval interrupt service routine???
          actual_level = system_failure_level;
          if(system_failure_level > max_level)
          {
            actual_level = max_level;
          }
          if(system_failure_level < min_level)
          {
            actual_level = min_level;
          }
          status_information |= LAMP_ARC_POWER_ON;
          status_information &= ~FADE_RUNNING;
          IE1 |= WDTIE;
          TI_DALI_Update_Callback();  //SJ3080611 - Routine's only sttmt: TACCTL1 |= CCIE; It enable ISR for PWM routine
        }
      #else
        ;
      #endif
      }
      else if (bytes_received == VALID_BYTES)
      {
        /*Decode address*/
        command_type = TI_DALI_Match_Address(rx_msg[0]);
        // if address is a match return a 1 or 2
        if(command_type == 1)  //SJ4090611 - Either a broadcast, Group, or Short address
        {
          response = TI_DALI_Command(rx_msg);
        }
        if(command_type == 2)  //SJ4090611 - Special commands
        {
          response = TI_DALI_Special_Command(rx_msg);

        #if 0  //SJ4201212 - Added to acknowledge GUI the state of address assignment
          //if ((gstate_var.general_ctrl_flag & ATTACH_ON) && (rx_msg[0] == 0xB7))  //SJ4201212 - At the moment ignore 0xB7 1st
          if (gstate_var.general_ctrl_flag & ATTACH_ON)  //SJ4201212 - because ATTACH_ON is only assigned in 0xB7 of special_command.c
          {
            AZ_DALI_Response(0xB7);  //SJ5070912 #define GENERIC_QUERY  3 in PLC_Comm.h
            gstate_var.general_ctrl_flag &= ATTACH_OFF;  //SJ4201212 - Clear acknowledgement flag
          }
        #endif
        }
        if(response)
        {
          /* Respond, if the response is 'NO', then control gear does not     */
          /* react. The response must occur between 7 and 22 TE after the     */
          /* forward frame. 7 TE ~ 3ms , 22*TE ~ 9ms                          */
          /*  Use the WDT to determine the amount of time that must be waited */
          /*  after the return from the callback function.                    */
          idle_time = 0;
          TI_DALI_Idle_Callback();  //SJ3080611 - Routine's only sttmt: __no_operation();
          while(idle_time < RESPONSE_MAX_TIME);   //SJ4170512 - equiv 7*(8192/8000)=7.168
      #if 0  //SJ5070912 - ORG. Dali code
          TI_DALI_Tx(response);
      #else  //SJ2180912 - Replaced.
          //AZ_DALI_Response(3);  //SJ5070912 #define GENERIC_QUERY  3 in PLC_Comm.h
      #endif
        }
        else
        {
          TI_DALI_Idle_Callback();
        }
        TI_DALI_Reset_Check(); // if the varaibles are no longer consistent with RESET
                       // then clear the bit from the status register
      }//ignore command if does not meet length criteria  //SJ3080611 - if(bytes_received == SYSTEM_FAILURE) { ... } else if (...) { ... }
  }// END WHILE(1)
}


//#define  AZ_DEV_LOC   65//17  //SJ4061212 - For switch project //62
//#define  AZ_CMD_LOC   66//18  //SJ4061212 - For switch project //63

/******************************************************************************/
// TI_DALI_Rx
// This function stores the received data into the array rx_msg[] and returns 
// the number of bytes receieved.  The return value is also used to report if 
// a system failure or timing violation has occured.
/******************************************************************************/
//SJ3080611 - SJNOTE: This routine can be used as it without changes except for the port used as input.
//SJ3150611 - This routine process incoming message sent out from DALI control unit.
unsigned char TI_DALI_Rx(unsigned char* rx_msg)
{
 #if 1  //SJ4020812 - To be replaced with our own code
    unsigned int room_ID;

    //SJ5100812 - For Quick test. Need to enhance this function.

    //rx_msg[0] = ((int) ggeneral_buffer[82] - 48);  //SJ5100812 - Device short address
    //rx_msg[1] = ((int) ggeneral_buffer[83] - 48);

  if ((ggeneral_buffer[AZ_DEV_ADDR_MSB] == 0xFF) && (ggeneral_buffer[AZ_DEV_LOC] == 0xFF))
  {
      return FALSE;  //SJ5010313 - Packet is meant for GUI, so individual device no need to process.
  }
  else
  {
    room_ID = (ggeneral_buffer[AZ_ROOM_ID_LOC_H] << 8) + ggeneral_buffer[AZ_ROOM_ID_LOC_L];
   #if 0  //SJ2260213 - ORG. No protocol and data structure version control
    if (room_ID == gstate_var.Room_ID)
   #else  //SJ2260213 - Replaced. Protocol and Data structure version control
    if (room_ID == gstate_var.Room_ID && ggeneral_buffer[AZ_PROTOCOL_VER] <= gstate_var.protocol_ver &&
        ggeneral_buffer[AZ_DAT_STRUCT_VER] <= gstate_var.data_struct_ver)
   #endif
    {
//#define AZ_DEV_ADDR_MSB  64
//#define AZ_DEV_LOC  65
//#define AZ_CMD_LOC  66

        rx_msg[0] = (ggeneral_buffer[AZ_DEV_LOC]);  //SJ5100812 - Device short address
        rx_msg[1] = (ggeneral_buffer[AZ_CMD_LOC]);

        return VALID_BYTES;
    }
    else
        return AZ_INVALID_ROOM_ID;

  }

 #else  //SJ4020812 - ORG code from TI. Skip
  unsigned long fault_check=0,stop_check=0;
  unsigned char rx_bytes,rx_bits,temp_msg;
  unsigned int rx_mask;
  rx_bytes=rx_bits=0;
  P2IE &= ~(BIT0);  //SJ3080611 - after negate equal 11111110. Purpose is to disable interrupt request when P2IFG is set.

  if(!(P2IN & BIT0))  //SJ3080611 - If sttm is true when bit 0 of port 2 is low.
  {
    P2IES &= ~BIT0;       // Low-to-High Transition
    P2IFG &= ~BIT0;  //SJ3080611 - P2IFG will get 11111110; purpose is to clear interrupt flag
    fault_check=0;
    while (~P2IFG & BIT0)     // Wait for rising edge
    {
      //SJ3150611 - below 500*MCLK_mSEC_COUNT = 500 * 8000 = 4000000
      //SJ3150611 - If port 2 bit 0 never goes high, after 500ms report as failure.
      //SJ4160611 - With current timer setting, it takes around 16.2 seconds to get pass thru.
      if (fault_check++ > 500*MCLK_mSEC_COUNT) // Interface failure when > 500ms
      {
        return SYSTEM_FAILURE;
      }
    }
  }
  P2IES |= BIT0;        // High-to-Low Transition
  P2IFG &= ~BIT0;
  /*** START BIT ****/
  while(~P2IFG & BIT0);   // Wait for Falling Edge
  P2IES &= ~BIT0;       // Low-to-High Transition
  P2IFG &= ~BIT0;
  /****************************************************************************/
  // Fault detection: if the input is low for greater than 500ms, then the
  // light should go to the fault setting.
  /****************************************************************************/
  fault_check=0;
  while (~P2IFG & BIT0)     // Wait for rising edge
  {
    //SJ3150611 - below 505*MCLK_uSEC_COUNT = 505 * (8000/1000) = 4040
    if (fault_check++ > 505*MCLK_uSEC_COUNT) // bit failure if greater than
    {                                        // tmax = 500us
      return 0;
    }
  }
  /* Start pulse is special case, receive first bit */
  __delay_cycles(505*MCLK_uSEC_COUNT); // maximum width is 500us
  rx_msg[rx_bytes] = 0;
  rx_mask = (P2IN & BIT0) ^ BIT0;  //SJ3150611 - If high, value is 0. If low, value is 1
  rx_msg[rx_bytes] |= (rx_mask<<7);
  rx_bits++;
  /* wait for edge */
  fault_check=0;
  if (rx_mask)
  {
    P2IES &= ~BIT0;       // Low-to-High Transition
  }
  else
  { 
    P2IES |= BIT0;        // High-to-Low Transition
  }
  P2IFG &= ~BIT0;
  while (~P2IFG & BIT0)  // wait for change in state
  {
    if (fault_check++ > 505*MCLK_uSEC_COUNT) // Interface failure when > 500ms
    {
      return 0;                           // timing fault
    }
  }
  while (rx_bytes<VALID_BYTES)  //SJ3150611 - When this while loop starts, rx_bits is 1
  {
    P2DIR |= BIT2;  // debug
    P2OUT ^= BIT2;  // debug
    while (rx_bits <8)  //SJ3150611 - This while loop receives all subsequent 7 bits of the data byte
    {
      __delay_cycles(505*MCLK_uSEC_COUNT);
      // maximum TE is 500us + 5us for extra hold time  
      rx_mask = (P2IN & BIT0) ^ BIT0;
      fault_check=0;
      rx_mask = rx_mask<<(7-rx_bits);
      rx_msg[rx_bytes] |= rx_mask;
      rx_bits++;
      if (rx_mask)
      {
        P2IES &= ~BIT0;       // Low-to-High Transition
      }
      else
      { 
        P2IES |= BIT0;        // High-to-Low Transition
      }
      P2IFG &= ~BIT0;
      while (~P2IFG & BIT0)  // wait for change in state
      {
        if (fault_check++ > 505*MCLK_uSEC_COUNT)
        { 
          return 0;
        } 
      }
    } // End rx_bits while loop
    /* Sample possible first bit of next byte */
    rx_bytes++;
    temp_msg = 0;
    rx_bits=0;
    stop_check=0;
    // maximum TE is 500us + 5us for extra hold time
    __delay_cycles(505*MCLK_uSEC_COUNT);
    rx_mask = (P2IN & BIT0) ^ BIT0;
    temp_msg |= rx_mask<<7;
    rx_bits++;
    /* wait for edge */
    if (rx_mask)
    {
      P2IES &= ~BIT0;       // Low-to-High Transition
    }
    else
    { 
      P2IES |= BIT0;        // High-to-Low Transition
    }
    P2IFG &= ~BIT0;
    while (~P2IFG & BIT0)  // wait for change in state
    {
      /*check to see if no edges have occured, then this is a stop condition*/
      /* Stop bits are 4*TE or 4*416.67us = 1667                            */
      /* x cycles per while loop                                            */
      if (stop_check > 50*MCLK_uSEC_COUNT) 
      {
        P2DIR |= BIT2;  // debug  //SJ3150611 - Set pin 3 of port 2 as output
        P2OUT ^= BIT2;  // debug
        return rx_bytes;
      }
      stop_check++;
    }
      if(rx_bytes >= VALID_BYTES)
      {
        return 0;
      }
      else
      {
        rx_msg[rx_bytes] = temp_msg;  //SJ3150611 - Bit 7 of second byte.
      }
  } // End rx_bytes Whileloop
  return 0;
 #endif  //SJ4020812 - #if 1 ... #else ... #endif
}

/******************************************************************************/
// TI_DALI_Tx(response)
// This function transmits the query response. The response is requested by
// the command query.
// P2.1 is TX line
// Start bit, data byte, 2 stop bits (idle line)
/******************************************************************************/

void TI_DALI_Tx(unsigned char response)
{
 #if 1  //SJ4020812 - To be replaced with our own code
 ;
 #else  //SJ4020812 - ORG code from TI. Skip.
  unsigned char bit_mask=0x80;
  P2IE &= ~(BIT1);
  /* Start Bit  */
  P2OUT &= ~BIT1;
  __delay_cycles(416*MCLK_uSEC_COUNT);
  P2OUT |= BIT1;
  __delay_cycles(416*MCLK_uSEC_COUNT);
  while(bit_mask)
  {
    if(bit_mask & response)
    { /* transmit a '1' */
      P2OUT &= ~BIT1;
      __delay_cycles(416*MCLK_uSEC_COUNT);
      P2OUT |= BIT1;
      __delay_cycles(416*MCLK_uSEC_COUNT);
    }
    else
    { /* transmit a '0' */
      P2OUT |= BIT1;
      __delay_cycles(416*MCLK_uSEC_COUNT);
      P2OUT &= ~BIT1;
      __delay_cycles(416*MCLK_uSEC_COUNT);
    }
    bit_mask = bit_mask >> 1;
  }
  P2OUT |= BIT1; /* IDLE condition */
 #endif  //SJ4020812 - #if 1 ... #else ... #endif
}

/******************************************************************************/
// TI_DALI_Match_Address()
//
// Is the address field a Broadcast, group, or short address
// rx_msg[0]?= broadcast (b)
//          ?= group     (g)
//          ?= short     (s)
// Return a 1 if it is a match with either a b,g, or s
// return a 2 if it is a special command
/******************************************************************************/
unsigned char TI_DALI_Match_Address(unsigned char address){
  unsigned char temp;
 #ifdef SJEXT_ADDR
  unsigned int temp_dev_addr;  //SJ1210113 - SJEXT_ADDR: Un-remark for DALI extended address
 #endif
  //SJ4090611 - SJNOTE: Sepcial commands ranges from 177 to 199
  //SJ4090611 -         Group address ranges from 128 to 159
  //SJ4090611 -         Broadcast address are 254 and 255
  //SJ1060812 - 0x9F = 10011111 (159), 0xFE = 11111110 (254)
  if((address > 0x9F) && (address < 0xFE))  //SJ4090611 - equiv 159 and 254
  {
    /* Special commands are address 0xB1 through 0xC7         */
    /* while valid address values range from 0x00 to 0x7F     */
    /* group address values range from 0x80 to 0x9F           */
    /* and the broadcast address values are 0xFE and 0xFF     */
    return 2;
  }
  if(address & BIT7)  //SJ4090611 - If bit 7 is on, either broadcast or group addressing.
  {/* group or broadcast */
    address = address>>1;
    address = address & 0x7F;  //SJ4090611 - Here we extract 7 bits
    if(address==0x7F)  //SJ5170611 - If true, address is likely either 254 or 255
    {
      return 1; /* broadcast message */
    }
    address = address & 0x0F;  /* group id is now in lower nibble */
    temp = 0x01;
    if(address<8)
    {
      temp = temp<<address;
      if(temp & group_0_7)  //SJ4090611 - group_0_7 = 0x00
      {
        return 1;
      }
    }
    else
    {
      address = address - 8;
      temp = temp<<address;
      if(temp & group_8_f)  //SJ4090611 - group_8_f = 0x00
      {
        return 1;
      }
    }
    return 0;  //SJ5191012 - Added by SJ to solve TI bug
    //SJ5170611 - SJNOTE: Why group_0_7 and group_8_f address are the same - 0x00 ???
  }

  //#if 1  //SJ1210113 - ORG. DALI code
  #ifndef SJEXT_ADDR
  //SJ5170611 - If reaches here, address is below 127 inclusive
  address = address>>1;
  address = address & 0x3F;  //SJ4090611 - Here we extract 6 bits. Short addr is less than 63 inclusive

  if(address == short_address)  //SJ4090611 - short_address retrieve from flash info area
  {
    return 1;
  }
  else
  {
    return 0;
  }
  #else  //SJ1210113 - SJEXT_ADDR: Replaced. Extended DALI address
//unsigned int temp_dev_addr;
//AZ_DEV_ADDR_MSB
  temp_dev_addr = address >> 1;
  temp_dev_addr = temp_dev_addr & 0x3F;  //SJ4090611 - Here we extract 6 bits. Short addr is less than 63 inclusive

  if (ggeneral_buffer[AZ_DEV_ADDR_MSB] != 0x00)  //SJ1210113 - #define  AZ_DEV_ADDR_MSB  64 defined in dali.h
  {
      address = address>>1;
      address = address & 0x3F;  //SJ4090611 - Here we extract 6 bits. Short addr is less than 63 inclusive

      temp_dev_addr = ggeneral_buffer[AZ_DEV_ADDR_MSB] << 6;
      temp_dev_addr |= address;  //SJ1210113 - This will yield device long address
  }

  if(temp_dev_addr == short_address)  //SJ4090611 - short_address retrieve from flash info area
  {
      return 1;
  }
  else
  {
      return 0;
  }
  #endif

}

/******************************************************************************/
// TI_DALI_Reset_Check()
// If the values in the information register match the RESET value
// , then the RESET bit must be set
/******************************************************************************/
void TI_DALI_Reset_Check(void)
{
  if(actual_level != ACTUAL_LEVEL_RESET)
  {
    status_information &= ~RESET_STATE;   //SJ5140912 - #define RESET_STATE BIT5
    return;
  }
  if(system_failure_level != SYSTEM_FAILURE_LEVEL_RESET)
  {
    status_information &= ~RESET_STATE;
    return;
  }
  if(power_on_level != POWER_ON_LEVEL_RESET)
  {
    status_information &= ~RESET_STATE;
    return;
  }
  if(min_level != MIN_LEVEL_RESET)
  {
    status_information &= ~RESET_STATE;
    return;
  }
  if(max_level != MAX_LEVEL_RESET)
  {
    status_information &= ~RESET_STATE;
    return;
  }
  if(fade_rate != FADE_RATE_RESET)
  {
    status_information &= ~RESET_STATE;
    return;
  }
  if(fade_time != FADE_TIME_RESET)
  {
    status_information &= ~RESET_STATE;
    return;
  }
  if(fade_time != FADE_TIME_RESET)
  {
    status_information &= ~RESET_STATE;
    return;
  }
  if(group_0_7 != GROUP_0_7_RESET)
  {
    status_information &= ~RESET_STATE;
    return;
  }
  if(group_8_f != GROUP_8_F_RESET)
  {
    status_information &= ~RESET_STATE;
    return;
  }
  if(scene[0] != SCENE_0_F_RESET)
  {
    status_information &= ~RESET_STATE;
    return;
  }
  if(dimming_curve != DIMMING_CURVE_RESET)
  {
    status_information &= ~RESET_STATE;
    return;
  }
  status_information |= RESET_STATE;
}


/******************************************************************************/
// TI_Flash_Update(FWKEY)
// Use the FLASH Controller to save these non-volatile settings. These
// settings will be used on the next Power up.
/******************************************************************************/
//void TI_DALI_Flash_Update(unsigned int key)
void TI_DALI_Flash_Update(unsigned int key, unsigned char wrt_area)  //SJ1171212 - Replaced
{
    /* move data to next location */
    volatile unsigned char i;
  #if 1  //SJ3190912 - Added
    unsigned char temp_infoC[INFO_C_LAST_LOC];  //SJ2111212 - 16 elements

    unsigned short crcvalue=0;    //SJ3190912 - Added by SJ
    //unsigned short stored_crc=0;  //SJ3190912 - Added by SJ

    //SJ3190912 - Backup info C to local array before info C is erased
    for (i=0; i<INFO_C_LAST_LOC; i++)
      //temp_infoC[i] = infoC_ptr[i];
//SJ2190213 - SJTODO: To merge infoC with infoD.
      temp_infoC[i] = flash_ptr[EXTENDED_INFO_D+i];   //SJ2190213 - EXTENDED_INFO_D = 32
  #endif

    IE1 &= ~WDTIE;
    WDTCTL = WDTPW+WDTHOLD;  // stop the WDT
    FCTL2 = key+FSSEL0+FLASH_DIVIDER; // 333kHz Flash Timing Generator
    FCTL3 = key;
//--- SJ1171212 - Start of flash write -----------------------------------------
  #if 1  //SJ2180912 - SJ
    FCTL1 = key+ERASE;
    //--- SJ1171212 - Introduce if sttmt to allow writing to 0x1080 ------------
    //SJ2150113 - Lighting portion of iSW
    if (wrt_area == 1)  //SJ2150113 - Write to segment B 0x1080
    {
      infoB_ptr[0] = 0;  //SJ2150113 - Dummy write to info B
      FCTL1 = key+WRT;
      for (i=0; i<EXT_SCENE_SZ; i++)  //SJ2150113 - EXT_SCENE_SZ is 64
      {
        infoB_ptr[i] = gextended_scene[i];
      }
    }
    else
    //--- SJ1171212 - END of new added codes. Writing 0x1080 -------------------
    {
    flash_ptr[POWER_ON_LEVEL] = 0;  //SJ2180912 - Dummy write

    __no_operation();
    __no_operation();
    __no_operation();

    //gstate_var.general_delay = 2;
    //while (gstate_var.general_delay) ;

    //FCTL1 = key+ERASE;  //SJ3270213 - I think this sttmt is redundant here. DO NOT REMOVE first. Monitor result
//    infoC_ptr[0] = 0;  //SJ3190912 - Dummy write info C #define PLC_LONG_ADDR_8_LOC 0 in global_var.h
//SJ2190213 - SJTODO: To merge infoC with infoD.
//SJ2190213 - No need to do above but DO NOT REMOVE since infoC maybe used to describe key
  #endif
    FCTL1 = key+WRT;
    flash_ptr[POWER_ON_LEVEL] = power_on_level;
    flash_ptr[SYSTEM_FAILURE_LEVEL] = system_failure_level;
    flash_ptr[MIN_LEVEL] = min_level;
    flash_ptr[MAX_LEVEL] = max_level;
    flash_ptr[FADE_RATE] = fade_rate;
    flash_ptr[FADE_TIME] = fade_time;
    flash_ptr[SHORT_ADDRESS] = 0;//short_address;  //SJ2220113 - SJEXT_ADDR: Since org loc is no longer used, no change
    flash_ptr[GROUP_0_7] = group_0_7;
    flash_ptr[GROUP_8_F] = group_8_f;
    for(i=0;i<NUMBER_OF_SCENES;i++)
    {
      flash_ptr[i+SCENE_0]= scene[i];
    }
    flash_ptr[RANDOM_ADDRESS_H] = random_address_byte_h;
    flash_ptr[RANDOM_ADDRESS_M] = random_address_byte_m;
    flash_ptr[RANDOM_ADDRESS_L] = random_address_byte_l;
    flash_ptr[FAST_FADE_TIME] = fast_fade_time;
    flash_ptr[FAILURE_STATUS] = failure_status;
    flash_ptr[OPERATING_MODE] = operating_mode;
    flash_ptr[DIMMING_CURVE] = dimming_curve;
    /* scene 0 through 15 are set to FF which is the value of erased flash */

  #if 1  //SJ3190912 - Added by SJ
    __no_operation();
    __no_operation();
    __no_operation();

//  gstate_var.general_delay = 2;
//  while (gstate_var.general_delay) ;

    CRC16_UpdateChecksum(&crcvalue, (const void *) flash_ptr, DATA_SIZE);
//SJ2190213 - SJTODO: To merge infoC with infoD.
//SJ2220113 - SJEXT_ADDR: Un-remark below codes for extended address capability
//SJ4070313 - No need below codes. Used short_address will be good enough
#ifdef SJEXT_ADDR
//    temp_infoC[PLC_LONG_ADDR_2_LOC] = (unsigned char) (short_address >> 8);
//    temp_infoC[PLC_LONG_ADDR_1_LOC] = (unsigned char) short_address;
#else  //SJ3200213 - ORG
    temp_infoC[PLC_LONG_ADDR_1_LOC] = short_address;
#endif

//    temp_infoC[CRC_CHECKSUM_MSB] = (unsigned char) (crcvalue >> 8);
//    temp_infoC[CRC_CHECKSUM_LSB] = (unsigned char) crcvalue; //(crcvalue & 0x00FF);
    //*(temp_infoC+CRC_CHECKSUM_MSB) = crcvalue;

   #if 0  //SJ2190213 - ORG. Before merging infoC with infoD
    infoC_ptr[PLC_LONG_ADDR_8_LOC] = temp_infoC[PLC_LONG_ADDR_8_LOC];  //SJ3190912 - 0
    infoC_ptr[PLC_LONG_ADDR_7_LOC] = temp_infoC[PLC_LONG_ADDR_7_LOC];
    infoC_ptr[PLC_LONG_ADDR_6_LOC] = temp_infoC[PLC_LONG_ADDR_6_LOC];
    infoC_ptr[PLC_LONG_ADDR_5_LOC] = temp_infoC[PLC_LONG_ADDR_5_LOC];
    infoC_ptr[PLC_LONG_ADDR_4_LOC] = temp_infoC[PLC_LONG_ADDR_4_LOC];
    infoC_ptr[PLC_LONG_ADDR_3_LOC] = temp_infoC[PLC_LONG_ADDR_3_LOC];
    infoC_ptr[PLC_LONG_ADDR_2_LOC] = temp_infoC[PLC_LONG_ADDR_2_LOC];
    infoC_ptr[PLC_LONG_ADDR_1_LOC] = temp_infoC[PLC_LONG_ADDR_1_LOC];  //SJ3190912 - 7, Dali short addressed is stored here
    infoC_ptr[CRC_CHECKSUM_MSB] = temp_infoC[CRC_CHECKSUM_MSB];
    infoC_ptr[CRC_CHECKSUM_LSB] = temp_infoC[CRC_CHECKSUM_LSB];  //SJ3190912 - 9
    infoC_ptr[DEVICE_TYPE_LOC] = temp_infoC[DEVICE_TYPE_LOC];
    infoC_ptr[ROOM_ID_MSB_LOC] = (unsigned char) ((gstate_var.Room_ID & 0xFF00) >> 8);
    infoC_ptr[ROOM_ID_LSB_LOC] = (unsigned char) (gstate_var.Room_ID & 0x00FF);
    infoC_ptr[PRODUCT_ID_LOC] = temp_infoC[PRODUCT_ID_LOC];  //SJ3190912 - 7, Dali short addressed is stored here
    infoC_ptr[RELEASE_CODE_LOC] = temp_infoC[RELEASE_CODE_LOC];
    infoC_ptr[VERSION_CODE_LOC] = temp_infoC[VERSION_CODE_LOC];  //SJ3190912 - 9
   #else  //SJ2190213 - Replaced.Merging infoC with infoD
    flash_ptr[EXTENDED_INFO_D+PROTOCOL_VER_LOC] = temp_infoC[PROTOCOL_VER_LOC];  //SJ3190912 - 0
    flash_ptr[EXTENDED_INFO_D+DATA_STRUCT_VER_LOC] = temp_infoC[DATA_STRUCT_VER_LOC];
    flash_ptr[EXTENDED_INFO_D+PLC_LONG_ADDR_6_LOC] = temp_infoC[PLC_LONG_ADDR_6_LOC];
    flash_ptr[EXTENDED_INFO_D+PLC_LONG_ADDR_5_LOC] = temp_infoC[PLC_LONG_ADDR_5_LOC];
    flash_ptr[EXTENDED_INFO_D+PLC_LONG_ADDR_4_LOC] = temp_infoC[PLC_LONG_ADDR_4_LOC];
    flash_ptr[EXTENDED_INFO_D+PLC_LONG_ADDR_3_LOC] = temp_infoC[PLC_LONG_ADDR_3_LOC];
//    flash_ptr[EXTENDED_INFO_D+PLC_LONG_ADDR_2_LOC] = temp_infoC[PLC_LONG_ADDR_2_LOC];
//    flash_ptr[EXTENDED_INFO_D+PLC_LONG_ADDR_1_LOC] = temp_infoC[PLC_LONG_ADDR_1_LOC];  //SJ3190912 - 7, Dali short addressed is stored here
    flash_ptr[EXTENDED_INFO_D+PLC_LONG_ADDR_2_LOC] = (unsigned char) (short_address >> 8);
    flash_ptr[EXTENDED_INFO_D+PLC_LONG_ADDR_1_LOC] = (unsigned char) short_address;  //SJ3190912 - 7, Dali short addressed is stored here
//    flash_ptr[EXTENDED_INFO_D+CRC_CHECKSUM_MSB] = temp_infoC[CRC_CHECKSUM_MSB];
//    flash_ptr[EXTENDED_INFO_D+CRC_CHECKSUM_LSB] = temp_infoC[CRC_CHECKSUM_LSB];  //SJ3190912 - 9
    flash_ptr[EXTENDED_INFO_D+CRC_CHECKSUM_MSB] = (unsigned char) (crcvalue >> 8);
    flash_ptr[EXTENDED_INFO_D+CRC_CHECKSUM_LSB] = (unsigned char) crcvalue;  //SJ3190912 - 9
    flash_ptr[EXTENDED_INFO_D+DEVICE_TYPE_LOC] = temp_infoC[DEVICE_TYPE_LOC];
    flash_ptr[EXTENDED_INFO_D+ROOM_ID_MSB_LOC] = (unsigned char) ((gstate_var.Room_ID & 0xFF00) >> 8);
    flash_ptr[EXTENDED_INFO_D+ROOM_ID_LSB_LOC] = (unsigned char) (gstate_var.Room_ID & 0x00FF);
    flash_ptr[EXTENDED_INFO_D+PRODUCT_ID_LOC] = temp_infoC[PRODUCT_ID_LOC];  //SJ3190912 - 7, Dali short addressed is stored here
    flash_ptr[EXTENDED_INFO_D+RELEASE_CODE_LOC] = temp_infoC[RELEASE_CODE_LOC];
    flash_ptr[EXTENDED_INFO_D+VERSION_CODE_LOC] = temp_infoC[VERSION_CODE_LOC];  //SJ3190912 - 9
   #endif

  #endif  //SJ3190912 - #if 1 ... #endif. End of Added by SJ
    }  //SJ1171212 - if () { ... } else { ... }
//--- SJ1171212 - End of flash write -------------------------------------------

    FCTL1 = key;  /* lock flash */
    FCTL3 = key+LOCK;
    /* update the global pointer */

    //SJ4200912 - Below watchdog timer is better moved outside
  #if 0//SJ1100912 - Below two sttmts added by SJ
    WDTCTL = FADE_INTERVAL;  //SJ4300812 - equiv (WDT_MDLY_8 - SMCLK/8192), 1 second will goto ISR 977 times.
    IE1 |= WDTIE;
  #endif
}
//  ----   temp_infoC

/******************************************************************************/
// Watch Dog Timer in Interval Mode
// The DALI specficiation requires that the device fade to the requested power
// level.  The fade rate can be one of 14 values defined in the DALI standard.
// This function uses the periodic interval of the WDT to determine if the
// power level needs to be updates because the controller is fading to a
// requested level.
// Timer is also used to evaluate four different time delays:
// Direct Arc Power Sequence (200ms)
// Configuration (100ms)
// Initialisation (15 minutes)
// Response idle time (7ms)
/******************************************************************************/
//SJ3050912 - SMCLK=8MHz interval=8192 --> 8000000/8192 ~ 977. Each second this ISR will be run 977 times
#pragma vector=WDT_VECTOR
__interrupt void ISR_WDT(void)
{

#if 0  //SJ2230713 - Debug purpose
if (gstate_var.led_timer == 0)
{
    if (gstate_var.wdt_flag)  //SJ2230713 - turn on red LED
    {
        P1OUT |= ON_RED_LED;  //SJ1081012 - Done during port initialisation
        gstate_var.wdt_flag = 0;
    }
    else  //SJ2230713 - turn off red LED
    {
        P1OUT &= OFF_RED_LED;  //SJ1081012 - Turn off red LED
        gstate_var.wdt_flag = 1;
    }
    gstate_var.led_timer = 30;
}
#endif

 #ifndef DEV_7  //SJ4230513 - Not relevant for device type 7.
  if(status_information & FADE_RUNNING)   //SJ4300812 - FADE_RUNNING is BIT4
  {
    if(fade_count == fade_step_size)
    {
      fade_count=0;
      if(target_level > actual_level)
      {
        actual_level++;
      }
      if(target_level < actual_level) 
      {
        actual_level--;
      }
      if(actual_level == target_level)
      {
        status_information &= ~FADE_RUNNING;
        if(target_level == (min_level-1)) 
        {                         // special case of fade to min and then off
          actual_level=OFF;
          status_information &= ~LAMP_ARC_POWER_ON;
        }
      }
      TI_DALI_Update_Callback();
    }
    else fade_count++;
  }
 #endif
  /* If the control gear does not receive a power level command within 0.6 */
  /* seconds, then the control shall go to the power_on_level immediately */
  if(start_time<POWER_ON_TIME)
  {
    start_time++;
    if((start_time == POWER_ON_TIME) && (actual_level == power_on_level))
    {
      TI_DALI_Update_Callback();
    }
  }
  /* The control gear must respone within 7 to 22 TE or 3 to 9 ms */
  if(idle_time<RESPONSE_MAX_TIME)
  {
    idle_time++;
  }
  /* Configuration commands must be duplicated */
  /* within 100ms or they are ignored */
  //SJ4060912 - Becomes valid when command 32 to 128 are issued.
  if(config_count<CONFIG_TIMEOUT)   //SJ3050912 - CONFIG_TIMEOUT = 98 it represents 100ms
  {
    config_count++;
  }
  /* DAPC sequence  */
  if(dapc_count<DAPC_TIMEOUT)  //SJ4060912 - Becomes valid when ENABLE_DAPC_SEQ command is issued.
  {
    dapc_count++;
  }
  /* Initialise sequence 15 minutes or 15*60*1000 = 15*60000 */
  if(initialise_count<INITIALISE_SEC)  //SJ2120612 - equiv 58594
  {
    initialise_count++;
  }
  else 
  {
    initialise_count=0;
    initialise_minute++;
    if(initialise_minute == INITIALISE_MIN)  //SJ3050912 - INITIALISE_MIN is 15 minutes
    {
      initialise_count=INITIALISE_SEC;
      initialise_minute = 0;
    }
  }
} // END WDT ISR


