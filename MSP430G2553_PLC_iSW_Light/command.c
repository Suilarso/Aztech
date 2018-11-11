/******************************************************************************/
// command.c                                                                  
// Definition of TI_DALI_Command(), DALI commands 0-255                 
//                                                                            
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

#include "msp430x21x1.h"
#include "dali.h"
#include "command.h"
#include "global_var.h"  //SJ4300812 - 
#include "PLC_Comm.h"  //SJ4300812 - PLC related materials

//SJ5100812 - External function prototype ---------------------------
//PLC_Lite.c
extern void AZ_Randomised_Long_Address(void);
//extern void UART_Rx_Acknowledgement(int validity_flag);  //SJ3010812 - 
//extern unsigned char AZ_Acknowledge_Basenode_Request(unsigned char Message_Type);  //SJ4300812 - Response to basenode request
//extern void AZ_DALI_Response(int validity_flag);  //SJ3220812 - ORG. Point-to-Point
extern void AZ_DALI_Response(int response_type);  //SJ3220812 - Replaced. Used in network type

//dali.c
//extern void TI_DALI_Transaction_Loop(void);  //SJ4090812 -
//extern unsigned char TI_DALI_Match_Address(unsigned char address);
//extern void TI_DALI_Flash_Update(unsigned int key);
extern void TI_DALI_Flash_Update(unsigned int key, unsigned char wrt_area);

//SJ5100812 - External global variables declaration ------------------
//PLC_iSwitch.c
extern unsigned char ggeneral_buffer[DATA_BUFFER_SZ];
#ifdef SWITCH_CONTROLLER
extern unsigned char gswitch_oper[SWITCH_OPER_SZ];
#else
extern unsigned char gextended_scene[EXT_SCENE_SZ];  //SJ1140113 - equiv 64
#endif
extern STATE_STRUCT  gstate_var;

//SJ2181212 - Aztech prototype ---------------------------------------
unsigned char AZ_Extended_Command(void);
//void SJ_Debugging_Fading(void);  //SJ5260413 - For debugging purpose only

/*** fade_200ms: fade up or down 200ms at the selected fade rate **************/
/* This number represents the number of to be added to or subtracted from the */
/* actual_level to achieve the new target_level.                              */ 
const unsigned char fade_200ms[16] = 
{ OFF, FADE_200ms_1, FADE_200ms_2, FADE_200ms_3, FADE_200ms_4, FADE_200ms_5,
  FADE_200ms_6, FADE_200ms_7, FADE_200ms_8, FADE_200ms_9, FADE_200ms_A,
  FADE_200ms_B, FADE_200ms_C, FADE_200ms_D, FADE_200ms_E, FADE_200ms_F
};

/*** fade_rate_table **********************************************************/
/* This array is used to set fade_step_size (in WDT intervals).  This         */
/* represents the number of times the WDT ISR is called before the arc power  */
/* level is updated. */ 
const unsigned int fade_rate_table[16] = 
{ OFF, FADE_RATE_1, FADE_RATE_2, FADE_RATE_3, FADE_RATE_4, FADE_RATE_5, 
  FADE_RATE_6, FADE_RATE_7, FADE_RATE_8, FADE_RATE_9, FADE_RATE_A, FADE_RATE_B,
  FADE_RATE_C, FADE_RATE_D, FADE_RATE_E, FADE_RATE_F
};

/*** fade_time_table **********************************************************/
/* This array represents the amount of time (in WDT intervals) that the       */
/* control gear will take to get from the actual_level to the target_level.   */
/* This array divided by the (target_level-acutal_level) provides the fade    */
/* rate which is loaded into fade_step_size.                                  */
const unsigned long int fade_time_table[16] = 
{ OFF, FADE_TIME_1, FADE_TIME_2, FADE_TIME_3, FADE_TIME_4, FADE_TIME_5,
  FADE_TIME_6, FADE_TIME_7, FADE_TIME_8, FADE_TIME_9, FADE_TIME_A, FADE_TIME_B,
  FADE_TIME_C, FADE_TIME_D, FADE_TIME_E, FADE_TIME_F
};

/*** fast_fade_time_table *****************************************************/
/* This fast fade time is to be used when fade time is 0.                     */
const unsigned long int fast_fade_time_table[28] = 
{ OFF, FAST_FADE_TIME_1, FAST_FADE_TIME_2, FAST_FADE_TIME_3, FAST_FADE_TIME_4,
  FAST_FADE_TIME_5, FAST_FADE_TIME_6, FAST_FADE_TIME_7, FAST_FADE_TIME_8,
  FAST_FADE_TIME_9, FAST_FADE_TIME_A, FAST_FADE_TIME_B, FAST_FADE_TIME_C,
  FAST_FADE_TIME_D, FAST_FADE_TIME_E, FAST_FADE_TIME_F, FAST_FADE_TIME_10,
  FAST_FADE_TIME_11, FAST_FADE_TIME_12, FAST_FADE_TIME_13, FAST_FADE_TIME_14,
  FAST_FADE_TIME_15, FAST_FADE_TIME_16, FAST_FADE_TIME_17, FAST_FADE_TIME_18,
  FAST_FADE_TIME_19, FAST_FADE_TIME_1A, FAST_FADE_TIME_1B
};

/******************************************************************************/
/*execute_command                                                             */
/******************************************************************************/
unsigned char TI_DALI_Command(unsigned char *rx_msg)
{ 
  unsigned char response=0;
  unsigned char update_value;
  //unsigned ret_flag;  //SJ4300812 - Added by SJ
  //SJ1171212 unsigned int temp_room_ID;

  if(rx_msg[0] & BIT0)  // Selector  bit  //SJ5180512 - If bit 0 is 0, ordinary address. If bit 0 is 1, direct address.
  { /* Command is found in rx_msg[1] Commands 0 through 255 */
    if((dapc_count < DAPC_TIMEOUT) && (rx_msg[1] != ENABLE_DAPC_SEQ))  //SJ4060912 - DAPC_TIMEOUT is around 200ms
    {
      dapc_count = DAPC_TIMEOUT; // receiving another command stops the 
    }                              // sequence
    switch(rx_msg[1]) 
    {
      case TURN_OFF:
      #ifndef DEV_7  //SJ4230513 - If device type is not 7.
          IE1 &= ~WDTIE; /* Critical update */
          actual_level=OFF;
          status_information &= ~(FADE_RUNNING+LAMP_ARC_POWER_ON);
          status_information &= ~(LIMIT_ERROR+STATUS_POWER_FAILURE);
          IE1 |= WDTIE;
          TI_DALI_Update_Callback();
//SJ3220513 - Above codes is being replaced with codes meant for device type 7
      #else
          P2OUT &= 0xF7;  //SJ3220513 - RELAY_ACCTRL off, P2.3 (11110111)
      #endif
          break;
      case UP:
//SJ3220513 - This case is not relevant to device type 7
      #ifndef DEV_7  //SJ4230513 - Not relevant for device type 7.
          if((actual_level != max_level) && 
              (status_information & LAMP_ARC_POWER_ON))
          {
            // Fade up for 200ms
            IE1 &= ~WDTIE; /* Critical update */
            fade_step_size = (unsigned long int)fade_rate_table[fade_rate];  //SJ4130912 - With default fade rate fade_rate_table[] will yield 21
            if(actual_level > (max_level - fade_200ms[fade_rate]))
            {
              target_level = max_level;
            }
            else
            { 
              target_level = actual_level + fade_200ms[fade_rate];  //SJ4130912 - With default fade rate fade_200ms[] will yield 8
            }
            status_information |= FADE_RUNNING;
            IE1 |= WDTIE;
            break;
          }
          status_information |= LIMIT_ERROR;
      #endif
          break;
      case DOWN:
//SJ3220513 - This case is not relevant to device type 7
      #ifndef DEV_7  //SJ4230513 - Not relevant for device type 7.
          if((actual_level!=min_level) && 
              (status_information & LAMP_ARC_POWER_ON))
          {
            // Fade down for 200ms 
            IE1 &= ~WDTIE; /* Critical update */
            fade_step_size = (unsigned long int)fade_rate_table[fade_rate];
            if(actual_level < (min_level+fade_200ms[fade_rate]))  //SJ1240912 - With default fade_rate, fade_200ms yields 8.8
            { 
              target_level=min_level;
            }
            else
            {
              target_level = actual_level-fade_200ms[fade_rate];
            }
            status_information |= FADE_RUNNING;
            IE1 |= WDTIE;
            break;
          }
          status_information |= LIMIT_ERROR;
      #endif
          break;
      case STEP_UP:
//SJ3220513 - This case is not relevant to device type 7
      #ifndef DEV_7  //SJ4230513 - Not relevant for device type 7.
          if((actual_level!=max_level) && 
              (status_information & LAMP_ARC_POWER_ON))
          {
            IE1 &= ~WDTIE; /* Critical update */
            actual_level++;
            status_information &= ~FADE_RUNNING;
            IE1 |= WDTIE;
            TI_DALI_Update_Callback();
          }
      #endif
          break;
      case STEP_DOWN:
//SJ3220513 - This case is not relevant to device type 7
      #ifndef DEV_7  //SJ4230513 - Not relevant for device type 7.
          if((actual_level!=min_level) && 
              (status_information & LAMP_ARC_POWER_ON))
          {
            IE1 &= ~WDTIE; /* Critical update */
            actual_level--;
            status_information &= ~FADE_RUNNING;
            IE1 |= WDTIE;
            TI_DALI_Update_Callback();
          }
      #endif
          break;
      case RECALL_MAX_LEVEL:
      #ifndef DEV_7  //SJ4230513 - If device type is not 7.
          IE1 &= ~WDTIE; /* Critical update */
          actual_level = max_level;
          //actual_level = 117;//SJ4160611 - Strictly for debugging only
          status_information &= ~FADE_RUNNING;
          status_information |= LAMP_ARC_POWER_ON;
          status_information &= ~STATUS_POWER_FAILURE;
          IE1 |= WDTIE;            
          TI_DALI_Update_Callback();
          gstate_var.error_delay = 10;  //SJ2110912 - Aztech
//SJ3220513 - Above codes is being replaced with codes meant for device type 7
      #else
          P2OUT |= 0x08;  //SJ3220512 - RELAY_ACCTRL on, P2.3 (xxxx1xxx)
      #endif
          break;
      case RECALL_MIN_LEVEL:
//SJ3220513 - This case is not relevant to device type 7
      #ifndef DEV_7  //SJ4230513 - Not relevant for device type 7.
          IE1 &= ~WDTIE; /* Critical update */
          actual_level = min_level;
          status_information &= ~FADE_RUNNING;
          status_information &= ~STATUS_POWER_FAILURE; 
          status_information |= LAMP_ARC_POWER_ON;
          IE1 |= WDTIE;
          TI_DALI_Update_Callback();
          gstate_var.error_delay = 10;  //SJ2110912 - Aztech
      #endif
          break;
      case STEP_DOWN_AND_OFF:
//SJ3220513 - This case is not relevant to device type 7
      #ifndef DEV_7  //SJ4230513 - Not relevant for device type 7.
          IE1 &= ~WDTIE; /* Critical update */
          status_information |= LIMIT_ERROR;
          if (actual_level > min_level)
          {
            actual_level--;
            status_information &= ~LIMIT_ERROR;
          }
          else 
          {// Turn off the PWMs
            actual_level=OFF;               
          }
          status_information &= ~FADE_RUNNING;
          status_information &= ~STATUS_POWER_FAILURE;
          IE1 |= WDTIE;
          TI_DALI_Update_Callback();                       
      #endif
          break;
      case ON_AND_STEP_UP:
//SJ3220513 - This case is not relevant to device type 7
      #ifndef DEV_7  //SJ4230513 - Not relevant for device type 7.
          IE1 &= ~WDTIE; /* Critical update */
          if (actual_level==OFF)
          { //Turn back on PWMs 
            actual_level = min_level;
          }
          status_information |= LIMIT_ERROR;
          if (actual_level < max_level)
          {
            actual_level++;
            status_information &= ~LIMIT_ERROR;
          }
          status_information &= ~FADE_RUNNING;
          status_information &= ~STATUS_POWER_FAILURE;
          status_information |= LAMP_ARC_POWER_ON;
          IE1 |= WDTIE;                       
          TI_DALI_Update_Callback();
          gstate_var.error_delay = 10;  //SJ2110912 - Aztech
      #endif
          break;
#ifdef ENABLE_DAPC_SEQ
      case ENABLE_DAPC_SEQ:
          IE1 &= ~WDTIE; /* Critical update */
          dapc_count = 0;
          IE1 |= WDTIE;                                
          break;
#endif

//SJ2181212 - Start of Aztech proprietary extended commands.
#if 1
      //SJ2181212 - Indicates what follows is Aztech proprietary extended commands. The actual command will be
      //SJ2181212 - located at location 67 of incoming data buffer.
      case AZ_PROPRIETARY_CMD:
          AZ_Extended_Command();
          break;

      case AZ_RANDOMISE_DEVICE:  //SJ1101212 - equiv 0x0B
          AZ_Randomised_Long_Address();
          break;
#endif
//SJ2181212 - End of Aztech proprietary extended commands.

      /*** Commands 16 through 31 are handled within the default case *********/

      /*** CONFIGURATION COMMANDS 32 -129 *************************************/
      /* These commands must be sent 2x within 100ms                          */
//SJ5070912 - SJNOTE: Don't think we can achieve 100ms with PLC; need to increase to 700ms
#ifdef RESET
      case RESET:
//SJ3200313 - SJBYPASS          if(config_count < CONFIG_TIMEOUT)  //SJ4060912 - CONFIG_TIMEOUT is around 100ms
//SJ3200313 - SJBYPASS          {
            /* reset specified DALI variables */
            power_on_level = POWER_ON_LEVEL_RESET;
            system_failure_level = SYSTEM_FAILURE_LEVEL_RESET;
            min_level = MIN_LEVEL_RESET;
            max_level = MAX_LEVEL_RESET;
            fade_rate = FADE_RATE_RESET;
            fade_time = FADE_TIME_RESET;
            // short_address no change
            group_0_7 = GROUP_0_7_RESET;  //SJ2120612 - equiv 0x00
            group_8_f = GROUP_8_F_RESET;  //SJ2120612 - equiv 0x00
            random_address_byte_h = RANDOM_ADDR_BYTE_RESET;  //SJ2120612 - equiv 0xFF
            random_address_byte_m = RANDOM_ADDR_BYTE_RESET;
            random_address_byte_l = RANDOM_ADDR_BYTE_RESET;
            actual_level = ACTUAL_LEVEL_RESET;
            search_address_byte_h = MASK;  //SJ2120612 - equiv 0xFF
            search_address_byte_m = MASK;
            search_address_byte_l = MASK;
            status_information &= ~STATUS_INFO_CLR_RESET;
            status_information |= STATUS_INFO_SET_RESET;

        //SJ5040113 - Below codes are added to implement removal and addition of existing and new devices
        #if 1
            //SJ2150113 - iSW lighting device
            for (update_value=0; update_value<EXT_SCENE_SZ; update_value++)  //SJ2150113 - EXT_SCENE_SZ is 64
                gextended_scene[update_value] = 0xFF;

            TI_DALI_Flash_Update(FWKEY, 1);
            gstate_var.general_ctrl_flag |= SYS_RESET_ON;
            //SYS_RESET_OFF

            short_address = 0;  //SJ2220113 - SJEXT_ADDR: No change
            gstate_var.Room_ID = 0x0000;

            for(update_value=0; update_value<NUMBER_OF_SCENES; update_value++)
                scene[update_value] = 0xFF;

            flash_update_request = 1;
        #endif
//SJ3200313 - SJBYPASS          }
//SJ3200313 - SJBYPASS          else
//SJ3200313 - SJBYPASS          {
//SJ3200313 - SJBYPASS            config_count=0;
//SJ3200313 - SJBYPASS          }
          break;
#endif
#ifdef STORE_ACTUAL_LEVEL_IN_THE_DTR
      case STORE_ACTUAL_LEVEL_IN_THE_DTR:
          if(config_count < CONFIG_TIMEOUT)
          {
            data_transfer_register = actual_level;
          }
          else
          {
            config_count=0;
          }
          break;
#endif
#ifdef STORE_THE_DTR_AS_MAX_LEVEL
      case STORE_THE_DTR_AS_MAX_LEVEL:
          if(config_count < CONFIG_TIMEOUT)
          {
            if(data_transfer_register == MASK)
            {
              max_level = MASK-1;
            }
            else
            {
              max_level = data_transfer_register;
            }
            if(actual_level > max_level)
            {
              IE1 &= ~WDTIE; /* Critical update */
              actual_level = max_level;
              status_information &= ~FADE_RUNNING;
              IE1 |= WDTIE;
              TI_DALI_Update_Callback();
            }
            flash_update_request=1;
          }
          else 
          {
            config_count=0;
          }
          break;
#endif
#ifdef STORE_THE_DTR_AS_MIN_LEVEL
      case STORE_THE_DTR_AS_MIN_LEVEL:  /* Command 43 */
          if(config_count < CONFIG_TIMEOUT)
          {
            if(data_transfer_register < PHYS_MIN_LEVEL)
            {
              min_level = PHYS_MIN_LEVEL;
            }
            else
            {
              min_level = data_transfer_register;
            }
            if((actual_level < min_level) && (actual_level != OFF))
            {
              IE1 &= ~WDTIE; /* Critical update */
              actual_level=min_level;
              status_information &= ~FADE_RUNNING; 
              IE1 |= WDTIE;
              TI_DALI_Update_Callback();
            }
            flash_update_request=1;
          }
          else 
          {
            config_count=0;
          }
          break;
#endif
#ifdef STORE_THE_DTR_AS_SYSTEM_FAILURE_LEVEL
      case STORE_THE_DTR_AS_SYSTEM_FAILURE_LEVEL:  /* Command 44 */
          if(config_count < CONFIG_TIMEOUT)
          {
            system_failure_level = data_transfer_register;
            flash_update_request=1;
          }
          else
          {
            config_count=0;
          }
          break;
#endif
#ifdef STORE_THE_DTR_AS_POWER_ON_LEVEL
      case STORE_THE_DTR_AS_POWER_ON_LEVEL:  /* Command 45 */
          if(config_count < CONFIG_TIMEOUT)
          {
            power_on_level = data_transfer_register;
            flash_update_request=1;
          }
          else 
          {
            config_count=0;
          }
          break;
#endif
#ifdef STORE_THE_DTR_AS_FADE_TIME
      case STORE_THE_DTR_AS_FADE_TIME:  /* Command 46 */
          if(config_count < CONFIG_TIMEOUT)
          {
            fade_time = data_transfer_register;
            flash_update_request=1;
          }
          else 
          {
            config_count=0;
          }
          break;
#endif
#ifdef STORE_THE_DTR_AS_FADE_RATE
      case STORE_THE_DTR_AS_FADE_RATE:  /* Command 47 */
          if(config_count < CONFIG_TIMEOUT)
          {
            fade_rate = data_transfer_register;
            flash_update_request=1;
          }
          else 
          {
            config_count=0;
          }
          break;
      /*** Commands 48 through 63 are reserved ********************************/
      /*** Commands 64 through 127 are handled within the default case ********/
#endif
#ifdef STORE_THE_DTR_AS_SHORT_ADDRESS
      case STORE_THE_DTR_AS_SHORT_ADDRESS:  /* Command 128 */
          if(config_count < CONFIG_TIMEOUT)
          {
//SJ2220113 - SJEXT_ADDR: This will have to change. Before issueing this command, special commands DATA_TRANSFER_REGISTER_1 (0xC3)
//            and DATA_TRANSFER_REGISTER_2 (0xC5) will have to be issued prior to this.
#ifdef SJEXT_ADDR
            short_address = (data_transfer_register1 << 8) | data_transfer_register2;
#else  //SJ3200213 - ORG
            short_address = data_transfer_register;
#endif
            flash_update_request=1;
            AZ_DALI_Response(0x80);  //SJ4201212 - #define STORE_THE_DTR_AS_SHORT_ADDRESS 0x80 defined in command.h
          }
          else 
          {
            config_count=0;
          }
          break;
#endif
#ifdef ENABLE_WRITE_MEMORY
      case ENABLE_WRITE_MEMORY:  /* Command 129 */
          /*** This command is not supported **********************************/
          break;
      /*** Commands 130 through 143 are reserved ******************************/
      /*** Query commands 144-223    */
#endif
      case QUERY_STATUS:  /* Command 144 **************************************/
        #if 0  //SJ4160812 - ORG
          response = status_information;  //SJ2220512 - Refer to dali.h macroes' Status Bits line 301 to 308. Default is STATUS_POWER_FAILURE
        //SJTIDY #else  //SJ4160812 - Replaced
          AZ_DALI_Response(2);  //SJ1270812 #define DEVICE_QUERY  2 in PLC_Comm.h
        #endif
          break;
      case QUERY_CONTROL_GEAR:  /* Command 145 */
          //SJTIDY response = YES;  //SJ2220512 - equiv 0xFF
          break;
      case QUERY_LAMP_FAILURE:  /* Command 146 */
          if(LAMP_FAILURE & status_information)  //SJ2220512 - BIT1
          {
            response = YES;
          }
          break;
      case QUERY_LAMP_POWER_ON:  /* Command 147 */
          if(LAMP_ARC_POWER_ON & status_information)  //SJ2220512 - Bit2
          {
            response = YES;
          }
          break;
      case QUERY_LIMIT_ERROR:  /* Command 148 */
        #if 0  //SJ4070313 SJTIDY
          if(LIMIT_ERROR & status_information)  //SJ2220512 - Bit3
          {
            response = YES;
          }
        #endif
          break;
      case QUERY_RESET_STATE:  /* Command 149 */
        #if 0  //SJ4070313 SJTIDY
          if(RESET_STATE & status_information)  //SJ2220512 - Bit5
          {
            response = YES;
          }
        #endif
          break;
      case QUERY_MISSING_SHORT_ADDRESS:  /* Command 150 */
        #if 0  //SJ4070313 SJTIDY
          if(MISSING_SHORT_ADDRESS & status_information)  //SJ2220512 - Bit6
          {
            response = YES;
          }
        #endif
          break;
      //SJ4070313 - All queries from 151 to 165 will be grouped into one generic query.
      case QUERY_VERSION_NUMBER:  /* Command 151 */
        #if 0  //SJ2180912 - ORG.
          response = version_number;
        //SJTIDY #else  //SJ2180912 - Replaced.
          //response = 1;
          //gstate_var.DALI_response = version_number;
          AZ_DALI_Response(3);  //SJ1240912 #define GENERIC_QUERY  3 in PLC_Comm.h
        #endif
          break;
      case QUERY_CONTENT_DTR:  /* Command 152 */
        #if 0  //SJ1240912 - ORG
          response = data_transfer_register;
        //SJTIDY #else  //SJ1240912 - Replaced
          AZ_DALI_Response(3);  //SJ1240912 #define GENERIC_QUERY  3 in PLC_Comm.h
        #endif
          break;
      case QUERY_DEVICE_TYPE:  /* Command 153 */
        #if 0  //SJ2180912 - ORG.
          response = device_type;
        //SJTIDY #else  //SJ2180912 - Replaced
          //response = 1;
          //gstate_var.DALI_response = device_type;
          AZ_DALI_Response(3);  //SJ1240912 #define GENERIC_QUERY  3 in PLC_Comm.h
        #endif
          break;
      case QUERY_PHYSICAL_MINIMUM:  /* Command 154 */
        #if 0  //SJ2180912 - ORG
          response = phys_min_lvl;
        //SJTIDY #else  //SJ2180912 - Replaced
          //response = 1;
          //gstate_var.DALI_response = phys_min_lvl;
          AZ_DALI_Response(3);  //SJ1240912 #define GENERIC_QUERY  3 in PLC_Comm.h
        #endif
          break;
      case QUERY_POWER_FAILURE:  /* Command 155 */
          if(STATUS_POWER_FAILURE & status_information)  //SJ2220512 - Bit7
          {
            response = YES;
          }
          break;
      case QUERY_CONTENT_DTR1:  /* Command 156 */
        #if 0  //SJ1240912 - ORG
          response = data_transfer_register1;
        //SJTIDY #else  //SJ1240912 - Replaced
          AZ_DALI_Response(3);  //SJ1240912 #define GENERIC_QUERY  3 in PLC_Comm.h
          break;
        #endif
      case QUERY_CONTENT_DTR2:  /* Command 157 */
        #if 0  //SJ1240912 - ORG
          response = data_transfer_register2;
        //SJTIDY #else  //SJ1240912 - Replaced
          AZ_DALI_Response(3);  //SJ1240912 #define GENERIC_QUERY  3 in PLC_Comm.h
          break;
        #endif
      /*** Commands 158 through 159 are reserved  *****************************/
      case QUERY_ACTUAL_LEVEL:  /* Command 160 */
        #if 0  //SJ2110912 - ORG.
          response = actual_level;
        //SJTIDY #else  //SJ2110912 - Replaced
          //response = 1;
          //gstate_var.DALI_response = actual_level;
          AZ_DALI_Response(3);  //SJ1240912 #define GENERIC_QUERY  3 in PLC_Comm.h
        #endif
          break;
      case QUERY_MAX_LEVEL:  /* Command 161 */
        #if 0  //SJ2180912 - ORG
          response = max_level;
        //SJTIDY #else  //SJ2180912 - Replaced
          //response = 1;
          //gstate_var.DALI_response = max_level;
          AZ_DALI_Response(3);  //SJ1240912 #define GENERIC_QUERY  3 in PLC_Comm.h
        #endif
          break;
      case QUERY_MIN_LEVEL:  /* Command 162 */
        #if 0  //SJ2180912 - ORG
          response = min_level;
        //SJTIDY #else  //SJ2180912 - Replaced
          //response = 1;
          //gstate_var.DALI_response = min_level;
          AZ_DALI_Response(3);  //SJ1240912 #define GENERIC_QUERY  3 in PLC_Comm.h
        #endif
          break;
      case QUERY_POWER_ON_LEVEL:  /* Command 163 */
        #if 0  //SJ2180912 - ORG
          response = power_on_level;
        //SJTIDY #else  //SJ2180912 - Replaced
          //response = 1;
          //gstate_var.DALI_response = power_on_level;
          AZ_DALI_Response(3);  //SJ1240912 #define GENERIC_QUERY  3 in PLC_Comm.h
        #endif
          break;
      case QUERY_SYSTEM_FAILUE_LEVEL:  /* Command 164 */
        #if 0  //SJ2180912 - ORG
          response = system_failure_level;
        //SJTIDY #else  //SJ2180912 - Replaced
          //response = 1;
          //gstate_var.DALI_response = system_failure_level;
          AZ_DALI_Response(3);  //SJ1240912 #define GENERIC_QUERY  3 in PLC_Comm.h
        #endif
          break;
      case QUERY_FADE_LEVEL:  /* Command 165 */
        #if 0  //SJ1240912 - ORG
          response = fade_time;
          response = response << 4;
          response &= 0xF0;
          response |= fade_rate;
        //SJTIDY #else  //SJ1240912 - Replaced
          AZ_DALI_Response(3);  //SJ1240912 #define GENERIC_QUERY  3 in PLC_Comm.h
        #endif
          break;
      /*** Commands 166 through 175 are reserved ******************************/
      /*** Commands 176 through 191 are Query Scene Level *********************/
      case QUERY_GROUPS_0_7:  /* Command 192 */
        #if 0  //SJ1170912 - ORG. DALI
          response = group_0_7;
        #else  //SJ1170912 - Replaced. Aztech
          ;//SJTBD AZ_DALI_Response(192);  //SJ1170912 - #define QUERY_GROUPS_0_7 192 in command.h
        #endif
          break;
      case QUERY_GROUPS_8_F:  /* Command 193 */
        #if 0  //SJ1170912 - ORG. DALI
          response = group_8_f;
        #else  //SJ1170912 - Replaced. Aztech
          ;//SJTBD AZ_DALI_Response(192);  //SJ1170912 - #define QUERY_GROUPS_0_7 192 in command.h
        #endif
          break;
      case QUERY_RANDOM_ADDR_H:  /* Command 194 */
        #if 0  //SJ4130912 - ORG. DALI
          response = random_address_byte_h;
        #else  //SJ4130912 - Replaced. Aztech
        if (short_address == 0)  //SJ2220113 - SJEXT_ADDR: No change
        {
            AZ_DALI_Response(194);  //SJ4130912 - #define QUERY_RANDOM_ADDR_H 194 in command.h
        }
        #endif
          break;
      case QUERY_RANDOM_ADDR_M:  /* Command 195 */
        /*   //SJ2120313 - Skip from compilation
        #if 0  //SJ4130912 - ORG. DALI
          response = random_address_byte_m;
        #else  //SJ4130912 - Replaced. Aztech
        if (short_address == 0)  //SJ2220113 - SJEXT_ADDR: No change
        {
            AZ_DALI_Response(194);  //SJ4130912 - #define QUERY_RANDOM_ADDR_H 194 in command.h
        }
        #endif
        */
          break;
      case QUERY_RANDOM_ADDR_L:  /* Command 196 */
        /*   //SJ2120313 - Skip from compilation
        #if 0  //SJ4130912 - ORG. DALI
          response = random_address_byte_l;
        #else  //SJ4130912 - Replaced. Aztech
        if (short_address == 0)  //SJ2220113 - SJEXT_ADDR: No change
        {
            AZ_DALI_Response(194);  //SJ4130912 - #define QUERY_RANDOM_ADDR_H 194 in command.h
        }
        #endif
        */
          break;
      case READ_MEMORY_LOCATION: /* Command 197 */
        #if 0  //SJ4070313 - Don't think is usefull
          if(data_transfer_register1 == BANK_0)
          {
            /* Read from BANK 0 */
            if(data_transfer_register > LAST_BNK0_ADDR)
            {
              break;
            }
            response = memory_bank_0[data_transfer_register];
            data_transfer_register++;  //SJ5140912 - Increase to next element for subsequent READ_MEMORY_LOCATION command
            if(data_transfer_register > LAST_BNK0_ADDR)
            {
              break;
            }
            data_transfer_register2 = memory_bank_0[data_transfer_register]; 
          }
        #endif
          break;
      /*** Commands 198 through 223 are reserved             */
      /*** Commands 224 through 227 are not supported        */
      case STORE_DTR_AS_FAST_FADE_TIME:  /* Command 228 */
          if((config_count < CONFIG_TIMEOUT) && enable_device_type)
          {
            if((data_transfer_register == 0) || 
                (data_transfer_register > min_fast_fade_time))
            {
              fast_fade_time = data_transfer_register;
            }
            else
            {
              fast_fade_time = min_fast_fade_time;
            }
            enable_device_type = 0;
          }
          else 
          {
            config_count=0;
          }
          break;
      /***  Commands 229 through 236 are reserved *****************************/
      case QUERY_GEAR_TYPE:  /* Command 237 */
        #if 0  //SJ4070313 - Don't think is usefull
          if(enable_device_type)
            {
              response = gear_type;  //SJ5140912 - gear_type = 0. What does it mean?????
              enable_device_type = 0;
            }
        #endif
          break;
      case QUERY_DIMMING_CURVE:  /* Command 238 */
        #if 0  //SJ4070313 - Don't think is usefull
          if(enable_device_type)
          {
            response = dimming_curve;  //SJ5140912 - It seems like dimming_curve var will never change its content!!!
            enable_device_type = 0;
          }
        #endif
          break;
      case QUERY_POSSIBLE_OPERATING_MODES:  /* Command 239 */
        #if 0  //SJ4070313 - Don't think is usefull
          if(enable_device_type)
          {
            response = possible_operating_modes;  //SJ5140912 - possible_operationg_modes = 0
            enable_device_type = 0;
          }
        #endif
          break;
      case QUERY_FEATURES:  /* Command 240 */
        #if 0  //SJ4070313 - Don't think is usefull
          if(enable_device_type)
          {
            response = features;  //SJ5140912 - features = 0
            enable_device_type = 0;
          }
        #endif
          break;
      case QUERY_FAILURE_STATUS:  /* Command 241 */
        #if 0  //SJ4070313 - Don't think is usefull
          if(enable_device_type)
          {
            response = failure_status;
            enable_device_type = 0;
          }
        #endif
          break;
/*  Commands 242 through 251 are not supported        */
      case QUERY_OPERATING_MODE:  /* Command 252 */
        #if 0  //SJ4070313 - Don't think is usefull
          if(enable_device_type)
          {
            response = operating_mode;
            enable_device_type = 0;
          }
        #endif
          break;
      case QUERY_FAST_FADE_TIME:  /* Command 253 */
        #if 0  //SJ4070313 - Don't think is usefull
          if(enable_device_type)
          {
            response = fast_fade_time;
            enable_device_type = 0;
          }
        #endif
          break;
      case QUERY_MIN_FAST_FADE_TIME:  /* Command 254 */
        #if 0  //SJ4070313 - Don't think is usefull
          if(enable_device_type)
          {
            response = min_fast_fade_time;  //SJ5140912 - min_fast_fade_time = 11
            enable_device_type = 0;
          }
        #endif
          break;
      case QUERY_EXTENDED_VERSION_NUMBER:  /* Command 255 */
        #if 0  //SJ4070313 - Don't think is usefull
          if(enable_device_type)
          {
            response = extended_version_number;  //SJ5140912 - extended_version_number = 1
            enable_device_type = 0;
          }
        #endif
          break;
      default:
          /*** Commands 16 through 31 are GO_TO_SCENE_X ***********************/
          if(((GO_TO_SCENE_0-1) < rx_msg[1]) &&
             (rx_msg[1] < (GO_TO_SCENE_F+1)))  //SJ5070912 - True if Scene command is within valid range of allowed scene
          {
//SJ4180713 - SJTO_TRY: If UART blocking is not able to overcome the issue, try using FADE_RUNNING flag
//if(status_information & FADE_RUNNING)  break;
            if(scene[rx_msg[1]-GO_TO_SCENE_0] == MASK)  //SJ4131212 - If the corresponding scene is not set, do nothing.
            {
              break;  // Do nothing
            }
      #ifdef DEV_7
            else if(scene[rx_msg[1]-GO_TO_SCENE_0] == 0)
            {
                P2OUT &= 0xF7;  //SJ4230513 - RELAY_ACCTRL off, P2.3 (11110111)
            }
            //SJ1300913 - Added 
            else if(scene[rx_msg[1]-GO_TO_SCENE_0] == 0xFE)
            {
                P2OUT |= 0x08;  //SJ1300913 - RELAY_ACCTRL on, P2.3 (xxxx1xxx)
            }
//SJ3220513 - Below codes is being replaced with codes meant for device type 7
      #else  //SJ4230513 - If device type is not 7 (Original code for TI DALI source).
            IE1 &= ~WDTIE; /* Critical Update */
            status_information &= ~STATUS_POWER_FAILURE;
            target_level = scene[rx_msg[1]-GO_TO_SCENE_0];
            if(target_level > max_level)//;  //SJ3090113 - Why empty sttmt here? What happen to target_level = max_level;?????
            {
              target_level = max_level;
            }
            if(target_level < min_level)
            {
              ;//target_level = min_level;  //SJ5260413 - ORG. TI code set target_level to 90
            }
            /* fade to arc_level stored for scene */
            /* rx_msg[] is 16 through 31 */
            //if((target_level == actual_level) || (fade_time == OFF))  //SJ4230413 - ORG
            //SJ3240413 - Temporily solutin to overcome min_level issue
            if((target_level == actual_level) || (fade_time == OFF) || (target_level == OFF))
            {
              /* If already at the requested then result is nothing. */
              /* If no fade, then go to level immediately.           */
              status_information &= ~FADE_RUNNING;
              actual_level = scene[rx_msg[1]-GO_TO_SCENE_0];
              TI_DALI_Update_Callback();
            }
            else
            {
              /* Fade to level */
              status_information |= FADE_RUNNING;
              /* Calculate fade rate based upon fade time and change in power */
              /* level. */
              if(target_level > actual_level)
              {//SJ1220713 - fade_time_table[fade_time] compute to 716.
               //SJ1220713 - If target_level is 254 and actual_level is 0, 716/254=2 (From off to max)
               //SJ1220713 - If difference between target_level and actual_level is 1, 716/1=716
                fade_step_size = fade_time_table[fade_time]/(target_level-actual_level);
              }
              else
              {
                fade_step_size = fade_time_table[fade_time]/(actual_level-target_level);
              }
              fade_count = 0;  //SJ5260413 - Added by SJ to reset counter. Counter out of sync if prev fading is incomplete
            }
            IE1 |= WDTIE;
      #endif
            gstate_var.error_delay = 10;  //SJ2110912 - Aztech
            break;
          }
          /*** Commands 64 through 79 are STORE THE DTR AS SCENE **************/
          if(((STORE_THE_DTR_AS_SCENE_0-1) < rx_msg[1]) &&
              (rx_msg[1] < (STORE_THE_DTR_AS_SCENE_F+1)))  //SJ5070912 - True if Scene command is within valid range of allowed scene
          {
//SJ3200313 - SJBYPASS            if(config_count < CONFIG_TIMEOUT)
//SJ3200313 - SJBYPASS            {
              scene[rx_msg[1]-STORE_THE_DTR_AS_SCENE_0]=data_transfer_register;
              flash_update_request=1;
//SJ3200313 - SJBYPASS            }
//SJ3200313 - SJBYPASS            else
//SJ3200313 - SJBYPASS            {
//SJ3200313 - SJBYPASS              config_count=0;  //SJ5070912 - Start the countdown for duplicate command within 100ms
//SJ3200313 - SJBYPASS            }
            break;
          }
          /*** Commands 80 through 95 are REMOVE FROM SCENE *******************/
          if(((REMOVE_FROM_SCENE_0-1) < rx_msg[1]) &&
             (rx_msg[1] < (REMOVE_FROM_SCENE_F+1)))
          {
//SJ5111013 - SJBYPASS            if(config_count < CONFIG_TIMEOUT)  //SJ5111013 - SJTODO: To by pass (SJBYPASS)
//SJ5111013 - SJBYPASS            {
              scene[rx_msg[1]-REMOVE_FROM_SCENE_0] = MASK;
              flash_update_request=1;
//SJ5111013 - SJBYPASS            }
//SJ5111013 - SJBYPASS            else 
//SJ5111013 - SJBYPASS            {
//SJ5111013 - SJBYPASS              config_count=0;
//SJ5111013 - SJBYPASS            }
            break;
          }
          /*** Commands 96 through 111 are ADD TO GROUP ***********************/
          if(((ADD_TO_GROUP_0-1) < rx_msg[1]) &&
             ( rx_msg[1] < (ADD_TO_GROUP_8)))  //SJ4130912 - True if group command represents group 1 and 8
          {
//SJ5111013 - SJBYPASS            if(config_count < CONFIG_TIMEOUT)  //SJ5111013 - SJTODO: To by pass (SJBYPASS)
//SJ5111013 - SJBYPASS            {
              update_value = 1;
              update_value = update_value<<(rx_msg[1]-ADD_TO_GROUP_0);
              group_0_7 |= update_value;  //SJ4130912 - Turn on the bit that represent the group command
              flash_update_request=1;
//SJ5111013 - SJBYPASS            }
//SJ5111013 - SJBYPASS            else 
//SJ5111013 - SJBYPASS            {
//SJ5111013 - SJBYPASS              config_count=0;
//SJ5111013 - SJBYPASS            }
            break;
          }
          if(((ADD_TO_GROUP_7) < rx_msg[1]) &&
             (rx_msg[1] < (ADD_TO_GROUP_F+1)))  //SJ4130912 - True if group command represents group 9 and 16
          {
//SJ5111013 - SJBYPASS            if(config_count < CONFIG_TIMEOUT)  //SJ5111013 - SJTODO: To by pass (SJBYPASS)
//SJ5111013 - SJBYPASS            {
              update_value = 1;
              update_value = update_value<<(rx_msg[1]-ADD_TO_GROUP_8);
              group_8_f |= update_value;  //SJ4130912 - Turn on the bit that represent the group command
              flash_update_request=1;
//SJ5111013 - SJBYPASS            }
//SJ5111013 - SJBYPASS            else
//SJ5111013 - SJBYPASS            {
//SJ5111013 - SJBYPASS              config_count=0;
//SJ5111013 - SJBYPASS            }
            break;
          }
          /*** Commands 112 through 127 are REMOVE FROM GROUP *****************/
          if(((REMOVE_FROM_GROUP_0-1) < rx_msg[1]) &&
             (rx_msg[1] < (REMOVE_FROM_GROUP_8)))  //SJ4130912 - True if group command represents group 1 and 8
          {
//SJ5111013 - SJBYPASS            if(config_count < CONFIG_TIMEOUT)  //SJ5111013 - SJTODO: To by pass (SJBYPASS)
//SJ5111013 - SJBYPASS            {
              update_value = 1;
              update_value= update_value<<(rx_msg[1]-REMOVE_FROM_GROUP_0);
              update_value = ~update_value;  //SJ4130912 - Turn off the bit that represent the group command
              group_0_7 &= update_value;
              flash_update_request=1;
//SJ5111013 - SJBYPASS            }
//SJ5111013 - SJBYPASS            else 
//SJ5111013 - SJBYPASS            {
//SJ5111013 - SJBYPASS              config_count=0;
//SJ5111013 - SJBYPASS            }
            break;
          }
          if(((REMOVE_FROM_GROUP_7) < rx_msg[1]) &&
             (rx_msg[1] < (REMOVE_FROM_GROUP_F+1)))  //SJ4130912 - True if group command represents group 9 and 16
          {
//SJ5111013 - SJBYPASS            if(config_count < CONFIG_TIMEOUT)  //SJ5111013 - SJTODO: To by pass (SJBYPASS)
//SJ5111013 - SJBYPASS            {
              update_value = 1;
              update_value = update_value<<(rx_msg[1]-REMOVE_FROM_GROUP_8);
              update_value = ~update_value;  //SJ4130912 - Turn off the bit that represent the group command
              group_8_f &= update_value;
              flash_update_request=1;
//SJ5111013 - SJBYPASS            }
//SJ5111013 - SJBYPASS            else 
//SJ5111013 - SJBYPASS            {
//SJ5111013 - SJBYPASS              config_count=0;
//SJ5111013 - SJBYPASS            }
            break;
          }
          /*** Commands 176 through 191 are Query Scene Level *****************/
          if(((QUERY_SCENE_LEVEL_0-1) < rx_msg[1]) &&
             (rx_msg[1] < (QUERY_SCENE_LEVEL_F+1)))
          {
          #if 0  //SJ3190912 - ORG. DALI
            response = scene[rx_msg[1]-QUERY_SCENE_LEVEL_0];
          #else  //SJ3190912 - Replaced. Aztech
            AZ_DALI_Response(176);  //SJ1170912 - #define QUERY_SCENE_LEVEL_0 176 in command.h
          #endif
            break;
          }
    } /* END CASE STATEMENT*/
  } /* END IF */
  /*** direct arc power level command found in rx_msg[1] **********************/
  else
  {
    status_information &= ~STATUS_POWER_FAILURE;
    fade_count=0;       // restart counter
    /* If running at the fastest rate and not updated before next command */
    /* instantaneously go to target level.  IEC 62386_102 Section 9.5     */
    if((status_information & FADE_RUNNING) && (FADE_RATE == 1))
    {
      actual_level = target_level;  //SJ2230713 - What will be target_level value be?????
      status_information &= ~FADE_RUNNING;
    }
    if((rx_msg[1] == 0x00) || (rx_msg[1] == MASK)) /* fade to min and then off */
    {
  #ifndef DEV_7  //SJ4230513 - If device type is not 7. (Below codes are original from TI DALI source)
      if(rx_msg[1])  //SJ4230513 - Turn on lighting device
      {
        status_information &= ~FADE_RUNNING;
        actual_level = target_level;  //SJ2230713 - What will be target_level value be?????
      }
      else  //SJ4230513 - Turn off lighting device
      {
        if(status_information & LAMP_ARC_POWER_ON)
        { /* If the lamp is on, then perform fade */
          IE1 &= ~WDTIE; /* Critical Update */
          status_information |= (FADE_RUNNING);
          status_information &= ~(LIMIT_ERROR);
          //target_level=min_level-1;  //SJ2230713 - ORG TI code
          target_level = 0;  //SJ2230713 - AZtech modified
          /* Special case where target_level is less than min_level.  The fade  */
          /* handler in dali.c will recognize this condition and turn off after */
          /* the min_level has been reached.                                    */
          if(fade_time == OFF)
          {
            if(fast_fade_time != OFF)
            {
              fade_step_size = fast_fade_time_table[fast_fade_time];
              fade_step_size = fade_step_size/(actual_level-target_level);
            }
            else
            { 
              actual_level = min_level;
              fade_step_size = fade_time_table[fade_time];
              fade_step_size = fade_step_size/(actual_level-target_level);
            }
          }
        }
        else 
        {
          fade_step_size = fade_time_table[fade_time];
          fade_step_size = fade_step_size/(actual_level-target_level);
        }
        IE1 |= WDTIE; 
      }
  #else
      if(rx_msg[1] == MASK)  //SJ4230513 - Turn on lighting device
      {
        P2OUT |= 0x08;  //SJ4230513 - RELAY_ACCTRL on, P2.3 (xxxx1xxx)
      }
      else if (rx_msg[1] == 0)  //SJ4230513 - Turn off lighting device
      {
        P2OUT &= 0xF7;  //SJ1130812 - AC_CNTRL off, P2.3 (11110111)
      }
  #endif  //SJ4230513 - #ifndef DEV_7 ... #else ... #endif
    }
//SJ3220513 - Below else portion is not relevant to device type 7
  #ifndef DEV_7  //SJ4230513 - Not relevant for device type 7.
    else /* fade to level in rx_msg */
    {
      if(dapc_count < DAPC_TIMEOUT)
      {
        actual_level = target_level;
        dapc_count = 0;
      }
      IE1 &= ~WDTIE; /* Critical Update */
      if(!(status_information & LAMP_ARC_POWER_ON))
      { /* If the lamp is off, then turn on at minimum level */
        actual_level=min_level;
        status_information |= LAMP_ARC_POWER_ON;
        TI_DALI_Update_Callback();
      }
      /* limit value to min and max range */
      if(rx_msg[1] > max_level)
      {
        target_level = max_level;
        status_information |= LIMIT_ERROR;
      }
      else if(rx_msg[1] < min_level)
      {
        target_level = min_level;
        status_information |= LIMIT_ERROR;
      }
      else
      {
        target_level = rx_msg[1];
        status_information &= ~(LIMIT_ERROR);
      }
      if((target_level == actual_level) || (fade_time == OFF))
      { 
          /* if already at the requested then result is nothing. */
          /* if no fade, then go to level immediately,           */
          status_information &= ~FADE_RUNNING;
          actual_level = target_level;
          TI_DALI_Update_Callback();
      }
      else
      {
        /* Fade to level */
        status_information |= FADE_RUNNING;
        /* Calculate fade rate based upon fade time and change in power level */
        if(target_level > actual_level)
        {
          fade_step_size = fade_time_table[fade_time];
          fade_step_size = fade_step_size/(target_level-actual_level);
        }
        else
        {
          fade_step_size = fade_time_table[fade_time];
          fade_step_size = fade_step_size/(actual_level-target_level);
        }
        if(dapc_count<DAPC_TIMEOUT)
        {
          fade_step_size = (unsigned long)DAPC_SEQ_SIZE;
        }
      }
      IE1 |= WDTIE;
    }
  #endif  //SJ4230513 - #ifndef DEV_7 ... #endif
    gstate_var.error_delay = 10;  //SJ1081012 - Aztech
  }/* End ELSE, Direct Arc Power level Command */
  return response;
}


//SJ2181212 - Below codes are meant for Aztech proprietary extended commands. They do not form part of original
//SJ2181212 - DALI commands.
#if 1
unsigned char AZ_Extended_Command(void)
{
    unsigned char ext_cmd;
    short int ndx=0;

/*****
    unsigned char button_no;
    
    button_no = ggeneral_buffer[AZ_DAT_LOC];

    if (button_no > 0 && button_no < 12)
    {
        gswitch_oper[button_no-1] = ggeneral_buffer[AZ_DAT_LOC+1];
        //flash_update_request = 1;
        TI_DALI_Flash_Update(FWKEY, 1);
    }
*****/

    ext_cmd = ggeneral_buffer[AZ_DAT_LOC];

    switch(ext_cmd)  //SJ2181212 - AZ_DAT_LOC 67
    {
//SJ4201212 - Place those codes meant for light device here.

        case AZ_EXT_CMD_REVERSE:
            //ndx = ggeneral_buffer[AZ_DAT_LOC+1] - GO_TO_SCENE_0;
            //if (scene[ndx] > 0 && scene[ndx] < MASK)  //SJ4201212 - If true, need to activate off
            if (ggeneral_buffer[AZ_DAT_LOC+1] < AZ_EXT_SCENE_32)  //SJ2150113 - #define AZ_EXT_SCENE_32 0x20 defined in global_var.h
                ndx = scene[ggeneral_buffer[AZ_DAT_LOC+1] - GO_TO_SCENE_0];
            else
                ndx = gextended_scene[ggeneral_buffer[AZ_DAT_LOC+1] - AZ_EXT_SCENE_32];

            //if (scene[ggeneral_buffer[AZ_DAT_LOC+1] - GO_TO_SCENE_0] > 0 && scene[ggeneral_buffer[AZ_DAT_LOC+1] - GO_TO_SCENE_0] < MASK)  //SJ4201212 - If true, need to activate off
            if (ndx > 0 && ndx < MASK)  //SJ4201212 - If true, need to activate off
            {
            #ifndef DEV_7  //SJ4230513 - If device type is not 7.
                IE1 &= ~WDTIE; /* Critical update */
                actual_level=OFF;
                status_information &= ~(FADE_RUNNING+LAMP_ARC_POWER_ON);
                status_information &= ~(LIMIT_ERROR+STATUS_POWER_FAILURE);
                IE1 |= WDTIE;
                TI_DALI_Update_Callback();
//SJ3220513 - Above code will be replaced with code meanth for device type 7.
            #else  //SJ4230513 - For device type 7
                P2OUT &= 0xF7;  //SJ3220512 - DELAY_ACCTRL off, P2.3 (11110111)
            #endif
            }
            break;

      #if 0  //SJ5190413 - Implementation for continous button press
//SJ5190413 - This implementation does not include extended scene yet.
        case AZ_STEPPING_LEVEL:  //SJ519043 - #define AZ_STEPPING_LEVEL 0x11 defined in command.h
            //SJ5190413 - Here we get actual_level
            if (ggeneral_buffer[AZ_DAT_LOC+1] < AZ_EXT_SCENE_32)  //SJ2150113 - #define AZ_EXT_SCENE_32 0x20 defined in global_var.h
                ndx = scene[ggeneral_buffer[AZ_DAT_LOC+1] - GO_TO_SCENE_0];
            else
                ndx = gextended_scene[ggeneral_buffer[AZ_DAT_LOC+1] - AZ_EXT_SCENE_32];

            //SJ5190413 - Up direction
            if (ggeneral_buffer[AZ_DAT_LOC+2] == UP_BUTTON)
            {
                if (ndx < actual_level)
                    ndx = actual_level;

                ndx = ndx + (5 * ggeneral_buffer[AZ_DAT_LOC+3]);
            }
            //SJ5190413 - Down direction
            else
            {
                if (ndx > actual_level)
                    ndx = actual_level;

                ndx = ndx - (5 * ggeneral_buffer[AZ_DAT_LOC+3]);
            }

            if (ndx < 20 || ndx > 255)
                break;

            actual_level = ndx;
            TI_DALI_Update_Callback();
           #if 0
            IE1 &= ~WDTIE; /* Critical Update */
            target_level = ndx;  //scene[rx_msg[1]-GO_TO_SCENE_0];
//SJ5190413 - SJTODO: Need to study again whether to include adjustment for actual_level or not.
            //if(target_level > max_level)//;  //SJ3090113 - Why empty sttmt here? What happen to target_level = max_level;?????
            //{
            //  target_level = max_level;
            //}
            //if(target_level < min_level)
            //{
            //  target_level = min_level;
            //}

            /* Fade to level */
            status_information |= FADE_RUNNING;
            /* Calculate fade rate based upon fade time and change in power */
            /* level. */
            if(target_level > actual_level)
            {
              fade_step_size = fade_time_table[fade_time]/(target_level-actual_level);
            }
            else
            {
              fade_step_size = fade_time_table[fade_time]/(actual_level-target_level);
            }
            IE1 |= WDTIE;
           #endif
            break;
//actual_level = scene[rx_msg[1]-GO_TO_SCENE_0];
      #endif

//#define AZ_EXT_SCENE_32      0x20
//#define AZ_EXT_SCENE_95      0x5F
//rx_msg[0] = (ggeneral_buffer[AZ_DEV_LOC]);  //65
//rx_msg[1] = (ggeneral_buffer[AZ_CMD_LOC]);  //66
//loc 65 contains device short address
//loc 66 contains 0x0A to indicate Aztech proprietary command
//loc 67 contains 0xBD to indicate store extended scene
//loc 68 contains 0x11 to 0x50 to indicate the selected extended scene
//loc 69 contains decimal value representing the extended scene value

        case AZ_STORE_EXP_SCENE:  //SJ4100113 - equiv 189. Store a scene value to an extended scene
            //SJ1140113 - Extended scene range from 17 (0x11) to 80 (0x50)
            if(((AZ_EXT_SCENE_32-1) < ggeneral_buffer[68]) &&
               (ggeneral_buffer[68] < (AZ_EXT_SCENE_95+1)))  //SJ5070912 - True if Scene command is within valid range of allowed scene
//            if(ggeneral_buffer[68] < (AZ_EXT_SCENE_95+1))  //SJ5070912 - True if Scene command is within valid range of allowed scene
            {
              //scene[rx_msg[1]-STORE_THE_DTR_AS_SCENE_0]=data_transfer_register;
              //flash_update_request=1;
              gextended_scene[ggeneral_buffer[68]-AZ_EXT_SCENE_32] = ggeneral_buffer[69];
//              gextended_scene[ggeneral_buffer[68]] = ggeneral_buffer[69];
              TI_DALI_Flash_Update(FWKEY, 1);
              //SJ2011013 - Without these sttmt watchdog timer will not be re-activated.
              WDTCTL = (WDT_MDLY_8);  //SJ4200912 - FADE_INTERVAL equiv (WDT_MDLY_8 - SMCLK/8192), 1 second will goto ISR 977 times.
		      IE1 |= WDTIE;
            }
            break;

        case AZ_REMOVE_EXP_SCENE:  //SJ4100113 - equiv 190. Remove an extended scene
            //SJ1140113 - Aztech proprietary extended scene range for 17 to 80
            if(((AZ_EXT_SCENE_32-1) < ggeneral_buffer[68]) &&
               (ggeneral_buffer[68] < (AZ_EXT_SCENE_95+1)))
//            if(ggeneral_buffer[68] < (AZ_EXT_SCENE_95+1))
            {
              //scene[rx_msg[1]-REMOVE_FROM_SCENE_0] = MASK;
              //flash_update_request=1;
              gextended_scene[ggeneral_buffer[68]-AZ_EXT_SCENE_32] = MASK;
//              gextended_scene[ggeneral_buffer[68]] = MASK;
              TI_DALI_Flash_Update(FWKEY, 1);
              //SJ2011013 - Without these sttmt watchdog timer will not be re-activated.
              WDTCTL = (WDT_MDLY_8);  //SJ4200912 - FADE_INTERVAL equiv (WDT_MDLY_8 - SMCLK/8192), 1 second will goto ISR 977 times.
		      IE1 |= WDTIE;
            }
            break;

        case AZ_GOTO_EXP_SCENE:  //SJ4100113 - equiv 191. Go to an extended scene
            if(((AZ_EXT_SCENE_32-1) < ggeneral_buffer[68]) &&
               (ggeneral_buffer[68] < (AZ_EXT_SCENE_95+1)))  //SJ2150113 - True if extended Scene is within valid range
//            if(ggeneral_buffer[68] < (AZ_EXT_SCENE_95+1))  //SJ2150113 - True if extended Scene is within valid range
            {
//SJ4180713 - SJTO_TRY: If UART blocking is not able to overcome the issue, try using FADE_RUNNING flag
//if(status_information & FADE_RUNNING)  break;
                if(gextended_scene[ggeneral_buffer[68]-AZ_EXT_SCENE_32] == MASK)  //SJ4131212 - If the corresponding scene is not set, do nothing.
//                if(gextended_scene[ggeneral_buffer[68]] == MASK)  //SJ4131212 - If the corresponding scene is not set, do nothing.
                {
                    break;  // Do nothing
                }
            #ifdef DEV_7
                else if(gextended_scene[ggeneral_buffer[68]-AZ_EXT_SCENE_32] == 0)
                {
                    P2OUT &= 0xF7;  //SJ4230513 - RELAY_ACCTRL off, P2.3 (11110111)
                }
                //SJ1300913 - Added 
                else if(gextended_scene[ggeneral_buffer[68]-AZ_EXT_SCENE_32] == 0xFE)
                {
                    P2OUT |= 0x08;  //SJ1300913 - RELAY_ACCTRL on, P2.3 (xxxx1xxx)
                }
//SJ3220513 - Below codes is being replaced with codes meant for device type 7
            #else  //SJ4230513 - If device type is not 7 (Original code for TI DALI source).
                IE1 &= ~WDTIE; /* Critical Update */
                status_information &= ~STATUS_POWER_FAILURE;
                target_level = gextended_scene[ggeneral_buffer[68]-AZ_EXT_SCENE_32];
//                target_level = gextended_scene[ggeneral_buffer[68]];
                if(target_level > max_level)//;  //SJ3090113 - Why empty sttmt here? What happen to target_level = max_level;?????
                {
                    target_level = max_level;
                }
                if(target_level < min_level)
                {
                    ;//target_level = min_level;
                }
                /* fade to arc_level stored for scene */
                /* rx_msg[] is 16 through 31 */

                //if((target_level == actual_level) || (fade_time == OFF))  //SJ3240413 - ORG
                //SJ3240413 - Temporily solutin to overcome min_level issue
                if((target_level == actual_level) || (fade_time == OFF) || (target_level == OFF))
                {
                    /* If already at the requested then result is nothing. */
                    /* If no fade, then go to level immediately.           */
                    status_information &= ~FADE_RUNNING;
                    actual_level = gextended_scene[ggeneral_buffer[68]-AZ_EXT_SCENE_32];
//                    actual_level = gextended_scene[ggeneral_buffer[68]];
                    TI_DALI_Update_Callback();
                }
                else
                {
                    /* Fade to level */
                    status_information |= FADE_RUNNING;
                    /* Calculate fade rate based upon fade time and change in power */
                    /* level. */
                    if(target_level > actual_level)
                    {
                        fade_step_size = fade_time_table[fade_time]/(target_level-actual_level);
                    }
                    else
                    {
                        fade_step_size = fade_time_table[fade_time]/(actual_level-target_level);
                    }
                    fade_count = 0;  //SJ5260413 - Added by SJ to reset counter. Counter out of sync if prev fading is incomplete
                }
                IE1 |= WDTIE;
            #endif
                gstate_var.error_delay = 10;  //SJ2110912 - Aztech
                //break;
            }
            break;

//SJ4201212 - For both switch and light place here.

        default:
          //#ifdef SWITCH_CONTROLLER  //SJ2181212 - If device is switch.
          //  if ((ggeneral_buffer[AZ_DAT_LOC] > 0) && (ggeneral_buffer[AZ_DAT_LOC] < (AZ_EXT_CMD_BUTTON_L+1)))
          //  {
          //      gswitch_oper[ggeneral_buffer[AZ_DAT_LOC]-1] = ggeneral_buffer[AZ_DAT_LOC+1];
          //      flash_update_request = 1;
          //  }
          //#endif
            break;

    }  //SJ2181212 - switch() { ... }

    return 0;
}  //SJ2181212 - AZ_Extended_Command() { ... }
#endif
//SJ2181212 - END of Aztech proprietary extended command.

//SJ5260413 - For debugging fading issue
#if 0
void SJ_Debugging_Fading(void)
{
    int temp_step=0;
//SJ5260413 - Iteration rate is 4 seconds.
//SJ5260413 - Alternate between scale up of 25% to 75% and scale down of 75% to 25% in each iteration.
    gstate_var.PLC_status = 1;
    //gstate_var.general_delay = TIMER_4_SECONDS;
    //fade_time = 1;//15;
    actual_level = 63;  //SJ5260413 - Around 25%
    target_level = 254; //SJ5260413 - 100%

    while (1)
    {
        temp_step = fade_time_table[fade_time];
        if(target_level > actual_level)
        {
            //fade_step_size = fade_time_table[fade_time]/(target_level-actual_level);
            fade_step_size = temp_step / (target_level-actual_level);
        }
        else
        {
            //fade_step_size = fade_time_table[fade_time]/(actual_level-target_level);
            fade_step_size = temp_step / (actual_level-target_level);
            fade_step_size = temp_step / (target_level-actual_level);
        }
    }

    temp_step = actual_level + target_level;
/*****
    while (1)
    {
        if (gstate_var.general_delay == 0 && !(status_information & FADE_RUNNING))
        {
            if (gstate_var.PLC_status == 1)  //SJ5260413 - Fades up
            {
                gstate_var.PLC_status = 2;
                actual_level = 63;  //SJ5260413 - Around 25%
                target_level = 254; //SJ5260413 - 100%
            }
            else  //SJ5260413 - Fades down
            {
                gstate_var.PLC_status = 1;
                actual_level = 254;  //SJ5260413 - Around 25%
                target_level = 63; //SJ5260413 - 100%
            }

            IE1 &= ~WDTIE; // Critical Update
            status_information &= ~STATUS_POWER_FAILURE;
            status_information |= FADE_RUNNING;
            // Calculate fade rate based upon fade time and change in power
            // level.
            if(target_level > actual_level)
            {
                fade_step_size = fade_time_table[fade_time]/(target_level-actual_level);
            }
            else
            {
                fade_step_size = fade_time_table[fade_time]/(actual_level-target_level);
            }
            IE1 |= WDTIE;
            gstate_var.error_delay = 10;  //SJ2110912 - Aztech
            gstate_var.general_delay = TIMER_4_SECONDS;
        }
    }
*****/

}  //SJ5260413 - End of SJ_Debugging_Fading() function
#endif
