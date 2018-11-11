
/***********************************************************************/
/*                                                                     */
/*  FILE        : PLC_Comm.h                                           */
/*  DATE        : Fri, July 13, 2012                                   */
/*  DESCRIPTION : Global variables definition                          */
/*  CPU TYPE    : MSP430G2553                                          */
/*  PROJECT     : PLC Light                                            */
/*  AUTHOR      : Suilarso Japit                                       */
/*  VERSION     : 100.00                                               */
/*                                                                     */
/***********************************************************************/

#ifndef __PLC_COMM_H
#define __PLC_COMM_H


#define MAX_RETRIAL  1  //SJ5200712 - Maximum re-trial
#define ZERO_CONFIGURATION


//SJ2170712 - Macro for Message Header receive sequence
#define MSG_HEADER_TYPE        0
#define MSG_HEADER_ATTR        1
#define MSG_PAYLOAD_LEN_LSB    2
#define MSG_PAYLOAD_LEN_MSB    3
#define MSG_HEADER_CRC_LSB     4
#define MSG_HEADER_CRC_MSB     5
#define MSG_PAYLOAD_CRC_LSB    6
#define MSG_PAYLOAD_CRC_MSB    7
#define MESSAGE_HEADER_LEN     8

//SJ2170712 - Macro for System initialization various steps
#define SYSTEM_INIT_STEP_1     1  //SJ2170712 - PORT_DESIGNATION
#define SYSTEM_INIT_STEP_2     2  //SJ2170712 - SYSTEM_CONFIGURATION
#define SYSTEM_INIT_STEP_3     3  //SJ2170712 - G3_CONFIGURATION
#define SYSTEM_INIT_STEP_4     4  //SJ2170712 - SHUTDOWN
#define SYSTEM_INIT_END        5  //SJ3180712 - Indicate last step. If more steps are introduced, change this value

#define NETWORK_INIT_SET_INFO_1 1
#define NETWORK_INIT_SET_INFO_2 2
#define NETWORK_INIT_SET_INFO_3 3
#define NETWORK_INIT_DISCOVER   4  //SJ5200712 - NETWORK_DISCOVER
#define NETWORK_INIT_ATTACH     5  //SJ5200712 - MSG_TYPE__ATTACH
#define NETWORK_INIT_END        6  //SJ5200712 - Indicate last step of network initialization.

//SJ3220812 - Macroes for response type
#define REVEAL_DALI_ADDRESS     1  //SJ4130912 - Not used at the moment
#define DEVICE_QUERY            2
#define GENERIC_QUERY           3
#define DEVICE_MALFUNCTION      4
//#define RANDOM_ADDRESS_QUERY    5   //SJ4130912 - use the macro defined in command.h
//#define GROUPING_QUERY          6   //SJ1170912 - use the macro defined in command.h
#define SENSOR_DETECTED         7
//#define SCENE_LEVEL_QUERY       8   //SJ3190912 - use #define QUERY_SCENE_LEVEL_0 176 defined in command.h
//#define BROADCAST_FROM_SN       8  //SJ4041012 - Testing broadcast from service node

//SJ5130712 - Macro definitions for Message Type
#define MSG_TYPE__DATA_TRANSFER        0x00
#define MSG_TYPE__GET_SYSTEM_INFO      0x01
#define MSG_TYPE__GET_PHY_PIB          0x02
#define MSG_TYPE__GET_MAC_PIB          0x03
#define MSG_TYPE__SET_INFO             0x04
#define MSG_TYPE__SHUTDOWN             0x05
#define MSG_TYPE__SETUP_ALARM          0x06
#define MSG_TYPE__ALARM                0x07
#define MSG_TYPE__NW_REGISTER          0x08
#define MSG_TYPE__NW_UNREGISTER        0x09
#define MSG_TYPE__CONNECT              0x0A
#define MSG_TYPE__DISCONNCET           0x0B
#define MSG_TYPE__LOAD_SYSTEM_CONFIG   0x0C
#define MSG_TYPE__SET_MAC_PIB          0x0D
#define MSG_TYPE__CLEAR_PHY_PIB        0x0E
#define MSG_TYPE__CLEAR_MAC_PIB        0x0F
#define MSG_TYPE__ATTACH               0x10
#define MSG_TYPE__DETACH               0x11
#define MSG_TYPE__DISCOVER             0x12
#define MSG_TYPE__FIRMWARE_UPGRADE     0x13
#define MSG_TYPE__GET_INFO             0x14


//SJ5130712 - Macro for config type of MSG_TYPE__LOAD_SYSTEM_CONFIG
#define PORT_DESIGNATION         0x0001  //SJ5130712 Port designation
#define SYSTEM_CONFIGURATION     0x0003  //SJ5130712 System Configuration
#define PHY_CONFIGURATION        0x0004  //SJ5130712 PHY Configuration
#define MAC_CONFIGURATION        0x0005  //SJ5130712 MAC Configuration
#define IPV_CONFIGURATION        0x0006  //SJ5130712 CL IPv4/v6 Configuration
#define PRIME_LLC_CONFIGURATION  0x0007  //SJ5130712 PRIME LLC Configuration
#define G3_CONFIGURATION         0x0008  //SJ5130712 G3 configuration
#define FLEX_LITE_CONFIGURATION  0x0009  //SJ5130712 FLEX LITE Band configuration


//SJ2170712 - Macro for discovery type of MSG_TYPE__DISCOVER
#define NETWORK_DISCOVER         0x00
#define ROUTE_DISCOVER           0x01
#define PATH_DISCOVER            0x02


//SJ3180712 - Macroes for Message Status error codes. (Table 4 of PLC Suite Host Message Protocol Specification)
#define MSG_REQUEST_OK          0x0000  //SJ3180712 - Message request success
//#define GENERAL ERROR CODES     0x0xxx  //SJ3180712 - General Message Error Codes
#define INVALID_PARAMETER       0x0002  //SJ3180712 - Invalid message parameter(s)
#define FEATURE_UNSUPPORTED     0x0003  //SJ3180712 - Feature is not supported
#define PAYLOAD_CRC_ERROR       0x0004  //SJ3180712 - Payload CRC error
#define NO_EEPROM               0x0005  //SJ3180712 - No EEPROM on the device
#define HEADER_CRC_ERROR        0x0006  //SJ3180712 - Message Header CRC error
#define INVALID_HANDLE          0x0007  //SJ3180712 - The handle is invalid
#define INVALID_FORMAT          0x0008  //SJ3180712 - The message format is incorrect
#define RPY_NOT_BLOCKED         0x000A  //SJ3180712 - An RPY unblock msg (see DATA_TRANSFER) was recv’d but not currently blocked.
#define INVALID_MESSAGE_LENGTH  0x000B  //SJ3180712 - The message length is invalid
#define ERR_NO_HANDLER          0x00F3  //SJ3180712 - No handler for the message type
#define PENDING                 0x00FC  //SJ3180712 - The operation is pending
#define ERR_TIMEOUT             0x00FD  //SJ3180712 - The operation timed-out
#define ERR_NO_MEMORY           0x00FE  //SJ3180712 - Out of memory
#define GENERAL_FAILURE         0x00FF  //SJ3180712 - General failure or error
//#define PHY_ERROR_CODES         0x1xxx  //SJ3180712 - PHY error codes
//#define MAC_ERROR_CODES         0x2xxx  //SJ3180712 - MAC error codes
//#define CL_ERROR_CODES          0x3xxx  //SJ3180712 - CL error codes
//#define ECA_ERROR_CODES         0x4xxx  //SJ3180712 - ECA error codes

//SJ4190712 - Macroes for Discover request status


//Discovery Type  Status Code  Status Code Value 
//SJ4190712 Network Discover 
#define STAT_SUCCESS           0x0000
#define STAT_NO_BEACON         0x008B
#define STAT_NOT_PERMITTED     0x008D
#define STAT_ERR_FAILURE       0x0081
//SJ4190712 Route Discover  
//#define STAT_SUCCESS           0x0000
#define STAT_INVALID_REQUEST   0x0085
#define STAT_ROUTE_ERR         0x0088
//SJ4190712 Path Discover
//#define STAT_SUCCESS           0x0000
//#define STAT_INVALID_REQUEST   0x0085
//#define STAT_ROUTE_ERR         0x0088


//SJ3220812 - Macroes for Info Type
#define PRIME_PHY_TX_PARAM_LSB       0x00  //0x0000
#define PRIME_PHY_TX_PARAM_MSB       0x00  //
#define PRIME_PHY_RX_PARAM_LSB       0x01  //0x0001
#define PRIME_PHY_RX_PARAM_MSB       0x00  //0x0001   
#define G3_PHY_TX_PARAM_LSB          0x02  //0x0002
#define G3_PHY_TX_PARAM_MSB          0x00  //0x0002   
#define G3_PHY_RX_PARAM_LSB          0x03  //0x0003
#define G3_PHY_RX_PARAM_MSB          0x00  //0x0003   
#define VENDOR_N_PRODUCT_ID_LSB      0x04  //0x0004
#define VENDOR_N_PRODUCT_ID_MSB      0x00  //0x0004   
//#define RESERVED_1  0x0005   
//#define RESERVED_2  0x0006   
//#define RESERVED_3  0x0007   
//#define RESERVED_4  0x0008   
#define TX_PHY_ATTNT_GAIN_PARAM_LSB  0x09  //0x0009
#define TX_PHY_ATTNT_GAIN_PARAM_MSB  0x00  //0x0009   
//#define RESERVED_5  0x000A  


//SJ3281112 - Below macroes are meant for AZ used only.
#ifdef ZERO_CONFIGURATION
    #define AZ_DATA_LOC        82
#else
    #define AZ_DATA_LOC        10
#endif

//SJ2271112 - Macro for ggeneral_buffer location
#define AZ_DEVICE_ID_LOC       (AZ_DATA_LOC + 0)    //SJ2271112 - If running from zero configuration
#define AZ_GROUP_ID_LOC        (AZ_DATA_LOC + 2)
#define AZ_COMMAND_LOC         (AZ_DATA_LOC + 4)

//SJ3281112 - Macro for command
#define AZ_CMD_LED_OFF   0x00
#define AZ_CMD_LED_ON    0x01


//Direction

//Misc

//Macro definition for LED controller






#endif  //__PLC_COMM
