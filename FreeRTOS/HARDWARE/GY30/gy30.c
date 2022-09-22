#include "gy30.h"
#include "delay.h"

BYTE BUF[8]; /* 接收数据缓存区 */

void IIC_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    GPIO_PinRemapConfig(GPIO_Remap_SWJ_Disable, ENABLE);
    // 改变指定管脚的映射 GPIO_Remap_SWJ_Disable SWJ 完全禁用（JTAG+SW-DP）
    GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);
    // 改变指定管脚的映射 GPIO_Remap_SWJ_JTAGDisable ，JTAG-DP 禁用 + SW-DP 使能
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
{ /* 光照传感器起始信号 */
    IIC_Start();
}

void BH1750_Stop(void)
{ /* 光照传感器停止信号 */
    IIC_Stop();
}

void BH1750_SendACK(BYTE ack)
{ /* 发送应答信号。参数ack为0是ACK，为1是NAK */
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
{ /* 接收应答信号 */
    return IIC_Wait_Ack();
}

void BH1750_SendByte(BYTE dat)
{ /* 向IIC总线发送一个字节数据 */
    IIC_Send_Byte(dat);
    BH1750_RecvACK();
}

BYTE BH1750_RecvByte(void)
{ /* 从IIC总线接收一个字节数据 */
    return IIC_Read_Byte(0);
}

void Single_Write_BH1750(uchar REG_Address)
{
    BH1750_Start();                /* 起始信号 */
    BH1750_SendByte(SlaveAddress); /* 发送设备地址 + 写信号 */
    BH1750_SendByte(REG_Address);  /* 内部寄存器地址 */
    BH1750_Stop();                 /* 发送停止信号 */
}

void BH1750_Init(void)
{ /* BH1750初始化函数 */
    IIC_Init();
    delay_xms(50);
    Single_Write_BH1750(0x01);
}

void Multiple_read_BH1750(void)
{ /* 连续读出BH1750内部数据 */
    uchar i;
    BH1750_Start();                    /* 起始信号 */
    BH1750_SendByte(SlaveAddress + 1); /* 发送设备地址 + 读信号 */

    for (i = 0; i < 3; i++)
    {                               /* 连续读取2个地址数据，存储在BUF中 */
        BUF[i] = BH1750_RecvByte(); /* BUF[0]存储0x32地址中的数据 */

        if (i == 3)
        {
            BH1750_SendACK(1); /* 最后一个数据需要回应NOACK */
        }
        else
        {
            BH1750_SendACK(0); /* 回应ACK */
        }
    }

    BH1750_Stop(); /* 停止信号 */
    delay_xms(5);
}


void Get_BH1750Data(u8 *BH1750Data_H , u8 *BH1750Data_L)
{
    u16 dis_data = 0;
    float temp = 0.0f;         /* 要显示的数值 */
    Single_Write_BH1750(0x01); /* power on */
    Single_Write_BH1750(0x10); /* H-resolution mode */
    // delay_ms ( 180 ); /* 延时180ms */
    Multiple_read_BH1750(); /* 连续读出数据，存储在BUF中 */
    dis_data = BUF[0];
    dis_data = (dis_data << 8) + BUF[1]; /* 合成数据，即光照数据 */
    temp = (float)dis_data / 1.2;

    *BH1750Data_H = (int)temp >> 8;
    *BH1750Data_L = (int)temp & 0x00ff;

    /* sprintf(show_buf, "Light is %d LX\r\n", ( int ) temp );
	printf("%x  %x  %f\r\n", BH1750Data_H, BH1750Data_L, temp );
    memset(show_buf, 0, sizeof ( show_buf ) ); */
}
