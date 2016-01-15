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
 ** �� �� ���� timer.c
 **
 ** �ļ��������Ͷ�ʱ���йصĺ����������ͱ���
 **
 ** �汾��ʷ:
 ** 2015-10-05 V1.00  EH   ��һ����ʽ�汾
 **
 ******************************************************************************/
#include "mcu.h"

/**
 ******************************************************************************
 ** \��  ��  ������ʾһ��ʱ��
 **
 ** \��  ��  32λ����
 ** \����ֵ  none
 ******************************************************************************/
void waitDelay(u32 count)
{
    while(count--){ 
        __no_operation(); __no_operation();
    }
}

/**
 ******************************************************************************
 ** \��  ��  ʹ��dual timer 1 ʵ��num��etu����ʱ etu = 128/13.56MHz = 9.4uS
 **
 ** \��  ��  ��ʱʱ��*9.4uS, ��Χ1--255
 ** \����ֵ  none
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
 ** \��  ��  ʹ��dual timer 1 ʵ��10uS����ʱ
 **
 ** \��  ��  ��ʱʱ��*10uS, ��Χ1--65535
 ** \����ֵ  none
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
 ** \��  ��  ʹ��dual timer 1 ʵ��100uS����ʱ
 **
 ** \��  ��  ��ʱʱ��*100uS����Χ1--65535
 ** \����ֵ  none
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
 ** \��  ��  ʹ��dual timer 1 ʵ��1mS����ʱ
 **
 ** \��  ��  ��ʱʱ��*1mS����Χ1--65535
 ** \����ֵ  none
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
 ** \��  ��  ʹ��dual timer 1 ʵ��10mS����ʱ
 **
 ** \��  ��  ��ʱʱ��*10mS����Χ1--255
 ** \����ֵ  none
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
 ** \��  ��  ʹ��dual timer 1 ʵ��100mS����ʱ
 **
 ** \��  ��  ��ʱʱ��*100mS����Χ1--255
 ** \����ֵ  none
 ******************************************************************************/
void delay100ms(u8 num)
{
    FM_DT->TIMER1INTCLR = 0xFFFFFFFFu;            // Clearing interrupt output from the counter
    FM_DT->TIMER1CONTROL = 0x0Bu;                 // free-run Mode; Interrupt disabled; CLK/256; 32-bit mode; one-shot
    FM_DT->TIMER1LOAD = 12500*num;
    FM_DT->TIMER1CONTROL_f.TIMEREN = 1u;          // Enable Timer 
    while(bFM0P_DT_TIMER1RIS_TIMER1RIS == 0);
}
