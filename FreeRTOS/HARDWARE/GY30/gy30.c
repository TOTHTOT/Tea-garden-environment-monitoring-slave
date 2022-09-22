#include "gy30.h"
#include "delay.h"

BYTE BUF[8]; /* �������ݻ����� */

void IIC_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    GPIO_PinRemapConfig(GPIO_Remap_SWJ_Disable, ENABLE);
    // �ı�ָ���ܽŵ�ӳ�� GPIO_Remap_SWJ_Disable SWJ ��ȫ���ã�JTAG+SW-DP��
    GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);
    // �ı�ָ���ܽŵ�ӳ�� GPIO_Remap_SWJ_JTAGDisable ��JTAG-DP ���� + SW-DP ʹ��
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3 | GPIO_Pin_4;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    IIC_SCL = 1;
    IIC_SDA = 1;
}

void IIC_Start(void)
{
    SDA_OUT();
    IIC_SDA = 1;
    IIC_SCL = 1;
    delay_us(4);
    IIC_SDA = 0;
    delay_us(4);
    IIC_SCL = 0;
}

void IIC_Stop(void)
{
    SDA_OUT();
    IIC_SCL = 0;
    IIC_SDA = 0;
    delay_us(4);
    IIC_SCL = 1;
    IIC_SDA = 1;
    delay_us(4);
}

u8 IIC_Wait_Ack(void)
{
    u8 ucErrTime = 0;
    SDA_IN();
    IIC_SDA = 1;
    delay_us(1);
    IIC_SCL = 1;
    delay_us(1);

    while (READ_SDA)
    {
        ucErrTime++;

        if (ucErrTime > 250)
        {
            IIC_Stop();
            return 1;
        }
    }

    IIC_SCL = 0;
    return 0;
}

void IIC_Ack(void)
{
    IIC_SCL = 0;
    SDA_OUT();
    IIC_SDA = 0;
    delay_us(2);
    IIC_SCL = 1;
    delay_us(2);
    IIC_SCL = 0;
}

void IIC_NAck(void)
{
    IIC_SCL = 0;
    SDA_OUT();
    IIC_SDA = 1;
    delay_us(2);
    IIC_SCL = 1;
    delay_us(2);
    IIC_SCL = 0;
}

void IIC_Send_Byte(u8 txd)
{
    u8 t;
    SDA_OUT();
    IIC_SCL = 0;

    for (t = 0; t < 8; t++)
    {
        IIC_SDA = (txd & 0x80) >> 7;
        txd <<= 1;
        delay_us(2);
        IIC_SCL = 1;
        delay_us(2);
        IIC_SCL = 0;
        delay_us(2);
    }
}

u8 IIC_Read_Byte(unsigned char ack)
{
    unsigned char i, receive = 0;
    SDA_IN();

    for (i = 0; i < 8; i++)
    {
        IIC_SCL = 0;
        delay_us(2);
        IIC_SCL = 1;
        receive <<= 1;

        if (READ_SDA)
        {
            receive++;
        }

        delay_us(1);
    }

    return receive;
}

void BH1750_Start(void)
{ /* ���մ�������ʼ�ź� */
    IIC_Start();
}

void BH1750_Stop(void)
{ /* ���մ�����ֹͣ�ź� */
    IIC_Stop();
}

void BH1750_SendACK(BYTE ack)
{ /* ����Ӧ���źš�����ackΪ0��ACK��Ϊ1��NAK */
    if (ack == 0)
    {
        IIC_Ack();
    }
    else
    {
        IIC_NAck();
    }
}

BYTE BH1750_RecvACK(void)
{ /* ����Ӧ���ź� */
    return IIC_Wait_Ack();
}

void BH1750_SendByte(BYTE dat)
{ /* ��IIC���߷���һ���ֽ����� */
    IIC_Send_Byte(dat);
    BH1750_RecvACK();
}

BYTE BH1750_RecvByte(void)
{ /* ��IIC���߽���һ���ֽ����� */
    return IIC_Read_Byte(0);
}

void Single_Write_BH1750(uchar REG_Address)
{
    BH1750_Start();                /* ��ʼ�ź� */
    BH1750_SendByte(SlaveAddress); /* �����豸��ַ + д�ź� */
    BH1750_SendByte(REG_Address);  /* �ڲ��Ĵ�����ַ */
    BH1750_Stop();                 /* ����ֹͣ�ź� */
}

void BH1750_Init(void)
{ /* BH1750��ʼ������ */
    IIC_Init();
    delay_xms(50);
    Single_Write_BH1750(0x01);
}

void Multiple_read_BH1750(void)
{ /* ��������BH1750�ڲ����� */
    uchar i;
    BH1750_Start();                    /* ��ʼ�ź� */
    BH1750_SendByte(SlaveAddress + 1); /* �����豸��ַ + ���ź� */

    for (i = 0; i < 3; i++)
    {                               /* ������ȡ2����ַ���ݣ��洢��BUF�� */
        BUF[i] = BH1750_RecvByte(); /* BUF[0]�洢0x32��ַ�е����� */

        if (i == 3)
        {
            BH1750_SendACK(1); /* ���һ��������Ҫ��ӦNOACK */
        }
        else
        {
            BH1750_SendACK(0); /* ��ӦACK */
        }
    }

    BH1750_Stop(); /* ֹͣ�ź� */
    delay_xms(5);
}


void Get_BH1750Data(u8 *BH1750Data_H , u8 *BH1750Data_L)
{
    u16 dis_data = 0;
    float temp = 0.0f;         /* Ҫ��ʾ����ֵ */
    Single_Write_BH1750(0x01); /* power on */
    Single_Write_BH1750(0x10); /* H-resolution mode */
    // delay_ms ( 180 ); /* ��ʱ180ms */
    Multiple_read_BH1750(); /* �����������ݣ��洢��BUF�� */
    dis_data = BUF[0];
    dis_data = (dis_data << 8) + BUF[1]; /* �ϳ����ݣ����������� */
    temp = (float)dis_data / 1.2;

    *BH1750Data_H = (int)temp >> 8;
    *BH1750Data_L = (int)temp & 0x00ff;

    /* sprintf(show_buf, "Light is %d LX\r\n", ( int ) temp );
	printf("%x  %x  %f\r\n", BH1750Data_H, BH1750Data_L, temp );
    memset(show_buf, 0, sizeof ( show_buf ) ); */
}
