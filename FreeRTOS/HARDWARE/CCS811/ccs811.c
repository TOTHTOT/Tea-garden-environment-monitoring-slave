/////////////////////////////////////////////////////////////////
//
// ������ֻ��ѧϰʹ�ã�δ��������ɣ��������������κ���;
// STM32F103C8T6���İ�	   
// by Pang
// �޸�����:2019/01/02
// �汾��V1.0
// ��Ȩ���У�����ؾ���
// All rights reserved
//
////////////////////////////////////////////////////////////////

#include "delay.h"
#include "usart.h"
#include "ccs811.h"

u8 CSS811_BUF[12];
u8 Information[10];
u8 MeasureMode, Status, Error_ID;
u8 FlagGetId = 1;
u8 n         = 4;                                        // 3�ζ�ȡID������˵��û����
u8 CSS811_temp      = 0x5a;
ccs811_measurement_t CCS;

#define STEP_DELAY 100
 void CCS811_I2C_GPIO_Config(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	
	/* ʹ���� I2C�йص�ʱ�� */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE );  

	/* PB10-I2C_SCL��PB11-I2C_SDA*/
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4| GPIO_Pin_5; 
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz; 
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD; 
	GPIO_Init(GPIOA, &GPIO_InitStructure); 
	
	SCL_H;
	SDA_H;
}
void delay_1us(u8 x)//������ʱ,iic_40K
{
	u8 i = 20;
	x = i * x;
	while(x--);
}
//////// IIC��ʼ���� //////////
/*
IIC��ʼ:��SCL���ڸߵ�ƽ�ڼ䣬SDA�ɸߵ�ƽ��ɵ͵�ƽ����һ���½��أ�Ȼ��SCL����
*/
u8 I2C_Start(void)
{
		SDA_H; 
		delay_1us(5);	//��ʱ��֤ʱ��Ƶ�ʵ���40K���Ա�ӻ�ʶ��
		SCL_H;
		delay_1us(5);//��ʱ��֤ʱ��Ƶ�ʵ���40K���Ա�ӻ�ʶ��
		if(!SDA_read) return 0;//SDA��Ϊ�͵�ƽ������æ,�˳�
		SDA_L;   //SCL���ڸߵ�ƽ��ʱ��SDA����
		delay_1us(5);
	  if(SDA_read) return 0;//SDA��Ϊ�ߵ�ƽ�����߳���,�˳�
		SCL_L;
	  delay_1us(5);
	  return 1;
}
//**************************************
//IICֹͣ�ź�
/*
IICֹͣ:��SCL���ڸߵ�ƽ�ڼ䣬SDA�ɵ͵�ƽ��ɸߵ�ƽ����һ��������
*/
//**************************************
void CCS811_I2C_Stop(void)
{
    SDA_L;
		SCL_L;
		delay_1us(5);
		SCL_H;
		delay_1us(5);
		SDA_H;//��SCL���ڸߵ�ƽ�ڼ䣬SDA�ɵ͵�ƽ��ɸߵ�ƽ             //��ʱ
}
//**************************************
//IIC����Ӧ���ź�
//��ڲ���:ack (0:ACK 1:NAK)
/*
Ӧ�𣺵��ӻ����յ����ݺ�����������һ���͵�ƽ�ź�
��׼����SDA��ƽ״̬����SCL�ߵ�ƽʱ����������SDA
*/
//**************************************
void I2C_SendACK(u8 i)
{
    if(1==i)
			SDA_H;	             //׼����SDA��ƽ״̬����Ӧ��
    else 
			SDA_L;  						//׼����SDA��ƽ״̬��Ӧ�� 	
	  SCL_H;                    //����ʱ����
    delay_1us(5);                 //��ʱ
    SCL_L ;                  //����ʱ����
    delay_1us(5);    
} 
///////�ȴ��ӻ�Ӧ��////////
/*
������(����)������һ�����ݺ󣬵ȴ��ӻ�Ӧ��
���ͷ�SDA���ôӻ�ʹ�ã�Ȼ��ɼ�SDA״̬
*/
/////////////////
u8 I2C_WaitAck(void) 	 //����Ϊ:=1��ACK,=0��ACK
{
	uint16_t i=0;
	SDA_H;	        //�ͷ�SDA
	SCL_H;         //SCL���߽��в���
	while(SDA_read)//�ȴ�SDA����
	{
		i++;      //�ȴ�����
		if(i==500)//��ʱ����ѭ��
		break;
	}
	if(SDA_read)//�ٴ��ж�SDA�Ƿ�����
	{
		SCL_L; 
		return RESET;//�ӻ�Ӧ��ʧ�ܣ�����0
	}
  delay_1us(5);//��ʱ��֤ʱ��Ƶ�ʵ���40K��
	SCL_L;
	delay_1us(5); //��ʱ��֤ʱ��Ƶ�ʵ���40K��
	return SET;//�ӻ�Ӧ��ɹ�������1
}
//**************************************
//��IIC���߷���һ���ֽ�����
/*
һ���ֽ�8bit,��SCL�͵�ƽʱ��׼����SDA��SCL�ߵ�ƽʱ���ӻ�����SDA
*/
//**************************************
void I2C_SendByte(u8 dat)
{
  u8 i;
	SCL_L;//SCL���ͣ���SDA׼��
  for (i=0; i<8; i++)         //8λ������
  {
		if(dat&0x80)//SDA׼��
		SDA_H;  
		else 
		SDA_L;
    SCL_H;                //����ʱ�ӣ����ӻ�����
    delay_1us(5);        //��ʱ����IICʱ��Ƶ�ʣ�Ҳ�Ǹ��ӻ������г���ʱ��
    SCL_L;                //����ʱ�ӣ���SDA׼��
    delay_1us(5); 		  //��ʱ����IICʱ��Ƶ��
		dat <<= 1;          //�Ƴ����ݵ����λ  
  }					 
}
//**************************************
//��IIC���߽���һ���ֽ�����
//**************************************
u8 I2C_RecvByte()
{
    u8 i;
    u8 dat = 0;
    SDA_H;//�ͷ�SDA�����ӻ�ʹ��
    delay_1us(1);         //��ʱ���ӻ�׼��SDAʱ��            
    for (i=0; i<8; i++)         //8λ������
    { 
		  dat <<= 1;
			
      SCL_H;                //����ʱ���ߣ������ӻ�SDA
     
		  if(SDA_read) //������    
		   dat |=0x01;      
       delay_1us(5);     //��ʱ����IICʱ��Ƶ��		
       SCL_L;           //����ʱ���ߣ�������յ�������
       delay_1us(5);   //��ʱ���ӻ�׼��SDAʱ��
    } 
    return dat;
}
//**************************************
//��IIC�豸д��һ���ֽ�����
//**************************************
u8 CCS811_WriteI2C_byte(u8 Slave_Address,u8 REG_Address,u8 data)
{
	  if(I2C_Start()==0)  //��ʼ�ź�
		{CCS811_I2C_Stop(); return RESET;}           

    I2C_SendByte(Slave_Address);   //�����豸��ַ+д�ź�
 	  if(!I2C_WaitAck()){CCS811_I2C_Stop(); return RESET;}
   
		I2C_SendByte(REG_Address);    //�ڲ��Ĵ�����ַ��
 	  if(!I2C_WaitAck()){CCS811_I2C_Stop(); return RESET;}
   
		I2C_SendByte(data);       //�ڲ��Ĵ������ݣ�
	  if(!I2C_WaitAck()){CCS811_I2C_Stop(); return RESET;}
		
		CCS811_I2C_Stop();   //����ֹͣ�ź�
		
		return SET;
}

u8 CCS811_MWriteI2C_byte(u8 Slave_Address,u8 REG_Address,u8 const *data,u8 length)
{
	  if(I2C_Start()==0)  //��ʼ�ź�
		{CCS811_I2C_Stop(); return RESET;}           

    I2C_SendByte(Slave_Address);   //�����豸��ַ+д�ź�
 	  if(!I2C_WaitAck()){CCS811_I2C_Stop(); return RESET;}
   
		I2C_SendByte(REG_Address);    //�ڲ��Ĵ�����ַ��
 	  if(!I2C_WaitAck()){CCS811_I2C_Stop(); return RESET;}
   
	while(length)
	{
		I2C_SendByte(*data++);       //�ڲ��Ĵ������ݣ�
	   if(!I2C_WaitAck()){CCS811_I2C_Stop(); return RESET;}           //Ӧ��
		length--;
	}
	//	I2C_SendByte(*data);       //�ڲ��Ĵ������ݣ�
 	//	if(!I2C_WaitAck()){CCS811_I2C_Stop(); return RESET;}
		CCS811_I2C_Stop();   //����ֹͣ�ź�		
		return SET;
}

//**************************************
//��IIC�豸��ȡһ���ֽ�����
//**************************************
u8 CCS811_ReadI2C(u8 Slave_Address,u8 REG_Address,u8 *REG_data,u8 length)
{
 if(I2C_Start()==0)  //��ʼ�ź�
		{CCS811_I2C_Stop(); return RESET;}          
	 
	I2C_SendByte(Slave_Address);    //�����豸��ַ+д�ź�
 	if(!I2C_WaitAck()){CCS811_I2C_Stop(); return RESET;} 
	
	I2C_SendByte(REG_Address);     //���ʹ洢��Ԫ��ַ
 	if(!I2C_WaitAck()){CCS811_I2C_Stop(); return RESET;} 
	
	if(I2C_Start()==0)  //��ʼ�ź�
			{CCS811_I2C_Stop(); return RESET;}            

	I2C_SendByte(Slave_Address+1);  //�����豸��ַ+���ź�
 	if(!I2C_WaitAck()){CCS811_I2C_Stop(); return RESET;}
	
	while(length-1)
	{
		*REG_data++=I2C_RecvByte();       //�����Ĵ�������
		I2C_SendACK(0);               //Ӧ��
		length--;
	}
	*REG_data=I2C_RecvByte();  
	I2C_SendACK(1);     //����ֹͣ�����ź�
	CCS811_I2C_Stop();                    //ֹͣ�ź�
	return SET;
}


void CCS811_GPIO_Config()
{

	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);  //���˿�ʱ��

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;					 		 // CCS811-CS
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;	     //�������
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	CCS811_CS_OFF();

	CCS811_I2C_GPIO_Config(); 													   //IIC GPIO Configure
}

void CCS811Init()
{
	u8 idCount = 0;               // count the correct times of id.
	CCS811_CS_ON(); 					   	//nWAKE pin is asserted at least 50��s before the transaction and kept asserted throughout,nWAKE pin is active low
	delay_xms(STEP_DELAY);
	// get CCS811 device id,when addr pin connect to GND and the id is 0x81(129)
	while( FlagGetId)
	{
		CCS811_ReadI2C(CCS811_Add, 0x20, Information, 1); //Read CCS's information  ,ID
		if(Information[0] == 0x81)
		{
			if(++idCount == n)
			{
				FlagGetId = 0;
			}
			else
			{
				printf("id=%d,correct %d!\r\n", Information[0], idCount);
			}
		}
		else
		{
			printf("id=%d,incorrect,continuing...\r\n", Information[0]);
		}
		delay_xms(STEP_DELAY);
	}
	printf("id correct,initing...\r\n");
	delay_xms(STEP_DELAY);
	CCS811_ReadI2C(CCS811_Add, 0x23, &Information[1], 2);	  //FW_Boot_Version
	delay_xms(STEP_DELAY);
	CCS811_ReadI2C(CCS811_Add, 0x24, &Information[3], 2); 	//FW_App_Version
	delay_xms(STEP_DELAY);
	CCS811_ReadI2C(CCS811_Add, 0x00, &Status, 1);	          //Firstly the status register is read and the APP_VALID flag is checked.
	delay_xms(STEP_DELAY);
	// if there is valid application firmware loaded
	if(Status & 0x10)
	{

		while(!(Status & 0x80)) // if firmware not in application mode but boot mode.
		{
			CCS811_WriteI2C_byte(CCS811_Add, 0xF3, 0xF0);	      // Application Verify
			printf("trying to transition the CCS811 state from boot to application mode...\r\n");
			CCS811_MWriteI2C_byte(CCS811_Add, 0xF4, &CSS811_temp, 0);	//Used to transition the CCS811 state from boot to application mode, a write with no data is required.
			delay_xms(STEP_DELAY);
			CCS811_ReadI2C(CCS811_Add, 0x00, &Status, 1);
			delay_xms(STEP_DELAY);

		}
		printf("CCS811 is already in application mode!\r\n");
	}
	delay_xms(STEP_DELAY);
	CCS811_ReadI2C(CCS811_Add, 0x01, &MeasureMode, 1);
	delay_xms(STEP_DELAY);
	MeasureMode &= 0x70;                                       // get measure mode
	//if measure mode incorrect,and reset the measure mode.
	while(MeasureMode != DRIVE_MODE_1SEC)
	{
		CCS811_WriteI2C_byte(CCS811_Add, 0x01, DRIVE_MODE_1SEC); // Write Measure Mode Register,sensor measurement every second,no interrupt
		delay_xms(STEP_DELAY);
		CCS811_ReadI2C(CCS811_Add, 0x01, &MeasureMode, 1);
		MeasureMode &= 0x70;
		printf("trying to enter measure mode...\r\n");
		delay_xms(STEP_DELAY);
	}
	delay_xms(STEP_DELAY);
	CCS811_ReadI2C(CCS811_Add, 0x00, &Status, 1);
	delay_xms(STEP_DELAY);
	CCS811_ReadI2C(CCS811_Add, 0x01, &MeasureMode, 1);
	delay_xms(STEP_DELAY);
	CCS811_CS_OFF();
	delay_xms(STEP_DELAY);
	CCS811_ReadI2C(CCS811_Add, 0xE0, &Error_ID, 1);
	printf("status=%d error_id=%d measureMode=%d \r\n", Status, Error_ID, MeasureMode);

}

void CCS811GetData()
{
	CCS811_CS_ON(); 	// nWAKE pin is asserted at least 50��s before the transaction and kept asserted throughout,nWAKE pin is active low
	delay_xms(10);
	CCS811_ReadI2C(CCS811_Add, 0x02, CSS811_BUF, 8);
	delay_xms(10);
	CCS811_ReadI2C(CCS811_Add, 0x20, Information, 1); // Read CCS's information  ,ID
	delay_xms(10);
	CCS811_ReadI2C(CCS811_Add, 0xE0, &Error_ID, 1);
	CCS811_CS_OFF();
	CCS.eco2 = (u16)CSS811_BUF[0] * 256 + CSS811_BUF[1];
	CCS.tvoc = (u16)CSS811_BUF[2] * 256 + CSS811_BUF[3];
	CCS.device_id  = Information[0];
	CCS.error_id   = Error_ID;
	Error_ID       = 0;
	Information[0] = 0;
}

void CCS811ClearData()
{
	CCS.device_id = 0;
	CCS.eco2      = 0;
	CCS.status    = 0;
	CCS.tvoc      = 0;
	CCS.error_id  = 0;

}
