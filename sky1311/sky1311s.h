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
 ** 文 件 名： sky1311s.h
 **
 ** 文件简述： 程序中变量的基本类型、结构体和其它类型定义
 **
 ** 版本历史:
 ** 2015-10-05 V1.00  Mark   正式版本
 **
 ******************************************************************************/
#ifndef  __SKY1311S_H_
#define  __SKY1311S_H_

#include "sky1311reg.h"
#include "typedef.h"
/**
 ******************************************************************************
 ** \简述：全局预定义 
 **
 ******************************************************************************/

/**
 ******************************************************************************
 ** \简述： 全局变量定义 
 **
 ******************************************************************************/ 
//extern u8 MaxRCFreqVal;             // AD最大频率
//extern u8 bak_advalue;	            // 备份的AD值
//extern u8 curr_ad_val;                 // 当前AD值
//extern u8 maxADVal;    
extern u8  tx_buf[270];
extern u8  rx_buf[270];
extern u16 rx_len;

extern u8  err_sta;
extern u8  irq_sta;
extern u8  irq_sta_n;
extern u8  irq_sta_h;
extern u8  pcb_p;

/**
 ******************************************************************************
 ** \简述： 函数声明 
 **
 ******************************************************************************/  
void SetBitMask(u8 regAddr, u8 mask);
void ClearBitMask(u8 regAddr, u8 mask);
void analogInit(void);
void directModeInit(u8 mfout_sel);
void pcdAntennaOn(u8 pcdType);
void pcdAntennaOff(void);
void irqClearAll(void);
void irqClear(u8 irq);
void bitRateSet(u16 high_rate);
void typeAOperate(void);
void typeBOperate(void);
void initSky1311s(void);
void tx_func(u16 txLen, u8 txType);
sta_result_t rx_func(u8 rxType, u8 rate_type);
sta_result_t data_tx_rx(u8 txLen, u8 tx_rx_type, u8 rate_type);

u8 sky1311RCFreqCali(void);
u8 checkCardNear(u8 MaxRCFreqVal);
u8 checkCardNearXtal(void);

 
#endif