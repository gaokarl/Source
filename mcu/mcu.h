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
 ** �� �� ���� mcu.h
 **
 ** �ļ������� MCU�ײ����еĺ����������������������Լ�Ԥ���壬���ⲿ���á�
 **
 ** �汾��ʷ:
 ** 2015-10-05 V1.00  Mark   ��ʽ�汾
 **
 ******************************************************************************/
#ifndef __MCU_H__
#define __MCU_H__

#include "typedef.h"
/******************************************************************************
 ** \brief Include RTE Device
 ** 
 ******************************************************************************/
/*
#ifndef __RTE_DEVICE_H
  #include "RTE_Device.h"
#endif
*/
/******************************************************************************
 ** \brief MCU header file include
 ** 
 ******************************************************************************/
#ifndef _S6E1C3XC_H_
  #include "s6e1c3xc.h"
#endif

/******************************************************************************
 ** \brief MCU system start-up header file include
 ** 
 ******************************************************************************/
#ifndef _SYSTEM_S6E1C3_H_
  #include "system_s6e1c3.h"
#endif

/******************************************************************************
 ** \brief GPIO macros
 ** 
 ******************************************************************************/
#ifndef __GPIO_H__
  #include "gpio.h"
#endif
#if !defined(__FGPIO_H__) && defined(FM_CORE_TYPE_FM0P)
  #include "fgpio.h"
#endif

#if defined(FM_DEVICE_PACKAGE_S6E_C)
    #ifndef __GPIO_S6E1C3XC_H__
      #include "gpio_s6e1c3xc.h"
    #endif
#else
    #error Please define the proper device package in RTE_Device.h
#endif

/**
 ******************************************************************************
 ** \��  ��  Ԥ����
 ******************************************************************************/
#define LED1        bFM0P_GPIO_PDOR4_PC
#define LED2        bFM0P_GPIO_PDOR4_PD
#define LED3        bFM0P_GPIO_PDOR4_PE
#define LEDRED      bFM0P_GPIO_PDOR4_P6
#define LEDGRN      bFM0P_GPIO_PDOR4_P7

#define BUTTON1     bFM0P_GPIO_PDIR5_P2
#define BUTTON2     bFM0P_GPIO_PDIR5_P3

#define KEY_COL1    bFM0P_GPIO_PDIR3_P0
#define KEY_COL2    bFM0P_GPIO_PDIR3_P1
#define KEY_COL3    bFM0P_GPIO_PDIR3_P2
#define KEY_COL4    bFM0P_GPIO_PDIR3_P3
#define KEY_ROW1    bFM0P_GPIO_PDOR3_P4
#define KEY_ROW2    bFM0P_GPIO_PDOR3_PA
#define KEY_ROW3    bFM0P_GPIO_PDOR3_PB
#define KEY_ROW4    bFM0P_GPIO_PDOR3_PC

#define PD          bFM0P_GPIO_PDOR1_PB   // sky1311 power control
#define IRQ_COME    bFM0P_GPIO_PDIR1_PA   // sky1311 IRQ output
#define SPI_CS      bFM0P_GPIO_PDOR1_P4   // sky1311 SPI CS pin


/**
 ******************************************************************************
 ** \��  ��  ��������
 **
 ******************************************************************************/
/* mcuinit.c */ 
void initGpio(void);
void initMCU(void);
/* adc.c */ 
void initADC(void);
static void startADC(void);
static void disableADC(void);
static unsigned char readADC(void);

/* comm.c */
void initUart(void);
void uartPutChar(u8 data);
u8 uartGetChar(void);

void initSPI(void);
void SpiWriteByte(u8 data);
u8 SpiReadByte(void);

/* timer.c */
void waitDelay(u32 count);
void wait_etu(u8 num);
void delay10us(u16 num);
void delay100us(u16 value);
void delay1ms(u16 value);
void delay10ms(u8 num);
void delay100ms(u8 num);

/* key.c */
extern u8 btnVal;
void initButton();

#endif  /* __MCU_H__ */