/*
 * @Author: TOTHTOT
 * @Date: 2022-02-19 19:48:43
 * @LastEditTime: 2022-02-23 12:50:14
 * @LastEditors: Please set LastEditors
 * @Description: 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 * @FilePath: \USER\main.c
 */
#include "led.h"
#include "delay.h"
#include "sys.h"
#include "main.h"
#include "timer.h"
#include "malloc.h"
#include "string.h"
#include "usart2.h"
#include "usart3.h"
#include "lora.h"
#include "dht11.h"
#include "gy30.h"
#include "timer.h"
#include "adc.h"
#include "ccs811.h"
#include "event_groups.h"
#include "semphr.h"
#include "ccs811.h"
#include "gy30.h"

// 开启任务时间统计功能时执行以下代码
#if (configGENERATE_RUN_TIME_STATS == 1)
char RunTimeInfo[400]; //保存任务运行时间信息
char InfoBuff[1000];   //保存任务运行状态信息
#endif

//声明1个二值信号量
xSemaphoreHandle Start_Cmd_BinSema;

// 声明一个事件标志组
EventGroupHandle_t EventGroup_Handler;

int main(void)
{
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4); //设置系统中断优先级分组4
	delay_init();									//延时函数初始化
	uart_init(115200);								//初始化串口
	LED_Init();										//初始化LED
	mem_init();										//初始化内部内存池
	TIM4_Int_Init(100 - 1, 7200 - 1);				//定时器7初始化 1ms没刷新计数值就判定接收完成

	//LORA判忙
	while(AUX_Check())
	{
		printf("LORA正忙\r\n");
		delay_xms(500);
	}

	//创建开始任务
	xTaskCreate((TaskFunction_t)start_task,			 //任务函数
				(const char *)"start_task",			 //任务名称
				(uint16_t)START_STK_SIZE,			 //任务堆栈大小
				(void *)NULL,						 //传递给任务函数的参数
				(UBaseType_t)START_TASK_PRIO,		 //任务优先级
				(TaskHandle_t *)&StartTask_Handler); //任务句柄
	vTaskStartScheduler();							 //开启任务调度
}

//开始任务任务函数
void start_task(void *pvParameters)
{
	taskENTER_CRITICAL(); //进入临界区
	EventGroup_Handler = xEventGroupCreate();		// 创建事件标志组
	Start_Cmd_BinSema = xSemaphoreCreateBinary();		// 创建二值信号量
	//创建LED0任务
	xTaskCreate((TaskFunction_t)led0_task,
				(const char *)"led0_task",
				(uint16_t)LED0_STK_SIZE,
				(void *)NULL,
				(UBaseType_t)LED0_TASK_PRIO,
				(TaskHandle_t *)&LED0Task_Handler);
	//等待主机发送获取数据命令
	xTaskCreate((TaskFunction_t)wait_Startcmd_task,
				(const char *)"waitstartcmd_task",
				(uint16_t)WAIT_STARTCMD_STK_SIZE,
				(void *)NULL,
				(UBaseType_t)WAIT_STARTCMD_TASK_PRIO,
				(TaskHandle_t *)&WAIT_STARTCMD_Task_Handler);
	//获取传感器数据
	xTaskCreate((TaskFunction_t)get_sensor_data_task,
				(const char *)"gatsensor_task",
				(uint16_t)GET_SENSOR_DATA_STK_SIZE,
				(void *)NULL,
				(UBaseType_t)GET_SENSOR_DATA_TASK_PRIO,
				(TaskHandle_t *)&GET_SENSOR_DATA_Task_Handler);
	taskEXIT_CRITICAL();			//退出临界区
	usart3_init(115200);							//串口3初始化
	LORA_Init();									// Lora初始化
	DHT11_Init();									// DHT11初始化
	BH1750_Init();		 							// BH1750初始化
	Adc_Init();			  							// ADC初始化
	CCS811_GPIO_Config(); 							// CCS811IO口初始化
	delay_xms(1000);								//延迟一段时间
	CCS811Init();									//CCS811初始化
	vTaskDelete(StartTask_Handler); //删除开始任务
}

//LED0任务函数
void led0_task(void *pvParameters)
{
	//int err;
	while (1)
	{
	LED0 = ~LED0;
//		err = xSemaphoreGive(Start_Cmd_BinSema);
//						if(err != pdPASS)
//						{
//							printf("释放失败\r\n");
//						}
// 开启任务时间统计功? 苁敝葱幸韵麓?码
#if (configGENERATE_RUN_TIME_STATS == 1)
		printf("LED0任务正在执行\r\n");
		memset(RunTimeInfo, 0, 400);	   //信息缓冲区清零
		vTaskGetRunTimeStats(RunTimeInfo); //获取任务运行时间信息
		printf("任务名\t\t\t运行时间\t运行所占百分比\r\n");
		printf("%s\r\n", RunTimeInfo);
		memset(InfoBuff, 0, 1000); //信息缓冲区清零
		vTaskList(InfoBuff);
		printf("%s\r\n", InfoBuff);
#endif
		//xEventGroupSetBits(EventGroup_Handler, 1 << 0);
		delay_ms(1500);
	}
}

//等待开始命令任务
void wait_Startcmd_task(void *pvParameters)
{
	BaseType_t err = pdFALSE;
	EventBits_t EventValue;
	
	while (1)
	{
		//不等于null说明创建成功
		if(EventGroup_Handler != NULL)
		{
			//获取标志事件组第0位和第1位,获取完后置0, 等待的事件有1个被置1后返回值不为0
			EventValue = xEventGroupWaitBits(EventGroup_Handler, 1 << 0 | 1 << 1, pdTRUE, pdFALSE, portMAX_DELAY);	
			if (EventValue != 0)
				{
					switch (EventValue)
					{
					case 1 << 0:
						printf("释放二值信号量\r\n");
						err = xSemaphoreGive(Start_Cmd_BinSema);
						if(err != pdPASS)
						{
							printf("释放失败\r\n");
						}
						break;
					case 1 << 1:
						LED0 = 1;
						break;
					default:
						break;
					}
					printf("EventValue:%d\r\n", EventValue);
				}
		}
		else
		{
			printf("事件标志组创建失败\r\n");
		}
		delay_ms(500);
	}
}

extern ccs811_measurement_t CCS;

// 获取传感器数据任务
void get_sensor_data_task(void *pvParameters)
{
	BaseType_t err = pdFALSE;
	u8 dat[100];
	int temperature, humidity; //温湿度
	u16 ADC_Value = 0;
	u8 BH1750Data_H = 0, BH1750Data_L = 0, ADC1_1_H = 0, ADC1_1_L = 0, ECO2_H = 0, ECO2_L = 0, TVOC_H, TVOC_L;

	while (1)
	{
		
		// 获取二值信号量
		if (Start_Cmd_BinSema != NULL)
		{
			err = xSemaphoreTake(Start_Cmd_BinSema, portMAX_DELAY);
			if (err == pdTRUE)
			{
				printf("正在执行获取数据任务\r\n");
				DHT11_Read_Data(&temperature, &humidity); //温湿度
				Get_BH1750Data(&BH1750Data_H, &BH1750Data_L);
				ADC_Value = Get_Adc_Average(1, 20);
				CCS811GetData();
				//因为ccs811非常不稳定,当读取的tvoc为65201时基本可以断定ccs811跑飞所以发送重启
				if (CCS.tvoc > 60000)
				{
					__set_FAULTMASK(1);
					NVIC_SystemReset();
				}
				ADC1_1_H = ADC_Value >> 8;
				ADC1_1_L = ADC_Value & 0x00ff;
				ECO2_H = CCS.eco2 >> 8;
				ECO2_L = CCS.eco2 & 0x00ff;
				TVOC_H = CCS.tvoc >> 8;
				TVOC_L = CCS.tvoc & 0x00ff;
				printf("eco2=%d  tvoc=%d id=%d\r\n", CCS.eco2, CCS.tvoc, CCS.device_id);
				printf("SH_L:%d\r\n", ADC1_1_L);
				printf("温度:%d  湿度:%d  光照:%d  ADC:%d\r\n", temperature, humidity, BH1750Data_H, ADC_Value);
				dat[0] = '&';
				dat[1] = '&';
				dat[2] = temperature;
				dat[3] = '#';
				dat[4] = humidity;
				dat[5] = '#';
				dat[6] = BH1750Data_H;
				dat[7] = '#';
				dat[8] = BH1750Data_L;
				dat[9] = '#';
				dat[10] = ADC1_1_H;
				dat[11] = '#';
				dat[12] = ADC1_1_L;
				dat[13] = '#';
				dat[14] = ECO2_H;
				dat[15] = '#';
				dat[16] = ECO2_L;
				dat[17] = '#';
				dat[18] = TVOC_H;
				dat[19] = '#';
				dat[20] = TVOC_L;
				dat[21] = '$';
				dat[22] = '$';
				printf("%d  %c  %d  %d  %d  %d  %d  \r\n", dat[2], dat[4], dat[6], dat[8], dat[10], dat[14], dat[16]);
				Send_Data(MASTER_LORA_ADDH, MASTER_LORA_ADDL, MASTER_LORA_CHANNEL, dat);
				CCS811ClearData();
			}
			else
			{
				printf("获取失败\r\n");
			}
		}
		else
		{
			printf("信号量NULL\r\n");
		}
		printf("获取数据任务完成\r\n");
		delay_ms(500);
	}
}
