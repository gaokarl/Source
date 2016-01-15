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
 ** �� �� ���� sky1311sDemo.c
 **
 ** �ļ������� sky1311s �����������ļ�
 **
 ** �汾��ʷ:
 ** 2015-10-05 V1.00  Mark   ��ʽ�汾
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
 ** \������ ȫ�ֱ������� 
 **
 ******************************************************************************/ 
u8 uid[5];              // PICC��UID
u8 uidLen=0;            // PICC��UID���ȣ��˲��Գ���̶�4�ֽ�
  
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
    
    /* ��һ��ͨ�йص���Ϣ */
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
    /* ����� */
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
    /* ��ȡ���׼�¼ */
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
    /* �����֤UID */
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
 ** \��  ��  �������ԣ� ����Type A -->Smart Card, Mifare 1��
 **                         Type B
 **
 ** \��  ��  none
 ** \����ֵ  Ok, ������; ������û������
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
    delay100ms(1);      // ��ʱ100ms��ʹLED���ܿ����
    LEDGRN = LED_OFF;
    return res;
}

/**
 ******************************************************************************
 ** \��  ��  �͹���ѯ������������
 **
 ** \��  ��  none
 ** \����ֵ  none
 ******************************************************************************/  
void lpCardOpera(void)
{
#if CHECKCARD == 1
    static u8 chkState=LARGEN;    // ��һ��У׼RCƵ��
    static u8 ana1Param=0;         // analog1 RCƵ�ʲ���
    static boolean cardInField = false;        // ָʾ�Ƿ��п��ڳ��ڣ���ֹ�ظ�����.
    switch(chkState){               // ���ADֵ�ñ仯���
    case NOCHANGE:                  // û��
        chkState = checkCardNear(ana1Param&0x0F);  // ѯ��
        break;
    case LARGEN:                    // ADֵ��󣬴ﵽ���ֵ   
        if(cardInField){            // ֮ǰ�п�������Ϊ���Ѿ��뿪
            cardInField = false;
            chkState = NOCHANGE;
        }
        else{                       // ֮ǰû���������½���RCƵ��ɨ��У׼
            ana1Param = sky1311RCFreqCali();    // RCƵ��ɨ��
            if(0x10==(ana1Param & 0x10)){       // Ƶ��У׼���
                chkState = NOCHANGE; 
            }
            else{
                delay100ms(10);
            }
        }
        break;
    case LESSEN:                     // ADֵ��С������ֵ���»������С
        if(!cardInField){            // ֮ǰû�п��ڳ���
            sta_result_t sta = readCard_test(); // ���Զ���
            if(sta == Ok){
                cardInField = true;
            }
            else{
                cardInField = false;
                
            }
        }
        chkState = NOCHANGE;    // ������ADֵ
        break;    
    }
#endif
}
/**
 ******************************************************************************
 ** \��  ��  �����в���
 **
 ** \��  ��  none
 ** \����ֵ  none
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
    u8 operaMode;         // ����ģʽ��0�����������1�����Ѱ����2������
    initMCU();
    delay10ms(100);  
    sky1311Enable(); 
  	//delay1ms(5);
  	sky1311WriteCmd(CMD_SW_RST);     // soft reset
  	delay1ms(5);
  	initSky1311s();        // init running param
    readCard_test();        // �ϵ糢�Զ���1��
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
        
        if(operaMode == 0){         /* ������� */
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
        else if(operaMode == 1){    /* ���Ѱ�� */
            LED1 = LED_OFF;
            LED2 = LED_ON;
            LED3 = LED_OFF;
#if CHECKCARD == 1
            delay10ms(34);
            lpCardOpera();
            intoCmdFlag = false;
#endif
            
        }
        else if(operaMode == 2){    /* ��������� */
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

