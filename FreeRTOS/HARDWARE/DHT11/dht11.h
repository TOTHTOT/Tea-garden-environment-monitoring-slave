#ifndef __DHT11_H
#define	__DHT11_H

#include "sys.h"
#include "delay.h"

#define PA7(a)	if (a)	\
					GPIO_SetBits(GPIOA,GPIO_Pin_7);\
					else		\
					GPIO_ResetBits(GPIOA,GPIO_Pin_7)
					
#define PA7_IN   GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_7)
					
void DHT11_IO_OUT(void);
int DHT11_Check(void);
int DHT11_Read_Data(int *temp,int *humi);
void DHT11_Init(void);
#endif

