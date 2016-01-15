/******************************************************************************/
/*               (C) ˹���������������Ƽ����޹�˾(SKYRELAY)                   */
/*                                                                            */
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
 ** �� �� ���� comm.c
 **
 ** �ļ������� �ʹ���ͨ���йصĺ����������ͱ���
 **
 ** �汾��ʷ:
 ** 2015-10-05 V1.00  EH   ��һ����ʽ�汾
 **
 ******************************************************************************/
#include "mcu.h"
/**
 ******************************************************************************
 ** \��  ��  ��ʼ��3,SIN3_1(P50), SOT3_1(P51).115200,8,N,1
 **
 ** \��  ��  none
 ** \����ֵ  none
 ******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
void initUart(void)
{
    /* SIN3_1 Pin */ 
    bFM0P_GPIO_PFR5_P0 = 1;
    FM0P_GPIO->EPFR07_f.SIN3S = 0x02;
    /* SOT3_1 Pin */
    bFM0P_GPIO_PFR5_P1 = 1;
    FM0P_GPIO->EPFR07_f.SOT3B = 0x02;
    
    FM0P_MFS3_UART->SCR  = 0x80;
    FM0P_MFS3_UART->SMR  = 0x01;
    FM0P_MFS3_UART->SSR  = 0x80;
    FM0P_MFS3_UART->ESCR = 0x00;
    FM0P_MFS3_UART->BGR  = 32000000 / 115200 - 1;  // 115.2kbaud @ 32 MHz
    FM0P_MFS3_UART->SCR  = 0x03;
}
/**
 ******************************************************************************
 ** \��  ��  UART ����һ���ֽ�����
 **
 ** \��  ��  Ҫ���͵�8λ��
 ** \����ֵ  none
 ******************************************************************************/
void uartPutChar(u8 data)
{
    while (bFM0P_MFS3_UART_SSR_TDRE == 0 );	// wait for transmit buffer empty
    FM0P_MFS3_UART->TDR = data;             // put ch into buffer
}
/**
 ******************************************************************************
 ** \��  ��  UART ����һ���ֽ�����
 **
 ** \��  ��  none
 ** \����ֵ  Ҫ���յ�8λ��
 ******************************************************************************/
u8 uartGetChar(void)
{
    if (bFM0P_MFS3_UART_SSR_RDRF == 0 ){
        return 0;
    }
    else if (FM0P_MFS3_UART->SSR & 0x38){ 
    	/* overrun or parity error */
        bFM0P_MFS3_UART_SSR_REC = 1;
        return 0;
    } 
    else{
        return (FM0P_MFS3_UART->RDR);
    }
}
/**
 ******************************************************************************
 ** \��  ��  ��ʼ��SPI, CS--P14, CLK--SCK1_1(P13), MOSI--SOT1_1(P12), MISO--SIN1_1(P11)
 **
 ** \��  ��  none
 ** \����ֵ  none
 ******************************************************************************/
void initSPI(void)
{
    /* SPI_CS Pin */
    bFM0P_GPIO_PDOR1_P4 = 1;   // CS = High
    bFM0P_GPIO_DDR1_P4 = 1;
    
    /* SCK1_1 Pin */
    bFM0P_GPIO_PFR1_P3 = 1;
    FM0P_GPIO->EPFR07_f.SCK1B = 0x02;
    /* SIN1_1 Pin */ 
    bFM0P_GPIO_PFR1_P1 = 1;
    FM0P_GPIO->EPFR07_f.SIN1S = 0x02;
    /* SOT1_1 Pin */
    bFM0P_GPIO_PFR1_P2 = 1;
    FM0P_GPIO->EPFR07_f.SOT1B = 0x02;
    
    FM0P_MFS1_CSIO->SCR  = 0x80;
    FM0P_MFS1_CSIO->SMR  = 0x4F;
    FM0P_MFS1_CSIO->SSR  = 0x80;
    FM0P_MFS1_CSIO->ESCR = 0x00;
    FM0P_MFS1_CSIO->BGR  = 32000000 / 4000000 - 1;  // 4m baud @ 32 MHz
    FM0P_MFS1_CSIO->SCR  = 0x23;
}
/**
 ******************************************************************************
 ** \��  ��  SPI�ӿ����һ���ֽڵ�����
 **
 ** \��  ��  Ҫ������ֽ�
 ** \����ֵ  none
 ******************************************************************************/
void SpiWriteByte(u8 data)
{
    while (bFM0P_MFS1_CSIO_SSR_TDRE == 0 );	// wait for transmit buffer empty
    FM0P_MFS1_CSIO->TDR = data;             // put data into buffer
    //while(bFM0P_MFS1_CSIO_SSR_TDRE == 0);  // wait for start of transmission (or ongoing)
    __no_operation();__no_operation();
    __no_operation();__no_operation();
    __no_operation();__no_operation();
    while(bFM0P_MFS1_CSIO_SSR_TBI == 0);   // wait for transmit finished
}
/**
 ******************************************************************************
 ** \��  ��  SPI�ӿ�����һ���ֽڵ�����
 **
 ** \��  ��  none
 ** \����ֵ  ������ֽ�����
 ******************************************************************************/
u8 SpiReadByte(void)
{
    u8 res;
    res = FM0P_MFS1_CSIO->RDR;            // flush reception register
    bFM0P_MFS1_CSIO_SSR_REC = 1;            // Clear possible errors (REC)
    while (bFM0P_MFS1_CSIO_SSR_TDRE == 0 );	// wait for transmit buffer empty
    FM0P_MFS1_CSIO->TDR = 0xFF;             // put 0xFF into buffer
    while(bFM0P_MFS1_CSIO_SSR_RDRF == 0);   // wait for RDRF
    res = FM0P_MFS1_CSIO->RDR;            // read data buffer
    return res;
}

 