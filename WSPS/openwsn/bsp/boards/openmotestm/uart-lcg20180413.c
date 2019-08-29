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
UART_HandleTypeDef huart1;
//=========================== prototypes ======================================

//=========================== public ==========================================

void uart_init() {
    uint8_t vectcTxBuff[20]={1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0};
    // reset local variables
    memset(&uart_vars,0,sizeof(uart_vars_t));
    
    //when this value is 0, we are send the first data
    uart_vars.startOrend = 0;
    //flag byte for start byte and end byte
    uart_vars.flagByte = 0x7E;
	huart1.Instance = USART1;
	huart1.Init.BaudRate =115200;
	huart1.Init.WordLength = UART_WORDLENGTH_8B;
	huart1.Init.StopBits = UART_STOPBITS_1;
	huart1.Init.Parity = UART_PARITY_NONE;
	huart1.Init.Mode = UART_MODE_TX_RX;
	huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	huart1.Init.OverSampling = UART_OVERSAMPLING_16;
	huart1.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
	huart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
	if (HAL_MultiProcessor_Init(&huart1, 0, UART_WAKEUPMETHOD_IDLELINE) != HAL_OK)
	{
		while(1);//_Error_Handler(__FILE__, __LINE__);
	}
	//HAL_UART_Transmit(&huart1,vectcTxBuff,20,2000);
	////printf("let us begin our game!\n");
	
}

void uart_setCallbacks(uart_tx_cbt txCb, uart_rx_cbt rxCb) {
    
    uart_vars.txCb = txCb;
    uart_vars.rxCb = rxCb;
    
    //enable nvic uart.
     NVIC_uart();
}

void uart_enableInterrupts(){
    __HAL_UART_ENABLE_IT(&huart1, UART_IT_RXNE);
    //USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);//zyx
}

void uart_disableInterrupts(){
    __HAL_UART_DISABLE_IT(&huart1, UART_IT_RXNE|UART_IT_TXE);
    //USART_ITConfig(USART1, USART_IT_TXE, DISABLE);//zyx
    //USART_ITConfig(USART1, USART_IT_RXNE, DISABLE);//zyx
}

void uart_clearRxInterrupts(){
	__HAL_UART_CLEAR_FLAG(&huart1, UART_CLEAR_RTOF);//清除超时中断的标志位zyx20180306
    //__HAL_UART_CLEAR_FLAG(huart1,);
    //USART_ClearFlag(USART1,USART_FLAG_RXNE);//zyx
}

void uart_clearTxInterrupts(){
    __HAL_UART_CLEAR_FLAG(&huart1, UART_CLEAR_TCF);
    //USART_ClearFlag(USART1,USART_FLAG_TXE);//zyx
}

void uart_writeByte(uint8_t byteToWrite) {
	HAL_UART_Transmit(&huart1,&byteToWrite, 1,2);  //lcg  20180412
	#if 0
	
	 	 
	while((USART1->ISR&0X40)==0);//循环发送,直到发送完毕	 
	USART1->TDR = byteToWrite;
	#endif
    //start or end byte?
    if(byteToWrite == uart_vars.flagByte){
        uart_vars.startOrend = (uart_vars.startOrend == 0)?1:0;
        //start byte
        if(uart_vars.startOrend == 1) {
            __HAL_UART_ENABLE_IT(&huart1, UART_IT_TXE);
        } else {
            __HAL_UART_DISABLE_IT(&huart1, UART_IT_TXE);
        }
    }
	
	
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
	temp=(uint8_t)(huart1.Instance->RDR);
    return (uint8_t)temp;
}

int fputc(int ch, FILE *f)//zyx
{ 	
		while((USART1->ISR&0X40)==0);//循环发送,直到发送完毕   
		USART1->TDR = (uint8_t) ch;      
		return ch;
}

//=========================== interrupt handlers ==============================

kick_scheduler_t uart_tx_isr() {
    
    uart_vars.txCb();
    return DO_NOT_KICK_SCHEDULER;
}

kick_scheduler_t uart_rx_isr() {
    
    uart_vars.rxCb();
    return DO_NOT_KICK_SCHEDULER;
}
