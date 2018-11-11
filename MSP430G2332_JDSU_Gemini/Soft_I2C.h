
/***********************************************************************/
/*                                                                     */
/*  FILE        : Soft_I2C.h                                           */
/*  DATE        : Thur, April 04, 2013                                 */
/*  DESCRIPTION : Software I2C protocol                                */
/*  CPU TYPE    : MSP430G2553                                          */
/*  PROJECT     : PLC Switch                                           */
/*  AUTHOR      : Suilarso Japit                                       */
/*  VERSION     :                                                      */
/*                                                                     */
/***********************************************************************/

#ifndef __SOFT_I2C_H
#define __SOFT_I2C_H

//#define NO_I2C_INTERRUPT

#define TMP431A_SlaveAddr  0x4C  //SJ3030713 - Bit 7-1 1001100x
#define TMP431A_WriteCmd   0x98  //SJ3030713 - Bit 7-0 10011000
#define TMP431A_ReadCmd    0x99  //SJ3030713 - Bit 7-0 10011001

//SJ3231013 - Microchip Technology Inc. 24AA256/24LC256/24FC256
#define MC24LC256_Addr     0x51  //SJ3231013 - Bit 7-1 1010001x
#define MC24LC256_WriteCmd 0xA2  //SJ3231013 - Bit 7-0 10100010
#define MC24LC256_ReadCmd  0xA3  //SJ3231013 - Bit 7-0 10100011

#define DEV_SlaveAddress   MC24LC256_Addr      //SJ3231013 - Bit 7-1
#define DEV_WriteCommand   MC24LC256_WriteCmd  //SJ3231013 - Bit 7-0
#define DEV_ReadCommand    MC24LC256_ReadCmd   //SJ3231013 - Bit 7-0

#define I2C_WRITE_ERROR       1
#define I2C_READ_ERROR        1

#define FULL_INITIALIZATION
#define MAX_I2C_BUFFER    80  //60  //50

//#define TOLERANCE         5000  //SJ2090413 - Serve as tolerance to delay
#define TOLERANCE         10  //SJ2090413 - Serve as tolerance to delay


void SW_I2CStart(void);
void SW_I2CStop(void);
unsigned char I2CCheckAcknowledgement(void);
//uint8_t One_Touch_I2CWrite(int8_t NumOfByte);
unsigned char SW_I2CWrite(char NumOfByte);
//uint8_t One_Touch_I2CReadByte(void);
unsigned char SW_I2CReadByte(void);
//uint8_t One_Touch_I2CRead(int NumOfByte);
unsigned char SW_I2CRead(int NumOfByte);
//void I2C_DELAY(int32_t delay);
void I2C_DELAY(long delay);
//void I2CAckPolling(void);

#endif // __SOFT_I2C_H
