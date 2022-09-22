/*
 * @Author: your name
 * @Date: 2022-02-20 11:11:39
 * @LastEditTime: 2022-02-21 18:57:42
 * @LastEditors: Please set LastEditors
 * @Description: 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 * @FilePath: \USERe:\Learn\stm32\实例\茶园环境监测_从机\FreeRTOS\HARDWARE\GY30\gy30.h
 */
#ifndef _GY_30_H_
#define _GY_30_H_

#include "sys.h"

#define uchar unsigned char
#define uint unsigned int

typedef unsigned char BYTE;
typedef unsigned short WORD;

#define SlaveAddress 0x46

#define SDA_IN()  {GPIOB->CRL &= 0XFFF0FFFF; GPIOB->CRL |= 8 << (4*4);}
#define SDA_OUT() {GPIOB->CRL &= 0XFFF0FFFF; GPIOB->CRL |= 3 << (4*4);}

#define IIC_SCL  PBout(3)
#define IIC_SDA  PBout(4)
#define READ_SDA PBin(4)

extern BYTE BUF[8]; /* 接收数据缓存区 */

void IIC_Init ( void );
void IIC_Start ( void );
void IIC_Stop ( void );
void IIC_Send_Byte ( u8 txd );
u8 IIC_Read_Byte ( unsigned char ack );
u8 IIC_Wait_Ack ( void );
void IIC_Ack ( void );
void IIC_NAck ( void );
void IIC_Write_One_Byte ( u8 daddr, u8 addr, u8 data );
u8 IIC_Read_One_Byte ( u8 daddr, u8 addr );


void BH1750_Start ( void );
void BH1750_Stop ( void );
void BH1750_SendACK ( BYTE ack );
BYTE BH1750_RecvACK ( void );
void BH1750_SendByte ( BYTE dat );
BYTE BH1750_RecvByte ( void );
void Single_Write_BH1750 ( uchar REG_Address );
void BH1750_Init ( void );
void Multiple_read_BH1750 ( void );
void Get_BH1750Data(u8 *BH1750Data_H , u8 *BH1750Data_L);
#endif
