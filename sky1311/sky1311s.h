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
 ** �� �� ���� sky1311s.h
 **
 ** �ļ������� �����б����Ļ������͡��ṹ����������Ͷ���
 **
 ** �汾��ʷ:
 ** 2015-10-05 V1.00  Mark   ��ʽ�汾
 **
 ******************************************************************************/
#ifndef  __SKY1311S_H_
#define  __SKY1311S_H_

#include "sky1311reg.h"
#include "typedef.h"
/**
 ******************************************************************************
 ** \������ȫ��Ԥ���� 
 **
 ******************************************************************************/

/**
 ******************************************************************************
 ** \������ ȫ�ֱ������� 
 **
 ******************************************************************************/ 
//extern u8 MaxRCFreqVal;             // AD���Ƶ��
//extern u8 bak_advalue;	            // ���ݵ�ADֵ
//extern u8 curr_ad_val;                 // ��ǰADֵ
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
 ** \������ �������� 
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