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
 ** 文 件 名： sky1311sDemo.c
 **
 ** 文件简述： sky1311s 测试主程序文件
 **
 ** 版本历史:
 ** 2015-10-05 V1.00  Mark   正式版本
 **
 ******************************************************************************/

#include "sky1311s.h"
#include "mcu.h"
#include "iso14443.h"
#include "driver.h"
#include "test.h"
#include "userdef.h"
#include "UsbDeviceCdcCom.h"
////////////////////////////////////////////////////////////////////////////////
/**
 ******************************************************************************
 ** \简述： 全局变量定义 
 **
 ******************************************************************************/ 
u8 uid[5];              // PICC的UID
u8 uidLen=0;            // PICC的UID长度，此测试程序固定4字节
  
/**
  * @brief type A card read test
  * @param None
  * @return error code
  */
sta_result_t typeA_test(void)
{
    sta_result_t sta;
    u8 card_type = TYPE_A;
    u8 rand_bit  = 0x01;
#if DEBUG==1
    NEW_ROW(); 
    PUTS("---Type A Test---"); 
    NEW_ROW();
#endif
    typeAOperate();
    card_type = TYPE_A;
    rand_bit = 1 - rand_bit;
    // Off and Open PCD antenna one time  
    //delay1ms(10);
    pcdAntennaOff();
    delay1ms(10);
    pcdAntennaOn(card_type);
    delay1ms(10);
    
    sky1311WriteReg(ADDR_ANA_CFG3, 0x81);	// automatic gain control reset, AGC:12dB
    sta = piccRequestA();					// send Request command to card
#if DEBUG==1
    NEW_ROW(); PUTS("rx_agc_results(Analog3):"); //NEW_ROW();
    PRINT(sky1311ReadReg(ADDR_ANA_CFG3)); NEW_ROW();
#endif
    if(sta == NoResponse){
    	return NoResponse;
    }
    else{
        LEDGRN = LED_ON;
#if DEBUG==1
    	PUTS("request TypeA tag : ");
    	NEW_ROW();
    	print_fifo(rx_len, card_type);
#endif
    }

    sta = piccAntiA(rand_bit, uid, &uidLen);		// card ANTICOLLISION
    if(sta == NoResponse)
    	return NoResponse;
    else{
#if DEBUG == 1
    	PUTS("Tag UID ");
    	NEW_ROW();
    	print_fifo(rx_len, card_type);
#endif
    }
    sta = piccSelectA(uid);			// select card
    if(sta == NoResponse)
    	return NoResponse;
    else{
#if DEBUG==1
        NEW_ROW();
    	PUTS("Tag selection ");
    	NEW_ROW();
    	print_fifo(rx_len, card_type);
#endif
    }

    return Ok;
}

/**
  *	@brife M1 card test
  *	@param None
  * @return None
**/
sta_result_t mifare1_test(void)
{
    sta_result_t sta;
    u8 card_type = TYPE_A;

    u32 RA = 0xAACBD156;
    u8 sector_num = 4;
    u8 block_num = 1;
    u8 block_addr = sector_num*4 + block_num;
    u8 key_m1[] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
#if  M1_TEST==1
    sta = M1_3Pass(block_addr, key_m1, uid, RA);
    if(sta == NoResponse)
    	return NoResponse; 
    
    PUTS("1. READ Block -->");
    sta = M1_Read(block_addr);
    if(sta == NoResponse)
    	return NoResponse;
    	
    else
    	print_fifo(rx_len, card_type);

    PUTS("2. WRITE Block -->");
    //sta = M1_Write(block_addr, 0x11111111);
    sta = M1_Write(block_addr, 0xAAAAAAAA);
    if(sta == NoResponse)
    	return NoResponse;
    else
    	print_fifo(rx_len, card_type);

    PUTS("3. READ Block -->");
    sta = M1_Read(block_addr);
    if(sta == NoResponse)
    	return NoResponse;
    else
    	print_fifo(rx_len, card_type);

    PUTS("4. INCREMENT Block -->");
    sta = M1_Increment(block_addr, 0x11111111);
    if(sta == NoResponse)
    	return NoResponse;
    else
    	print_fifo(rx_len, card_type);

    PUTS("5. READ Block -->");
    sta = M1_Read(block_addr);
    if(sta == NoResponse)
    	return NoResponse;
    else
    	print_fifo(rx_len, card_type);
    /*
    PUTS("2. WRITE Block -->");
    sta = M1_Write(block_addr, 0x11111111);
    if(sta == NoResponse)
    	return NoResponse;
    else
    	print_fifo(rx_len, card_type);
    */
    PUTS("6. INCREMENT Block -->");
    sta = M1_Increment(block_addr, 0x12345678);
    if(sta == NoResponse)
    	return NoResponse;
    else
    	print_fifo(rx_len, card_type);

    PUTS("7. READ Block -->");
    sta = M1_Read(block_addr);
    if(sta == NoResponse)
    	return NoResponse;
    else
    	print_fifo(rx_len, card_type);

    PUTS("8. DECREMENT Block -->");
    sta = M1_Decrement(block_addr, 3);
    if(sta == NoResponse)
    	return NoResponse;
    else
    	print_fifo(rx_len, card_type);

    PUTS("9 READ Block -->");
    sta = M1_Read(block_addr);
    if(sta == NoResponse)
    	return NoResponse;
    else
    	print_fifo(rx_len, card_type);

    PUTS("0 RESTORE Block -->");
    sta = M1_Restore(block_addr);
    if(sta == NoResponse)
    	return NoResponse;
    else
    	print_fifo(rx_len, card_type);

#endif  // end of M1_TEST	
	return Ok;
}
/**
  * @smartTypa A test
  * @param None
  * @return Error code
  */
sta_result_t smartTypeA_test(void)
{
    sta_result_t sta;
    u8 rate_flag;
    u8 card_type = TYPE_A;

    sta = piccATS();
    pcb_p=0x03;
    if(sta == NoResponse)
    	return NoResponse; 
    else{
    	PUTS("RATS : ");
    	NEW_ROW();
    	print_fifo(rx_len, card_type);
    }
    rate_flag = RATE_OFF;
#if  RATE_TEST==1
        sta = A_pps(212);
    if(sta == NoResponse)
    	return NoResponse;
    else{
        rate_flag = RATE_ON;
        piccPPS(212);
        PUTS("PPS  ");NEW_ROW();
        print_fifo(rx_len, card_type);
    }
#endif  // end of RATE_TEST
    /* select MF file */
    PUTS("File Operation ");NEW_ROW();
    tx_buf[0] = 0x02;
    tx_buf[1] = 0x00;
    tx_buf[2] = 0xA4;   // command: select file
    tx_buf[3] = 0x00;
    tx_buf[4] = 0x00;
    tx_buf[5] = 0x02;
    tx_buf[6] = 0x3F;
    tx_buf[7] = 0x00;   // MF 
    sta = data_tx_rx(8, card_type, rate_flag);
    if(sta == NoResponse)
    	return NoResponse;
    else
    	print_fifo(rx_len, card_type);

    /*tx_buf[0] = 0x03;
    tx_buf[1] = 0x00;
    tx_buf[2] = 0xA4;
    tx_buf[3] = 0x00;
    tx_buf[4] = 0x00;
    tx_buf[5] = 0x02;
    tx_buf[6] = 0x00;
    tx_buf[7] = 0x04;*/
    
    /* 读一卡通有关的信息 */
    tx_buf[0] = 0x03;
    tx_buf[1] = 0x00;
    tx_buf[2] = 0xA4;
    tx_buf[3] = 0x00;
    tx_buf[4] = 0x00;
    tx_buf[5] = 0x02;
    tx_buf[6] = 0x10;
    tx_buf[7] = 0x01;
    sta = data_tx_rx(8, card_type, rate_flag);
    if(sta == NoResponse)
    	return NoResponse;
    else
    	print_fifo(rx_len, card_type);
    NEW_ROW();
    /* 读余额 */
    tx_buf[0] = 0x02;
    tx_buf[1] = 0x80;
    tx_buf[2] = 0x5C;
    tx_buf[3] = 0x00;
    tx_buf[4] = 0x02;
    tx_buf[5] = 0x04;
    /*tx_buf[0] = 0x02;
    tx_buf[1] = 0x00;
    tx_buf[2] = 0xB0;
    tx_buf[3] = 0x95;
    tx_buf[4] = 0x00;
    tx_buf[5] = 0x2B;*/
    sta = data_tx_rx(6, card_type, rate_flag);
    if(sta == NoResponse)
    	return NoResponse;
    else
    	print_fifo(rx_len, card_type);
    /* 读取交易记录 */
    tx_buf[0] = 0x03;
    tx_buf[1] = 0x00;   // CLA Class
    tx_buf[2] = 0xB2;   // INS Instruction
    tx_buf[3] = 0x01;   // P1 Parameter 1
    tx_buf[4] = 0xC5;   // P2 Parameter 2
    tx_buf[5] = 0x00;   // Le
    sta = data_tx_rx(6, card_type, rate_flag);
    if(sta == NoResponse)
    	return NoResponse;
    else
    	print_fifo(rx_len, card_type);

    /*
    tx_buf[0] = 0x03;
    tx_buf[1] = 0x00;
    tx_buf[2] = 0xD6;
    tx_buf[3] = 0x00;
    tx_buf[4] = 0x00;
    tx_buf[5] = 0x08;
    tx_buf[6] = 0x30;
    tx_buf[7] = 0x31;
    tx_buf[8] = 0x32;
    tx_buf[9] = 0x33;
    tx_buf[10]= 0x34;
    tx_buf[11]= 0x35;
    tx_buf[12]= 0x36;
    tx_buf[13]= 0x37;
    sta = data_tx_rx(0x0E, card_type, rate_flag);
    if(sta == NoResponse)
    	return NoResponse;
    else
    	print_fifo(rx_len, card_type);    
    */
    NEW_ROW(); PUTS("rx_agc_results:"); 
    PRINT(sky1311ReadReg(ADDR_ANA_CFG3)); NEW_ROW();
    return Ok;
}

//--------------------------- type B -------------------------------------------
sta_result_t typeB_test(void)
{
    sta_result_t sta;
    u8 card_type = TYPE_A;
    u8 rate_flag;
    u8 param[3];
    typeBOperate();
    card_type = TYPE_B;
    rate_flag = RATE_OFF;
    delay1ms(10);

    sta = piccRequestB(0x00, uid, param);
    if(sta == NoResponse){
        return NoResponse;
    }
    else{
        LEDGRN = LED_ON;
    	print_fifo(rx_len, card_type);
    }

    sta = piccAttrib(uid, param);
    if(sta == NoResponse)
    	return NoResponse;
    else
    	print_fifo(rx_len, card_type);
    /* 读身份证UID */
    tx_buf[0] = 0x00;
    tx_buf[1] = 0x36;
    tx_buf[2] = 0x00;
    tx_buf[3] = 0x00;
    tx_buf[4] = 0x08;
    sta = data_tx_rx(5, card_type, rate_flag);
    if(sta == NoResponse)
    	return NoResponse;
    else
    	print_fifo(rx_len, card_type);
    /*
    tx_buf[0] = 0x0A;
    tx_buf[1] = param[1];
    tx_buf[2] = 0x00;
    tx_buf[3] = 0x84;
    tx_buf[4] = 0x00;
    tx_buf[5] = 0x00;
    tx_buf[6] = 0x08;
    sta = data_tx_rx(7, card_type, rate_flag);
    if(sta == NoResponse)
    	return NoResponse;
    else
    	print_fifo(rx_len, card_type);

    tx_buf[0] = 0x0B;
    tx_buf[1] = param[1];
    tx_buf[2] = 0x00;
    tx_buf[3] = 0xE4;
    tx_buf[4] = 0x00;
    tx_buf[5] = 0x00;
    tx_buf[6] = 0x04;
    tx_buf[7] = 0x01;
    tx_buf[8] = 0xE0;
    tx_buf[9] = 0x00;
    tx_buf[10] = 0xF0;
    sta = data_tx_rx(11, card_type, rate_flag);
    if(sta == NoResponse)
    	return NoResponse;
    else
    	print_fifo(rx_len, card_type);
    */
    return Ok;
}

/**
 ******************************************************************************
 ** \简  述  读卡测试， 尝试Type A -->Smart Card, Mifare 1，
 **                         Type B
 **
 ** \参  数  none
 ** \返回值  Ok, 读到卡; 其它：没读到卡
 ******************************************************************************/  
sta_result_t readCard_test(void)
{
    sta_result_t res;
    NEW_ROW();
    PUTS("/============= ISO14443 Reader Test BEGIN =============/");
    NEW_ROW();
    sky1311Enable(); 	    // chip enable PD2
    initSky1311s();			// init sky1311s
	res = typeA_test();
	if(Ok == res){
#if CARDTYPE == AUTO
        u8 sak = rx_buf[0];
        if((sak & 0x24) == 0x20){      //UID complete, PICC compliant with ISO/IEC 14443-4
            NEW_ROW(); PUTS("---Smart Card Test---"); NEW_ROW();
		    res = smartTypeA_test();
        }
        else if((sak & 0x24) == 0x00){  // UID complete, PICC not compliant with ISO/IEC 14443-4
            NEW_ROW(); PUTS("---Mifare 1 Test---"); NEW_ROW();
		    res = mifare1_test();
        }
#elif CARDTYPE == ONLYM1
        NEW_ROW(); PUTS("---Mifare 1 Test---"); NEW_ROW();
	    res = mifare1_test();
#elif CARDTYPE == ONLYSMART
        NEW_ROW(); PUTS("---Smart Card Test---"); NEW_ROW();
		res = smartTypeA_test();
#endif
	}
	else{
		NEW_ROW(); PUTS("---Type B Test---"); NEW_ROW();
        res = typeB_test();	
	}
    delay100ms(1);      // 延时100ms，使LED更能看清楚
    LEDGRN = LED_OFF;
    return res;
}

/**
 ******************************************************************************
 ** \简  述  低功耗询卡，读卡操作
 **
 ** \参  数  none
 ** \返回值  none
 ******************************************************************************/  
void lpCardOpera(void)
{
#if CHECKCARD == 1
    static u8 chkState=LARGEN;    // 第一次校准RC频率
    static u8 ana1Param=0;         // analog1 RC频率参数
    static boolean cardInField = false;        // 指示是否有卡在场内，防止重复读卡.
    switch(chkState){               // 检查AD值得变化情况
    case NOCHANGE:                  // 没变
        chkState = checkCardNear(ana1Param&0x0F);  // 询卡
        break;
    case LARGEN:                    // AD值变大，达到最大值   
        if(cardInField){            // 之前有卡，则认为卡已经离开
            cardInField = false;
            chkState = NOCHANGE;
        }
        else{                       // 之前没卡，则重新进行RC频率扫描校准
            ana1Param = sky1311RCFreqCali();    // RC频率扫描
            if(0x10==(ana1Param & 0x10)){       // 频率校准完成
                chkState = NOCHANGE; 
            }
            else{
                delay100ms(10);
            }
        }
        break;
    case LESSEN:                     // AD值变小到绝对值以下或持续变小
        if(!cardInField){            // 之前没有卡在场内
            sta_result_t sta = readCard_test(); // 尝试读卡
            if(sta == Ok){
                cardInField = true;
            }
            else{
                cardInField = false;
                
            }
        }
        chkState = NOCHANGE;    // 继续读AD值
        break;    
    }
#endif
}
/**
 ******************************************************************************
 ** \简  述  命令行操作
 **
 ** \参  数  none
 ** \返回值  none
 ******************************************************************************/ 
void cmdOperation(boolean flag)
{
#if USEDCMD == 1
    
    static u8 cmd = 0;
    static u8 cmd_bak=0;
    static boolean outputMsg0;
    static boolean outputMsg2;
    if(!flag){
        outputMsg0=true;
        outputMsg2 = true;
    }
    if(cmd == 0){
        if(outputMsg0){
            NEW_ROW();
            PUTS("Input a command:");
            outputMsg0 = false;
            cmd_bak = 0;
        }
        cmd = UsbDeviceCdcCom_ReceiveByte1();
        if(cmd !=0xff && cmd != cmd_bak){
            cmd_bak = cmd;
            //UsbDeviceCdcCom_SendByte(cmd);
            NEW_ROW();
        }
        else{
            cmd = 0;
        }
    }
    else if(cmd == '1'){         // open reader Field
        cmd = 0;
        outputMsg0 = true;
        sky1311Enable(); 	    // chip enable PD2
        delay1ms(10);
        sky1311WriteCmd(CMD_SW_RST);     // soft reset
        initSky1311s();			// init sky1311s
        delay1ms(10);
        pcdAntennaOff();
        delay1ms(10);
        pcdAntennaOn(1);
        delay1ms(10);
        NEW_ROW();
        PUTS("Reader field open...");
        NEW_ROW();
    }
    else if(cmd == '2'){    // continuous Request A
        
        if(outputMsg2){
            outputMsg2 = false;
            NEW_ROW();
            PUTS("Request type A command, continue...");
            NEW_ROW();
            PUTS("Input a command:");
        }
        
        sky1311Enable(); 	    // chip enable PD2
        delay1ms(10);
        sta_result_t sta = piccRequestA();
        sky1311Disable();
        
        cmd = UsbDeviceCdcCom_ReceiveByte1();
        
        
        if(cmd == 0xff){ 
            cmd = cmd_bak;
        }
        else{ 
            outputMsg2 = true;
            NEW_ROW();
        }
    }
    else if(cmd == '3'){
        cmd = 0;
        outputMsg0 = true;
        readCard_test();
        sky1311Disable();
    }
    else if(cmd == '0'){
        cmd = 0;
        outputMsg0 = true;
        pcdAntennaOff();
        sky1311Disable(); 	    
        delay1ms(10);
        sky1311WriteCmd(CMD_SW_RST);     // soft reset
        NEW_ROW();
        PUTS("Reader field closed");
        NEW_ROW();NEW_ROW();
    }
    else{
        cmd = 0;
        outputMsg0 = true;
    }
#endif
}

//                    Main programme session                                 //
/**
  * @main programm
  * @param none
  * @return none
  */

void main()
{
    static boolean intoCmdFlag = false;
    u8 operaMode;         // 操作模式，0：间隔读卡，1：间隔寻卡，2：命令
    initMCU();
    delay10ms(100);  
    sky1311Enable(); 
  	//delay1ms(5);
  	sky1311WriteCmd(CMD_SW_RST);     // soft reset
  	delay1ms(5);
  	initSky1311s();        // init running param
    readCard_test();        // 上电尝试读卡1次
    pcdAntennaOff();
    sky1311Disable();
    operaMode = 0;
    LED1 = LED_ON;
    while(1){
        if(btnVal == 1){
            btnVal = 0;
            if(operaMode<2)
                operaMode++;
            else
                operaMode = 0;
        }
        else if(btnVal == 2){
            btnVal = 0;
            if(operaMode>0)
                operaMode--;
            else
                operaMode = 2;
        }
        
        if(operaMode == 0){         /* 间隔读卡 */
            LED1 = LED_ON;
            LED2 = LED_OFF;
            LED3 = LED_OFF;
#if READERCARD == 1
            delay100ms(4);
            readCard_test();
            sky1311Disable();
            intoCmdFlag = false;
#endif
        }
        else if(operaMode == 1){    /* 间隔寻卡 */
            LED1 = LED_OFF;
            LED2 = LED_ON;
            LED3 = LED_OFF;
#if CHECKCARD == 1
            delay10ms(34);
            lpCardOpera();
            intoCmdFlag = false;
#endif
            
        }
        else if(operaMode == 2){    /* 按命令操作 */
            LED1 = LED_OFF;
            LED2 = LED_OFF;
            LED3 = LED_ON;
#if USEDCMD == 1
            if(!intoCmdFlag){
                sky1311Disable();
            }
            cmdOperation(intoCmdFlag);
            intoCmdFlag = true;
#endif
        }
        
    }
}

