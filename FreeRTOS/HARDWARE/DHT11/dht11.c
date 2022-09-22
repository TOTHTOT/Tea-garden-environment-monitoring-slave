#include "dht11.h"
#include "delay.h"
#include "usart.h"

void DHT11_IO_OUT(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE); // ʹ��PC�˿�ʱ��
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;			  //ѡ���Ӧ������
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure); //��ʼ��PC�˿�
	GPIO_SetBits(GPIOA, GPIO_Pin_7);	   //����
}
void DHT11_IO_IN(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE); // ʹ��PC�˿�ʱ��
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;			  //ѡ���Ӧ������
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure); //��ʼ��PC�˿�
	GPIO_SetBits(GPIOA, GPIO_Pin_7);	   //����
}
//��λDHT11
void DHT11_Rst(void)
{
	DHT11_IO_OUT(); //����Ϊ���
	PA7(0);			//����DQ
	delay_xms(20);	//��������18ms
	PA7(1);			// DQ=1
	delay_us(30);	//��������20~40us
}
// DHT11����Ƿ���ɳ�ʼ��
int DHT11_Check(void)
{
	int retry = 0;
	DHT11_IO_IN();
	while (PA7_IN && retry < 100) // DHT11������40~80us
	{
		retry++;
		delay_us(1);
	};
	if (retry >= 100)
	{
		return 1;
	}
	else
		retry = 0;
	while (!PA7_IN && retry < 100) // DHT11���ͺ���ٴ�����40~80us
	{
		retry++;
		delay_us(1);
	};
	if (retry >= 100)
	{
		return 1;
	}
	return 0;
}

//��DHT11��ȡһ��λ
//����ֵ��1/0
int DHT11_Read_Bit(void)
{
	int retry = 0;
	while (PA7_IN && retry < 100) //�ȴ���Ϊ�͵�ƽ
	{
		retry++;
		delay_us(1);
	}
	retry = 0;
	while (!PA7_IN && retry < 100) //�ȴ���ߵ�ƽ
	{
		retry++;
		delay_us(1);
	}
	delay_us(40); //�ȴ�40us
	if (PA7_IN)
		return 1;
	else
		return 0;
}

//��DHT11��ȡһ���ֽ�
//����ֵ������������
int DHT11_Read_Byte(void)
{
	int i, dat;
	dat = 0;
	for (i = 0; i < 8; i++)
	{
		dat <<= 1;
		dat |= DHT11_Read_Bit();
	}
	return dat;
}
//��DHT11��ȡһ������
// temp:�¶�ֵ(��Χ:0~50��)
// humi:ʪ��ֵ(��Χ:20%~90%)
//����ֵ��0,����;1,��ȡʧ��
int DHT11_Read_Data(int *temp, int *humi)
{
	int buf[5];
	int i;
	DHT11_Rst();
	if (DHT11_Check() == 0)
	{
		for (i = 0; i < 5; i++) //��ȡ40λ����
		{
			buf[i] = DHT11_Read_Byte();
		}
		if ((buf[0] + buf[1] + buf[2] + buf[3]) == buf[4])
		{
			*humi = buf[0];
			*temp = buf[2];
		}
	}
	else
		return 1;
	return 0;
}

void DHT11_Init(void)
{
	while (DHT11_Check())
	{
		DHT11_IO_OUT();
		PA7(0);
		delay_xms(20);
		PA7(1);
		delay_us(35);
		printf("DHT11�����......");
	}
	printf("DHT11���ɹ�");
}
