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
 ** 文 件 名： adc.c
 **
 ** 文件简述： 和ADC有关的函数，常量和变量
 **
 ** 版本历史:
 ** 2015-10-05 V1.00  EH   第一个正式版本
 **
 ******************************************************************************/
#include "mcudef.h"
/**
 ******************************************************************************
 ** \简  述  初始化ADC，8bit分辨率，2.5V内部参考源，单通道IO P61
 **
 ** \参  数  none
 ** \返回值  none
 ******************************************************************************/
void initADC(void)
{
    FM_GPIO->ADE_f.AN00 = 1u; // Select analogue pin -> AN00

    FM_ADC0->SCIS3 = 0x00u;   // Scan channel select AN31-AN24
    FM_ADC0->SCIS2 = 0x00u;   // Scan channel select AN23-AN16
    FM_ADC0->SCIS1 = 0x00u;   // Scan channel select AN15-AN08
    FM_ADC0->SCIS0 = 0x01u;   // Scan channel select AN07-AN00 -> AN00
    
    FM_ADC0->ADST1 = 0x2Fu;   // Sampling Time 1
    FM_ADC0->ADST0 = 0x2Fu;   // Sampling Time 0
    
    FM_ADC0->ADSS3 = 0x00u;   // Sampling Time Select AN31-AN24
    FM_ADC0->ADSS2 = 0x00u;   // Sampling Time Select AN23-AN16 
    FM_ADC0->ADSS1 = 0x00u;   // Sampling Time Select AN15-AN08
    FM_ADC0->ADSS0 = 0x00u;   // Sampling Time Select AN07-AN00 -> Use Sampling Time 0 for AN00
    
    FM_ADC0->ADCT  = 0x00u;   // Comparison Time = 14 x (ADCT + 2) / HCLK
    
    FM_ADC0->ADCEN = 0x01u;   // Enable ADC    
    
    while (3u != FM_ADC0->ADCEN); // wait until ADC operation is enabled
    
    FM_ADC0->CMPCR = 0x00u;   // No comparison
    FM_ADC0->CMPD  = 0x00u;   // No comparison value
    
    FM_ADC0->SFNS  = 0x00u;   // Set FIFO Stage Count Interrupt
    FM_ADC0->ADCR  = 0x00u;   // Disable ADC interrupts  
    FM_ADC0->ADSR  = 0x40u;   // Result on LSB side
    
    FM_ADC0->SCCR = 0x11u;    // FIFO clear, start ADC Single Conversion   
}
/**
 ******************************************************************************
 ** \简  述  启动ADC，进行AD转换
 **
 ** \参  数  none
 ** \返回值  none
 ******************************************************************************/
static void startADC(void)
{
    ADC12CTL0 |= ADC12SC;                   // Start conversion
}
/**
 ******************************************************************************
 ** \简  述  禁止AD转换
 **
 ** \参  数  none
 ** \返回值  none
 ******************************************************************************/
static void disableADC(void)
{
    ADC12CTL0 &= ~ADC12ENC;                 // Disable conversions
    ADC12CTL0 &= !ADC12ON;
}
/**
 ******************************************************************************
 ** \简  述  读取AD值
 **
 ** \参  数  none
 ** \返回值  8位AD值
 ******************************************************************************/
static unsigned char readADC(void)
{
    while (!(ADC12IFG & BIT0));             // wait conversion end
    return (unsigned char)ADC12MEM0;
}
