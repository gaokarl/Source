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
 ** �� �� ���� sky1311s.c
 **
 ** �ļ������� ����sky1311s�����к����������ͱ���
 **
 ** �汾��ʷ:
 ** 2015-10-05 V1.00  EH   ��һ����ʽ�汾
 **
 ******************************************************************************/
#include "mcu.h"
#include "sky1311reg.h"
#include "driver.h"
#include "userdef.h"
#include "test.h"
#include "UsbDeviceCdcCom.h"
/**
 ******************************************************************************
 ** \������ ȫ�ֱ������� 
 **
 ******************************************************************************/ 
//sta_field_t pcdFieldState=NoObjectIn;      // ��������״̬���Ƿ��������ڳ��ڣ�
#if CHECKCARD==1
    u8 maxRCADVal=MAXADVAL;         // ��⵽�����ADֵ
    boolean freqScanNeed=true;  // ϵͳ��ҪRCƵ��ɨ��
#endif

u8  tx_buf[255];                // FIFO���ݷ��ͻ�����������ʵ�����������С
u8  rx_buf[255];                // FIFO���ݽ��ջ�����������ʵ�����������С
u16 rx_len=0;                   // FIFO�������ݳ���

u8  err_sta;
u8  irq_sta;
u8  irq_sta_n;
u8  irq_sta_h;
u8  pcb_p=0x03;

/**
 ******************************************************************************
 ** \��  ��  ���üĴ�������λ
 **
 ** \��  ��  u8 regAddr �Ĵ�����ַ�� u8 mask Ҫ���õ�"����"�ֽ�
 ** \����ֵ  none
 ******************************************************************************/ 
void SetBitMask(u8 regAddr, u8 mask)
{
    u8 tmp;
    tmp = sky1311ReadReg(regAddr);
    sky1311WriteReg(regAddr, tmp|mask);
}
/**
 ******************************************************************************
 ** \��  ��  ����Ĵ�������λ
 **
 ** \��  ��  u8 regAddr �Ĵ�����ַ�� u8 mask Ҫ�����"����"�ֽ�
 ** \����ֵ  none
 ******************************************************************************/ 
void ClearBitMask(u8 regAddr, u8 mask)
{
    u8 tmp;
    tmp = sky1311ReadReg(regAddr);
    sky1311WriteReg(regAddr, tmp & ~mask);
}
/**
 ******************************************************************************
 ** \��  ��  ģ�������ʼ��������ģ������Ĵ��� 
 **
 ** \��  ��  none
 ** \����ֵ  none
 ******************************************************************************/ 
void analogInit(void)
{
    //sky1311WriteReg(ADDR_ANA_CFG0, 0x40);     // PA=2.3V, external OSC, 13.56MHz
    sky1311WriteReg(ADDR_ANA_CFG0, 0x88);       // PA=2.5V, external OSC, 13.56MHz
    //sky1311WriteReg(ADDR_ANA_CFG0, 0x90);     // PA=2.8V, external OSC, 13.56MHz
    //sky1311WriteReg(ADDR_ANA_CFG0, 0x98);    // PA=3.0V, external OSC, 13.56MHz
    sky1311WriteReg(ADDR_ANA_CFG1, 0xE1);
    sky1311WriteReg(ADDR_ANA_CFG2, 0x78);
    sky1311WriteReg(ADDR_ANA_CFG3, 0x81);
}
/**
 ******************************************************************************
 ** \��  ��  ģ�������ʼ��������ģ������Ĵ��� 
 **
 ** \��  ��  ѡ��ͨ��MFOUT��������źţ�
 **          bit2,1,0
 **             0 0 0 rx phase signal after demod, before sample
 **             0 0 1 tx bit stream
 **             0 1 0 rx bit stream
 **             0 1 1 rx data from analog
 **             1 0 0 DO from digital
 ** \����ֵ  none
 ******************************************************************************/ 
void directModeInit(u8 mfout_sel)
{
    sky1311WriteReg(ADDR_MFOUT_SEL,  mfout_sel);
}

/**
 ******************************************************************************
 ** \��  ��  ��PCD���� 
 **
 ** \��  ��  PCD���ͣ� TYPE_A, TYPE_B
 ** \����ֵ  none
 ******************************************************************************/
void pcdAntennaOn(u8 pcdType)
{
    if(pcdType == TYPE_A){
        sky1311WriteReg(ADDR_ANA_CFG2, ANA2_A);
    }else if(pcdType == TYPE_B){
        sky1311WriteReg(ADDR_ANA_CFG2, ANA2_B);
    }
    SetBitMask(ADDR_TX_CTRL, TX_POLE);
}

/**
 ******************************************************************************
 ** \��  ��  �ر�PCD���� 
 **
 ** \��  ��  none
 ** \����ֵ  none
 ******************************************************************************/
void pcdAntennaOff(void)
{
    sky1311WriteReg(ADDR_ANA_CFG2, ANA2_A);
    ClearBitMask(ADDR_TX_CTRL, TX_POLE);
}
/**
 ******************************************************************************
 ** \��  ��  ��������жϱ��
 **
 ** \��  ��  none
 ** \����ֵ  none
 ******************************************************************************/
void irqClearAll(void)
{
    //sky1311WriteReg(ADDR_IRQ_STA, IRQ_TOUT|IRQ_TX|IRQ_RX|IRQ_HIGH|IRQ_LOW|IRQ_OSC|IRQ_ERR);
    sky1311WriteReg(ADDR_IRQ_STA, 0x7F);
}
/**
 ******************************************************************************
 ** \��  ��  ���ָ�����жϱ��
 **
 ** \��  ��  u8 irq
 ** \����ֵ  none
 ******************************************************************************/
void irqClear(u8 irq)
{
    sky1311WriteReg(ADDR_IRQ_STA,irq);
}
/**
 ******************************************************************************
 ** \��  ��  ����sky1311�뿨ͨ�ŵı���, ISO14443-4���ֹ涨���౶�ٵ�PICC
 **
 ** \��  ��  ���õı��٣�212��424
 ** \����ֵ  none
 ******************************************************************************/
void bitRateSet(u16 high_rate)
{
    sky1311WriteReg(ADDR_FSM_STATE, RATE_SEL);                           // enable high-speed mode

    if(high_rate == 212){
        sky1311WriteReg(ADDR_TX_CTRL, TX_212|TX_POLE_HIGH|TX_CRC_EN|TX_PARITY_ODD);
        sky1311WriteReg(ADDR_RATE_CTRL, RX_RATE_WID_2|RX_RATE_212|RX_RATE_PAR_ODD|RX_RATE_CRC_EN|RX_RATE_PAR_EN);
        sky1311WriteReg(ADDR_TX_PUL_WID,0x14);                           // tx pulse width = 1.5u
        sky1311WriteReg(ADDR_RATE_FRAME_END,0x40);                       // stop counter = 0x40
        sky1311WriteReg(ADDR_RATE_THRES,0x20);                           // threshold = 0x20
    }else if(high_rate == 424){
        sky1311WriteReg(ADDR_TX_CTRL, TX_424|TX_POLE_HIGH|TX_CRC_EN|TX_PARITY_ODD);
        sky1311WriteReg(ADDR_RATE_CTRL, RX_RATE_WID_2|RX_RATE_424|RX_RATE_PAR_ODD|RX_RATE_CRC_EN|RX_RATE_PAR_EN);
        sky1311WriteReg(ADDR_TX_PUL_WID,0x0A);                           // tx pulse width = 0.8u
        sky1311WriteReg(ADDR_RATE_FRAME_END,0x20);                       // stop counter = 0x20
        sky1311WriteReg(ADDR_RATE_THRES,0x10);                           // threshold = 0x10
    }
}
/**
 ******************************************************************************
 ** \��  ��  ѡ��type A ����Ϊ��������
 **
 ** \��  ��  none
 ** \����ֵ  none
 ******************************************************************************/
void typeAOperate(void)
{
    sky1311WriteReg(ADDR_ANA_CFG2, ANA2_A);             // analogA select
    sky1311WriteReg(ADDR_FSM_STATE, TYPE_A_SEL);        // typeA select
    sky1311WriteReg(ADDR_TX_PUL_WID,0x26);              // set to default value
    sky1311WriteReg(ADDR_CRC_CTRL, CRC_A);              // crcA enable
    sky1311WriteReg(ADDR_M1_CTRL, 0x00);                // disable M1 operation
} 
/**
 ******************************************************************************
 ** \��  ��  ѡ��type B ����Ϊ��������
 **
 ** \��  ��  none
 ** \����ֵ  none
 ******************************************************************************/
void typeBOperate(void)
{
    sky1311WriteReg(ADDR_ANA_CFG2, ANA2_B);            // analogB select
    sky1311WriteReg(ADDR_FSM_STATE, TYPE_B_SEL);       // typeB select
    sky1311WriteReg(ADDR_CRC_CTRL, CRC_B);             // crcB enable
} 
/**
 ******************************************************************************
 ** \��  ��  ��ʼ��SKY1311S
 **
 ** \��  ��  none
 ** \����ֵ  none
 ******************************************************************************/
void initSky1311s(void)
{
    analogInit();
    directModeInit(MFOUT_ANALOG_RX);
    sky1311WriteReg(ADDR_TIME_OUT2, 0x8F);       // time_out timer stop condition = beginning of RX SOF
    sky1311WriteReg(ADDR_TIME_OUT1, 0xFF);
    sky1311WriteReg(ADDR_TIME_OUT0, 0xFF);
    //sky1311WriteReg(ADDR_RX_PUL_DETA, 0x44);
    sky1311WriteReg(ADDR_RX_PUL_DETA, 0x64);     // ��4λʶ������˹��������Խ���ݴ�����Խǿ
 //   sky1311WriteReg(ADDR_RX_PRE_PROC, 0x26);
    sky1311WriteReg(ADDR_IRQ_EN, IRQ_TOUT_EN|IRQ_TX_EN|IRQ_RX_EN|IRQ_HIGH_EN|IRQ_LOW_EN);    // enable all IRQ
}

/**
 ******************************************************************************
 ** \��  ��  ���ݷ��亯����������д��FIFO�в�ͨ����Ƶ�ӿڷ��͸�PICC
 **          ���͵����ݱ�����ȫ�ֱ���tx_buf�У����ͻ�����
 **          
 ** \��  ��  u16 txLen ���ݳ���, u8 txType �������ͣ�TYPE_A | TYPE_B��
 ** \����ֵ  none
 ******************************************************************************/
void tx_func(u16 txLen, u8 txType)
{
    u16 tx_res = txLen;
    u8  irq_sta=0x00;
    volatile u16 delayCount;
    //boolean exitFlag = false;
    sky1311WriteCmd(CMD_IDLE);              // reset state machine to Idle mode
    sky1311WriteCmd(CMD_CLR_FF);            // clear FIFO
    irqClearAll();                          // clear all IRQ state
    //sky1311WriteReg(ADDR_FIFO_CTRL,8);      // set water-level of FIFO
    /* write numbers */
    if(txType == TYPE_A){
        sky1311WriteReg(ADDR_TX_BYTE_NUM, txLen&0x00ff);
        if(txLen>255)
            sky1311WriteReg(ADDR_TX_BIT_NUM, ((txLen&0x0f00)>>4|8));// write the length to tx_byte register
    }else if(txType == TYPE_B){
        sky1311WriteReg(ADDR_TX_B_BYTE_NUM, txLen&0x00ff);
        if(txLen>255)
            sky1311WriteReg(ADDR_TX_B_EGT_NUM, (txLen&0x0300)>>4);
    }
    /* when TX length<=FIFO's depth, write all data to FIFO */
    if(txLen <= 64){
        sky1311WriteFifo(tx_buf, txLen);
        sky1311WriteCmd(CMD_TX_RX);             // transceive & into receive mode
    }
    /* when TX length > FIFO's depth */
    else{               // txLen > 64
        sky1311WriteFifo(tx_buf, 64);       // send 64 bytes
        sky1311WriteCmd(CMD_TX_RX);
        tx_res = txLen - 64;
        while(tx_res>0){                    // send remain bytes
            if(IRQ_COME){
                irq_sta = sky1311ReadReg(ADDR_IRQ_STA);    
                if(irq_sta & IRQ_LOW){  // FIFO low
                    if(tx_res>=56){
                        sky1311WriteFifo(&tx_buf[txLen - tx_res], 56);  
                        tx_res -=56;
                    }
                    else{
                        sky1311WriteFifo(&tx_buf[txLen - tx_res], tx_res);
                        tx_res = 0;
                    }
                    irqClearAll();
                }
                else        // error
                    return;
            }    
        }
    }
    /* wait TX finished */
    while(1){           
        delayCount = 0xFFFF;                  // delay 500ms
        while(!IRQ_COME && delayCount--);   // waiting for TX STOP IRQ
        if(!IRQ_COME)
            return;
        irq_sta = sky1311ReadReg(ADDR_IRQ_STA);
        if(irq_sta & IRQ_TX){
            irqClear(IRQ_TX);//irqClearAll();
            return;
        }
        else{
            irqClearAll();
        }
    }
}
/**
 ******************************************************************************
 ** \��  ��  ���ݽ��պ�������sky1311s��FIFO�ж�ȡ����PICC������
 **          ���յ������ݱ����ڽ��ջ������У�ȫ�ֱ���rx_buf
 **          ���յ������ݳ��ȱ�����ȫ�ֱ���rx_len��
 **
 ** \��  ��  u8 rxType ���յ�PICC���ͣ�TYPE_A | TYPE_B��
 **          u8 rate_type �Ƿ�ʹ�ö౶��
 ** \����ֵ  Ok:��ȷ���յ����ݣ�������û�յ�����
 ******************************************************************************/
sta_result_t rx_func(u8 rxType, u8 rate_type)
{
    u16 rx_buf_cnt=0;
    u8  byte_num_H;
    u8  byte_num_L;
    u8  temp_len;
    u8  bit_n=0;
    u8  irq_sta=0x00;
    volatile u16 delayCount;

    while(1){
        delayCount=0xFFFF;                        // delay 
        while(!IRQ_COME && delayCount--);       // waiting for TX STOP IRQ
        if(!IRQ_COME)
            return NoResponse;
            
        irq_sta = sky1311ReadReg(ADDR_IRQ_STA);// PRINT(irq_sta);
        if(irq_sta & IRQ_TOUT){             // tiemout
            sky1311WriteCmd(CMD_IDLE);
            irqClearAll();
            return NoResponse;
        }
        else if(irq_sta & IRQ_HIGH){        // FIFO High
            sky1311ReadFifo(&rx_buf[rx_buf_cnt], 56);              // load next 56 bytes into FIFO
            rx_buf_cnt += 56;
            irq_sta_h = sky1311ReadReg(ADDR_IRQ_STA);
            irqClear(IRQ_HIGH);//irqClearAll();
        }
        else if(irq_sta & IRQ_RX){          // Received
            if( ((sky1311ReadReg(ADDR_FIFO_LEN))<=3) && ((sky1311ReadReg(ADDR_ERR_STA))&CRC_ERROR) ){
                sky1311WriteCmd(CMD_CLR_FF);                                    // noise occur, restart the rx
                sky1311WriteCmd(CMD_RX);
                irq_sta_n = sky1311ReadReg(ADDR_IRQ_STA);
                irqClearAll();
            }       // end of NOISE_TEST
            else{
                irqClearAll();//PRINT(irq_sta);PRINT(err_sta);
                temp_len = sky1311ReadReg(ADDR_FIFO_LEN);                        // get & print FIFO length
                sky1311ReadFifo(&rx_buf[rx_buf_cnt], temp_len);        // get & print data <-- FIFO
                rx_buf_cnt += temp_len;
                err_sta = sky1311ReadReg(ADDR_ERR_STA);
                if(rxType == TYPE_A){
                    if(rate_type){
                        byte_num_H = sky1311ReadReg(ADDR_RATE_RX_BIT);   
                        byte_num_L = sky1311ReadReg(ADDR_RATE_RX_BYTE);
                    }else{
                        byte_num_H = sky1311ReadReg(ADDR_RX_NUM_H);      
                        byte_num_L = sky1311ReadReg(ADDR_RX_NUM_L);
                        bit_n = ((sky1311ReadReg(ADDR_RX_NUM_H)) & 0xf0)>>4;          // print bit num
                        if(bit_n) sky1311ReadFifo(&rx_buf[rx_buf_cnt], 1);
                    }
                        rx_len  = ((byte_num_H&0x01) << 8) | byte_num_L;
                        if(bit_n) rx_len++;
                }
                else if(rxType == TYPE_B){
                    byte_num_H = sky1311ReadReg(ADDR_RX_B_CTRL);     
                    byte_num_L = sky1311ReadReg(ADDR_RX_B_BYTE_NUM);
                    rx_len  = ( (byte_num_H&0x80) << 1 ) | byte_num_L;
                }
                return Ok;
            }
        }
        else{
            irqClearAll();
            return NoResponse;
        }
    }
}
/**
 ******************************************************************************
 ** \��  ��  �������ݺ�����ת���������
 **          Ҫ���͵����ݱ�����ȫ�ֱ���tx_buf�У����ͻ�������
 **          ���յ����ݱ�����ȫ�ֱ���rx_buf�У����յ������������ڱ���rx_len��   
 ** \��  ��  u8 txLen �������ݵĳ��ȣ����255�ֽ�
 **          u8 tx_rx_type ���ͺͽ��յ�PICC���� ��TYPE_A | TYPE_B��
 **          u8 rate_type �Ƿ�ʹ�ö౶��
 ** \����ֵ  
 ******************************************************************************/
sta_result_t data_tx_rx(u8 txLen, u8 tx_rx_type, u8 rate_type)
{
    sta_result_t sta;
    tx_func(txLen, tx_rx_type);
    sta = rx_func(tx_rx_type, rate_type);
    return sta;
}
#if 0
/**
 ******************************************************************************
 ** \��  ��  
 **
 ** \��  ��  
 ** \����ֵ  
 ******************************************************************************/
sta_result_t data_tx_rx_14443(u8 txLen, u8 tx_rx_type, u8 rate_type)
{
    sta_result_t sta;
    u16 i;
    pcb_p ^= 0x01;
    for(i=txLen-1; i>0; i--){
        tx_buf[i+2] = tx_buf[i];
    }
    tx_buf[i+2] = tx_buf[i]; tx_buf[1] = 0x00; tx_buf[0] = pcb_p;
    txLen += 2;

    tx_func(txLen, tx_rx_type);
    rx_len = rx_func(tx_rx_type, rate_type);
    sta = ( ((rx_buf[rx_len-4]&0xff)<<8) | (rx_buf[rx_len-3]&0xff) );
    return sta;
}
#endif

#if CHECKCARD == 1
/**
 ******************************************************************************
 ** \��  ��  �͹���ѯ���������ò�����ʹ���ڲ�RCʱ�ӿ�ʼѯ��
 **
 ** \��  ��  boolean �Ƿ�ͬʱ��TX�� u8 RCƵ��ֵ
 ** \����ֵ  none
 ******************************************************************************/
static void sky1311StartCheckField(boolean txEnable, u8 rcFreqVal)
{	
	sky1311Disable();				        // PD low, power down sky1311s
	sky1311WriteCmd(CMD_SW_RST);
	sky1311WriteReg(ADDR_ANA_CFG1, 0x00);	// disable all TX, RX, Crystal, RC
	sky1311WriteReg(ADDR_ANA_CFG0, 0x03);   // PA=2.3, internal RC
	if(txEnable){
	    sky1311WriteReg(ADDR_ANA_CFG1, 0x90|rcFreqVal); // enable TX & RC,
	}
	else{
	    sky1311WriteReg(ADDR_ANA_CFG1, 0x10|rcFreqVal); // set RC
	}
    sky1311WriteReg(ADDR_ANA_CFG3, 0x80);
	sky1311Enable();			            // PD high, power up sky1311s
}
/**
 ******************************************************************************
 ** \��  ��  �͹���ѯ�����ܹر�
 **
 ** \��  ��  none
 ** \����ֵ  none
 ******************************************************************************/	
static void sky1311StopCheckField(void)
{
	 sky1311Disable();                      // PD low, power down sky1311s
	 sky1311WriteCmd(CMD_SW_RST);
     sky1311WriteReg(ADDR_ANA_CFG1, 0x00);	// disable all TX, RX, Crystal, RC
}

/**
 ******************************************************************************
 ** \��  ��  ��ʼ���ڲ�ADC������������ʱ�����ں󼴿ɶ�ȡADֵ��
 **          ʵ��AD���ֵӦ��00H -- 0BH ֮��ȽϺ�
 **
 ** \������rang, �ڲ�AD�Ĳ�����Χ����Ҫ����оƬ�ڲ�500ŷķ�����ϲɼ��ĵ�ѹ��Χȷ��
 **         000��1.6 �� 2.46V
 **         001��1.28��2.14V
 **         011��0.96��1.82V
 **         111��0.64��1.5V
 ** \����ֵ  none
 ******************************************************************************/
static void intADStart(u8 rang)
{
    sky1311WriteReg(ADDR_ANA_CFG4, 0x28+rang);   // 15us
    sky1311WriteReg(ADDR_ANA_CFG4, 0xA8+rang);
}
/**
 ******************************************************************************
 ** \��  ��  ֹͣ�ڲ�ADC
 **
 ** \��  ��  none
 ** \����ֵ  none
 ******************************************************************************/
static void intADStop(void)
{
    u8 tmp = sky1311ReadReg(ADDR_ANA_CFG4);
    sky1311WriteReg(ADDR_ANA_CFG4, (tmp & 0xF7));
}
/**
 ******************************************************************************
 ** \��  ��  ��ȡ�ڲ�ADֵ
 **
 ** \��  ��  none
 ** \����ֵ  ��ȡ��8λ��ֵ
 ******************************************************************************/
static u8 intADRead(void)
{
    u8 res = sky1311ReadReg(ADDR_ANA_CFG5);
    return (res & 0x0F);
}

/**
 ******************************************************************************
 ** \��  ��  ɨ�����ŵ�RCƵ�ʣ�Ϊ�͹���ѯ���ã�
 **          ע�⣺��Ҫ�ڶ���������û���κε��������ڵ�ʱɨ��
 **
 ** \��  ��  none
 ** \����ֵ  ��4λ��ʾ��ֵ���ʱ��Ƶ��ֵ����ӦAnalog1�Ĵ����ĵ�4λ��
   		 bit4 ��ʾ�Ƿ�ɨ�赽���ֵ��1--ɨ�赽��0--û�С�
   		 bit5--7 û��ʹ�ã�����Ϊ0
 ******************************************************************************/
u8 sky1311RCFreqCali(void)
{
    u8 currAdcVal=0;       // ��ǰADֵ
    u8 maxAdcVal=0;        // ���ADֵ
    u8 currRCFreqVal=0;     // ��ǰƵ��ֵ
    u8 MaxRCFreqVal = 0;    // ����Ƶ��ֵ
   	sky1311StartCheckField(false,currRCFreqVal);	// set registor
   	#if USEDINTADC
   	#else
   	initADC();				        // init ADC
   	#endif
	for(currRCFreqVal=0;currRCFreqVal<16;currRCFreqVal++)   // from 0000 to 1111 scan
	{
	    sky1311WriteReg(ADDR_ANA_CFG1, 0x90 | currRCFreqVal );	// set a frequency
	    sky1311Enable();			// PD high, power up sky1311s
	    #if USEDINTADC
        intADStart(3);              // rang 0.65 -- 1.5V
        delay10us(1);   			    // delay about 30us
		currAdcVal = intADRead();							// read AD value
		#else
		AD_start();
		currAdcVal = readADC();							// read AD value
		#endif
		sky1311Disable();								// disable
		if(currAdcVal > maxAdcVal){			// ��ǰƵ�ʵ�ADֵ���������ֵ
		    maxAdcVal = currAdcVal; 			// �õ�ǰֵȡ�����ֵ
		    MaxRCFreqVal = currRCFreqVal;   	// ���µ�ǰƵ��ֵ
		}
	}
    sky1311StopCheckField();
    
    if(maxAdcVal>MINADVAL){
        //freqScanNeed = false;
        maxRCADVal = maxAdcVal;
#if DEBUG==1
        NEW_ROW();
        PUTS("--RC frequency calibate, ");
        PUTS("RC Param = ");
        PRINT(MaxRCFreqVal);
        NEW_ROW();
#endif    
        return (0x10 | MaxRCFreqVal);		// ���سɹ���Ǻ�Ƶ��ֵ
    }
    else{
        maxRCADVal = MAXADVAL;
#if DEBUG==1
        NEW_ROW();
        PUTS("--RC frequency calibrate: Error!");
        NEW_ROW();
#endif
        return 0;
    }
}
/**
 ******************************************************************************
 ** \��  ��  ����Ƿ��п���������뿪��(�͹���ѯ��)
 **
 ** \��  ��  u8 RCƵ���趨ֵ
 ** \����ֵ  û�ж�����0  (���õ͹���ѯ�����)
 **          �п����룬1  (���ö������)
 **          �п��뿪��2  (��������У׼��ѯ��Ƶ�ʱ��)
 ******************************************************************************/
u8 checkCardNear(u8 MaxRCFreqVal)
{
	u8 res = NOCHANGE;
	u8 currAdcVal;                          // ��ǰADֵ
    
    static u8 bakAdcVal[3]={ADTHRESHOLD,ADTHRESHOLD,ADTHRESHOLD};// ����3��ADֵ
    static u8 adcValIndex = 0;
	static boolean firstTime=true;          // �Ƿ��һ�μ��
    u8 i;
#if DEBUG == 1
    NEW_ROW();
    PUTS("AGC: ");
	PRINT(sky1311ReadReg(ADDR_ANA_CFG3));
    NEW_ROW();
#endif
    res = NOCHANGE;
#if USEDINTADC
    sky1311StartCheckField(false,MaxRCFreqVal);	// set registor
    intADStart(3);                          // rang 0.65 -- 1.5V, Start internal ADC    
    currAdcVal=intADRead();                // read AD value
    intADStop();
	sky1311StopCheckField();                // disable sky1311
#else
    initADC(); 				                // init ADC
    sky1311StartCheckField(true,MaxRCFreqVal);  // enable sky1311
    waitDelay(10);   			             
	startADC();
	currAdcVal=readADC();                  // read AD value
	sky1311StopCheckField();                // disable sky1311
    disableADC();                           // disable ADC
#endif
	if(!firstTime){
        
	    /*if(currAdcVal<=MINADVAL){           // �ﵽԤ�������Сֵ
            res = MINIMUM; 
	    } 
        else if(currAdcVal>maxRCADVal){     // ����Ԥ��������ֵ
	        //res = MAXIMUM;   
            maxRCADVal = currAdcVal;        // �������ֵ
            
	    }
        */
        if(currAdcVal<bakAdcVal[adcValIndex]){    // ֵ�ڱ�С
            if(currAdcVal<=MINADVAL){             //С�ھ������� 
                res = LESSEN;
            }
            else{
                for(i=0;i<3;i++){
                    if((currAdcVal+ADTHRESHOLD)<=bakAdcVal[i]){ // ��ǰ3������һ�μ�С������ֵ
                        res = LESSEN;
                    }
                }
            }
        }
        else if(currAdcVal>bakAdcVal[adcValIndex]){     // ֵ�ڱ��
                if(currAdcVal>maxRCADVal-ADTHRESHOLD)   // �ﵽ���ֵ-��ֵ
                   res = LARGEN;
        }
        else{                                       // ֵû�б�
            res = NOCHANGE;
        }
    }
    else{
        firstTime = false;
        for(i=0;i<3;i++){
            bakAdcVal[i] = currAdcVal;
        }
    }
    adcValIndex++;
    if(adcValIndex>=3)
        adcValIndex = 0;
    bakAdcVal[adcValIndex] = currAdcVal;  
    if(currAdcVal>maxRCADVal){          // ��ǰֵ�������ֵ
        maxRCADVal = currAdcVal;        // �������ֵΪ��ǰֵ    
	}
    
#if DEBUG==1
    NEW_ROW();
    PUTS("--CHECK CARD, Current AD Value:  ");
    PRINT(currAdcVal);
    NEW_ROW();
#endif
	return res;
}

u8 checkCardNearXtal(void)
{
    u8 res = 0;
	u8 currAdcVal;                         // ��ǰADֵ
	static u8 bak_ad_val = ADTHRESHOLD;     // �ϴ�ADֵ
	static boolean firstTime=true;          // �Ƿ��һ�μ��
    
	sky1311WriteCmd(CMD_SW_RST);
	sky1311Enable();			            // PD high, power up sky1311s
	delay10us(10);
	//sky1311WriteReg(ADDR_ANA_CFG1, 0x00);	// disable all TX, RX, Crystal, RC
	sky1311WriteReg(ADDR_ANA_CFG0, 0x02);   // PA=2.3, 13.56MHz Crystal
	sky1311WriteReg(ADDR_ANA_CFG1, 0x20);   // enable Crystal,
	delay1ms(2);
	sky1311WriteReg(ADDR_ANA_CFG0, 0x00);   // release sysclk 13.56MHz xtal
#if USEDINTADC
    intADStart(7);                          // rang 0.65 -- 1.5V, Start internal ADC
    delay10us(1);   			            // delay about 10us
	currAdcVal=intADRead();                // read AD value
    intADStop();
#else
    initADC(); 				                // init ADC
    delay10us(1);   			            // delay about 10us
	startADC();
	currAdcVal=readADC();                  // read AD value
	disableADC();                           // disable ADC
#endif
    sky1311StopCheckField();                // disable sky1311
	if(bak_ad_val<ADTHRESHOLD)
		bak_ad_val = ADTHRESHOLD;           // ��ֹ���ָ���
	
	if(!firstTime){
	    if(currAdcVal<=MINADVAL){
            res = NOCHANGE;
	    }
	    
        else if(currAdcVal>maxRCADVal){     // �ﵽ���ADֵ
	        res = MAXIMUM;  //FREQCALI;   
	    }
	    else if(currAdcVal<=(bak_ad_val-ADTHRESHOLD)){ // AD��ֵ��С������ֵ 
	        res = LESSEN;   //CARDOPERAT;   
	    }
	    else if(currAdcVal>(bak_ad_val+ADTHRESHOLD)){   // AD��ֵ���󳬹���ֵ
	        res = LARGEN;
	    }
	    else{
	        res = NOCHANGE; //CARDCHECK;
	    }
	   
    }
    else{
        firstTime = false;
    }
    bak_ad_val = currAdcVal;      // ���浱ǰֵ
    PRINT(currAdcVal);
	return res;
} 
#endif

