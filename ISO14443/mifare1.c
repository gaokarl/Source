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
 ** �� �� ���� mifare1.c
 **
 ** �ļ������� ����Mifare1�����������к����������ͱ���
 **
 ** �汾��ʷ:
 ** 2015-10-05 V1.00  EH   ��һ����ʽ�汾
 **
 ******************************************************************************/
#include "mcu.h"
#include "sky1311s.h"
#include "driver.h" 
#include "iso14443.h"

/**
 ******************************************************************************
 ** \��  ��  ����M1��ʱ�򿪽��յ�CRCУ��(sky1311s)
 **
 ** \��  ��  none
 ** \����ֵ  none
 ******************************************************************************/
void sky1311RxCrcOn(void)
{
    sky1311WriteReg(ADDR_RX_CTRL, RX_MIFARE_ON|RX_CRC_EN|RX_PARITY_EN|RX_PARITY_ODD);
}
/**
 ******************************************************************************
 ** \��  ��  ����M1��ʱ�رս��յ�CRCУ��(sky1311s)
 **
 ** \��  ��  none
 ** \����ֵ  none
 ******************************************************************************/
void sky1311RxCrcOff()
{
    sky1311WriteReg(ADDR_RX_CTRL, RX_MIFARE_ON|RX_PARITY_EN|RX_PARITY_ODD);
}
/**
 ******************************************************************************
 ** \��  ��  ����M1��ʱ�򿪷��͵�CRCУ��(sky1311s)
 **
 ** \��  ��  none
 ** \����ֵ  none
 ******************************************************************************/
void sky1311TxCrcOn(void)
{
    sky1311WriteReg(ADDR_TX_CTRL, TX_CRC_EN|TX_PARITY_ODD|TX_POLE_HIGH);
}
/**
 ******************************************************************************
 ** \��  ��  ����M1��ʱ�رշ��͵�CRCУ��(sky1311s)
 **
 ** \��  ��  none
 ** \����ֵ  none
 ******************************************************************************/
void sky1311TxCrcOff(void)
{
    sky1311WriteReg(ADDR_TX_CTRL, (!TX_CRC_EN)|TX_PARITY_ODD|TX_POLE_HIGH);
}
/**
 ******************************************************************************
 ** \��  ��  M1����3�μ�����֤
 **
 ** \��  ��  u8 blockAddr Ҫ������������ַ��0--63��
 **          u8* key ��Կ��key A��
             u8* UID ����UID��
             u32 RA �����
 ** \����ֵ  ��֤״̬
 ******************************************************************************/
sta_result_t M1_3Pass(u8 blockAddr, u8 *m1Key, u8 *uid, u32 RA)
{
    sta_result_t sta;
    volatile u16 delayCount;
    sky1311TxCrcOn(); 
    sky1311RxCrcOff();
    // load M1_KEY ( Ӧ�ð������Ķ��ķ��ֽ�˳��д��)
    sky1311WriteReg(ADDR_M1_KEY, m1Key[0]);
    sky1311WriteReg(ADDR_M1_KEY, m1Key[1]);
    sky1311WriteReg(ADDR_M1_KEY, m1Key[2]);
    sky1311WriteReg(ADDR_M1_KEY, m1Key[3]);
    sky1311WriteReg(ADDR_M1_KEY, m1Key[4]);
    sky1311WriteReg(ADDR_M1_KEY, m1Key[5]);

   // load UID �� �Ѿ����ֽ�д�룬UID����Ҫ�ٷ���
    sky1311WriteReg(ADDR_M1_ID, uid[3]);
    sky1311WriteReg(ADDR_M1_ID, uid[2]);
    sky1311WriteReg(ADDR_M1_ID, uid[1]);
    sky1311WriteReg(ADDR_M1_ID, uid[0]);
    
    sky1311WriteReg(ADDR_M1_CTRL, 0x53);                                 // pass1
    tx_buf[0] = M1_AUTH;                    // ��֤ KEY A
    tx_buf[1] = blockAddr;
    sta = data_tx_rx(2, TYPE_A, RATE_OFF);
    if(sta == NoResponse)   
      return NoResponse;

    sky1311WriteReg(ADDR_M1_CTRL, 0x05);                                 // pass2
    sky1311TxCrcOff();
    sky1311RxCrcOff();
    tx_buf[0] = (u8)(RA >> 24);
    tx_buf[1] = (u8)(RA >> 16);
    tx_buf[2] = (u8)(RA >> 8);
    tx_buf[3] = (u8)(RA >> 0);
   
    delayCount = 0xFFFF;
    while(((sky1311ReadReg(ADDR_M1_SUC_STATE)&0x01)==0) && delayCount--);    // waiting for SUC64 ready
    
    tx_buf[4] = sky1311ReadReg(ADDR_M1_SUC64_3);
    tx_buf[5] = sky1311ReadReg(ADDR_M1_SUC64_2);
    tx_buf[6] = sky1311ReadReg(ADDR_M1_SUC64_1);
    tx_buf[7] = sky1311ReadReg(ADDR_M1_SUC64_0);
    tx_func(8, TYPE_A);
    sky1311WriteReg(ADDR_M1_CTRL, 0x09);                                 // pass3
    sta = rx_func(TYPE_A, RATE_OFF);
    if(sta == NoResponse)
        return NoResponse;

//  Token BA authentication
    if(sky1311ReadReg(ADDR_M1_SUC96_3) == rx_buf[0] &&
       sky1311ReadReg(ADDR_M1_SUC96_2) == rx_buf[1] &&
       sky1311ReadReg(ADDR_M1_SUC96_1) == rx_buf[2] &&
       sky1311ReadReg(ADDR_M1_SUC96_0) == rx_buf[3]
    )       
        return Ok;
    else    
        return NoResponse;
}
/**
 ******************************************************************************
 ** \��  ��  M1���޸���������
 **
 ** \��  ��  u8 blockAddr Ҫ�������������ַ��0--63��
 **          u8* key_a ��ԿA
             u8* key_b ����B
 ** \����ֵ  ����״̬
 ******************************************************************************/
sta_result_t M1_PwdChange(u8 blockAddr, u8 *key_a, u8 *key_b)
{
    sta_result_t sta;
    sky1311TxCrcOn();             
    sky1311RxCrcOff();
    tx_buf[0] = M1_WRITE;       
    tx_buf[1] = blockAddr;
    sta = data_tx_rx(2, TYPE_A, RATE_OFF);
    if(sta == NoResponse)           
        return NoResponse;
    if(rx_buf[0] != M1_ACK)     
        return M1Error;                    // check ACK

    tx_buf[0]    =   key_a[0];
    tx_buf[1]    =   key_a[1];
    tx_buf[2]    =   key_a[2];
    tx_buf[3]    =   key_a[3];
    tx_buf[4]    =   key_a[4];
    tx_buf[5]    =   key_a[5];
    tx_buf[6]    =   0xff;
    tx_buf[7]    =   0x07;
    tx_buf[8]    =   0x80;
    tx_buf[9]    =   0x69;          // ffH 07H 80H 69H ��Ĭ�ϵĿ�����
    tx_buf[10]   =   key_b[0];
    tx_buf[11]   =   key_b[1];
    tx_buf[12]   =   key_b[2];
    tx_buf[13]   =   key_b[3];
    tx_buf[14]   =   key_b[4];
    tx_buf[15]   =   key_b[5];
    sta = data_tx_rx(16, TYPE_A, RATE_OFF);

    if(sta == NoResponse)           
        return NoResponse;
    if(rx_buf[0] != M1_ACK)     
        return M1Error;                    // check ACK
    return sta;
}
/**
 ******************************************************************************
 ** \��  ��  M1��д��������
 **
 ** \��  ��  u8 blockAddr Ҫ�������������ַ��0--63��
             u32 Ҫд�������
 ** \����ֵ  ����״̬
 ******************************************************************************/
sta_result_t M1_Write(u8 blockAddr, u32 value)
{
    sta_result_t sta;
    sky1311TxCrcOn();             sky1311RxCrcOff();
    tx_buf[0] = M1_WRITE;       tx_buf[1] = blockAddr;
    sta = data_tx_rx(2, TYPE_A, RATE_OFF);
    if(sta == NoResponse)           
        return NoResponse;
    if(rx_buf[0] != M1_ACK)     
        return M1Error;

    tx_buf[0]    =   (u8)value;
    tx_buf[1]    =   (u8)(value>>8);
    tx_buf[2]    =   (u8)(value>>16);
    tx_buf[3]    =   (u8)(value>>24);
    tx_buf[4]    =  ~tx_buf[0];
    tx_buf[5]    =  ~tx_buf[1];
    tx_buf[6]    =  ~tx_buf[2];
    tx_buf[7]    =  ~tx_buf[3];
    tx_buf[8]    =   tx_buf[0];
    tx_buf[9]    =   tx_buf[1];
    tx_buf[10]   =   tx_buf[2];
    tx_buf[11]   =   tx_buf[3];
    tx_buf[12]   =   blockAddr;
    tx_buf[13]   =  ~blockAddr;
    tx_buf[14]   =   blockAddr;
    tx_buf[15]   =  ~blockAddr;
    sta = data_tx_rx(16, TYPE_A, RATE_OFF);
    if(sta == NoResponse)           return NoResponse;
    if(rx_buf[0] != M1_ACK)     return M1Error;
    return sta;
}
/**
 ******************************************************************************
 ** \��  ��  M1�����������ݣ���ȡ�����ݱ�����ȫ�ֱ��� rx_buf��
 **
 ** \��  ��  u8 blockAddr Ҫ�������������ַ��0--63��
 ** \����ֵ  ����״̬
 ******************************************************************************/
sta_result_t M1_Read(u8 blockAddr)
{
    sta_result_t sta;
    sky1311TxCrcOn();             
    sky1311RxCrcOn();
    tx_buf[0] = M1_READ;        
    tx_buf[1] = blockAddr;
    sta = data_tx_rx(2, TYPE_A, RATE_OFF);
    return sta;
}
/**
 ******************************************************************************
 ** \��  ��  M1���������ݼ�ֵ
 **
 ** \��  ��  u8 blockAddr Ҫ�������������ַ��0--63��
             u32 Ҫ���ӵ���
 ** \����ֵ  ����״̬
 ******************************************************************************/
sta_result_t M1_Increment(u8 blockAddr, u32 value)
{
    sta_result_t sta;
    sky1311TxCrcOn();              
    sky1311RxCrcOff();
    tx_buf[0] = M1_INCREMENT;    
    tx_buf[1] = blockAddr;
    sta = data_tx_rx(2, TYPE_A, RATE_OFF);
    if(sta == NoResponse)           
        return NoResponse;
    if(rx_buf[0] != M1_ACK)     
        return M1Error;
    tx_buf[0] = (u8)value;
    tx_buf[1] = (u8)(value>>8);
    tx_buf[2] = (u8)(value>>16);
    tx_buf[3] = (u8)(value>>24);
    sta = data_tx_rx(4, TYPE_A, RATE_OFF);                          // no answer

    delay1ms(1);
    sta = M1_Transfer(blockAddr);
    return sta;
}
/**
 ******************************************************************************
 ** \��  ��  M1���������ݼ�ֵ
 **
 ** \��  ��  u8 blockAddr Ҫ�������������ַ��0--63��
             u32 Ҫ���ٵ���
 ** \����ֵ  ����״̬
 ******************************************************************************/
sta_result_t M1_Decrement(u8 blockAddr, u32 value)
{
    sta_result_t sta;
    sky1311TxCrcOn();             
    sky1311RxCrcOff();
    tx_buf[0] = M1_DECREMENT;   
    tx_buf[1] = blockAddr;
    sta = data_tx_rx(2, TYPE_A, RATE_OFF);
    if(sta == NoResponse)           
        return NoResponse;
    if(rx_buf[0] != M1_ACK)     
        return M1Error;

    tx_buf[0] = (u8)value;
    tx_buf[1] = (u8)(value>>8);
    tx_buf[2] = (u8)(value>>16);
    tx_buf[3] = (u8)(value>>24);
    sta = data_tx_rx(4, TYPE_A, RATE_OFF);                          // no answer
    delay1ms(0x1);
    sta = M1_Transfer(blockAddr);
    return sta;
}
/**
 ******************************************************************************
 ** \��  ��  M1������д�뵽����EERPOM
 **
 ** \��  ��  u8 blockAddr Ҫ�������������ַ��0--63��
 ** \����ֵ  ����״̬
 ******************************************************************************/
sta_result_t M1_Transfer(u8 blockAddr)
{
    sta_result_t sta;
    sky1311TxCrcOn();             
    sky1311RxCrcOff();
    tx_buf[0] = M1_TRANSFER;    
    tx_buf[1] = blockAddr;
    sta = data_tx_rx(2, TYPE_A, RATE_OFF);

    if(sta == NoResponse)           
        return NoResponse;
    if(rx_buf[0] != M1_ACK)     
        return M1Error;
    return sta;
}
/**
 ******************************************************************************
 ** \��  ��  M1�����ݻָ�
 **
 ** \��  ��  u8 blockAddr Ҫ�������������ַ��0--63��
 ** \����ֵ  ����״̬
 ******************************************************************************/
sta_result_t M1_Restore(u8 blockAddr)
{
    sta_result_t sta;
    sky1311TxCrcOn();             
    sky1311RxCrcOff();
    tx_buf[0] = M1_RESTORE;     
    tx_buf[1] = blockAddr;
    sta = data_tx_rx(2, TYPE_A, RATE_OFF);

    if(sta == NoResponse)           
        return NoResponse;
    if(rx_buf[0] != M1_ACK)     
        return M1Error;
    return sta;
}
 