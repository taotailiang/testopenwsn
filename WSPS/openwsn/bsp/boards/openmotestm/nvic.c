/**
\brief openmoteSTM32 definition of the NVIC.

\author Chang Tengfei <tengfei.chang@gmail.com>,  July 2012.
*/

//#include "stm32f10x_lib.h"//zyx
#include "stm32l4xx_hal.h"
#include "radio.h"
//=========================== defines =========================================

//=========================== variables =======================================

//=========================== prototypes ======================================

//=========================== public ==========================================

void NVIC_init(void) {
    
    // Set the Vector Table base location at 0x08000000
    //NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0x0);//zyx   
}

//configuration uart interrput
void NVIC_uart(void) {
	////HAL_NVIC_SetPriority(USART3_IRQn,3,3);
	HAL_NVIC_SetPriority(DMA1_Channel4_IRQn,3,3);
	HAL_NVIC_SetPriority(DMA1_Channel5_IRQn,3,3);
    #if 0//zyx
    //Configure NVIC: Preemption Priority = 3 and Sub Priority = 3
    NVIC_InitTypeDef NVIC_InitStructure;
    NVIC_InitStructure.NVIC_IRQChannel                      = USART1_IRQChannel;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority    = 3;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority           = 3;
    NVIC_InitStructure.NVIC_IRQChannelCmd                   = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
    #endif
}

//configuration spi interrput
void NVIC_spi(void) {
   #if 0//zyx 没有开SPI的中断
#ifdef SPI_IN_INTERRUPT_MODE
    //Configure NVIC: Preemption Priority = 2 and Sub Priority = 2
    NVIC_InitTypeDef NVIC_InitStructure;
    NVIC_InitStructure.NVIC_IRQChannel                      = SPI1_IRQChannel;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority    = 2;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority           = 2;
    NVIC_InitStructure.NVIC_IRQChannelCmd                   = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
#endif
	#endif
}

//configuration radiotimer interrput
void NVIC_sctimer(void) 
{
	HAL_NVIC_SetPriority(LPTIM1_IRQn,1,1);
	HAL_NVIC_EnableIRQ(LPTIM1_IRQn);
    #if 0//zyx
    NVIC_InitTypeDef NVIC_InitStructure;
    //Configure RTC Alarm interrupt:
    //Configure NVIC: Preemption Priority = 1 and Sub Priority = 1
    NVIC_InitStructure.NVIC_IRQChannel                      = RTCAlarm_IRQChannel;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority    = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority           = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd                   = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
    #endif
}

//configuration radio interrput
void NVIC_radio(void){
#if 0//zyx
 //Configure NVIC: Preemption Priority = 0 and Sub Priority = 0
NVIC_InitTypeDef  NVIC_InitStructure;
NVIC_InitStructure.NVIC_IRQChannel                      = EXTI15_10_IRQChannel;
NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority    = 1;
NVIC_InitStructure.NVIC_IRQChannelSubPriority           = 0;
NVIC_InitStructure.NVIC_IRQChannelCmd                   = ENABLE;
NVIC_Init(&NVIC_InitStructure);
#endif
#ifdef USE_2_Radio_Interrupt
HAL_NVIC_SetPriority(EXTI2_IRQn,1,1);//lowest level  lcg 20180522 pri:3,3
HAL_NVIC_EnableIRQ(EXTI2_IRQn);//
HAL_NVIC_SetPriority(EXTI15_10_IRQn,0,0);//Highest level
HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);
#else
HAL_NVIC_SetPriority(EXTI15_10_IRQn,1,0);
HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);
#endif
}

//configuration of interrupt on openmotestm32
void NVIC_Configuration(void){
    
    //Set the Vector Table base location
    NVIC_init();
  	HAL_NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_2);
    //2 bits for Preemption Priority and 2 bits for Sub Priority
    //NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//zyx
}
