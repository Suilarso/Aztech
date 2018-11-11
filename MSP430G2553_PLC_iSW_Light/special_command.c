/******************************************************************************/
// special_command.c
// Defines TI_DALI_Special_Command()
// Special commands are commands 256 through 271, extended special commands
// have the same structure and are commands 272 - 275, 276-383 are reserved
// for future use.
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

#include "msp430x21x1.h"
#include "dali.h"
#include "special_command.h"
#if 1  //SJ1101212 - Az
#include "global_var.h"  //SJ1101212 - Added by SJ


//SJ4201212 - External function prototype ---------------------------
//PLC_Lite.c
//extern void AZ_Randomised_Long_Address(void);
//extern void UART_Rx_Acknowledgement(int validity_flag);  //SJ3010812 -
//extern unsigned char AZ_Acknowledge_Basenode_Request(unsigned char Message_Type);  //SJ4300812 - Response to basenode request
extern void AZ_DALI_Response(int response_type);  //SJ3220812 - Replaced. Used in network type


//SJ1101212 - External global variables declaration ------------------
//PLC_iSwitch.c
extern unsigned char ggeneral_buffer[DATA_BUFFER_SZ];
extern STATE_STRUCT  gstate_var;

//extern unsigned char gextended_scene[EXT_SCENE_SZ];  //SJ1140113 - equiv 64

//SJ1101212 - Function defined by Az
void AZ_Verify_Search_Address(void);
#endif


unsigned char TI_DALI_Special_Command(unsigned char *rx_msg)
{
/* Commands 256 through 275  */
//SJ2220113 - SJEXT_ADDR: Use below declaration.
#ifdef SJEXT_ADDR
  unsigned int temp_address;
#else  //SJ3200213 - ORG
  unsigned char temp_address;
#endif
  unsigned char response=0;
  switch(rx_msg[0])
  {
#ifdef TERMINATE
    case TERMINATE: /* Command 256  */
        initialise_count=INITIALISE_SEC;  //SJ3050912 - INITIALISE_SEC is 58594, it represents 1 minute
        break;
#endif
#ifdef DATA_TRANSFER_REGISTER
    case DATA_TRANSFER_REGISTER:/* Command 257  */
        data_transfer_register = rx_msg[1];
        break;
#endif
#ifdef INITIALISE
    /* A 15 minute timer is started with Initialise        */
    /* During this time commands 259-270 can be processed  */
    case INITIALISE:            /*  Commands 258 */
        /* rx_msg[1] = 0 : broadcast
        *            = 0xFF : all devices without a short address
        *            = 0AAA AAA1 : device with address AA AAAA  */
//SJ2220113 - SJEXT_ADDR: DALI protocol is already violated, we can continue to use this case as it by having GUI assign rx_msg[1] = 0
//            to indicate broadcast message. But if there is a need to use this case for a specific address, below two sttmts need to
//            change, and the GUI also need to change accordingly.
        temp_address = rx_msg[1]>>1;
        temp_address &= ~(BIT7+BIT6);       //SJ4130912 - Retrieving short address
        if((rx_msg[1] == 0))
        {
            temp_address = short_address;
        }
        if((rx_msg[1] == MASK) && (status_information & MISSING_SHORT_ADDRESS))
        {
            temp_address = short_address;
        }
        if (temp_address == short_address)  //SJ2220113 - SJEXT_ADDR: Becareful with this condition for extended DALI address
        {
          if(config_count < CONFIG_TIMEOUT)  //SJ4060912 - CONFIG_TIMEOUT is 100ms
          { /* need to receive the command twice */
            initialise_count=0;
            initialise_minute=0;
          }
          else
          {
            config_count=0;
          }
        }
        break;
#endif
#ifdef RANDOMISE
    case RANDOMISE:            /*  Commands 259 */
      #if 0  //SJ5010313 - Since it is not used, don't compile
        if(initialise_count==INITIALISE_SEC)  //SJ4130912 - I doubt this if sttmt is logically correct!!!!!!! When will it be true???
        {
          break;
        }
        if(config_count < CONFIG_TIMEOUT)
        { /* need to receive the command twice */
          /* generate random address  0 - FFFFFF*/
          if(random_address_byte_m == CALDCO_1MHZ)
          {
            random_address_byte_m = CALDCO_12MHZ;  //SJ3120912 - CALDCO_12MHZ is 0xAB
          }
          else
          {
            random_address_byte_m = CALDCO_1MHZ;   //SJ3120912 - CALDCO_1MHZ is 0xDC
          }
          if(random_address_byte_h == CALDCO_1MHZ)
          {
            random_address_byte_h = CALDCO_12MHZ;
          }
          else
          {
            random_address_byte_h = CALDCO_1MHZ;
          }
          flash_update_request=1;
        }
        else
        {
          config_count=0;
        }
      #endif
        break;
#endif
#ifdef COMPARE
    case COMPARE:            /*  Commands 260 */
      #if 0  //SJ1040313 - Exclude from compilation
        if(initialise_count==INITIALISE_SEC)
        {
          break;
        }
        if(random_address_byte_h > search_address_byte_h)
        {
          break;
        }
        if(random_address_byte_h == search_address_byte_h)
        {
          if(random_address_byte_m > search_address_byte_m)
          {
            break;
          }
          if(random_address_byte_m == search_address_byte_m)
          {
            if(random_address_byte_l > search_address_byte_l)
            {
              break;
            }
          }
        }
        response = YES;
      #endif
        break;
#endif
#ifdef WITHDRAW
    case WITHDRAW:            /*  Commands 261 */
        /* exclude from compare process if random_address = search_address */
        initialise_count=INITIALISE_SEC;
        break;
#endif
/* Commands 262 and 263 are reserved for future use  */
#ifdef SEARCHADDRH
    case SEARCHADDRH:            /*  Commands 264 */
//SJ4130912 - I doubt the logic is correct. Once ISR_WDT() kick in, there won't be a time when initialise_count is larger than INITIALISE_SEC
        //if(initialise_count<INITIALISE_SEC)
        //{
         #if 0  //SJ1101212 - ORG
          search_address_byte_h = rx_msg[1];
         #else  //SJ1101212 - Aztech
          if (short_address == 0)
          {
            AZ_Verify_Search_Address();
          }
         #endif
        //}
        break;
#endif
#ifdef SEARCHADDRM
    case SEARCHADDRM:            /*  Commands 265 */
        /*   //SJ2120313 - Skip from compilation.
        if(initialise_count<INITIALISE_SEC)
        {
         #if 0  //SJ1101212 - ORG
          search_address_byte_m = rx_msg[1];
         #else  //SJ1101212 - Aztech
          AZ_Verify_Search_Address();
         #endif
        }
        */
        break;
#endif
#ifdef SEARCHADDRL
    case SEARCHADDRL:            /*  Commands 266 */
        /*   //SJ2120313 - Skip from compilation
        if(initialise_count<INITIALISE_SEC)
        {
         #if 0  //SJ1101212 - ORG
          search_address_byte_l = rx_msg[1];
         #else  //SJ1101212 - Aztech
          AZ_Verify_Search_Address();
         #endif
        }
        */
        break;
#endif
#ifdef PROGRAM_SHORT_ADDRESS
    case PROGRAM_SHORT_ADDRESS:  /* Command 267  */
        //SJ2120313 - Below if() sttmt is DALI ORG. skip from compilation
        /*if(initialise_count==INITIALISE_SEC)
        {
          break;
        }*/
        //SJ2120313 - Below if() sttmt is added for iSwitch project
        if (short_address != 0)
        {
            break;
        }

        if(search_address_byte_h == random_address_byte_h)
        {
          if(search_address_byte_m == random_address_byte_m)
          {
            if(search_address_byte_l == random_address_byte_l)
            {

              //physical_selection = 1;  //SJ5010313 - ORG. DALI

              //SJ2120313 - Below onward are added

              //if (short_address != 0)
              //{
              //  BCSCTL3 |= LFXT1S_2;  //SJ5060712 - ACLK = VLO
              //  WDTCTL = WDT_ARST_1000;  //SJ4230812 - For development use only. Remove after that.
              //  while (1);
              //}

              search_address_byte_h = 0xFF;
              search_address_byte_m = 0xFF;
              search_address_byte_l = 0xFF;

              physical_selection = 0;
              short_address = (ggeneral_buffer[AZ_CMD_LOC] << 8) | ggeneral_buffer[AZ_DAT_LOC];
              AZ_DALI_Response(0xB7);  //SJ4201212 - Replaced above method.
              flash_update_request = 1;

              //SJ2120313 - End of added
            }
          }
        }
 #if 0  //SJ5010313 - Skip this portion
        if(physical_selection)
        {
  #ifndef SJEXT_ADDR  //SJ4210213 - Skip below official code for implementation of DALI extended address
          if(rx_msg[1] & BIT7)  //SJ2220113 - SJEXT_ADDR: Need to re-do; as of now, don't know how to do it yet.
          {
            /* Delete short address */
    //SJ2220113 - SJEXT_ADDR: Use below sttmt.
    #ifdef SJEXT_ADDR
            short_address = 0xFFFF;
    #else  //SJ3200213 - ORG
            short_address = MASK;
    #endif
            status_information |= MISSING_SHORT_ADDRESS;
          }
          else
  #endif
          {
//SJ2220113 - SJEXT_ADDR: Since by extending DALI address, we already violating DALI protocol, as such we don't have to follow
//            below two original sttmts from DALI; instead, replace them with our own code and make necessary change in GUI.
//            NOTE: above if(rx_msg[1] & BIT7) need to make change also
          #ifdef SJEXT_ADDR  //SJ4210213 - [AZ_CMD_LOC 66] - MSB, [AZ_DAT_LOC 67] - LSB
            short_address = (ggeneral_buffer[AZ_CMD_LOC] << 8) | ggeneral_buffer[AZ_DAT_LOC];
          #else  //SJ3200213 - ORG
            short_address = rx_msg[1]>>1;        // Address  xAAAAAA1
            short_address &= ~(BIT7+BIT6);       //
          #endif
          #if 1  //SJ1101212 - Aztech used
            //SJ1101212 - Location 67 and 68 contains room ID
            //gstate_var.Room_ID = (ggeneral_buffer[AZ_CMD_LOC+1] << 8) + ggeneral_buffer[AZ_CMD_LOC+2];
            physical_selection = 0;  //SJ2111212 - Prevent from duplicate writing. TAKE NOTE FOR SIDE EFFECT
            //gstate_var.general_ctrl_flag |= ATTACH_ON;  //SJ4201212
            AZ_DALI_Response(0xB7);  //SJ4201212 - Replaced above method.
          #endif
          }
          flash_update_request=1;
        }
 #endif
        break;
#endif
    case VERIFY_SHORT_ADDRESS: /* Command 268  */
  #if 0  //SJ1040313 - Exclude from compilation
        if(initialise_count==INITIALISE_SEC)
        {
          break;
        }
//SJ2220113 - SJEXT_ADDR: Since by extending DALI address we already violating DALI protocol, as such we don't have to follow
//            below two original sttmt from DALI; we can replace them with our own sttmts and modify GUI accordingly.
    #ifdef SJEXT_ADDR
        temp_address = (ggeneral_buffer[AZ_CMD_LOC] << 8) | ggeneral_buffer[AZ_DAT_LOC];
    #else  //SJ3200213 - ORG
        temp_address = rx_msg[1]>>1;
        temp_address &= ~(BIT7+BIT6);
    #endif
        if (temp_address == short_address)
        {
          response = YES;
        }
  #endif
        break;
    case QUERY_SHORT_ADDRESS: /* Command 269  */
      #if 0  //SJ1040313 - Exclude from compilation.
        if(initialise_count==INITIALISE_SEC)
        {
          break;
        }
        if(search_address_byte_h == random_address_byte_h)
        {
          if(search_address_byte_m == random_address_byte_m)
          {
            if(search_address_byte_l == random_address_byte_l)
            {
              physical_selection = 1;
            }
          }
        }
        if(physical_selection)
        {
//SJ2220113 - SJEXT_ADDR: With extended DALI address, this special command can no longer be used.
          response = short_address;
        }
      #endif
        break;
    case PHYSICAL_SELECTION: /* Command 270  */
      #if 0  //SJ4280213 - Since it is not used, remove from compilation, but not physical removal
        if(initialise_count==INITIALISE_SEC)
        {
          break;
        }
        physical_selection = 1;  //SJ4130912 - Once selected, when can it be de-selected?????
      #endif
        break;
    case ENABLE_DEVICE_TYPE_X: /* Command 272  */
        if(rx_msg[1] == device_type)
        {
          enable_device_type = 1;
        }
        break;
#ifdef DATA_TRANSFER_REGISTER_1
    case DATA_TRANSFER_REGISTER_1:/* Command 273  */
        data_transfer_register1 = rx_msg[1];
    #ifdef SJEXT_ADDR  //SJ4210213 - DALI Extended address scheme
        data_transfer_register2 = ggeneral_buffer[AZ_DAT_LOC];
    #endif
        break;
#endif
#ifdef DATA_TRANSFER_REGISTER_2
    case DATA_TRANSFER_REGISTER_2:/* Command 274  */
        data_transfer_register2 = rx_msg[1];
        break;
#endif
    default:
        break;
  } //END SWITCH
  return response;
}

//SJ1101212 - Below codes are implemented to enhance standard DALI special command
//SJ1101212 - to execute deviated tasks
#if 1
void AZ_Verify_Search_Address(void)
{
//SJ2220113 - SJEXT_ADDR: Un-remark below sttms
#ifdef SJEXT_ADDR
    unsigned char temp_addr;
#endif

//#define  AZ_DEV_LOC   65//17  //SJ4061212 - For switch project //62
//#define  AZ_CMD_LOC   66//18  //SJ4061212 - For switch project //63

    search_address_byte_h = ggeneral_buffer[AZ_CMD_LOC];
    search_address_byte_m = ggeneral_buffer[AZ_CMD_LOC+1];
    search_address_byte_l = ggeneral_buffer[AZ_CMD_LOC+2];

    //SJ1101212 - If search address matched random address, issue DALI command to turn on LED light
    if(search_address_byte_h == random_address_byte_h)
    {
        if(search_address_byte_m == random_address_byte_m)
        {
            if(search_address_byte_l == random_address_byte_l)
            {
//SJ2220113 - SJEXT_ADDR: Use below codes for extended DALI address
#ifdef SJEXT_ADDR
                temp_addr = short_address & 0x003F;  //SJ2220113 - Extracting first six bits. 0000000000111111
                ggeneral_buffer[AZ_DEV_ADDR_MSB] = short_address >> 6;
                ggeneral_buffer[AZ_DEV_LOC] = 0x00 | (temp_addr << 1);
#else  //SJ3200213 - ORG
                ggeneral_buffer[AZ_DEV_LOC] = 0x00 | (short_address << 1);
#endif
//SJ1300913 - Added for remote on-off light (Device type 7)
#ifndef DEV_7
                ggeneral_buffer[AZ_CMD_LOC] = 0x7F;  //SJ1101212 - Mid-level.
#else
                ggeneral_buffer[AZ_CMD_LOC] = 0xFF;  //SJ1300913 - MASK defined in dali.h
#endif
                TI_DALI_Transaction_Loop();
            }
        }
    }

}  //SJ1101212 - End of AZ_Verify_Search_Address(void) { ... }

#endif
