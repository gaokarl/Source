/******************************************************************************/
/*               (C) ˹���������������Ƽ����޹�˾(SKYRELAY)                   */
/*                                                                            */
/* �˴�����˹���������������Ƽ����޹�˾Ϊ֧�ֿͻ���д��ʾ�������һ����       */
/* ����ʹ��˹���������оƬ���û������޳�ʹ�øô��룬���豣������������       */
/* �������Ϊ��ʾʹ�ã�����֤�������е�Ӧ��Ҫ�󣬵����ض��淶������ʱ��       */
/* ʹ����������ȷ�ϴ����Ƿ����Ҫ�󣬲���Ҫ��ʱ����������Ӧ���޸ġ�           */
/* (V1.00)                                                                    */
/******************************************************************************/
/**
 ******************************************************************************
 ** �� �� ���� adc.c
 **
 ** �ļ������� ��ADC�йصĺ����������ͱ���
 **
 ** �汾��ʷ:
 ** 2015-10-05 V1.00  EH   ��һ����ʽ�汾
 **
 ******************************************************************************/
#include "mcudef.h"
/**
 ******************************************************************************
 ** \��  ��  ��ʼ��ADC��8bit�ֱ��ʣ�2.5V�ڲ��ο�Դ����ͨ��IO P61
 **
 ** \��  ��  none
 ** \����ֵ  none
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
 ** \��  ��  ����ADC������ADת��
 **
 ** \��  ��  none
 ** \����ֵ  none
 ******************************************************************************/
static void startADC(void)
{
    ADC12CTL0 |= ADC12SC;                   // Start conversion
}
/**
 ******************************************************************************
 ** \��  ��  ��ֹADת��
 **
 ** \��  ��  none
 ** \����ֵ  none
 ******************************************************************************/
static void disableADC(void)
{
    ADC12CTL0 &= ~ADC12ENC;                 // Disable conversions
    ADC12CTL0 &= !ADC12ON;
}
/**
 ******************************************************************************
 ** \��  ��  ��ȡADֵ
 **
 ** \��  ��  none
 ** \����ֵ  8λADֵ
 ******************************************************************************/
static unsigned char readADC(void)
{
    while (!(ADC12IFG & BIT0));             // wait conversion end
    return (unsigned char)ADC12MEM0;
}
