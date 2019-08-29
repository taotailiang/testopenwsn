/**
\brief openmoteSTM32 definition of the "uart" bsp module.

\author Chang Tengfei <tengfei.chang@gmail.com>,  July 2012.
*/

//#include "stm32f10x_lib.h"//zyx
#include "stm32l4xx_hal.h"
#include "stdio.h"
#include "stdint.h"
#include "string.h"
#include "uart.h"
#include "leds.h"
#include <stdio.h>
#include "rcc.h"
#include "nvic.h"
#include "zyx.h"
#include "opendefs.h"
#include "openserial.h"
//#define OPEN_USART//openwsn interrupt usart


//=========================== defines =========================================

//=========================== variables =======================================

typedef struct {
    uart_tx_cbt txCb;
    uart_rx_cbt rxCb;
    uint8_t     startOrend;
    uint8_t     flagByte;
    bool        flag;
} uart_vars_t;

uart_vars_t uart_vars;
UART_HandleTypeDef huart1;//
#ifdef OPEN_DEBUG_USART2
UART_HandleTypeDef huart2;//
DMA_HandleTypeDef hdma_usart2_tx;
#endif

DMA_HandleTypeDef hdma_usart1_rx;
DMA_HandleTypeDef hdma_usart1_tx;
uint8_t Uart_TX_BUF[1024];
uint8_t Uart_RX_BUF[1024];
uint16_t Uart_Tx_BUF_LEN;
extern bool Last_Packet_SendDone;

//=========================== prototypes ======================================

//=========================== public ==========================================

void uart_init() {
    uint8_t vectcTxBuff[20]={1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0};
    // reset local variables
    memset(&uart_vars,0,sizeof(uart_vars_t));
    
    //when this value is 0, we are send the first data
    uart_vars.startOrend = 0;
    __HAL_RCC_DMA1_CLK_ENABLE();
    //flag byte for start byte and end byte
    #if 1//#ifdef OPEN_USART
    uart_vars.flagByte = 0x7E;
	huart1.Instance = USART1;
	huart1.Init.BaudRate =115200;
	huart1.Init.WordLength = UART_WORDLENGTH_8B;
	huart1.Init.StopBits = UART_STOPBITS_1;
	huart1.Init.Parity = UART_PARITY_NONE;
	huart1.Init.Mode = UART_MODE_TX_RX;
	huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	huart1.Init.OverSampling = UART_OVERSAMPLING_16;//
	huart1.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
	huart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
	
/*
	huart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_AUTOBAUDRATE_INIT;
	huart1.AdvancedInit.AutoBaudRateEnable = UART_ADVFEATURE_AUTOBAUDRATE_ENABLE;
	huart1.AdvancedInit.AutoBaudRateMode = UART_ADVFEATURE_AUTOBAUDRATE_ONSTARTBIT;*/
	if (HAL_MultiProcessor_Init(&huart1, 0, UART_WAKEUPMETHOD_IDLELINE) != HAL_OK)
	{
		while(1);//_Error_Handler(__FILE__, __LINE__);
	}
	#endif

	#ifdef	OPEN_DEBUG_USART2
	huart2.Instance = USART2;
	huart2.Init.BaudRate =115200;
	huart2.Init.WordLength = UART_WORDLENGTH_8B;
	huart2.Init.StopBits = UART_STOPBITS_1;
	huart2.Init.Parity = UART_PARITY_NONE;
	huart2.Init.Mode = UART_MODE_TX;//UART_MODE_TX_RX;//only tx enough zyx 
	huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	huart2.Init.OverSampling = UART_OVERSAMPLING_16;//
	huart2.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
	huart2.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
	if (HAL_MultiProcessor_Init(&huart2, 0, UART_WAKEUPMETHOD_IDLELINE) != HAL_OK)
	{
		while(1);//_Error_Handler(__FILE__, __LINE__);
	}
	#endif
	HAL_NVIC_SetPriority(DMA1_Channel4_IRQn,3,3);
	HAL_NVIC_SetPriority(DMA1_Channel5_IRQn,3,3);
	#ifdef	OPEN_DEBUG_USART2
	HAL_NVIC_EnableIRQ(DMA1_Channel7_IRQn);
	HAL_NVIC_SetPriority(DMA1_Channel7_IRQn,3,3);
	#endif
	HAL_NVIC_EnableIRQ(DMA1_Channel4_IRQn);
	HAL_NVIC_EnableIRQ(DMA1_Channel5_IRQn);
	
	//HAL_UART_Transmit(&huart1,vectcTxBuff,20,2000);
	///printf("let us begin our game!\n");
	
}

void uart_setCallbacks(uart_tx_cbt txCb, uart_rx_cbt rxCb) {
    
    uart_vars.txCb = txCb;
    uart_vars.rxCb = rxCb;
    
    //enable nvic uart.
    #ifdef OPEN_USART
     NVIC_uart();
	#endif
}

void uart_enableInterrupts(){
#ifdef OPEN_USART
    __HAL_UART_ENABLE_IT(&huart2, UART_IT_RXNE);  //lcg  20180413
    __HAL_UART_ENABLE_IT(&huart2, UART_IT_TXE);  //lcg  20180413
#endif
    //USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);//zyx
}

void uart_disableInterrupts(){
#ifdef OPEN_USART
    __HAL_UART_DISABLE_IT(&huart2, UART_IT_RXNE);
    __HAL_UART_DISABLE_IT(&huart2, UART_IT_TXE);
#endif
    //USART_ITConfig(USART3, USART_IT_TXE, DISABLE);//zyx
    //USART_ITConfig(USART3, USART_IT_RXNE, DISABLE);//zyx
}

void uart_disableTxInterrupts()
{
#ifdef OPEN_USART
    __HAL_UART_DISABLE_IT(&huart2, UART_IT_TXE);
#endif
    //USART_ITConfig(USART2, USART_IT_TXE, DISABLE);//zyx
    //USART_ITConfig(USART2, USART_IT_RXNE, DISABLE);//zyx
}

void uart_clearRxInterrupts()
{
#ifdef OPEN_USART
	__HAL_UART_CLEAR_FLAG(&huart2, UART_CLEAR_RTOF);//zyx20180306
#endif
    //__HAL_UART_CLEAR_FLAG(huart1,);
    //USART_ClearFlag(USART1,USART_FLAG_RXNE);//zyx
}

void uart_clearTxInterrupts()
{
#ifdef OPEN_USART
    __HAL_UART_CLEAR_FLAG(&huart2, UART_CLEAR_TCF);
#endif
    //USART_ClearFlag(USART1,USART_FLAG_TXE);//zyx
}

void uart_writeByte(uint8_t byteToWrite) 
{
#ifdef OPEN_USART
	uart_clearTxInterrupts();
	huart2.Instance->TDR=byteToWrite;
	//while(huart2.TxXferCount > 0);



	//HAL_UART_Transmit(&huart1,&byteToWrite, 1,1000);
#endif
	#if 0
	
	 	 
	while((USART1->ISR&0X40)==0);//循环发送,直到发送完毕	 
	USART1->TDR = byteToWrite;
	#endif
    //start or end byte?
    #if  0//def 0 //OPEN_USART
    if(byteToWrite == uart_vars.flagByte)
	{
        uart_vars.startOrend = (uart_vars.startOrend == 0)?1:0;
        //start byte
        if(uart_vars.startOrend == 1) 
			{
            __HAL_UART_ENABLE_IT(&huart1, UART_IT_TXE);
        } else 
        {
            __HAL_UART_DISABLE_IT(&huart1, UART_IT_TXE);
        }
    }
	#endif
	
	
    #if 0//zyx
    USART_SendData(USART1,(uint16_t)byteToWrite);
    while(USART_GetFlagStatus(USART1,USART_FLAG_TXE) == RESET);
    //start or end byte?
    if(byteToWrite == uart_vars.flagByte){
        uart_vars.startOrend = (uart_vars.startOrend == 0)?1:0;
        //start byte
        if(uart_vars.startOrend == 1) {
            USART_ITConfig(USART1, USART_IT_TXE, ENABLE);
        } else {
            USART_ITConfig(USART1, USART_IT_TXE, DISABLE);
        }
    }
    #endif
}

uint8_t uart_readByte() 
{
    
    uint8_t temp;
    //temp = USART_ReceiveData(USART1);//zyx
    //HAL_UART_Receive(&huart1, &temp, 1, 100);
    #ifdef OPEN_USART
	temp=(uint8_t)(huart2.Instance->RDR);
	#endif
    return (uint8_t)temp;
}

#ifdef __GNUC__
int __io_putchar(int ch)
{
			//#ifdef OPEN_USART
		while((USART2->ISR&0X40)==0);//
		USART2->TDR = (uint8_t) ch;      
		//#endif
		return ch;
}
#else
int fputc(int ch, FILE *f)//zyx
{ 	
		//#ifdef OPEN_USART
		while((USART2->ISR&0X40)==0);//
		USART2->TDR = (uint8_t) ch;      
		//#endif
		return ch;
}
#endif
void USART_DMA_START(void)
{
	//HAL_USART_Receive_DMA(&hdma_usart1_rx, Uart_RX_BUF,1000);
	HAL_UART_Receive_DMA(&huart1,Uart_RX_BUF,1000);
}
void USART_DMA_STOP(void)
{
	uint16_t Uart_datalen;
	uint16_t Openserialinputbuf_len;
	uint16_t ;
	uint16_t i=0;
	//HAL_USART_DMAStop(&hdma_usart1_rx);
	Uart_datalen=1000-hdma_usart1_rx.Instance->CNDTR;

    //datalen=192;

	//UART_EndRxTransfer(&huart1);
	CLEAR_BIT(huart1.Instance->CR3, USART_CR3_DMAR);//clear the request bits
	HAL_DMA_Abort(huart1.hdmarx);//STOP the uart dma 
	CLEAR_BIT(huart1.Instance->CR1, (USART_CR1_RXNEIE | USART_CR1_PEIE));
	CLEAR_BIT(huart1.Instance->CR3, USART_CR3_EIE);
	huart1.RxState = HAL_UART_STATE_READY;
	////UART_EndRxTransfer(&huart1);//
	///HAL_UART_DMAStop(&huart1);
	//while(1);
	
	if(Uart_datalen!=0)//
	{
		//1,check if is config
                printf("hello");
		if(ISConfigPacket(Uart_RX_BUF, Uart_datalen))
		{
			AnalyzePacket(Uart_RX_BUF,Uart_datalen,Uart_TX_BUF,&Uart_Tx_BUF_LEN);
		}
		else if(ieee154e_isSynch())
		{
			if(ieee154e_islowpower()&&(!idmanager_getIsDAGroot()))//added lowerpower run more time mode slave send radio data 
			{
				ieee154e_changemode(LOWERPOWER_RUN_MORE_TIME);
			}
			Put_Uartdata_to_Openserialinputbuf(Uart_RX_BUF, Uart_datalen);
		}else
		{
			leds_error_toggle();
			printc("I received data when I am not sync,I will give up it!");
		}
		
	}
	else
	{
        huart1.Instance->ICR=0xFFFF;//clear the ISR
		leds_error_toggle();
		printc("Com datalen is 0\n");
		//HAL_NVIC_SystemReset();//add reset mode 20180824
	}
	
}
bool UARTDMA_IS_BUSY(void)
{
	if(huart1.RxState==HAL_UART_STATE_BUSY)
	{
		return TRUE;
	}
	if(huart1.gState==HAL_UART_STATE_BUSY_TX)
	{
		return TRUE;
	}
	if((huart1.Instance->ISR&UART_FLAG_TC)==0)//added zyx 20181129
	{
		return TRUE;
	}
	if(HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_1))
	{
		return TRUE;
	}
	#ifdef OPEN_DEBUG_USART2
	if((huart2.Instance->ISR&UART_FLAG_TC)==0)
	{
		return TRUE;
	}
	if(huart1.gState==HAL_UART_STATE_BUSY_TX)
	{
		return TRUE;
	}
	#endif
	return FALSE;
}

//=========================== interrupt handlers ==============================

kick_scheduler_t uart_tx_isr() 
{
    uart_vars.txCb();
    return DO_NOT_KICK_SCHEDULER;
}

kick_scheduler_t uart_rx_isr() 
{
    uart_vars.rxCb();
    return DO_NOT_KICK_SCHEDULER;
}
/*void UART_DMATransmitCplt(DMA_HandleTypeDef *hdma)
{
	huart1.gState=HAL_UART_STATE_READY;
}*/
/*void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
	huart->gState=HAL_UART_STATE_READY;
}*/


