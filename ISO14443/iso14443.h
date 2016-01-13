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
 ** 文 件 名： iso14443.h
 **
 ** 文件简述： 与ISO14443以及M1卡有关的操作函数，变量以及常量定义
 **
 ** 版本历史:
 ** 2015-10-05 V1.00  Mark   正式版本
 **
 ******************************************************************************/
#ifndef __ISO14443_H_
#define __ISO14443_H_

#include "typedef.h"
 /**
 ******************************************************************************
 ** \简述：全局预定义 
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
 ** \简述： 全局变量定义 
 **
 ******************************************************************************/ 

/**
 ******************************************************************************
 ** \简述： 函数声明 
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
