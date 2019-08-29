
/**
\brief AT86RF231-specific definition of the "radio" bsp module.

\author Thomas Watteyne <watteyne@eecs.berkeley.edu>, February 2012.
*/


#include "board.h"
#include "radio.h"
#include "at86rf231.h"
#include "S2LP_Regs.h"
#include "spi.h"
#include "debugpins.h"
#include "leds.h"
#include "sctimer.h"
//#include "S2LP_Radio.h"
#include "S2LP_Config.h"
#include "S2LP_Regs.h"
#include "SDK_Configuration_Common.h"
#include "zyx.h"
#include "opendefs.h"
#include "IEEE802154E.h"//zyxadd20180619
#include "idmanager.h"//zyxadd20180619
//#define debugradio
#define radio_sleep
//=========================== defines =========================================

//=========================== variables =======================================

typedef struct {
   radio_capture_cbt         startFrame_cb;
   radio_capture_cbt         endFrame_cb;
   radio_state_t             state; 
} radio_vars_t;
//S2LP
radio_vars_t radio_vars;
extern SPI_HandleTypeDef	hspi2;
S2LPIrqs xIrqStatus;
#ifdef USE_2_Radio_Interrupt
__IO PORT_TIMER_WIDTH Globle_capturedTime;
#endif
uint16_t ADC_CNT=0;
extern ieee154e_vars_t    ieee154e_vars;
//=========================== prototypes ======================================

void    radio_spiWriteReg(uint8_t reg_addr, uint8_t reg_setting);
uint8_t radio_spiReadReg(uint8_t reg_addr);
void    radio_spiWriteTxFifo(uint8_t* bufToWrite, uint8_t lenToWrite);
void    radio_spiReadRxFifo(uint8_t* pBufRead,
                            uint8_t* pLenRead,
                            uint8_t  maxBufLen,
                            uint8_t* pLqi);
uint8_t radio_spiReadRadioInfo(void);
//zyx added//
StatusBytes radio_spiWriteS2Reg(uint8_t cRegAddress,uint8_t cNbBytes,uint8_t* pcBuffer);
StatusBytes radio_spiReadS2Reg(uint8_t cRegAddress,uint8_t cNbBytes,uint8_t* pcBuffer);
StatusBytes	radio_spiSendS2CommandStrobes(uint8_t cCommandCode);
StatusBytes	radio_spiWriteS2Fifo(uint8_t cNbBytes,uint8_t* pcBuffer);
StatusBytes	radio_spiReadS2Fifo(uint8_t cNbBytes,uint8_t* pcBuffer);
//=========================== public ==========================================
uint8_t	testspisendbuf[100]={0,0,0,4,5,6,7,8,9,10,1,2,3,4,5,6,7,8,9,0};
uint8_t testspirecbuf[20];
uint8_t izyx;
//===== admin
void radio_init() 
{
	uint8_t BuftoWrite;
	SRadioInit	xRadioInit=
	{
        457500000,//BASE_FREQUENCY,462
		MOD_2FSK,//MODULATION_SELETC,
		250000,//DATARATE,
		61000,//FREQ_DEVIATION,50
		550000//BANDWIDTH
	};
	SGpioInit	xGpioIRQ=
	{
		S2LP_GPIO_3,
		S2LP_GPIO_MODE_DIGITAL_OUTPUT_LP,
		S2LP_GPIO_DIG_OUT_IRQ
	};
	PktBasicInit xBasicInit=
	{
		64,//PREAMBLE_LENGTH,//   lcg 20180523  64=16byte
		32,//SYNC_LENGTH,//
		SYNC_WORD,
		VARIABLE_LENGTH,
		EXTENDED_LENGTH_FIELD,
		CRC_MODE,
		EN_ADDRESS,
		EN_FEC,
		EN_WHITENING	
	};
	StatusBytes status;
	uint8_t tmp;
	do
	{
		PORT_PIN_RADIO_RESET_HIGH();
		//HAL_Delay(1);
		for(volatile uint16_t i=0;i!=0x1500;i++);	//2ms
		 PORT_PIN_RADIO_RESET_LOW();
		// HAL_Delay(1);
		for(volatile uint16_t i=0;i!=0x1600;i++);	//>2ms
		status=SdkEvalSpiReadRegisters(0x8E,1,&tmp);
	}
	while (status.MC_STATE!=MC_STATE_READY);
	

	//1)关闭radio，2)在TRX_END和RX开始时应打开中断;3)清除外部中断标志4)使用片上天线5）打开radio计算CRC
	S2LPRadioSetXtalFrequency(50000000);
#ifdef	USE_FLASH_PARAMETER
	xRadioInit.lFrequencyBase=*(__IO uint8_t *)(ADDR_PARAMENT_STARTADDRESS+OFFSETADDR_FIRST_WIRELESS_CHANNEL);
	xRadioInit.lFrequencyBase=xRadioInit.lFrequencyBase<<18;
	xRadioInit.lFrequencyBase+=413000000;
	
#endif






	
	#if 1////////////////////////初始化参数20180108
	S2LPRadioInit(&xRadioInit);
	S2LPGpioInit(&xGpioIRQ);
	#else
	SpiritBaseConfiguration();
	#endif
	#if 1
	xGpioIRQ.xS2LPGpioPin=S2LP_GPIO_1;//
	xGpioIRQ.xS2LPGpioMode=S2LP_GPIO_MODE_DIGITAL_INPUT;//
	xGpioIRQ.xS2LPGpioIO=S2LP_GPIO_DIG_IN_TX_COMMAND;
	S2LPGpioInit(&xGpioIRQ);
	
	xGpioIRQ.xS2LPGpioPin=S2LP_GPIO_0;//
	xGpioIRQ.xS2LPGpioMode=S2LP_GPIO_MODE_DIGITAL_OUTPUT_LP;//
	xGpioIRQ.xS2LPGpioIO=S2LP_GPIO_DIG_OUT_GND;
	S2LPGpioInit(&xGpioIRQ);
	xGpioIRQ.xS2LPGpioPin=S2LP_GPIO_2;//
	xGpioIRQ.xS2LPGpioMode=S2LP_GPIO_MODE_DIGITAL_OUTPUT_LP;//
	xGpioIRQ.xS2LPGpioIO=S2LP_GPIO_DIG_OUT_GND;
	S2LPGpioInit(&xGpioIRQ);
	#endif
	
	S2LPPktBasicInit(&xBasicInit);
/////	S2LPRadioAfcInit(&xSAfcInit);//lcg 20190309 add
/////	S2LPRadioSetRssiThreshdBm(66);//lcg 20190309 add
	//BuftoWrite=0x42;//0x42对应SLEEPA   配置还在，FIFO丢失
	/*配置SLEEP的状态*/
	BuftoWrite=0x43;//0x43对应SLEEPB 配置还在，FIFO还在,7对应发射功率调到最大
	S2LPSpiWriteRegisters(PM_CONF0_ADDR,1,&BuftoWrite);
	
	#ifdef	USE_FLASH_PARAMETER
	BuftoWrite=*(__IO uint8_t *)(ADDR_PARAMENT_STARTADDRESS+OFFSETADDR_FREQUENCYOFFSET);
	S2LPSpiWriteRegisters(CH_SPACE_ADDR,1,&BuftoWrite);//
	#else
	BuftoWrite=0xFF;
	S2LPSpiWriteRegisters(CH_SPACE_ADDR,1,&BuftoWrite);//frequency space 389.1k
	#endif
	BuftoWrite=0x0F;//检测前导码长度设为3位
	S2LPSpiWriteRegisters(QI_ADDR,1,&BuftoWrite);
	//BuftoWrite=0x79;
	//S2LPSpiWriteRegisters(PM_CONF1_ADDR,1,&BuftoWrite);//打开电池检测状态，将电池阈值设为2.1V
	#ifdef debugradio
		S2LPGpioIrqConfig(RX_DATA_DISC,S_ENABLE);
		S2LPGpioIrqConfig(RX_DATA_READY,S_ENABLE);
		S2LPPktBasicSetPayloadLength(20);
		S2LPTimerSetRxTimerUs(3280000);
		S2LPGpioIrqClearStatus();
		S2LPCmdStrobeRx();
		while(1);
	
	#else
	S2LPCmdStrobeFlushTxFifo();
	/*RX的中断设置*/
	
	/*S2LPGpioIrqConfig(RX_DATA_DISC,S_ENABLE);
	S2LPGpioIrqConfig(RX_DATA_READY,S_ENABLE);
	S2LPGpioIrqConfig(VALID_SYNC,S_ENABLE);
	S2LPGpioIrqConfig(TX_DATA_SENT,S_ENABLE);
	S2LPGpioIrqDeInit(&xIrqStatus);*/
	S2LPGpioIrqDeInit(&xIrqStatus);
	#if 1
	//S2LPGpioIrqConfig(RX_DATA_DISC,S_ENABLE);
	BuftoWrite=0;
	S2LPSpiWriteRegisters(IRQ_MASK0_ADDR,1,0);
	S2LPSpiWriteRegisters(IRQ_MASK1_ADDR,1,0);
	S2LPSpiWriteRegisters(IRQ_MASK2_ADDR,1,0);
	S2LPSpiWriteRegisters(IRQ_MASK3_ADDR,1,0);
	S2LPGpioIrqConfig(RX_DATA_READY,S_ENABLE);//
	S2LPGpioIrqConfig(VALID_SYNC,S_ENABLE);//
	//I am a slave and I am in lower power calling on mode
	if(*(__IO uint8_t *)(ADDR_PARAMENT_STARTADDRESS+OFFSETADDR_MYIP_ADDRESS)!=0&&*(__IO uint8_t *)(ADDR_PARAMENT_STARTADDRESS+OFFSETADDR_LOWERPOWER_CALLINGON)==1)
	{
		S2LPGpioIrqConfig(VALID_PREAMBLE,S_ENABLE);
	}
	//S2LPGpioIrqConfig(VALID_PREAMBLE,S_ENABLE);
	//S2LPGpioIrqConfig(CRC_ERROR,S_ENABLE);
	//S2LPGpioIrqConfig(RX_DATA_DISC,ENABLE);
	//S2LPTimerSetRxTimerUs(3000000);
	#endif
	S2LPGpioIrqClearStatus();
	S2LPRadioSetMaxPALevel(S_DISABLE);
	BuftoWrite=*(__IO uint8_t *)(ADDR_PARAMENT_STARTADDRESS+OFFSETADDR_TRANSMIT_POWER);
	if(BuftoWrite>14)
	{
		BuftoWrite=14;
	}
	S2LPRadioSetPALeveldBm(7,BuftoWrite);//max14 sendPower
	
	S2LPRadioSetPALevelMaxIndex(7);
	//S2LPGpioIrqDeInit(NULL);
	S2LPGpioIrqConfig(TX_DATA_SENT , S_ENABLE);
	S2LPGpioIrqClearStatus();
	#ifdef radio_sleep
	//S2LPCmdStrobeReady();
	///radio_spiSendS2CommandStrobes(RFCMD_SABORT);
	///S2LPCmdStrobeSabort();
	S2LPCmdStrobeSleep();
	//S2LPCmdStrobeStandby();///////////
	#endif
		
	#endif
	//radio_spiSendS2CommandStrobes(RFCMD_SABORT);
	//S2LPCmdStrobeRx();
	//while(1);
	
	//***************************************//
	/*
	S2LPSpiReadRegisters(PCKT_FLT_OPTIONS_ADDR, 1, &BuftoWrite);
	BuftoWrite=BuftoWrite&(~0x40);
	S2LPSpiWriteRegisters(PCKT_FLT_OPTIONS_ADDR, 1, &BuftoWrite);*/
	/*BuftoWrite=0x0A;
	S2LPSpiWriteRegisters(PROTOCOL0_ADDR,1,&BuftoWrite);*/
	//S2LPGpioIrqConfig(TX_DATA_SENT,S_ENABLE);//关闭
	
   // clear variables
   memset(&radio_vars,0,sizeof(radio_vars_t));
   
   // change state
   radio_vars.state          = RADIOSTATE_STOPPED;


   // change state
  
   radio_vars.state          = RADIOSTATE_RFOFF;
}
/*calling when reset redio at the time progme running*/
void radio_reinit() 
{
		uint8_t BuftoWrite;
	SRadioInit	xRadioInit=
	{
        457500000,//BASE_FREQUENCY,462
		MOD_2FSK,//MODULATION_SELETC,
		250000,//DATARATE,
		61000,//FREQ_DEVIATION,50
		550000//BANDWIDTH
	};
	SGpioInit	xGpioIRQ=
	{
		S2LP_GPIO_3,
		S2LP_GPIO_MODE_DIGITAL_OUTPUT_LP,
		S2LP_GPIO_DIG_OUT_IRQ
	};
	PktBasicInit xBasicInit=
	{
		64,//PREAMBLE_LENGTH,//   lcg 20180523  64=16byte
		32,//SYNC_LENGTH,//
		SYNC_WORD,
		VARIABLE_LENGTH,
		EXTENDED_LENGTH_FIELD,
		CRC_MODE,
		EN_ADDRESS,
		EN_FEC,
		EN_WHITENING	
	};
	StatusBytes status;
	uint8_t tmp;
	do
	{
		PORT_PIN_RADIO_RESET_HIGH();
		//HAL_Delay(1);
		for(volatile uint16_t i=0;i!=0x1500;i++);	//2ms
		 PORT_PIN_RADIO_RESET_LOW();
		// HAL_Delay(1);
		for(volatile uint16_t i=0;i!=0x1600;i++);	//>2ms
		status=SdkEvalSpiReadRegisters(0x8E,1,&tmp);
	}
	while (status.MC_STATE!=MC_STATE_READY);
	

	//1)关闭radio，2)在TRX_END和RX开始时应打开中断;3)清除外部中断标志4)使用片上天线5）打开radio计算CRC
	S2LPRadioSetXtalFrequency(50000000);
#ifdef	USE_FLASH_PARAMETER
	xRadioInit.lFrequencyBase=*(__IO uint8_t *)(ADDR_PARAMENT_STARTADDRESS+OFFSETADDR_FIRST_WIRELESS_CHANNEL);
	xRadioInit.lFrequencyBase=xRadioInit.lFrequencyBase<<18;
	xRadioInit.lFrequencyBase+=413000000;
	
#endif






	
	#if 1////////////////////////初始化参数20180108
	S2LPRadioInit(&xRadioInit);
	S2LPGpioInit(&xGpioIRQ);
	#else
	SpiritBaseConfiguration();
	#endif
	#if 1
	xGpioIRQ.xS2LPGpioPin=S2LP_GPIO_1;//
	xGpioIRQ.xS2LPGpioMode=S2LP_GPIO_MODE_DIGITAL_INPUT;//
	xGpioIRQ.xS2LPGpioIO=S2LP_GPIO_DIG_IN_TX_COMMAND;
	S2LPGpioInit(&xGpioIRQ);
	/*
	xGpioIRQ.xS2LPGpioPin=S2LP_GPIO_0;//
	xGpioIRQ.xS2LPGpioMode=S2LP_GPIO_MODE_DIGITAL_INPUT;//
	xGpioIRQ.xS2LPGpioIO=S2LP_GPIO_DIG_IN_TX_COMMAND;
	S2LPGpioInit(&xGpioIRQ);
	xGpioIRQ.xS2LPGpioPin=S2LP_GPIO_2;//
	xGpioIRQ.xS2LPGpioMode=S2LP_GPIO_MODE_DIGITAL_INPUT;//
	xGpioIRQ.xS2LPGpioIO=S2LP_GPIO_DIG_IN_TX_COMMAND;
	S2LPGpioInit(&xGpioIRQ);*/
	#endif
	
	S2LPPktBasicInit(&xBasicInit);
/////	S2LPRadioAfcInit(&xSAfcInit);//lcg 20190309 add
/////	S2LPRadioSetRssiThreshdBm(66);//lcg 20190309 add
	//BuftoWrite=0x42;//0x42对应SLEEPA   配置还在，FIFO丢失
	/*配置SLEEP的状态*/
	BuftoWrite=0x43;//0x43对应SLEEPB 配置还在，FIFO还在,7对应发射功率调到最大
	S2LPSpiWriteRegisters(PM_CONF0_ADDR,1,&BuftoWrite);
	
	#ifdef	USE_FLASH_PARAMETER
	BuftoWrite=0xFF;//BuftoWrite=*(__IO uint8_t *)(ADDR_PARAMENT_STARTADDRESS+OFFSETADDR_FREQUENCYOFFSET);
	S2LPSpiWriteRegisters(CH_SPACE_ADDR,1,&BuftoWrite);//frequency space 389.1k
	#else
	BuftoWrite=0xFF;
	S2LPSpiWriteRegisters(CH_SPACE_ADDR,1,&BuftoWrite);//frequency space 389.1k
	#endif
	BuftoWrite=0x0F;//检测前导码长度设为3位
	S2LPSpiWriteRegisters(QI_ADDR,1,&BuftoWrite);
	//BuftoWrite=0x79;
	//S2LPSpiWriteRegisters(PM_CONF1_ADDR,1,&BuftoWrite);//打开电池检测状态，将电池阈值设为2.1V
	#ifdef debugradio
		S2LPGpioIrqConfig(RX_DATA_DISC,S_ENABLE);
		S2LPGpioIrqConfig(RX_DATA_READY,S_ENABLE);
		S2LPPktBasicSetPayloadLength(20);
		S2LPTimerSetRxTimerUs(3280000);
		S2LPGpioIrqClearStatus();
		S2LPCmdStrobeRx();
		while(1);
	
	#else
	S2LPCmdStrobeFlushTxFifo();
	/*RX的中断设置*/
	
	/*S2LPGpioIrqConfig(RX_DATA_DISC,S_ENABLE);
	S2LPGpioIrqConfig(RX_DATA_READY,S_ENABLE);
	S2LPGpioIrqConfig(VALID_SYNC,S_ENABLE);
	S2LPGpioIrqConfig(TX_DATA_SENT,S_ENABLE);
	S2LPGpioIrqDeInit(&xIrqStatus);*/
	S2LPGpioIrqDeInit(&xIrqStatus);
	#if 1
	//S2LPGpioIrqConfig(RX_DATA_DISC,S_ENABLE);
	BuftoWrite=0;
	S2LPSpiWriteRegisters(IRQ_MASK0_ADDR,1,0);
	S2LPSpiWriteRegisters(IRQ_MASK1_ADDR,1,0);
	S2LPSpiWriteRegisters(IRQ_MASK2_ADDR,1,0);
	S2LPSpiWriteRegisters(IRQ_MASK3_ADDR,1,0);
	S2LPGpioIrqConfig(RX_DATA_READY,S_ENABLE);//
	S2LPGpioIrqConfig(VALID_SYNC,S_ENABLE);//
	//I am a slave and I am in lower power calling on mode
	if(*(__IO uint8_t *)(ADDR_PARAMENT_STARTADDRESS+OFFSETADDR_MYIP_ADDRESS)!=0&&*(__IO uint8_t *)(ADDR_PARAMENT_STARTADDRESS+OFFSETADDR_LOWERPOWER_CALLINGON)==1)
	{
		S2LPGpioIrqConfig(VALID_PREAMBLE,S_ENABLE);
	}
	//S2LPGpioIrqConfig(VALID_PREAMBLE,S_ENABLE);
	//S2LPGpioIrqConfig(CRC_ERROR,S_ENABLE);
	//S2LPGpioIrqConfig(RX_DATA_DISC,ENABLE);
	//S2LPTimerSetRxTimerUs(3000000);
	#endif
	S2LPGpioIrqClearStatus();
	S2LPRadioSetMaxPALevel(S_DISABLE);
	BuftoWrite=*(__IO uint8_t *)(ADDR_PARAMENT_STARTADDRESS+OFFSETADDR_TRANSMIT_POWER);
	if(BuftoWrite>14)
	{
		BuftoWrite=14;
	}
	S2LPRadioSetPALeveldBm(7,BuftoWrite);//max14 sendPower
	
	S2LPRadioSetPALevelMaxIndex(7);
	//S2LPGpioIrqDeInit(NULL);
	S2LPGpioIrqConfig(TX_DATA_SENT , S_ENABLE);
	S2LPGpioIrqClearStatus();
	#ifdef radio_sleep
	//S2LPCmdStrobeReady();
	///radio_spiSendS2CommandStrobes(RFCMD_SABORT);
	///S2LPCmdStrobeSabort();
	S2LPCmdStrobeSleep();
	//S2LPCmdStrobeStandby();///////////
	#endif
		
	#endif
   // change state
   radio_vars.state          = RADIOSTATE_STOPPED;
   radio_vars.state          = RADIOSTATE_RFOFF;
}

void radio_setStartFrameCb(radio_capture_cbt cb) 
{
   radio_vars.startFrame_cb  = cb;
}

void radio_setEndFrameCb(radio_capture_cbt cb) {
   radio_vars.endFrame_cb    = cb;
}

//===== reset

void radio_reset() {
   //PORT_PIN_RADIO_RESET_LOW();
    PORT_PIN_RADIO_RESET_HIGH();//HIGH为不工作
}

//===== RF admin

void radio_setFrequency(uint8_t frequency) 
{
   // change state
   uint8_t fre_zyx=0;//frequency<<1;
   radio_vars.state = RADIOSTATE_SETTING_FREQUENCY;
   
   //configure the radio to the right frequecy
   //radio_spiWriteReg(RG_PHY_CC_CCA,0x20+frequency);
  //S2LPSpiWriteRegisters(CHNUM_ADDR,1,&frequency);//zyx
  S2LPSpiWriteRegisters(CHNUM_ADDR,1,&fre_zyx);
   // change state
   radio_vars.state = RADIOSTATE_FREQUENCY_SET;
}
void radio_setPreamble_len(uint8_t Preamble_len_Bytes)
{
	uint8_t temp;
	uint16_t Preamlen;
	Preamlen=Preamble_len_Bytes;
	Preamlen=Preamlen<<2;

	S2LPSpiReadRegisters(PCKTCTRL6_ADDR,1,&temp);
	temp&=0xFC;//last two bits set zero
	temp=temp|(Preamlen>>8);
	S2LPSpiWriteRegisters(PCKTCTRL6_ADDR,1,&temp);
	temp=Preamlen;
	S2LPSpiWriteRegisters(PCKTCTRL5_ADDR,1,&temp);
}
void radio_ENABLE_PREAMBLE_INTERRUPT(void)
{
	S2LPGpioIrqConfig(VALID_PREAMBLE,S_ENABLE);
}
void radio_DISABLE_PREAMBLE_INTERRUPT(void)
{
	S2LPGpioIrqConfig(VALID_PREAMBLE,S_DISABLE);
}
void radio_rfOn() {
   //PORT_PIN_RADIO_RESET_LOW();
   //PORT_PIN_RADIO_RESET_HIGH();//open the function20180308zyx
}

void radio_rfOff() {
   // change state
   
   // turn radio off
   if(radio_vars.state!= RADIOSTATE_TURNING_OFF)//zyx changed the logic 
   {
   	   radio_vars.state = RADIOSTATE_TURNING_OFF;
	   #ifdef radio_sleep
	   radio_spiSendS2CommandStrobes(RFCMD_SABORT);
	   S2LPCmdStrobeSleep();
	   #else
	   radio_spiSendS2CommandStrobes(RFCMD_SABORT);//ready
	   #endif
   }

   //S2LPGpioIrqClearStatus();
   #if 0
   radio_spiWriteReg(RG_TRX_STATE, CMD_FORCE_TRX_OFF);
   radio_spiWriteReg(RG_TRX_STATE, CMD_TRX_OFF);
   #endif
   // wiggle debug pin
   debugpins_radio_clr();
   leds_radio_off();
   
   // change state
   radio_vars.state = RADIOSTATE_RFOFF;
}

//===== TX

void radio_loadPacket(uint8_t* packet, uint16_t len) {
#if 0
	uint16_t zyxtest6;
	for(zyxtest6=0;zyxtest6<len;zyxtest6++)
		{
		printf("%x ",packet[zyxtest6]);
		//packet[zyxtest6]=zyxtest6;
		}
	printf("-----------------------\n");
	#endif
   // change state
   radio_vars.state = RADIOSTATE_LOADING_PACKET;
   
   // load packet in TXFIFO
   #if 0//zyx
   radio_spiWriteTxFifo(packet,len);
   #endif
   
   #ifdef radio_sleep
    
   //S2LPCmdStrobeReady();//TXEnable

   #endif
   S2LPCmdStrobeFlushTxFifo();
   S2LPPktBasicSetPayloadLength(len);
   S2LPSpiWriteFifo(len,packet);
   S2LPCmdStrobeLockTx();
   ///radio_txNow();
   // change state
   radio_vars.state = RADIOSTATE_PACKET_LOADED;
}

void radio_txEnable() {
   // change state
   radio_vars.state = RADIOSTATE_ENABLING_TX;
   
   // wiggle debug pin
   debugpins_radio_set();
   leds_radio_on();
   // turn on radio's PLL
   //radio_spiWriteReg(RG_TRX_STATE, CMD_PLL_ON);
   ////////////S2LPCmdStrobeTx();
   //radio_spiWriteReg(0x76,0x9C);//S2LP原版程序就是这样写的，没理解什么意思
   //radio_spiSendS2CommandStrobes(CMD_TX);//此处应配置芯片外部中断，等待发送完成的中断完成继续往下执行
   //while((radio_spiReadReg(RG_TRX_STATUS) & 0x1F) != PLL_ON); // busy wait until done
   
   // change state
   radio_vars.state = RADIOSTATE_TX_ENABLED;
   S2LPCmdStrobeReady();
}

void radio_txNow() {
   PORT_TIMER_WIDTH val;
   // change state
   radio_vars.state = RADIOSTATE_TRANSMITTING;
   
   // send packet by pulsing the SLP_TR_CNTL pin
   //PORT_PIN_RADIO_SLP_TR_CNTL_HIGH();
   //PORT_PIN_RADIO_SLP_TR_CNTL_LOW();
   //S2LPCmdStrobeTx();
   
   //HAL_Delay(1);
   HAL_GPIO_WritePin(GPIO_RFEXTI1,GPIO_PIN_RFEXTI1,GPIO_PIN_SET);
   HAL_GPIO_WritePin(GPIO_RFEXTI1,GPIO_PIN_RFEXTI1,GPIO_PIN_RESET);
//	#ifdef DAGROOT//
	if(ieee154e_vars.state==S_TXDATADELAY&&idmanager_getIsDAGroot())
	{
		HAL_GPIO_WritePin(GPIO_SecPulse,GPIO_PIN_SecPulse,GPIO_PIN_SET);
		HAL_GPIO_WritePin(GPIO_SecPulse,GPIO_PIN_SecPulse,GPIO_PIN_RESET);
		ADC_CNT=(ADC_CNT+1)&0x3F;
	}
//	#endif
   // The AT86RF231 does not generate an interrupt when the radio transmits the
   // SFD, which messes up the MAC state machine. The danger is that, if we leave
   // this funtion like this, any radio watchdog timer will expire.
   // Instead, we cheat an mimick a start of frame event by calling
   // ieee154e_startOfFrame from here. This also means that software can never catch
   // a radio glitch by which #radio_txEnable would not be followed by a packet being
   // transmitted (I've never seen that).
   if (radio_vars.startFrame_cb!=NULL) 
   {
      // call the callback
      val=sctimer_readCounter();
      radio_vars.startFrame_cb(val);
   }
}

//===== RX

void radio_rxEnable() {
   // change state
   radio_vars.state = RADIOSTATE_ENABLING_RX;
   
   //S2LPTimerSetRxTimerUs(3000000);//////////////////////////
   /*radio_spiWriteReg(0x76,0x90);//不知道为什么要往此地址写入此数
   radio_spiSendS2CommandStrobes(CMD_RX);*/
  // S2LPGpioIrqDeInit(&xIrqStatus);
	#if 0
	//S2LPGpioIrqDeInit(&xIrqStatus);
	S2LPGpioIrqConfig(RX_DATA_DISC,S_ENABLE);
	S2LPGpioIrqConfig(RX_DATA_READY,S_ENABLE);
	S2LPGpioIrqConfig(VALID_SYNC,S_ENABLE);
	S2LPTimerSetRxTimerUs(3280000);
	#endif
	#ifdef radio_sleep
	S2LPCmdStrobeReady();
	S2LPCmdStrobeFlushRxFifo();
	//S2LPGpioIrqClearStatus();
	
	//HAL_GPIO_WritePin(GPIO_LED1,GPIO_PIN_LED1,GPIO_PIN_RESET);
	#endif
	/////////////////S2LPCmdStrobeRx();
   // put radio in reception mode
   #if 0
   radio_spiWriteReg(RG_TRX_STATE, CMD_RX_ON);
   #endif
   
   // wiggle debug pin
   debugpins_radio_set();
   leds_radio_on();
   
   // busy wait until radio really listening
   #if 0//S2芯片不需要等待
   while((radio_spiReadReg(RG_TRX_STATUS) & 0x1F) != RX_ON);
   #endif
   // change state
   radio_vars.state = RADIOSTATE_LISTENING;
}

void radio_rxNow() {
	//HAL_GPIO_WritePin(GPIO_LED1,GPIO_PIN_LED1,GPIO_PIN_SET);
   // nothing to do
   S2LPCmdStrobeLockRx();
   for(uint16_t i=0x99;i>0;i--);//delay
   S2LPCmdStrobeRx();
}
void radio_getReceivedFrame(uint8_t* pBufRead,
                            uint8_t* pLenRead,
                            uint8_t  maxBufLen,
                             int8_t* pRssi,
                            uint8_t* pLqi,
                               bool* pCrc) {
   uint8_t tmp;
   uint8_t cRxData;
   //===== crc     
   
   S2LPSpiReadRegisters(PCKT_FLT_OPTIONS_ADDR, 1,&tmp);
   
   if(tmp!=0x41)
   {
   		printc("PCKT_FLT=%x\n",tmp);
   		 #if 1
   		leds_error_toggle();
   		leds_error_toggle();
   		leds_error_toggle();

   		#endif
   }
   *pCrc=(bool)(tmp&0x01);

   //===== rssi
   // as per section 8.4.3 of the AT86RF231, the RSSI is calculate as:
   // -91 + ED [dBm]
   *pRssi=(int8_t)S2LPRadioGetRssidBm();//获取DBM数值
   
   //===== packet
   #if 1
   cRxData = S2LPFifoReadNumberBytesRxFifo();
   
   *pLenRead=cRxData;
   
   S2LPSpiReadFifo(cRxData, pBufRead);
   
   #else
   *pLenRead=S2LPFifoReadNumberBytesRxFifo();
   S2LPCmdStrobeFlushRxFifo();
   
   S2LPSpiReadFifo(cRxData, pBufRead);
   #endif
   //////////S2LPCmdStrobeRx();
   #if 0
   radio_spiReadRxFifo(pBufRead,
                       pLenRead,
                       maxBufLen,
                       pLqi);
   #endif
}

//=========================== private =========================================


uint8_t radio_spiReadRadioInfo(){
  uint8_t	spi_tx_buffer[3];
	uint8_t spi_rx_buffer[3];
	spi_tx_buffer[0]=1;
	spi_tx_buffer[1]=DEVICE_INFO1_ADDR;//读取芯片ID，此处应填写ID存储位置
	spi_tx_buffer[2]=0;
	HAL_SPI_TransmitReceive(&hspi2,spi_tx_buffer,spi_rx_buffer,3,1000);
	return spi_rx_buffer[2];
}

void radio_spiWriteReg(uint8_t reg_addr, uint8_t reg_setting) {
   uint8_t spi_tx_buffer[3];
   uint8_t spi_rx_buffer[3];
   
   //spi_tx_buffer[0] = (0xC0 | reg_addr);        // turn addess in a 'reg write' address
   //spi_tx_buffer[1] = reg_setting;
   spi_tx_buffer[0]=0;
   spi_tx_buffer[1]=reg_addr;
   spi_tx_buffer[2]=reg_setting;
   HAL_SPI_TransmitReceive(&hspi2,spi_tx_buffer,spi_rx_buffer,3,1000);
}
StatusBytes radio_spiWriteS2Reg(uint8_t cRegAddress,uint8_t cNbBytes,uint8_t* pcBuffer)
{
	uint8_t tx_buff[2]={WRITE_HEADER,cRegAddress};
	uint8_t rx_buff[255];
	StatusBytes status;
	//进入临界段程序未加
	HAL_GPIO_WritePin(GPIO_RFCS, GPIO_PIN_RFCS,GPIO_PIN_RESET);
	HAL_SPI_TransmitReceive(&hspi2,tx_buff,rx_buff,2,1000);
	HAL_SPI_TransmitReceive(&hspi2,pcBuffer,&rx_buff[2],cNbBytes,1000);
	HAL_GPIO_WritePin(GPIO_RFCS,GPIO_PIN_RFCS,GPIO_PIN_SET);
	//退出临界段程序未加
	((uint8_t*)&status)[1]=rx_buff[0];
	((uint8_t*)&status)[0]=rx_buff[1];
	return status;
	
}
uint8_t radio_spiReadReg(uint8_t reg_addr) {
	uint8_t	spi_tx_buffer[3];
	uint8_t spi_rx_buffer[3];
	spi_tx_buffer[0]=1;
	spi_tx_buffer[1]=reg_addr;
	spi_tx_buffer[2]=0;
	HAL_SPI_TransmitReceive(&hspi2,spi_tx_buffer,spi_rx_buffer,3,1000);
	return spi_rx_buffer[2];
}
StatusBytes radio_spiReadS2Reg(uint8_t cRegAddress,uint8_t cNbBytes,uint8_t* pcBuffer)
{
	uint8_t tx_buff[255]={READ_HEADER,cRegAddress};
	uint8_t	rx_buff[2];
	StatusBytes	status;
	//进入临界态代码暂时没加
	HAL_GPIO_WritePin(GPIO_RFCS,GPIO_PIN_RFCS,GPIO_PIN_RESET);
	HAL_SPI_TransmitReceive(&hspi2, tx_buff, rx_buff, 2, 1000);
	HAL_SPI_TransmitReceive(&hspi2, tx_buff, pcBuffer, cNbBytes, 1000);
	HAL_GPIO_WritePin(GPIO_RFCS,GPIO_PIN_RFCS,GPIO_PIN_SET);
	//退出临界态代码暂时没加
	((uint8_t*)&status)[1]=rx_buff[0];
	((uint8_t*)&status)[0]=rx_buff[1];  
	return status;
}
/** for testing purposes, remove if not needed anymore**/
StatusBytes	radio_spiSendS2CommandStrobes(uint8_t cCommandCode)
{
	uint8_t tx_buff[2]={COMMAND_HEADER,cCommandCode};
	uint8_t rx_buff[2];
	StatusBytes status;
	//进入临界态代码暂时未加
	HAL_GPIO_WritePin(GPIO_RFCS,GPIO_PIN_RFCS,GPIO_PIN_RESET);
	HAL_SPI_TransmitReceive(&hspi2,tx_buff,rx_buff,2,1000);
	HAL_GPIO_WritePin(GPIO_RFCS,GPIO_PIN_RFCS,GPIO_PIN_SET);
	//退出临界态代码暂时未加
	((uint8_t*)&status)[1]=rx_buff[0];
	((uint8_t*)&status)[0]=rx_buff[1];

	return status;
}
void radio_spiWriteTxFifo(uint8_t* bufToWrite, uint8_t  lenToWrite) {

	S2LPPktBasicSetPayloadLength(lenToWrite);
	 S2LPCmdStrobeFlushTxFifo();
	S2LPSpiWriteFifo(lenToWrite,bufToWrite);

#if 0
   uint8_t spi_tx_buffer[2];
   uint8_t spi_rx_buffer[1+1+127];               // 1B SPI address, 1B length, max. 127B data
   
   spi_tx_buffer[0] = 0x60;                      // SPI destination address for TXFIFO
   spi_tx_buffer[1] = lenToWrite;                // length byte
   
   spi_txrx(spi_tx_buffer,
            sizeof(spi_tx_buffer),
            SPI_BUFFER,
            spi_rx_buffer,
            sizeof(spi_rx_buffer),
            SPI_FIRST,
            SPI_NOTLAST);
   
   spi_txrx(bufToWrite,
            lenToWrite,
            SPI_BUFFER,
            spi_rx_buffer,
            sizeof(spi_rx_buffer),
            SPI_NOTFIRST,
            SPI_LAST);
#endif

}



void radio_spiReadRxFifo(uint8_t* pBufRead,
                         uint8_t* pLenRead,
                         uint8_t  maxBufLen,
                         uint8_t* pLqi) {
   // when reading the packet over SPI from the RX buffer, you get the following:
   // - *[1B]     dummy byte because of SPI
   // - *[1B]     length byte
   // -  [0-125B] packet (excluding CRC)
   // - *[2B]     CRC
   // - *[1B]     LQI
   uint8_t spi_tx_buffer[125];
   uint8_t spi_rx_buffer[3];
   
   spi_tx_buffer[0] = 0x20;
   
   // 2 first bytes
   spi_txrx(spi_tx_buffer,
            2,
            SPI_BUFFER,
            spi_rx_buffer,
            sizeof(spi_rx_buffer),
            SPI_FIRST,
            SPI_NOTLAST);
   
   *pLenRead  = spi_rx_buffer[1];
   
   if (*pLenRead>2 && *pLenRead<=127) {
      // valid length
      
      //read packet
      spi_txrx(spi_tx_buffer,
               *pLenRead,
               SPI_BUFFER,
               pBufRead,
               125,
               SPI_NOTFIRST,
               SPI_NOTLAST);
      
      // CRC (2B) and LQI (1B)
      spi_txrx(spi_tx_buffer,
               2+1,
               SPI_BUFFER,
               spi_rx_buffer,
               3,
               SPI_NOTFIRST,
               SPI_LAST);
      
      *pLqi   = spi_rx_buffer[2];
      
   } else {
      // invalid length
      
      // read a just byte to close spi
      spi_txrx(spi_tx_buffer,
               1,
               SPI_BUFFER,
               spi_rx_buffer,
               sizeof(spi_rx_buffer),
               SPI_NOTFIRST,
               SPI_LAST);
   }
   S2LPCmdStrobeFlushRxFifo();//??????????????????????????????????????????????????????????????????????????????????

}
StatusBytes radio_spiWriteS2Fifo(uint8_t cNbBytes, uint8_t* pcBuffer)
{
	uint8_t tx_buff[2]={WRITE_HEADER,LINEAR_FIFO_ADDRESS};
	uint8_t rx_buff[130];
	StatusBytes status;

	//SPI_ENTER_CRITICAL();
	HAL_GPIO_WritePin(GPIO_RFCS,GPIO_PIN_RFCS,GPIO_PIN_RESET);

	HAL_SPI_TransmitReceive(&hspi2, tx_buff, rx_buff, 2, 1000);
	HAL_SPI_TransmitReceive(&hspi2, pcBuffer, &rx_buff[2], cNbBytes, 1000);
	HAL_GPIO_WritePin(GPIO_RFCS,GPIO_PIN_RFCS,GPIO_PIN_SET);
	//SPI_EXIT_CRITICAL();

	((uint8_t*)&status)[1]=rx_buff[0];
	((uint8_t*)&status)[0]=rx_buff[1];

	return status;
}
StatusBytes radio_ReadS2Fifo(uint8_t cNbBytes, uint8_t* pcBuffer)
{
	uint8_t tx_buff[130]={READ_HEADER,LINEAR_FIFO_ADDRESS};
	uint8_t rx_buff[2];
	StatusBytes status;

	//SPI_ENTER_CRITICAL();
	HAL_GPIO_WritePin(GPIO_RFCS,GPIO_PIN_RFCS,GPIO_PIN_RESET);

	HAL_SPI_TransmitReceive(&hspi2, tx_buff, rx_buff, 2, 1000);
	HAL_SPI_TransmitReceive(&hspi2, tx_buff, pcBuffer, cNbBytes, 1000);
	HAL_GPIO_WritePin(GPIO_RFCS,GPIO_PIN_RFCS,GPIO_PIN_SET);
	//SPI_EXIT_CRITICAL();

	((uint8_t*)&status)[1]=rx_buff[0];
	((uint8_t*)&status)[0]=rx_buff[1];

	return status;
}



//=========================== callbacks =======================================
uint8_t cRxData;
uint8_t vectcRxBuff[255];
uint8_t ii;
//=========================== interrupt handlers ==============================

kick_scheduler_t radio_isr() {
	   PORT_TIMER_WIDTH capturedTime;
	   //uint8_t temp[4];
	   // capture the time
//#ifndef DAGROOT
			//HAL_GPIO_WritePin(GPIO_SecPulse,GPIO_PIN_SecPulse,GPIO_PIN_SET);
			//HAL_GPIO_WritePin(GPIO_SecPulse,GPIO_PIN_SecPulse,GPIO_PIN_RESET);
	if(ieee154e_vars.state==S_RXDATALISTEN&&idmanager_getIsDAGroot()==FALSE)
	{
		HAL_GPIO_WritePin(GPIO_SecPulse,GPIO_PIN_SecPulse,GPIO_PIN_SET);
		HAL_GPIO_WritePin(GPIO_SecPulse,GPIO_PIN_SecPulse,GPIO_PIN_RESET);
		ADC_CNT=(ADC_CNT+1)&0x3F;
	}
//#endif
	   capturedTime = sctimer_readCounter();
	 #ifdef USE_2_Radio_Interrupt
	        Globle_capturedTime=capturedTime;
	 		EXTI->SWIER1|=0x04;
	 #else

	   RCC_Wakeup();//加在此处是否合理



	   // reading IRQ_STATUS causes radio's IRQ pin to go low
	   //HAL_GPIO_WritePin(GPIO_LED1,GPIO_PIN_LED1,GPIO_PIN_SET);
	   S2LPGpioIrqGetStatus(&xIrqStatus);
	   //HAL_GPIO_WritePin(GPIO_LED1,GPIO_PIN_LED1,GPIO_PIN_RESET);
	   /*S2LPSpiReadRegisters(IRQ_MASK3_ADDR,4,temp);
	   	printf("a=%x",temp[0]);
		printf("b=%x",temp[1]);
		printf("c=%x",temp[2]);
		printf("d=%x\n",temp[3]);*/

	   // start of frame event

	   /*if (xIrqStatus.IRQ_RX_DATA_DISC)//TIMEOUT
	   	{
	   		//HAL_GPIO_TogglePin(GPIO_LED4,GPIO_PIN_LED4);
	   		//printf("over\n");
	   		//S2LPCmdStrobeRx();//如果接受超时，则继续接收，发送时应把此处打开接收关闭掉
	   		//radio_rxEnable();
	   		return DO_NOT_KICK_SCHEDULER;
	   	}*/

	   if(xIrqStatus.IRQ_VALID_SYNC)////原程序判断标志AT_IRQ_RX_START
	   {
	      // change state
	       radio_vars.state = RADIOSTATE_RECEIVING;
		  //HAL_GPIO_TogglePin(GPIO_LED1,GPIO_PIN_LED1);
		  //HAL_GPIO_WritePin(GPIO_LED1,GPIO_PIN_LED1,GPIO_PIN_SET);
	      if (radio_vars.startFrame_cb!=NULL) {
	         // call the callback
	         radio_vars.startFrame_cb(capturedTime);
	         // kick the OS
	         //HAL_GPIO_WritePin(GPIO_LED1,GPIO_PIN_LED1,GPIO_PIN_RESET);
	         return KICK_SCHEDULER;

	      } else 
              {
	         while(1);
	      }
	   }
	   // end of frame event
	   if (xIrqStatus.IRQ_TX_DATA_SENT||xIrqStatus.IRQ_RX_DATA_READY) //AT_IRQ_TRX_END
	   {
	      // change state
	      //HAL_GPIO_WritePin(GPIO_LED1,GPIO_PIN_LED1,GPIO_PIN_SET);
	      #ifdef debugradio
			cRxData=S2LPFifoReadNumberBytesRxFifo();
			S2LPSpiReadFifo(cRxData, vectcRxBuff);
			S2LPCmdStrobeFlushRxFifo();
			S2LPCmdStrobeRx();
	   	        printf("data received\n");
			printf("%d\n",cRxData);
			for(ii=0;ii<cRxData;ii++)
				{
				printf("%x",vectcRxBuff[ii]);
				}
			printf("\n");
			#else

	      radio_vars.state = RADIOSTATE_TXRX_DONE;
		  /*#ifdef radio_sleep
		  	if(xIrqStatus.IRQ_TX_DATA_SENT)
	    		S2LPCmdStrobeSleep();
		  #endif*/
	      if (radio_vars.endFrame_cb!=NULL)
	      {
	         // call the callback
	         radio_vars.endFrame_cb(capturedTime);
	         // kick the OS
	         return KICK_SCHEDULER;
	      } else {
	         while(1);
	      }
			#endif

	   }

	  


	   /*if(xIrqStatus.IRQ_TX_FIFO_ERROR)
	   	{
	   		S2LPCmd(CMD_FLUSHTXFIFO);
	   	}*/
	   //清中断，保证下次还能进入zyx20171218
	   //S2LPGpioIrqClearStatus();
	   //S2LPGpioIrqGetStatus(&xIrqStatus);
	    //HAL_GPIO_WritePin(GPIO_LED1,GPIO_PIN_LED1,GPIO_PIN_RESET);
	 #endif
	   return DO_NOT_KICK_SCHEDULER;
}
#ifdef USE_2_Radio_Interrupt
kick_scheduler_t radio_isr_second(PORT_TIMER_WIDTH capturedTime)
{
	   RCC_Wakeup();//加在此处是否合理
	   S2LPGpioIrqGetStatus(&xIrqStatus);
	   
		if (xIrqStatus.IRQ_TX_DATA_SENT||xIrqStatus.IRQ_RX_DATA_READY) //AT_IRQ_TRX_END
		{
		  // change state
		  //HAL_GPIO_WritePin(GPIO_LED1,GPIO_PIN_LED1,GPIO_PIN_SET);
      #ifdef debugradio
			cRxData=S2LPFifoReadNumberBytesRxFifo();
			S2LPSpiReadFifo(cRxData, vectcRxBuff);
			S2LPCmdStrobeFlushRxFifo();
			S2LPCmdStrobeRx();
			printf("data received\n");
			printf("%d\n",cRxData);
			for(ii=0;ii<cRxData;ii++)
			{
				printf("%x",vectcRxBuff[ii]);
			}
			printf("\n");
		#else		

		  radio_vars.state = RADIOSTATE_TXRX_DONE;
		  if (radio_vars.endFrame_cb!=NULL)
		  {
			 // call the callback
			 radio_vars.endFrame_cb(capturedTime);
			 // kick the OS
			 return KICK_SCHEDULER;
		  } else {
			 while(1);
		  }
		#endif

	   }

	   if(xIrqStatus.IRQ_VALID_SYNC)////原程序判断标志AT_IRQ_RX_START
	   {
		  // change state
		   radio_vars.state = RADIOSTATE_RECEIVING;
		  //HAL_GPIO_TogglePin(GPIO_LED1,GPIO_PIN_LED1);
		  //HAL_GPIO_WritePin(GPIO_LED1,GPIO_PIN_LED1,GPIO_PIN_SET);
		  if (radio_vars.startFrame_cb!=NULL) {
			 // call the callback
			 radio_vars.startFrame_cb(capturedTime);
			 // kick the OS
			 //HAL_GPIO_WritePin(GPIO_LED1,GPIO_PIN_LED1,GPIO_PIN_RESET);
			 /*如果是接收模块，则受到同步字中断后给FCI发送脉冲，限制条件，IEEE802.154E状态在侦听同步字阶段20180619，并记录当前时刻的ASN数值*/

			 return KICK_SCHEDULER;

		  } else {
			 while(1);
		  }
	   }
		if(xIrqStatus.IRQ_VALID_PREAMBLE)/*when it receive a preamble*/
		{
			activity_si2();
			return KICK_SCHEDULER;
		}

		if(xIrqStatus.IRQ_RX_DATA_DISC) 
		{
			//S2LPCmdStrobeRx();//zyx added 20181205
			leds_error_toggle();
			printc("IRQ_RX_DATA_DISC\n");
			endSlot();
			return KICK_SCHEDULER;
		}
		if(xIrqStatus.IRQ_TX_FIFO_ERROR)
		{
			leds_error_toggle();
			printc("TXFIFOERROR\n");
			S2LPCmdStrobeFlushTxFifo();
			endSlot();
			return KICK_SCHEDULER;
		}
		if(xIrqStatus.IRQ_RX_FIFO_ERROR)
		{
			leds_error_toggle();
			printc("RXFIFOERROR\n");
			S2LPCmdStrobeFlushRxFifo();
			endSlot();
			return KICK_SCHEDULER;
		}
		leds_error_toggle();
		printc("radio error interrupt=%x\n",xIrqStatus);
	   // end of frame event
	   
 
	
	   return DO_NOT_KICK_SCHEDULER;

}

#endif

