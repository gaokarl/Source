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
#ifndef __TEST_H_
#define __TEST_H_
void PRINT(u8 data);
#define PUTS        UsbDeviceCdcCom_SendString //void PUTS(u8 *str);
void NEW_ROW();
void print_fifo(u16 num_a, u8 print_type);
uint8_t UsbDeviceCdcCom_ReceiveByte1(void); 
#endif