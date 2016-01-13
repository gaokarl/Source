/******************************************************************************/
/*               (C) 斯凯瑞利（北京）科技有限公司(SKYRELAY)                   */
/*                                                                            */
/* 此代码是斯凯瑞利（北京）科技有限公司为支持客户编写的示例程序的一部分       */
/* 所有使用斯凯瑞利相关芯片的用户可以无偿使用该代码，但需保留本声明部分       */
/* 本代码仅为演示使用，不保证符合特定的规范要求，当有特殊规范或安全性需求时， */
/* 使用者需自行确认代码是否符合要求，并可以做出相应的修改。                   */
/* (V1.00)                                                                    */
/******************************************************************************/
/**
 ******************************************************************************
 ** 文 件 名： test.c
 **
 ** 文件简述： 测试用函数和变量，主要是使用串口打印调试信息
 **
 ** 版本历史:
 ** 2015-10-05 V1.00  EH   第一个正式版本
 **
 ******************************************************************************/
#include "mcu.h"
#include "sky1311s.h"
#include "driver.h"
#include "UsbDeviceCdcCom.h"
#include "userdef.h"
//#define UART_TX     uartPutChar
//#define UART_TX     UsbDeviceCdcCom_SendByte
//#define PUTS        UsbDeviceCdcCom_SendString
#define SPI_read1   sky1311ReadReg

void UART_TX(u8 ch)
{
    if(UsbDeviceCdcCom_IsConnected()){
        putch(ch);
    }
}

void PRINT(u8 data)
{
    int k=1;
    u8 tmp;
    
    if(UsbDeviceCdcCom_IsConnected()){
        
        for(tmp=0; k>=0; k--){
          tmp = (data >> (4*k)) & 0x0f;
          if(tmp <= 9)           tmp = tmp + '0';
          else  if(tmp >= 10)    tmp = (tmp-10) + 'A';
          putch(tmp);    //UART_TX(tmp);
        }
        putch(' '); //UART_TX(' ');
    }
}
//------------------------------------------------------------------------------

void PUTS(char_t *str)
{
    /*int k=0;
    while(str[k] != '\0'){
      UART_TX(str[k]);
      k++;
    }*/
    if(UsbDeviceCdcCom_IsConnected()){
        puts(str);
    }
}

//------------------------------------------------------------------------------
void NEW_ROW()
{
    if(UsbDeviceCdcCom_IsConnected()){
        putch('\r');
        putch('\n');
    }
    /*UART_TX('\r'); UART_TX('\n');*/
}
//------------------------------------------------------------------------------
void print_fifo(u16 num_a, u8 print_type)
{
#if FIFO_TEST == 1
    u8 bit_n;
    if(UsbDeviceCdcCom_IsConnected()){
        putch('\t');//PUTS("\t");
        PRINT( (num_a&0xff00)>>8 ); PRINT(num_a&0x00ff);                // print byte num
        putch('\t');

        if(print_type == TYPE_A){
            bit_n = ((SPI_read1(ADDR_RX_NUM_H)) & 0xf0)>>4;             // print bit num
            PRINT(bit_n); 
        }

        err_sta = SPI_read1(ADDR_ERR_STA);
        putch('\t'); puts("ERR_STA:"); //PUTS("\t");
        PRINT(err_sta);                                                 // print the error status

        if((err_sta & 0xC0) == 0x80){       // receive Parity error
            puts("\tPARITY ERROR");
        }
        else if((err_sta & 0xC0) == 0x40){  // receive CRC error
            puts("\tCRC ERROR");
            delay100ms(20);
        }
        else if((err_sta & 0xC0) == 0xC0){  // receive CRC & Parity error
            puts("\tCRC & PARITY ERROR");
        }
        putch('\t'); //NEW_ROW();

        for(u16 i=0; i<num_a; i++){
            PRINT(rx_buf[i]);               // print the data
        }
        NEW_ROW();
    }
#endif
}
 