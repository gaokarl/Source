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
 ** 文 件 名： iso14443_3a.c
 **
 ** 文件简述： 符合ISO14443-3 type A的操作函数，常量和变量
 **
 ** 版本历史:
 ** 2015-10-05 V1.00  EH   第一个正式版本
 **
 ******************************************************************************/
#include "mcu.h"
#include "sky1311s.h"
#include "driver.h" 
#include "iso14443.h"

/**
 ******************************************************************************
 ** \简  述  type A "Request" 操作,command = 26H
 **
 ** \参  数  none
 ** \返回值  sta_result_t 操作状态，Ok：成功，其它：失败
 ******************************************************************************/
sta_result_t piccRequestA(void)
{
    sta_result_t sta;
    tx_buf[0] = REQA;       // 0x26
    sky1311WriteReg(ADDR_TX_CTRL, TX_POLE_HIGH|TX_PARITY_ODD);      // TX odd parity, no CRC
    sky1311WriteReg(ADDR_RX_CTRL, RX_PARITY_EN|RX_PARITY_ODD);      // RX odd parity, no CRC
    sky1311WriteReg(ADDR_TX_BIT_NUM, 0x07);
    sky1311WriteReg(ADDR_TX_BYTE_NUM, 0x01);
    sta = data_tx_rx(1, TYPE_A, RATE_OFF);
    return sta;
}

/**
 ******************************************************************************
 ** \简  述  type A "Wake-Up" 操作,command = 52H
 **
 ** \参  数  none
 ** \返回值  sta_result_t 操作状态，Ok：成功，其它：失败
 ******************************************************************************/
sta_result_t piccWakeupA(void)
{
    sta_result_t sta;
    tx_buf[0] = WUPA;       // 0x52
    sky1311WriteReg(ADDR_TX_CTRL, TX_POLE_HIGH|TX_PARITY_ODD);      // TX odd parity, no CRC
    sky1311WriteReg(ADDR_RX_CTRL, RX_PARITY_EN|RX_PARITY_ODD);      // RX odd parity, no CRC
    sky1311WriteReg(ADDR_TX_BIT_NUM, 0x07);
    sky1311WriteReg(ADDR_TX_BYTE_NUM, 0x01);
    sta = data_tx_rx(1, TYPE_A, RATE_OFF);
    return sta;
}
/**
 ******************************************************************************
 ** \简  述  type A卡防冲突循环
 **
 ** \参  数  u8 randBit 位冲突时选择的数，0或1; 
 **          u8* uid 获得的UID保存区域的首地址
 ** \返回值  防冲突返回状态值, 
 ******************************************************************************/
sta_result_t piccAntiA(u8 rand_bit, u8 *uid, u8* uidLen)
{
    u8 temp_len;
    u16 i;
    u8 temp_1;
    u8 uid_len;
    u8 irq_sta=0x00;

    u8 temp_byte;
    u8 temp_bit;
    u8 temp_cmd;
    u8 temp_tail;
    u8 temp_inter;
    u8 temp_cnt;
    u8 data_sum[10];
    u8 data_temp[10];

    u8 new_byte;
    u8 new_bit;
    u16 delayCount;
    
    /* config registor of sky1311s */    
    sky1311WriteReg(ADDR_RX_NUM_H, COLL_EN);        // anti-collision on
    sky1311WriteReg(ADDR_TX_CTRL, TX_POLE_HIGH|TX_PARITY_ODD);
    sky1311WriteReg(ADDR_TX_BYTE_NUM,0x02);
    sky1311WriteReg(ADDR_TX_BIT_NUM,0x08);
    sky1311WriteCmd(CMD_CLR_FF);                    
    
    irqClearAll();
    /* send 93H 20H to PICC */
    sky1311WriteReg(ADDR_FIFO,0x93);
    sky1311WriteReg(ADDR_FIFO,0x20);
    sky1311WriteCmd(CMD_TX_RX);                     
    
    delayCount = 0xFFFF;
    while(!IRQ_COME && delayCount--);              // waiting for TX STOP IRQ
    if(IRQ_COME==0)
        return NoResponse;
    irq_sta = sky1311ReadReg(ADDR_IRQ_STA);
    if(irq_sta & IRQ_TX)
        irqClear(IRQ_TX); //irqClearAll();
    else{
        irqClearAll();
        return NoResponse;
    }
    
    delayCount = 0xFFFF;                          // delay 500ms
    while(!IRQ_COME && delayCount--);           // waiting for TX STOP IRQ
    if(!IRQ_COME)
        return NoResponse;
    irq_sta = sky1311ReadReg(ADDR_IRQ_STA);
    if(irq_sta & IRQ_TOUT){
        sky1311WriteCmd(CMD_IDLE);
        irqClearAll();
        return NoResponse;
    }else if(!(irq_sta & IRQ_RX)){
        irqClearAll();
        return NoResponse;
    }
    irqClearAll();
    delay1ms(2);

    if((sky1311ReadReg(ADDR_RX_NUM_H) & RX_FORBID) == 0){   // no collision happens
        temp_len = sky1311ReadReg(ADDR_FIFO_LEN);           // get FIFO length
        sky1311ReadFifo(rx_buf, temp_len);                  // fetch FIFO data
        err_sta = sky1311ReadReg(ADDR_ERR_STA);             // get error status
        for(i=0; i<temp_len; i++){                          // copy UID from rx buffer
            uid[i] = rx_buf[i];         
        }
        *uidLen = temp_len;
        rx_len = temp_len;
        sky1311WriteReg(ADDR_RX_NUM_H, 0x00);               // disable anti-collision
        return Ok;
    }
    temp_byte = sky1311ReadReg(ADDR_RX_NUM_L);
    temp_bit = ((sky1311ReadReg(ADDR_RX_NUM_H) & 0xf0) >> 4);
    temp_len = sky1311ReadReg(ADDR_FIFO_LEN);
    for(i=0; i<temp_len; i++){ data_temp[i] = sky1311ReadReg(ADDR_FIFO); }
//----------------construct next data & command---------------------
NEXT_TX_CMD:
    if(temp_bit != 0){                                              // for rx_bit 1~7, byte_num not to add 1
        temp_tail   =   (data_temp[temp_len-1] >> (8 - temp_bit)) | (rand_bit << temp_bit);
        if(temp_bit != 7)   temp_cmd    =   ( (0x20 & 0xf0) + ((temp_len - 1) << 4) ) | ((0x20 & 0x0f) | (temp_bit + 1));
        else                temp_cmd    =   ( (0x20 & 0xf0) + (temp_len << 4) ) | (0x20 & 0x0f);

        sky1311WriteReg(ADDR_TX_BYTE_NUM, 0x02 + temp_len);
        sky1311WriteReg(ADDR_TX_BIT_NUM, temp_bit + 1);
        sky1311WriteReg(ADDR_FIFO, 0x93);
        sky1311WriteReg(ADDR_FIFO, temp_cmd);
        sky1311WriteFifo(data_temp, temp_len - 1);
        sky1311WriteReg(ADDR_FIFO, temp_tail);

    }else if(temp_bit == 0 && temp_byte == 0){
        temp_tail   =   rand_bit;
        temp_cmd    =   0x21;

        sky1311WriteReg(ADDR_TX_BYTE_NUM, 0x03);
        sky1311WriteReg(ADDR_TX_BIT_NUM, 0x01);
        sky1311WriteReg(ADDR_FIFO, 0x93);
        sky1311WriteReg(ADDR_FIFO, temp_cmd);
        sky1311WriteReg(ADDR_FIFO, temp_tail);
    }else if(temp_bit == 0 && temp_byte != 0){
        temp_tail   =   rand_bit;
        temp_cmd    =   ( (0x20 & 0xf0) + (temp_len << 4) ) | ( (0x20 & 0x0f) | 0x01 );

        sky1311WriteReg(ADDR_TX_BYTE_NUM, 0x02 + temp_len + 1);
        sky1311WriteReg(ADDR_TX_BIT_NUM, 0x01);
        sky1311WriteReg(ADDR_FIFO, 0x93);
        sky1311WriteReg(ADDR_FIFO, temp_cmd);
        sky1311WriteFifo(data_temp, temp_len);
        sky1311WriteReg(ADDR_FIFO, temp_tail);
    }
    sky1311WriteCmd(CMD_TX_RX);
    delayCount = 0xFFFF;
    while(!IRQ_COME && delayCount--);          // waiting for TX STOP IRQ
    if(!IRQ_COME)
        return NoResponse;
    irq_sta = sky1311ReadReg(ADDR_IRQ_STA);
    if(irq_sta & IRQ_TX)
        irqClearAll();
    else{
        irqClearAll();
        return NoResponse;
    }

   delayCount = 0xFFFF;                           // delay 500ms
    while(!IRQ_COME && delayCount--);          // waiting for TX STOP IRQ
    if(!IRQ_COME)
        return NoResponse;
    irq_sta = sky1311ReadReg(ADDR_IRQ_STA);
    if(irq_sta & IRQ_TOUT){
        sky1311WriteCmd(CMD_IDLE);
        irqClearAll();
        return NoResponse;
    }else if(!(irq_sta & IRQ_RX)){
        irqClearAll();
        return NoResponse;
    }

    irqClearAll();
    delay1ms(3);                                                   // delay for right data // change 2-->3 by wangxing

//---------construct the UID---------------
    if((sky1311ReadReg(ADDR_RX_NUM_H) & RX_FORBID) == 0){                // no collision happens
        temp_1 = sky1311ReadReg(ADDR_FIFO_LEN);
        for(i=0; i<temp_1; i++){ 
            data_sum[i] = sky1311ReadReg(ADDR_FIFO); 
        }

        if(temp_bit == 0 || temp_bit == 7)  
            uid_len = temp_len + temp_1;
        else if(temp_bit != 7)              
            uid_len = temp_len + temp_1 - 1;
        *uidLen = uid_len;
        
        if(temp_bit != 0){
            temp_inter = temp_tail | ( data_sum[0] & (0xff << (temp_bit + 1) ) );
            for(temp_cnt=0; temp_cnt<temp_len-1; temp_cnt++)
                uid[temp_cnt] = data_temp[temp_cnt];
            uid[temp_cnt]  = temp_inter;
            temp_cnt++;

            if(temp_bit != 7){
                for(i=0; temp_cnt<temp_len+temp_1-1; temp_cnt++, i++)
                    uid[temp_cnt] = data_sum[i+1];
            }else if(temp_bit == 7){
                for(i=0; temp_cnt<temp_len+temp_1; temp_cnt++, i++)
                    uid[temp_cnt] = data_sum[i];
            }
        }else if(temp_bit == 0 && temp_byte == 0){
            temp_inter = rand_bit | (data_sum[0] & 0xfe);
            temp_cnt = 0;
            uid[0] = temp_inter;
            temp_cnt++;

            for(i=1; temp_cnt<temp_1; temp_cnt++, i++)
                uid[temp_cnt] = data_sum[i];
        }else if(temp_bit == 0 && temp_byte != 0){
            temp_inter = temp_tail | (data_sum[0] & (0xff << (temp_bit + 1)));
            for(temp_cnt = 0; temp_cnt < temp_len; temp_cnt++)
                uid[temp_cnt]  =   data_temp[temp_cnt];
            uid[temp_cnt]  = temp_inter;
            temp_cnt++;

            for(i = 0; temp_cnt < temp_len + temp_1; temp_cnt++, i++)
                uid[temp_cnt]  =   data_sum[i+1];
        }

//        temp_len = uid_len;
        rx_len = uid_len;
        for(i=0; i<uid_len; i++){ 
            rx_buf[i] = uid[i]; 
        }
        err_sta = sky1311ReadReg(ADDR_ERR_STA);
        sky1311WriteReg(ADDR_RX_NUM_H, 0x00);                            // disable anti-collision
        return Ok; //temp_len;
    }
    else{                                                          //the loop of anti-collision
//-----------------------construct the new data-------------------------------------------------
        new_byte = sky1311ReadReg(ADDR_RX_NUM_L);
        if(new_byte == 0){
            if(temp_bit == 7)
                new_bit = ((sky1311ReadReg(ADDR_RX_NUM_H) & 0xf0) >> 4);
            else
                new_bit = ((sky1311ReadReg(ADDR_RX_NUM_H) & 0xf0) >> 4) - 1 - temp_bit;
        }else if(new_byte != 0){
            new_bit = ((sky1311ReadReg(ADDR_RX_NUM_H) & 0xf0) >> 4);
        }
        temp_1 = sky1311ReadReg(ADDR_FIFO_LEN);
        for(i=0;i<temp_1;i++){ data_sum[i] = sky1311ReadReg(ADDR_FIFO); }
//----------------------------------------------------------------------------------------
        if(temp_bit != 0){
            if(new_byte == 0){
                temp_inter = (temp_tail << (7-temp_bit-new_bit)) | (data_sum[0] & (0xff << (8-new_bit)));
            }else{
                temp_inter = temp_tail | (data_sum[0] & (0xff << (temp_bit + 1)));
            }
            //for(temp_cnt=0; temp_cnt<temp_len-1; temp_cnt++)
                //data_temp[temp_cnt] = data_temp[temp_cnt];
            //data_temp[temp_cnt]  = temp_inter;
            temp_cnt=temp_len-1;
            data_temp[temp_cnt] = temp_inter;
            temp_cnt++;

            if(temp_bit != 7){
                for(i=0; temp_cnt<temp_len+temp_1-1; temp_cnt++, i++)
                    data_temp[temp_cnt] = data_sum[i+1];
            }else if(temp_bit == 7){
                for(i=0; temp_cnt<temp_len+temp_1; temp_cnt++, i++)
                    data_temp[temp_cnt] = data_sum[i];
            }
        }else if(temp_bit == 0 && temp_byte == 0){
            if(new_byte == 0){
                temp_inter = (rand_bit << (7 - new_bit)) | (data_sum[0] & (0xff << (7 - new_bit)));
            }else{
                temp_inter = rand_bit | (data_sum[0] & 0xfe);
            }
            temp_cnt = 0;
            data_temp[0] = temp_inter;
            temp_cnt++;

            for(i=1; temp_cnt<temp_1; temp_cnt++, i++)
                data_temp[temp_cnt] = data_sum[i];
        }else if(temp_bit == 0 && temp_byte != 0){
            if(new_byte == 0){
               temp_inter = (temp_tail << (7 - new_bit)) | ( data_sum[0] & (0xff << (7 - new_bit)));
            }else{
               temp_inter = temp_tail | ( data_sum[0] & (0xff << (temp_bit+1)));
            }
            //for(temp_cnt=0; temp_cnt<temp_len; temp_cnt++)
            //    data_temp[temp_cnt] = data_temp[temp_cnt];
            data_temp[temp_cnt]  = temp_inter;
            temp_cnt++;

            for(i=0; temp_cnt<temp_len+temp_1; temp_cnt++, i++)
                data_temp[temp_cnt]  =   data_sum[i+1];
        }

        if (temp_bit == 0 || temp_bit == 7)
            temp_len = temp_len+temp_1;
        else if(temp_bit != 7)
            temp_len = temp_len+temp_1-1;

        if(new_byte == 0){
            if((temp_bit + new_bit) == 7){
                temp_byte = temp_byte+1;
                temp_bit = 0;
            }else{
                temp_byte = temp_byte+new_byte;
                temp_bit = temp_bit + new_bit + 1;
            }
        }else{
            temp_byte = temp_byte+new_byte;
            temp_bit = new_bit;
        }
        goto NEXT_TX_CMD;
    }
}
/**
 ******************************************************************************
 ** \简  述  type A选卡
 **
 ** \参  数  u8* uid 指定的UID
 ** \返回值  sta_result_t， 选卡状态值
 ******************************************************************************/
sta_result_t piccSelectA(u8 *uid)
{
    sta_result_t sta;
    sky1311WriteReg(ADDR_TX_CTRL, TX_CRC_EN|TX_PARITY_ODD|TX_POLE_HIGH); // TX odd parity, with CRC
    sky1311WriteReg(ADDR_RX_CTRL, RX_CRC_EN|RX_PARITY_EN|RX_PARITY_ODD);
    sky1311WriteReg(ADDR_TX_BIT_NUM, 0x08);                              // complete bytes

    tx_buf[0] = 0x93;   
    tx_buf[1] = 0x70;
    tx_buf[2] = uid[0]; 
    tx_buf[3] = uid[1]; 
    tx_buf[4] = uid[2];
    tx_buf[5] = uid[3]; 
    tx_buf[6] = uid[4];
    sta = data_tx_rx(7, TYPE_A, RATE_OFF);
    return sta;
}
/**
 ******************************************************************************
 ** \简  述  type A休眠,终止对卡的操作
 **
 ** \参  数  none
 ** \返回值  none
 ******************************************************************************/
void piccHaltA(void)
{
    sky1311WriteReg(ADDR_FIFO, 0x50);
    sky1311WriteReg(ADDR_FIFO, 0x00);
    sky1311WriteReg(ADDR_TX_BYTE_NUM, 0x02);
    sky1311WriteCmd(CMD_TX);                     // CMD TX
}
/*****************************************************************************/
/*                      以下是ISO14443-4内容                                 */
/*****************************************************************************/
/**
 ******************************************************************************
 ** \简  述  取消选卡
 **
 ** \参  数  卡类型
 ** \返回值  sta_result_t，状态
 ******************************************************************************/
sta_result_t piccDeselectA(u8 rate_type)
{
    sta_result_t sta;
    tx_buf[0] = 0xCA;
    tx_buf[1] = 0x00;

    sta = data_tx_rx(2, TYPE_A, rate_type);
    return sta;
}
/**
 ******************************************************************************
 ** \简  述  读取type A卡的ATS值
 **
 ** \参  数  none
 ** \返回值  sta_result_t，ATS值
 ******************************************************************************/
sta_result_t piccATS(void)
{
    sta_result_t sta;
    tx_buf[0] = 0xE0;
    tx_buf[1] = 0x80;
    sta = data_tx_rx(2, TYPE_A, RATE_OFF);
    return sta;
}

/**
 ******************************************************************************
 ** \简  述  读取PPS值
 **
 ** \参  数  设置的倍速
 ** \返回值  sta_result_t，PPS值
 ******************************************************************************/
sta_result_t piccPPS(u16 high_rate)
{
    sta_result_t sta;
    tx_buf[0] = 0xD0;
    tx_buf[1] = 0x11;
    if(high_rate == 212)        
        tx_buf[2] = 0x05;                   // RATE = 212k
    else if(high_rate == 424)   
        tx_buf[2] = 0x0A;                   // RATE = 424k
    sta = data_tx_rx(3, TYPE_A, RATE_OFF);
    return sta;
}
