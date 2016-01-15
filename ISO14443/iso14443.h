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
 ** �� �� ���� iso14443.h
 **
 ** �ļ������� ��ISO14443�Լ�M1���йصĲ��������������Լ���������
 **
 ** �汾��ʷ:
 ** 2015-10-05 V1.00  Mark   ��ʽ�汾
 **
 ******************************************************************************/
#ifndef __ISO14443_H_
#define __ISO14443_H_

#include "typedef.h"
 /**
 ******************************************************************************
 ** \������ȫ��Ԥ���� 
 **
 ******************************************************************************/
#define REQA        0x26
#define WUPA        0x52
#define REQB        0x05
#define WUPB        0x05
#define SLOT_MAKER  0x05
#define SELECT      0x90
#define ATTRIB      0x1D
#define HALTA       0x50
#define HALTB       0x50
#define PATS        0xE0
#define PPS         0xD0

//I-block (00xxxxxx)b (not (00xxx101)b)
//R-block (10xxxxxx)b (not (1001xxxx)b)
//S-block (11xxxxxx)b (not (1110xxxx)b and not (1101xxxx)b) 
/**
 ******************************************************************************
 ** \������ ȫ�ֱ������� 
 **
 ******************************************************************************/ 

/**
 ******************************************************************************
 ** \������ �������� 
 **
 ******************************************************************************/
sta_result_t M1_3Pass(u8 blockAddr, u8 *m1Key, u8 *uid, u32 RA);
sta_result_t M1_PwdChange(u8 blockAddr, u8 *key_a, u8 *key_b);
sta_result_t M1_Write(u8 blockAddr, u32 value);
sta_result_t M1_Read(u8 blockAddr);
sta_result_t M1_Increment(u8 blockAddr, u32 value);
sta_result_t M1_Decrement(u8 blockAddr, u32 value);
sta_result_t M1_Transfer(u8 blockAddr);
sta_result_t M1_Restore(u8 blockAddr);

sta_result_t piccRequestA(void);
sta_result_t piccWakeupA(void);
sta_result_t piccAntiA(u8 rand_bit, u8 *uid, u8* uidLen);
sta_result_t piccSelectA(u8 *uid);
void piccHaltA(void);
sta_result_t piccDeselectA(u8 rate_type);

sta_result_t piccRequestB(u8 n, u8 *uid, u8 *param);
sta_result_t piccSlotMarker(u8 n, u8 *uid, u8 *param);
sta_result_t piccAttrib(u8 *uid, u8 *param);
sta_result_t piccHaltB(u8 *uid);
sta_result_t piccDeselectB(u8 *param);

sta_result_t piccATS(void);
sta_result_t piccPPS(u16 high_rate);
#endif
