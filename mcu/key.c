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
 ** 文 件 名： key.c
 **
 ** 文件简述：和按键有关的函数，常量和变量
 **
 ** 版本历史:
 ** 2015-10-05 V1.00  EH   第一个正式版本
 **
 ******************************************************************************/
#include "mcu.h"

u8 btnVal=0;

/**
 ******************************************************************************
 ** \简  述  初始化2个按钮,button1: P52/INT02_0, button2: P53/int07_2
 **          使用外部中断检测2个按钮
 ** \参  数  none
 ** \返回值  none
 ******************************************************************************/
void initButton(void)
{
    bFM0P_GPIO_PFR5_P2 = 1;
    bFM0P_GPIO_PFR5_P2 = 1;
    FM0P_GPIO->EPFR06_f.EINT02S = 0;    // Use INT02_00 as input
    FM0P_GPIO->EPFR06_f.EINT07S = 3;    // Use INT07_2 as intput 

    FM0P_EXTI->ENIR = 0x00000000u;      // Disable all external interrupts
    //FM0P_EXTI->EIRR = 0x00000008u;      // Request external interrupt 0 detection
    FM0P_EXTI->EICL = 0x00000000u;      // Clear all interrupt flags
    FM0P_EXTI->ELVR = 0xC0030;          // Detects failling edge of INT07,INT02
    FM0P_EXTI->ENIR = 0x00000084u;          // Enable EINT0
  
    NVIC_ClearPendingIRQ(EXINT2_3_IRQn); 
    NVIC_EnableIRQ(EXINT2_3_IRQn);
    NVIC_SetPriority(EXINT2_3_IRQn, 1u);
    
    NVIC_ClearPendingIRQ(EXINT6_7_IRQn); 
    NVIC_EnableIRQ(EXINT6_7_IRQn);
    NVIC_SetPriority(EXINT6_7_IRQn, 1u);
}
/**
 ******************************************************************************
 ** \简  述  INT02 外部中断请求处理
 **          
 ** \参  数  none
 ** \返回值  none
 ******************************************************************************/
void EXINT2_3_IRQHandler(void)
{
    if(bFM0P_EXTI_EICL_ECL2==1){
        bFM0P_EXTI_EICL_ECL2 = 0;
        /* BUTTON1 press */
        btnVal = 1;
    }
}

/**
 ******************************************************************************
 ** \简  述  INT07 外部中断请求处理
 **          
 ** \参  数  none
 ** \返回值  none
 ******************************************************************************/
void EXINT6_7_IRQHandler(void)
{
    if(bFM0P_EXTI_EICL_ECL7==1){
        bFM0P_EXTI_EICL_ECL7 = 0;
        /* BUTTON2 press */
        btnVal = 2;
        
    }
}