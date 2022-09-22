/*
 * @Author: TOTHTOT
 * @Date: 2022-02-19 20:18:16
 * @LastEditTime: 2022-02-23 12:21:14
 * @LastEditors: Please set LastEditors
 * @Description: 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 * @FilePath: \USERe:\Learn\stm32\学习\C8T6FreeRTOS移植\HARDWARE\TIMER\timer.c
 */
#include "timer.h"
#include "led.h"
#include "led.h"
#include "usart.h"
#include "usart3.h"
#include "malloc.h"
#include "string.h"
#include "FreeRTOS.h" //FreeRTOS使用
#include "task.h"
#include "queue.h"
#include "event_groups.h"
#include "semphr.h"
#include "lora.h"

u32 FreeRTOSRunTimeTicks;

void ConfigureTimeForRunTimeStats(void)
{
	FreeRTOSRunTimeTicks = 0;
	TIM3_Int_Init(50 - 1, 72 - 1);
}

void TIM3_Int_Init(u16 arr, u16 psc)
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);

	TIM_TimeBaseStructure.TIM_Period = arr;
	TIM_TimeBaseStructure.TIM_Prescaler = psc;
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);

	TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE);

	NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 7;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	// 使能定时器
	TIM_Cmd(TIM3, ENABLE);
}

void TIM4_Int_Init(u16 arr, u16 psc)
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);

	TIM_TimeBaseStructure.TIM_Period = arr;
	TIM_TimeBaseStructure.TIM_Prescaler = psc;
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInit(TIM4, &TIM_TimeBaseStructure);

	TIM_ITConfig(TIM4, TIM_IT_Update, ENABLE);

	NVIC_InitStructure.NVIC_IRQChannel = TIM4_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 6;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}

void TIM3_IRQHandler(void)
{
	if (TIM_GetITStatus(TIM3, TIM_IT_Update) == SET)
	{
		FreeRTOSRunTimeTicks++;
	}
	TIM_ClearITPendingBit(TIM3, TIM_IT_Update);
}
extern EventGroupHandle_t EventGroup_Handler; //事件标志组句柄

void TIM4_IRQHandler(void)
{
	BaseType_t xHigherPriorityTaskWoken, err;
	// 触发中断
	// printf("触发中断\r\n");
	if (TIM_GetITStatus(TIM4, TIM_IT_Update) == SET)
	{
		/* if (TIM4_EN_COUN == 1)
		{
			TIM4_EN_COUN_NUM++;
			printf("判断是否接收完成\r\n");
			if (TIM4_EN_COUN_NUM >= 4)
			{
				USART3_RX_STA |= 0x8000;
				printf("接收完成:%s\r\n", USART3_RX_BUF);
				TIM4_EN_COUN = 0;
			}
		} */
		// printf("接收完成, 长度:%d\r\n", USART3_RX_STA&0x3fff);
		USART3_RX_STA |= 1 << 15;
		printf("接收完成, 长度%d\r\n", USART3_RX_STA & 0x3fff);
		printf("%s\r\n", USART3_RX_BUF);
		if (strstr((char *)USART3_RX_BUF, "OK") != NULL)
		{
			// printf("%S\r\n", USART3_RX_BUF);
			USART3_RX_STA = 0;
			memset(USART3_RX_BUF, '\0', USART3_MAX_RECV_LEN);
			Lora_Ok_Flag = 1;
		}
		if (strstr((char *)USART3_RX_BUF, "ERROR") != NULL)
		{
			// printf("%S\r\n", USART3_RX_BUF);
			USART3_RX_STA = 0;
			memset(USART3_RX_BUF, '\0', USART3_MAX_RECV_LEN);
			Lora_Ok_Flag = 2;
		}
		if (strstr((char *)USART3_RX_BUF, "start") != NULL)
		{
			// printf("%S\r\n", USART3_RX_BUF);
			USART3_RX_STA = 0;
			memset(USART3_RX_BUF, '\0', USART3_MAX_RECV_LEN);
			err = xEventGroupSetBitsFromISR(EventGroup_Handler, 1 << 0, &xHigherPriorityTaskWoken);
			if (err == pdFALSE)
			{
				printf("接收到start,置1错误\r\n");
			}
			portYIELD_FROM_ISR(xHigherPriorityTaskWoken); //如果需要的话进行一次任务切换
		}
		if (strstr((char *)USART3_RX_BUF, "reboot") != NULL)
		{
			// printf("%S\r\n", USART3_RX_BUF);
			USART3_RX_STA = 0;
			memset(USART3_RX_BUF, '\0', USART3_MAX_RECV_LEN);
			err = xEventGroupSetBitsFromISR(EventGroup_Handler, 1 << 1, &xHigherPriorityTaskWoken);
			if (err == pdFALSE)
			{
				printf("接收到reboot,置1错误\r\n");
			}
			portYIELD_FROM_ISR(xHigherPriorityTaskWoken); //如果需要的话进行一次任务切换
		}
		TIM_Cmd(TIM4, DISABLE);
	}
	TIM_ClearITPendingBit(TIM4, TIM_IT_Update);
}
