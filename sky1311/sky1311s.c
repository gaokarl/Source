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
 ** 文 件 名： sky1311s.c
 **
 ** 文件简述： 操作sky1311s的所有函数，常量和变量
 **
 ** 版本历史:
 ** 2015-10-05 V1.00  EH   第一个正式版本
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
 ** \简述： 全局变量定义 
 **
 ******************************************************************************/ 
//sta_field_t pcdFieldState=NoObjectIn;      // 读卡器场状态（是否有物体在场内）
#if CHECKCARD==1
    u8 maxRCADVal=MAXADVAL;         // 检测到的最大AD值
    boolean freqScanNeed=true;  // 系统需要RC频率扫描
#endif

u8  tx_buf[255];                // FIFO数据发送缓冲区，根据实际情况调整大小
u8  rx_buf[255];                // FIFO数据接收缓冲区，根据实际情况调整大小
u16 rx_len=0;                   // FIFO接收数据长度

u8  err_sta;
u8  irq_sta;
u8  irq_sta_n;
u8  irq_sta_h;
u8  pcb_p=0x03;

/**
 ******************************************************************************
 ** \简  述  设置寄存器掩码位
 **
 ** \参  数  u8 regAddr 寄存器地址， u8 mask 要设置的"掩码"字节
 ** \返回值  none
 ******************************************************************************/ 
void SetBitMask(u8 regAddr, u8 mask)
{
    u8 tmp;
    tmp = sky1311ReadReg(regAddr);
    sky1311WriteReg(regAddr, tmp|mask);
}
/**
 ******************************************************************************
 ** \简  述  清除寄存器掩码位
 **
 ** \参  数  u8 regAddr 寄存器地址， u8 mask 要清除的"掩码"字节
 ** \返回值  none
 ******************************************************************************/ 
void ClearBitMask(u8 regAddr, u8 mask)
{
    u8 tmp;
    tmp = sky1311ReadReg(regAddr);
    sky1311WriteReg(regAddr, tmp & ~mask);
}
/**
 ******************************************************************************
 ** \简  述  模拟参数初始化，配置模拟参数寄存器 
 **
 ** \参  数  none
 ** \返回值  none
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
 ** \简  述  模拟参数初始化，配置模拟参数寄存器 
 **
 ** \参  数  选择通过MFOUT脚输出的信号，
 **          bit2,1,0
 **             0 0 0 rx phase signal after demod, before sample
 **             0 0 1 tx bit stream
 **             0 1 0 rx bit stream
 **             0 1 1 rx data from analog
 **             1 0 0 DO from digital
 ** \返回值  none
 ******************************************************************************/ 
void directModeInit(u8 mfout_sel)
{
    sky1311WriteReg(ADDR_MFOUT_SEL,  mfout_sel);
}

/**
 ******************************************************************************
 ** \简  述  打开PCD天线 
 **
 ** \参  数  PCD类型， TYPE_A, TYPE_B
 ** \返回值  none
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
 ** \简  述  关闭PCD天线 
 **
 ** \参  数  none
 ** \返回值  none
 ******************************************************************************/
void pcdAntennaOff(void)
{
    sky1311WriteReg(ADDR_ANA_CFG2, ANA2_A);
    ClearBitMask(ADDR_TX_CTRL, TX_POLE);
}
/**
 ******************************************************************************
 ** \简  述  清除所有中断标记
 **
 ** \参  数  none
 ** \返回值  none
 ******************************************************************************/
void irqClearAll(void)
{
    //sky1311WriteReg(ADDR_IRQ_STA, IRQ_TOUT|IRQ_TX|IRQ_RX|IRQ_HIGH|IRQ_LOW|IRQ_OSC|IRQ_ERR);
    sky1311WriteReg(ADDR_IRQ_STA, 0x7F);
}
/**
 ******************************************************************************
 ** \简  述  清除指定的中断标记
 **
 ** \参  数  u8 irq
 ** \返回值  none
 ******************************************************************************/
void irqClear(u8 irq)
{
    sky1311WriteReg(ADDR_IRQ_STA,irq);
}
/**
 ******************************************************************************
 ** \简  述  设置sky1311与卡通信的倍速, ISO14443-4部分规定，多倍速的PICC
 **
 ** \参  数  设置的倍速，212或424
 ** \返回值  none
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
 ** \简  述  选择type A 卡作为操作对象
 **
 ** \参  数  none
 ** \返回值  none
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
 ** \简  述  选择type B 卡作为操作对象
 **
 ** \参  数  none
 ** \返回值  none
 ******************************************************************************/
void typeBOperate(void)
{
    sky1311WriteReg(ADDR_ANA_CFG2, ANA2_B);            // analogB select
    sky1311WriteReg(ADDR_FSM_STATE, TYPE_B_SEL);       // typeB select
    sky1311WriteReg(ADDR_CRC_CTRL, CRC_B);             // crcB enable
} 
/**
 ******************************************************************************
 ** \简  述  初始化SKY1311S
 **
 ** \参  数  none
 ** \返回值  none
 ******************************************************************************/
void initSky1311s(void)
{
    analogInit();
    directModeInit(MFOUT_ANALOG_RX);
    sky1311WriteReg(ADDR_TIME_OUT2, 0x8F);       // time_out timer stop condition = beginning of RX SOF
    sky1311WriteReg(ADDR_TIME_OUT1, 0xFF);
    sky1311WriteReg(ADDR_TIME_OUT0, 0xFF);
    //sky1311WriteReg(ADDR_RX_PUL_DETA, 0x44);
    sky1311WriteReg(ADDR_RX_PUL_DETA, 0x64);     // 高4位识别曼彻斯特码脉宽，越大容错能力越强
 //   sky1311WriteReg(ADDR_RX_PRE_PROC, 0x26);
    sky1311WriteReg(ADDR_IRQ_EN, IRQ_TOUT_EN|IRQ_TX_EN|IRQ_RX_EN|IRQ_HIGH_EN|IRQ_LOW_EN);    // enable all IRQ
}

/**
 ******************************************************************************
 ** \简  述  数据发射函数，将数据写到FIFO中并通过射频接口发送给PICC
 **          发送的数据保存在全局变量tx_buf中，发送缓冲区
 **          
 ** \参  数  u16 txLen 数据长度, u8 txType 发射类型（TYPE_A | TYPE_B）
 ** \返回值  none
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
 ** \简  述  数据接收函数，从sky1311s的FIFO中读取来自PICC的数据
 **          接收到的数据保存在接收缓冲区中，全局变量rx_buf
 **          接收到的数据长度保存在全局变量rx_len中
 **
 ** \参  数  u8 rxType 接收的PICC类型（TYPE_A | TYPE_B）
 **          u8 rate_type 是否使用多倍速
 ** \返回值  Ok:正确接收到数据，其它：没收到数据
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
 ** \简  述  发送数据后立即转入接收数据
 **          要发送到数据保存在全局变量tx_buf中（发送缓冲区）
 **          接收的数据保存在全局变量rx_buf中，接收到的数量保存在变量rx_len中   
 ** \参  数  u8 txLen 发送数据的长度，最多255字节
 **          u8 tx_rx_type 发送和接收的PICC类型 （TYPE_A | TYPE_B）
 **          u8 rate_type 是否使用多倍速
 ** \返回值  
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
 ** \简  述  
 **
 ** \参  数  
 ** \返回值  
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
 ** \简  述  低功耗询卡功能配置参数，使用内部RC时钟开始询卡
 **
 ** \参  数  boolean 是否同时打开TX， u8 RC频率值
 ** \返回值  none
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
 ** \简  述  低功耗询卡功能关闭
 **
 ** \参  数  none
 ** \返回值  none
 ******************************************************************************/	
static void sky1311StopCheckField(void)
{
	 sky1311Disable();                      // PD low, power down sky1311s
	 sky1311WriteCmd(CMD_SW_RST);
     sky1311WriteReg(ADDR_ANA_CFG1, 0x00);	// disable all TX, RX, Crystal, RC
}

/**
 ******************************************************************************
 ** \简  述  初始化内部ADC并启动，几个时钟周期后即可读取AD值，
 **          实测AD结果值应在00H -- 0BH 之间比较好
 **
 ** \参数：rang, 内部AD的采样范围，需要根据芯片内部500欧姆电阻上采集的电压范围确定
 **         000：1.6 ～ 2.46V
 **         001：1.28～2.14V
 **         011：0.96～1.82V
 **         111：0.64～1.5V
 ** \返回值  none
 ******************************************************************************/
static void intADStart(u8 rang)
{
    sky1311WriteReg(ADDR_ANA_CFG4, 0x28+rang);   // 15us
    sky1311WriteReg(ADDR_ANA_CFG4, 0xA8+rang);
}
/**
 ******************************************************************************
 ** \简  述  停止内部ADC
 **
 ** \参  数  none
 ** \返回值  none
 ******************************************************************************/
static void intADStop(void)
{
    u8 tmp = sky1311ReadReg(ADDR_ANA_CFG4);
    sky1311WriteReg(ADDR_ANA_CFG4, (tmp & 0xF7));
}
/**
 ******************************************************************************
 ** \简  述  读取内部AD值
 **
 ** \参  数  none
 ** \返回值  读取的8位数值
 ******************************************************************************/
static u8 intADRead(void)
{
    u8 res = sky1311ReadReg(ADDR_ANA_CFG5);
    return (res & 0x0F);
}

/**
 ******************************************************************************
 ** \简  述  扫描最优的RC频率，为低功耗询卡用，
 **          注意：需要在读卡器场内没有任何导电物质遮挡时扫描
 **
 ** \参  数  none
 ** \返回值  低4位表示幅值最大时的频率值，对应Analog1寄存器的低4位。
   		 bit4 表示是否扫描到最大值，1--扫描到，0--没有。
   		 bit5--7 没有使用，设置为0
 ******************************************************************************/
u8 sky1311RCFreqCali(void)
{
    u8 currAdcVal=0;       // 当前AD值
    u8 maxAdcVal=0;        // 最大AD值
    u8 currRCFreqVal=0;     // 当前频率值
    u8 MaxRCFreqVal = 0;    // 最大的频率值
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
		if(currAdcVal > maxAdcVal){			// 当前频率的AD值若大于最大值
		    maxAdcVal = currAdcVal; 			// 用当前值取代最大值
		    MaxRCFreqVal = currRCFreqVal;   	// 记下当前频率值
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
        return (0x10 | MaxRCFreqVal);		// 返回成功标记和频率值
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
 ** \简  述  检查是否有卡进入或者离开场(低功耗询卡)
 **
 ** \参  数  u8 RC频率设定值
 ** \返回值  没有动作，0  (设置低功耗询卡标记)
 **          有卡进入，1  (设置读卡标记)
 **          有卡离开，2  (设置重新校准查询卡频率标记)
 ******************************************************************************/
u8 checkCardNear(u8 MaxRCFreqVal)
{
	u8 res = NOCHANGE;
	u8 currAdcVal;                          // 当前AD值
    
    static u8 bakAdcVal[3]={ADTHRESHOLD,ADTHRESHOLD,ADTHRESHOLD};// 保存3次AD值
    static u8 adcValIndex = 0;
	static boolean firstTime=true;          // 是否第一次检测
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
        
	    /*if(currAdcVal<=MINADVAL){           // 达到预定义的最小值
            res = MINIMUM; 
	    } 
        else if(currAdcVal>maxRCADVal){     // 超过预定义的最大值
	        //res = MAXIMUM;   
            maxRCADVal = currAdcVal;        // 调整最大值
            
	    }
        */
        if(currAdcVal<bakAdcVal[adcValIndex]){    // 值在变小
            if(currAdcVal<=MINADVAL){             //小于绝对门限 
                res = LESSEN;
            }
            else{
                for(i=0;i<3;i++){
                    if((currAdcVal+ADTHRESHOLD)<=bakAdcVal[i]){ // 比前3次任意一次减小超过阈值
                        res = LESSEN;
                    }
                }
            }
        }
        else if(currAdcVal>bakAdcVal[adcValIndex]){     // 值在变大
                if(currAdcVal>maxRCADVal-ADTHRESHOLD)   // 达到最大值-阈值
                   res = LARGEN;
        }
        else{                                       // 值没有变
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
    if(currAdcVal>maxRCADVal){          // 当前值超过最大值
        maxRCADVal = currAdcVal;        // 调整最大值为当前值    
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
	u8 currAdcVal;                         // 当前AD值
	static u8 bak_ad_val = ADTHRESHOLD;     // 上次AD值
	static boolean firstTime=true;          // 是否第一次检测
    
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
		bak_ad_val = ADTHRESHOLD;           // 防止出现负数
	
	if(!firstTime){
	    if(currAdcVal<=MINADVAL){
            res = NOCHANGE;
	    }
	    
        else if(currAdcVal>maxRCADVal){     // 达到最大AD值
	        res = MAXIMUM;  //FREQCALI;   
	    }
	    else if(currAdcVal<=(bak_ad_val-ADTHRESHOLD)){ // AD数值减小超过阈值 
	        res = LESSEN;   //CARDOPERAT;   
	    }
	    else if(currAdcVal>(bak_ad_val+ADTHRESHOLD)){   // AD数值增大超过阈值
	        res = LARGEN;
	    }
	    else{
	        res = NOCHANGE; //CARDCHECK;
	    }
	   
    }
    else{
        firstTime = false;
    }
    bak_ad_val = currAdcVal;      // 保存当前值
    PRINT(currAdcVal);
	return res;
} 
#endif

