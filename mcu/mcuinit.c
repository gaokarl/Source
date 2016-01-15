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
 ** �� �� ���� mcuinit.c
 **
 ** �ļ������� MCU��ʼ�����ú����������ͱ���
 **
 ** �汾��ʷ:
 ** 2015-10-05 V1.00  Mark   ��ʽ�汾
 **
 ******************************************************************************/
#include "mcu.h"
#include "usb.h"
#include "usbdevice.h"
#include "UsbDeviceCdcCom.h"
/**
 ******************************************************************************
 ** \��  ��  ��ʼ��GPIO��ʹ�õ�IO������Ϊ��Ӧ״̬��ûʹ�õ�����Ϊ����͵�ƽ
 **
 ** \��  ��  none
 ** \����ֵ  none
 ******************************************************************************/
void initGpio(void)
{
    FM_GPIO->ADE = 0x01ul;      // Enable AN0, Disable all others ADC inputs
    
    /* init 2 buttons */
    bFM0P_GPIO_DDR5_P2 = 0;
    bFM0P_GPIO_PCR5_P2 = 1;
    bFM0P_GPIO_DDR5_P3 = 0;
    bFM0P_GPIO_PCR5_P3 = 1;
    
    /* init keypad*/
    FM0P_GPIO->PDOR3 = 0x1C10;
    FM0P_GPIO->DDR3  = 0x1C10;
    FM0P_GPIO->PCR3  = 0x000F;
    
    /* init LEDs*/
    FM0P_GPIO->PDOR4 = 0x70CC;  // P46,P47,P4C,P4D,P4E for output 1
    FM0P_GPIO->DDR4  = 0x70CC;  // P46,P47,P4C,P4D,P4E for output
    FM0P_DS->SUBOSC_CTL_f.SUBXC = 0x00; // X0A and X1A as digital IO pin
    
    
    /* init  */
    bFM0P_GPIO_PDOR1_PB = 0;
    bFM0P_GPIO_DDR1_PB = 1;     // P1B ( PD of sky1311 ) output 0
    
    bFM0P_GPIO_DDR1_PA = 0;     // P1A ( IRQ of sky1311 )for input
    
}
/**
 ******************************************************************************
 ** \��  ��  ��ʼ��MCUϵͳ���������Ź���ϵͳʱ�ӡ�GPIO�͸�����Ҫ������
 **
 ** \��  ��  none
 ** \����ֵ  none
 ******************************************************************************/
void initMCU(void)
{
    initGpio(); 
    
    LED1 = LED_ON;
    LED2 = LED_ON;
    LED3 = LED_ON;
    LEDRED = LED_ON;
    LEDGRN = LED_ON;
    delay100ms(2);
    LED1 = LED_OFF;
    LED2 = LED_OFF;
    LED3 = LED_OFF;
    LEDRED = LED_OFF;
    LEDGRN = LED_OFF;
    initUart();
    initSPI();
    initButton();
    UsbConfig_UsbInit();
    UsbDeviceCdcCom_SetSeparator('\r');     // there is the possibility to set end of buffer by a seperator
    UsbDeviceCdcCom_SetEchomode(TRUE);      // all input shall be echoed
    UsbConfig_SwitchMode(); 
    __enable_interrupt();
}
