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
 ** 文 件 名： mcuinit.c
 **
 ** 文件简述： MCU初始化配置函数，常量和变量
 **
 ** 版本历史:
 ** 2015-10-05 V1.00  Mark   正式版本
 **
 ******************************************************************************/
#include "mcu.h"
#include "usb.h"
#include "usbdevice.h"
#include "UsbDeviceCdcCom.h"
/**
 ******************************************************************************
 ** \简  述  初始化GPIO，使用的IO口设置为对应状态，没使用的设置为输出低电平
 **
 ** \参  数  none
 ** \返回值  none
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
 ** \简  述  初始化MCU系统，包括看门狗、系统时钟、GPIO和各种需要的外设
 **
 ** \参  数  none
 ** \返回值  none
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
