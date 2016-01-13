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
 ** 文 件 名： iso14443_3b.c
 **
 ** 文件简述： 符合ISO14443-3 type B的操作函数，常量和变量
 **
 ** 版本历史:
 ** 2015-10-05 V1.00  EH   第一个正式版本
 **
 ******************************************************************************/
#include "sky1311s.h"
#include "driver.h"  
#include "iso14443.h"

/**
 ******************************************************************************
 ** \简  述  type B 请求命令REQB, 返回值是 ATQB。函数做了如下设定：
 **          1）将AFI设置为0，使所有PICC都能够响应，
 **          2）将参数的REQB/WUPB位设置为0，即只发送REQB命令
 **
 ** \参  数  u8 N 时隙总数,取值范围0--4, 
 **          u8* uid 返回的ATQB中UID部分， 
 **          u8* param 返回的ATQB中协议类型和NAD/CID
 ** \返回值  状态，Ok：成功， 其它值：失败
 ******************************************************************************/
sta_result_t piccRequestB(u8 n, u8 *uid, u8 *param)
{
    sta_result_t sta;

    tx_buf[0] = REQB;                           // APf = 0x05
    tx_buf[1] = 0x00;                           // AFI, 选择所有PICC
    tx_buf[2] = (n & 0x07);
    sta = data_tx_rx(3, TYPE_B, RATE_OFF);

    if(sta == Ok){
        uid[0] = rx_buf[1]; 
        uid[1] = rx_buf[2];
        uid[2] = rx_buf[3]; 
        uid[3] = rx_buf[4];                     // 4 Bytes PUPI
        param[0] = (rx_buf[10]&0x0f);           // protocol type
        param[1] = (rx_buf[11]&0x01) ? 0x01 : 0x00; // NAD/CID
    }
    return sta;
}
/**
 ******************************************************************************
 ** \简  述  type B 时隙标记防碰撞 Slot-Marker，返回值是ATQB
 **
 ** \参  数  u8 时隙数目1--15,与piccRequestB中的N参数有关
 **          u8* uid 返回的ATQB中UID部分， 
 **          u8* param 返回的ATQB中协议类型和NAD/CID
 ** \返回值  状态，Ok：正确， 其它值：错误
 ******************************************************************************/
sta_result_t piccSlotMarker(u8 n, u8 *uid, u8 *param)
{
    sta_result_t sta;
    tx_buf[0] = ( (n<<4)|SLOT_MAKER );      // APf = xxxx0101b
    sta = data_tx_rx(1, TYPE_B, RATE_OFF);
    if(sta == Ok){
        uid[0] = rx_buf[1]; 
        uid[1] = rx_buf[2];
        uid[2] = rx_buf[3]; 
        uid[3] = rx_buf[4];                      // 4 Bytes PUPI
        param[0] = (rx_buf[10]&0x0f);          // protocol type
        param[1] = (rx_buf[11]&0x01) ? 0x01 : 0x00; // NAD/CID
    }
    return sta;
}
/**
 ******************************************************************************
 ** \简  述  根据UID选择特定的type B PICC
 **
 ** \参  数  u8* UID首地址， u8* 参数首地址
 ** \返回值  状态
 ******************************************************************************/
sta_result_t piccAttrib(u8 *uid, u8 *param)
{
    sta_result_t sta;
    tx_buf[0] = ATTRIB;     // 0x1D
    tx_buf[1] = uid[0];
    tx_buf[2] = uid[1];
    tx_buf[3] = uid[2];
    tx_buf[4] = uid[3];                             // 4 Bytes PUPI
    tx_buf[5] = 0x00;                               // param 1
    tx_buf[6] = 0x08;                               // param 2, rate and length
    tx_buf[7] = (0x00|param[0]);                    // param 3, 0000, [3:0] protocol
    if( (uid[0]==0x00)&&(uid[1]==0x00)&&(uid[2]==0x00)&&(uid[3]==0x00) )
    tx_buf[7] = 0x01;                               
    tx_buf[8] = param[1];                           // param 4, 0000, [3:0] CID
    sta = data_tx_rx(9, TYPE_B, RATE_OFF);
    return sta;
}
/**
 ******************************************************************************
 ** \简  述  将指定的type B PICC操作挂起
 **
 ** \参  数  u8* UID首地址
 ** \返回值  状态
 ******************************************************************************/
sta_result_t piccHaltB(u8 *uid)
{
    sta_result_t sta;
    tx_buf[0] = HALTB;      // 0x50;
    tx_buf[1] = uid[0];
    tx_buf[2] = uid[1];
    tx_buf[3] = uid[2];
    tx_buf[4] = uid[3];
    sta = data_tx_rx(5, TYPE_B, RATE_OFF);
    return sta;
}
/**
 ******************************************************************************
 ** \简  述  取消type B PICC选定
 **
 ** \参  数  u8* 参数首地址
 ** \返回值  状态
 ******************************************************************************/
sta_result_t piccDeselectB(u8 *param)
{
    sta_result_t sta;
    tx_buf[0] = 0xCA;
    tx_buf[1] = param[1];
    sta = data_tx_rx(2, TYPE_B, RATE_OFF);
    return sta;
}
 