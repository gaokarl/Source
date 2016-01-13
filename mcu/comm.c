/******************************************************************************/
/*               (C) 斯凯瑞利（北京）科技有限公司(SKYRELAY)                   */
/*                                                                            */
/******************************************************************************/
/*               (C) 斯凯瑞利（北京）科技有限公司(SKYRELAY)                   */
/*                                                                            */
/* 此代码是斯凯瑞利（北京）科技有限公司为支持客户编写的示例程序的一部分       */
/* 所有使用斯凯瑞利相关芯片的用户可以无偿使用该代码，但需保留本声明部分       */
/* 本代码仅为演示使用，不保证符合所有的应用要求，当有特定规范或需求时，       */
/* 使用者需自行确认代码是否符合要求，不合要求时自行做出相应的修改。           */
/* (V1.00)                                                                    */
/******************************************************************************/
/**
 ******************************************************************************
 ** 文 件 名： comm.c
 **
 ** 文件简述： 和串口通信有关的函数，常量和变量
 **
 ** 版本历史:
 ** 2015-10-05 V1.00  EH   第一个正式版本
 **
 ******************************************************************************/
#include "mcu.h"
/**
 ******************************************************************************
 ** \简  述  初始化3,SIN3_1(P50), SOT3_1(P51).115200,8,N,1
 **
 ** \参  数  none
 ** \返回值  none
 ******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
void initUart(void)
{
    /* SIN3_1 Pin */ 
    bFM0P_GPIO_PFR5_P0 = 1;
    FM0P_GPIO->EPFR07_f.SIN3S = 0x02;
    /* SOT3_1 Pin */
    bFM0P_GPIO_PFR5_P1 = 1;
    FM0P_GPIO->EPFR07_f.SOT3B = 0x02;
    
    FM0P_MFS3_UART->SCR  = 0x80;
    FM0P_MFS3_UART->SMR  = 0x01;
    FM0P_MFS3_UART->SSR  = 0x80;
    FM0P_MFS3_UART->ESCR = 0x00;
    FM0P_MFS3_UART->BGR  = 32000000 / 115200 - 1;  // 115.2kbaud @ 32 MHz
    FM0P_MFS3_UART->SCR  = 0x03;
}
/**
 ******************************************************************************
 ** \简  述  UART 发送一个字节数据
 **
 ** \参  数  要发送到8位数
 ** \返回值  none
 ******************************************************************************/
void uartPutChar(u8 data)
{
    while (bFM0P_MFS3_UART_SSR_TDRE == 0 );	// wait for transmit buffer empty
    FM0P_MFS3_UART->TDR = data;             // put ch into buffer
}
/**
 ******************************************************************************
 ** \简  述  UART 接收一个字节数据
 **
 ** \参  数  none
 ** \返回值  要接收到8位数
 ******************************************************************************/
u8 uartGetChar(void)
{
    if (bFM0P_MFS3_UART_SSR_RDRF == 0 ){
        return 0;
    }
    else if (FM0P_MFS3_UART->SSR & 0x38){ 
    	/* overrun or parity error */
        bFM0P_MFS3_UART_SSR_REC = 1;
        return 0;
    } 
    else{
        return (FM0P_MFS3_UART->RDR);
    }
}
/**
 ******************************************************************************
 ** \简  述  初始化SPI, CS--P14, CLK--SCK1_1(P13), MOSI--SOT1_1(P12), MISO--SIN1_1(P11)
 **
 ** \参  数  none
 ** \返回值  none
 ******************************************************************************/
void initSPI(void)
{
    /* SPI_CS Pin */
    bFM0P_GPIO_PDOR1_P4 = 1;   // CS = High
    bFM0P_GPIO_DDR1_P4 = 1;
    
    /* SCK1_1 Pin */
    bFM0P_GPIO_PFR1_P3 = 1;
    FM0P_GPIO->EPFR07_f.SCK1B = 0x02;
    /* SIN1_1 Pin */ 
    bFM0P_GPIO_PFR1_P1 = 1;
    FM0P_GPIO->EPFR07_f.SIN1S = 0x02;
    /* SOT1_1 Pin */
    bFM0P_GPIO_PFR1_P2 = 1;
    FM0P_GPIO->EPFR07_f.SOT1B = 0x02;
    
    FM0P_MFS1_CSIO->SCR  = 0x80;
    FM0P_MFS1_CSIO->SMR  = 0x4F;
    FM0P_MFS1_CSIO->SSR  = 0x80;
    FM0P_MFS1_CSIO->ESCR = 0x00;
    FM0P_MFS1_CSIO->BGR  = 32000000 / 4000000 - 1;  // 4m baud @ 32 MHz
    FM0P_MFS1_CSIO->SCR  = 0x23;
}
/**
 ******************************************************************************
 ** \简  述  SPI接口输出一个字节的内容
 **
 ** \参  数  要输出的字节
 ** \返回值  none
 ******************************************************************************/
void SpiWriteByte(u8 data)
{
    while (bFM0P_MFS1_CSIO_SSR_TDRE == 0 );	// wait for transmit buffer empty
    FM0P_MFS1_CSIO->TDR = data;             // put data into buffer
    //while(bFM0P_MFS1_CSIO_SSR_TDRE == 0);  // wait for start of transmission (or ongoing)
    __no_operation();__no_operation();
    __no_operation();__no_operation();
    __no_operation();__no_operation();
    while(bFM0P_MFS1_CSIO_SSR_TBI == 0);   // wait for transmit finished
}
/**
 ******************************************************************************
 ** \简  述  SPI接口输入一个字节的内容
 **
 ** \参  数  none
 ** \返回值  输入的字节数据
 ******************************************************************************/
u8 SpiReadByte(void)
{
    u8 res;
    res = FM0P_MFS1_CSIO->RDR;            // flush reception register
    bFM0P_MFS1_CSIO_SSR_REC = 1;            // Clear possible errors (REC)
    while (bFM0P_MFS1_CSIO_SSR_TDRE == 0 );	// wait for transmit buffer empty
    FM0P_MFS1_CSIO->TDR = 0xFF;             // put 0xFF into buffer
    while(bFM0P_MFS1_CSIO_SSR_RDRF == 0);   // wait for RDRF
    res = FM0P_MFS1_CSIO->RDR;            // read data buffer
    return res;
}

 