/*
 * @Author: TOTHTOT
 * @Date: 2022-02-19 19:48:43
 * @LastEditTime: 2022-02-23 12:50:14
 * @LastEditors: Please set LastEditors
 * @Description: ��koroFileHeader�鿴���� ��������: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
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

// ��������ʱ��ͳ�ƹ���ʱִ�����´���
#if (configGENERATE_RUN_TIME_STATS == 1)
char RunTimeInfo[400]; //������������ʱ����Ϣ
char InfoBuff[1000];   //������������״̬��Ϣ
#endif

//����1����ֵ�ź���
xSemaphoreHandle Start_Cmd_BinSema;

// ����һ���¼���־��
EventGroupHandle_t EventGroup_Handler;

int main(void)
{
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4); //����ϵͳ�ж����ȼ�����4
	delay_init();									//��ʱ������ʼ��
	uart_init(115200);								//��ʼ������
	LED_Init();										//��ʼ��LED
	mem_init();										//��ʼ���ڲ��ڴ��
	TIM4_Int_Init(100 - 1, 7200 - 1);				//��ʱ��7��ʼ�� 1msûˢ�¼���ֵ���ж��������

	//LORA��æ
	while(AUX_Check())
	{
		printf("LORA��æ\r\n");
		delay_xms(500);
	}

	//������ʼ����
	xTaskCreate((TaskFunction_t)start_task,			 //������
				(const char *)"start_task",			 //��������
				(uint16_t)START_STK_SIZE,			 //�����ջ��С
				(void *)NULL,						 //���ݸ��������Ĳ���
				(UBaseType_t)START_TASK_PRIO,		 //�������ȼ�
				(TaskHandle_t *)&StartTask_Handler); //������
	vTaskStartScheduler();							 //�����������
}

//��ʼ����������
void start_task(void *pvParameters)
{
	taskENTER_CRITICAL(); //�����ٽ���
	EventGroup_Handler = xEventGroupCreate();		// �����¼���־��
	Start_Cmd_BinSema = xSemaphoreCreateBinary();		// ������ֵ�ź���
	//����LED0����
	xTaskCreate((TaskFunction_t)led0_task,
				(const char *)"led0_task",
				(uint16_t)LED0_STK_SIZE,
				(void *)NULL,
				(UBaseType_t)LED0_TASK_PRIO,
				(TaskHandle_t *)&LED0Task_Handler);
	//�ȴ��������ͻ�ȡ��������
	xTaskCreate((TaskFunction_t)wait_Startcmd_task,
				(const char *)"waitstartcmd_task",
				(uint16_t)WAIT_STARTCMD_STK_SIZE,
				(void *)NULL,
				(UBaseType_t)WAIT_STARTCMD_TASK_PRIO,
				(TaskHandle_t *)&WAIT_STARTCMD_Task_Handler);
	//��ȡ����������
	xTaskCreate((TaskFunction_t)get_sensor_data_task,
				(const char *)"gatsensor_task",
				(uint16_t)GET_SENSOR_DATA_STK_SIZE,
				(void *)NULL,
				(UBaseType_t)GET_SENSOR_DATA_TASK_PRIO,
				(TaskHandle_t *)&GET_SENSOR_DATA_Task_Handler);
	taskEXIT_CRITICAL();			//�˳��ٽ���
	usart3_init(115200);							//����3��ʼ��
	LORA_Init();									// Lora��ʼ��
	DHT11_Init();									// DHT11��ʼ��
	BH1750_Init();		 							// BH1750��ʼ��
	Adc_Init();			  							// ADC��ʼ��
	CCS811_GPIO_Config(); 							// CCS811IO�ڳ�ʼ��
	delay_xms(1000);								//�ӳ�һ��ʱ��
	CCS811Init();									//CCS811��ʼ��
	vTaskDelete(StartTask_Handler); //ɾ����ʼ����
}

//LED0������
void led0_task(void *pvParameters)
{
	//int err;
	while (1)
	{
	LED0 = ~LED0;
//		err = xSemaphoreGive(Start_Cmd_BinSema);
//						if(err != pdPASS)
//						{
//							printf("�ͷ�ʧ��\r\n");
//						}
// ��������ʱ��ͳ�ƹ�? �ʱִ�����´?��
#if (configGENERATE_RUN_TIME_STATS == 1)
		printf("LED0��������ִ��\r\n");
		memset(RunTimeInfo, 0, 400);	   //��Ϣ����������
		vTaskGetRunTimeStats(RunTimeInfo); //��ȡ��������ʱ����Ϣ
		printf("������\t\t\t����ʱ��\t������ռ�ٷֱ�\r\n");
		printf("%s\r\n", RunTimeInfo);
		memset(InfoBuff, 0, 1000); //��Ϣ����������
		vTaskList(InfoBuff);
		printf("%s\r\n", InfoBuff);
#endif
		//xEventGroupSetBits(EventGroup_Handler, 1 << 0);
		delay_ms(1500);
	}
}

//�ȴ���ʼ��������
void wait_Startcmd_task(void *pvParameters)
{
	BaseType_t err = pdFALSE;
	EventBits_t EventValue;
	
	while (1)
	{
		//������null˵�������ɹ�
		if(EventGroup_Handler != NULL)
		{
			//��ȡ��־�¼����0λ�͵�1λ,��ȡ�����0, �ȴ����¼���1������1�󷵻�ֵ��Ϊ0
			EventValue = xEventGroupWaitBits(EventGroup_Handler, 1 << 0 | 1 << 1, pdTRUE, pdFALSE, portMAX_DELAY);	
			if (EventValue != 0)
				{
					switch (EventValue)
					{
					case 1 << 0:
						printf("�ͷŶ�ֵ�ź���\r\n");
						err = xSemaphoreGive(Start_Cmd_BinSema);
						if(err != pdPASS)
						{
							printf("�ͷ�ʧ��\r\n");
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
			printf("�¼���־�鴴��ʧ��\r\n");
		}
		delay_ms(500);
	}
}

extern ccs811_measurement_t CCS;

// ��ȡ��������������
void get_sensor_data_task(void *pvParameters)
{
	BaseType_t err = pdFALSE;
	u8 dat[100];
	int temperature, humidity; //��ʪ��
	u16 ADC_Value = 0;
	u8 BH1750Data_H = 0, BH1750Data_L = 0, ADC1_1_H = 0, ADC1_1_L = 0, ECO2_H = 0, ECO2_L = 0, TVOC_H, TVOC_L;

	while (1)
	{
		
		// ��ȡ��ֵ�ź���
		if (Start_Cmd_BinSema != NULL)
		{
			err = xSemaphoreTake(Start_Cmd_BinSema, portMAX_DELAY);
			if (err == pdTRUE)
			{
				printf("����ִ�л�ȡ��������\r\n");
				DHT11_Read_Data(&temperature, &humidity); //��ʪ��
				Get_BH1750Data(&BH1750Data_H, &BH1750Data_L);
				ADC_Value = Get_Adc_Average(1, 20);
				CCS811GetData();
				//��Ϊccs811�ǳ����ȶ�,����ȡ��tvocΪ65201ʱ�������Զ϶�ccs811�ܷ����Է�������
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
				printf("�¶�:%d  ʪ��:%d  ����:%d  ADC:%d\r\n", temperature, humidity, BH1750Data_H, ADC_Value);
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
				printf("��ȡʧ��\r\n");
			}
		}
		else
		{
			printf("�ź���NULL\r\n");
		}
		printf("��ȡ�����������\r\n");
		delay_ms(500);
	}
}
