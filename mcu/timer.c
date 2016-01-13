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
 ** 文 件 名： timer.c
 **
 ** 文件简述：和定时器有关的函数，常量和变量
 **
 ** 版本历史:
 ** 2015-10-05 V1.00  EH   第一个正式版本
 **
 ******************************************************************************/
#include "mcu.h"

/**
 ******************************************************************************
 ** \简  述  计数演示一段时间
 **
 ** \参  数  32位计数
 ** \返回值  none
 ******************************************************************************/
void waitDelay(u32 count)
{
    while(count--){ 
        __no_operation(); __no_operation();
    }
}

/**
 ******************************************************************************
 ** \简  述  使用dual timer 1 实现num个etu的延时 etu = 128/13.56MHz = 9.4uS
 **
 ** \参  数  延时时间*9.4uS, 范围1--255
 ** \返回值  none
 ******************************************************************************/
void wait_etu(u8 num)
{
    FM_DT->TIMER1INTCLR = 0xFFFFFFFFu;            // Clearing interrupt output from the counter
    FM_DT->TIMER1CONTROL = 0x03u;                 // free-run Mode; Interrupt disabled; CLK/1; 32-bit mode; one-shot
    FM_DT->TIMER1LOAD = (u32)(300*num)+(u32)(num*0.8);
    FM_DT->TIMER1CONTROL_f.TIMEREN = 1u;          // Enable Timer  
    while(bFM0P_DT_TIMER1RIS_TIMER1RIS == 0);
}

/**
 ******************************************************************************
 ** \简  述  使用dual timer 1 实现10uS的延时
 **
 ** \参  数  延时时间*10uS, 范围1--65535
 ** \返回值  none
 ******************************************************************************/
void delay10us(u16 num)
{
    FM_DT->TIMER1INTCLR = 0xFFFFFFFFu;            // Clearing interrupt output from the counter
    FM_DT->TIMER1CONTROL = 0x03u;                 // free-run Mode; Interrupt disabled; CLK/1; 32-bit mode; one-shot
    FM_DT->TIMER1LOAD = 320*num;
    FM_DT->TIMER1CONTROL_f.TIMEREN = 1u;          // Enable Timer  
    while(bFM0P_DT_TIMER1RIS_TIMER1RIS == 0);
}
/**
 ******************************************************************************
 ** \简  述  使用dual timer 1 实现100uS的延时
 **
 ** \参  数  延时时间*100uS，范围1--65535
 ** \返回值  none
 ******************************************************************************/
void delay100us(u16 num)
{
    FM_DT->TIMER1INTCLR = 0xFFFFFFFFu;            // Clearing interrupt output from the counter
    FM_DT->TIMER1CONTROL = 0x03u;                 // free-run Mode; Interrupt disabled; CLK/1; 32-bit mode; one-shot
    FM_DT->TIMER1LOAD = 3200*num;
    FM_DT->TIMER1CONTROL_f.TIMEREN = 1u;          // Enable Timer  
    while(bFM0P_DT_TIMER1RIS_TIMER1RIS == 0);
}
/**
 ******************************************************************************
 ** \简  述  使用dual timer 1 实现1mS的延时
 **
 ** \参  数  延时时间*1mS，范围1--65535
 ** \返回值  none
 ******************************************************************************/
void delay1ms(u16 num)
{
    FM_DT->TIMER1INTCLR = 0xFFFFFFFFu;            // Clearing interrupt output from the counter
    FM_DT->TIMER1CONTROL = 0x07u;                 // free-run Mode; Interrupt disabled; CLK/16; 32-bit mode; one-shot
    FM_DT->TIMER1LOAD = 2000*num;
    FM_DT->TIMER1CONTROL_f.TIMEREN = 1u;          // Enable Timer 
    while(bFM0P_DT_TIMER1RIS_TIMER1RIS == 0);
}

/**
 ******************************************************************************
 ** \简  述  使用dual timer 1 实现10mS的延时
 **
 ** \参  数  延时时间*10mS，范围1--255
 ** \返回值  none
 ******************************************************************************/
void delay10ms(u8 num)
{
    FM_DT->TIMER1INTCLR = 0xFFFFFFFFu;            // Clearing interrupt output from the counter
    FM_DT->TIMER1CONTROL = 0x0Bu;                 // free-run Mode; Interrupt disabled; CLK/256; 32-bit mode; one-shot
    FM_DT->TIMER1LOAD = 1250*num;
    FM_DT->TIMER1CONTROL_f.TIMEREN = 1u;          // Enable Timer 
    while(bFM0P_DT_TIMER1RIS_TIMER1RIS == 0);
}

/**
 ******************************************************************************
 ** \简  述  使用dual timer 1 实现100mS的延时
 **
 ** \参  数  延时时间*100mS，范围1--255
 ** \返回值  none
 ******************************************************************************/
void delay100ms(u8 num)
{
    FM_DT->TIMER1INTCLR = 0xFFFFFFFFu;            // Clearing interrupt output from the counter
    FM_DT->TIMER1CONTROL = 0x0Bu;                 // free-run Mode; Interrupt disabled; CLK/256; 32-bit mode; one-shot
    FM_DT->TIMER1LOAD = 12500*num;
    FM_DT->TIMER1CONTROL_f.TIMEREN = 1u;          // Enable Timer 
    while(bFM0P_DT_TIMER1RIS_TIMER1RIS == 0);
}
