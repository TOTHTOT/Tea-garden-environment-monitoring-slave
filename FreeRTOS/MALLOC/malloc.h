/*
 * @Author: your name
 * @Date: 2022-02-19 20:20:57
 * @LastEditTime: 2022-02-19 20:50:43
 * @LastEditors: Please set LastEditors
 * @Description: ��koroFileHeader�鿴���� ��������: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 * @FilePath: \USERe:\Learn\stm32\ѧϰ\C8T6FreeRTOS��ֲ\MALLOC\malloc.h
 */
#ifndef __MALLOC_H
#define __MALLOC_H
#include "stm32f10x.h"
//////////////////////////////////////////////////////////////////////////////////	 
//������ֻ��ѧϰʹ�ã�δ���������ɣ��������������κ���;
//ALIENTEK MiniSTM32������
//�ڴ���� ��������	   
//����ԭ��@ALIENTEK
//������̳:www.openedv.com
//�޸�����:2014/3/12
//�汾��V1.0
//��Ȩ���У�����ؾ���
//Copyright(C) �������������ӿƼ����޹�˾ 2009-2019
//All rights reserved									  
//////////////////////////////////////////////////////////////////////////////////


 
#ifndef NULL
#define NULL 0
#endif

//�ڴ�����趨.
#define MEM_BLOCK_SIZE			16  	  						//�ڴ���СΪ32�ֽ�
#define MEM_MAX_SIZE			3*1024  						//�������ڴ� 42K
#define MEM_ALLOC_TABLE_SIZE	MEM_MAX_SIZE/MEM_BLOCK_SIZE 	//�ڴ����С
 
		 
//�ڴ����������
struct _m_mallco_dev
{
	void (*init)(void);				//��ʼ��
	u8 (*perused)(void);		  	//�ڴ�ʹ����
	u8 	*membase;					//�ڴ�� 
	u16 *memmap; 					//�ڴ����״̬��
	u8  memrdy; 					//�ڴ�����Ƿ����
};
extern struct _m_mallco_dev mallco_dev;	//��mallco.c���涨��

void mymemset(void *s,u8 c,u32 count);	//�����ڴ�
void mymemcpy(void *des,void *src,u32 n);//�����ڴ�     
void mem_init(void);					 //�ڴ������ʼ������(��/�ڲ�����)
u32 mem_malloc(u32 size);		 		//�ڴ����(�ڲ�����)
u8 mem_free(u32 offset);		 		//�ڴ��ͷ�(�ڲ�����)
u8 mem_perused(void);					//���ڴ�ʹ����(��/�ڲ�����) 
////////////////////////////////////////////////////////////////////////////////
//�û����ú���
void myfree(void *ptr);  				//�ڴ��ͷ�(�ⲿ����)
void *mymalloc(u32 size);				//�ڴ����(�ⲿ����)
void *myrealloc(void *ptr,u32 size);	//���·����ڴ�(�ⲿ����)
#endif












