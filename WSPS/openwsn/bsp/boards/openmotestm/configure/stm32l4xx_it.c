/**
  ******************************************************************************
  * @file    stm32l4xx_it.c
  * @brief   Interrupt Service Routines.
  ******************************************************************************
  *
  * COPYRIGHT(c) 2017 STMicroelectronics
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/
#include "stm32l4xx_hal.h"
#include "stm32l4xx.h"
#include "stm32l4xx_it.h"
#include "zyx.h"
#include "sctimer.h"
#include "radio.h"
#include "uart.h"
#include "opendefs.h"
#include "openserial.h"
#include "Board.h"
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/* External variables --------------------------------------------------------*/
extern LPTIM_HandleTypeDef hlptim1;
extern UART_HandleTypeDef huart2;
extern DMA_HandleTypeDef hdma_usart1_rx;
extern DMA_HandleTypeDef hdma_usart1_tx;
extern DMA_HandleTypeDef hdma_usart2_tx;
extern uint8_t Uart_TX_BUF[];
extern uint8_t Uart_RX_BUF[];

#ifdef USE_2_Radio_Interrupt
extern __IO PORT_TIMER_WIDTH Globle_capturedTime;
#endif
/******************************************************************************/
/*            Cortex-M4 Processor Interruption and Exception Handlers         */ 
/******************************************************************************/

/**
* @brief This function handles Non maskable interrupt.
*/
void NMI_Handler(void)
{
  /* USER CODE BEGIN NonMaskableInt_IRQn 0 */

  /* USER CODE END NonMaskableInt_IRQn 0 */
  /* USER CODE BEGIN NonMaskableInt_IRQn 1 */

  /* USER CODE END NonMaskableInt_IRQn 1 */
}

/**
* @brief This function handles Hard fault interrupt.
*/
void HardFault_Handler(void)
{
  /* USER CODE BEGIN HardFault_IRQn 0 */
	uint16_t i;
  /* USER CODE END HardFault_IRQn 0 */
  while (1)
  {
      for(i=0;i<65535;i++);
      debugpins_radio_toggle();
  }
  /* USER CODE BEGIN HardFault_IRQn 1 */
	
  /* USER CODE END HardFault_IRQn 1 */
}

/**
* @brief This function handles Memory management fault.
*/
void MemManage_Handler(void)
{
  /* USER CODE BEGIN MemoryManagement_IRQn 0 */

  /* USER CODE END MemoryManagement_IRQn 0 */
  while (1)
  {
  }
  /* USER CODE BEGIN MemoryManagement_IRQn 1 */

  /* USER CODE END MemoryManagement_IRQn 1 */
}

/**
* @brief This function handles Prefetch fault, memory access fault.
*/
void BusFault_Handler(void)
{
  /* USER CODE BEGIN BusFault_IRQn 0 */

  /* USER CODE END BusFault_IRQn 0 */
  while (1)
  {
  }
  /* USER CODE BEGIN BusFault_IRQn 1 */

  /* USER CODE END BusFault_IRQn 1 */
}

/**
* @brief This function handles Undefined instruction or illegal state.
*/
void UsageFault_Handler(void)
{
  /* USER CODE BEGIN UsageFault_IRQn 0 */

  /* USER CODE END UsageFault_IRQn 0 */
  while (1)
  {
  }
  /* USER CODE BEGIN UsageFault_IRQn 1 */

  /* USER CODE END UsageFault_IRQn 1 */
}

/**
* @brief This function handles System service call via SWI instruction.
*/
void SVC_Handler(void)
{
  /* USER CODE BEGIN SVCall_IRQn 0 */

  /* USER CODE END SVCall_IRQn 0 */
  /* USER CODE BEGIN SVCall_IRQn 1 */

  /* USER CODE END SVCall_IRQn 1 */
}

/**
* @brief This function handles Debug monitor.
*/
void DebugMon_Handler(void)
{
  /* USER CODE BEGIN DebugMonitor_IRQn 0 */

  /* USER CODE END DebugMonitor_IRQn 0 */
  /* USER CODE BEGIN DebugMonitor_IRQn 1 */

  /* USER CODE END DebugMonitor_IRQn 1 */
}

/**
* @brief This function handles Pendable request for system service.
*/
void PendSV_Handler(void)
{
  /* USER CODE BEGIN PendSV_IRQn 0 */

  /* USER CODE END PendSV_IRQn 0 */
  /* USER CODE BEGIN PendSV_IRQn 1 */

  /* USER CODE END PendSV_IRQn 1 */
}

/**
* @brief This function handles System tick timer.
*/
void SysTick_Handler(void)
{
  /* USER CODE BEGIN SysTick_IRQn 0 */

  /* USER CODE END SysTick_IRQn 0 */
  HAL_IncTick();
  HAL_SYSTICK_IRQHandler();
  /* USER CODE BEGIN SysTick_IRQn 1 */

  /* USER CODE END SysTick_IRQn 1 */
}

/******************************************************************************/
/* STM32L4xx Peripheral Interrupt Handlers                                    */
/* Add here the Interrupt Handlers for the used peripherals.                  */
/* For the available peripheral interrupt handler names,                      */
/* please refer to the startup file (startup_stm32l4xx.s).                    */
/******************************************************************************/

/**
* @brief This function handles EXTI line[15:10] interrupts.
*/
void EXTI0_IRQHandler(void)
{
	debugpins_isr_set();
	__HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_0);
	sctimer_isr();//HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_0);
	debugpins_isr_clr();
}
void EXTI1_IRQHandler(void)
{
	debugpins_isr_set();
	__HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_1);
	if(HAL_GPIO_ReadPin(GPIOA , GPIO_PIN_1))
	{
		
		USART_DMA_START();
		
	}
	else
	{
		USART_DMA_STOP();
		//此处应有解包的函数
	}
	//HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_1);
	debugpins_isr_clr();
}
#ifdef USE_2_Radio_Interrupt
void EXTI2_IRQHandler(void)
{
	__HAL_GPIO_EXTI_CLEAR_FLAG(GPIO_PIN_2);
	debugpins_isr_set(); //lcg 20180521 add
	radio_isr_second(Globle_capturedTime);
	debugpins_isr_clr(); //lcg 20180521 add
}
#endif

void USART2_IRQHandler(void)
{
	//HAL_UART_IRQHandler(&huart2);
	uint32_t isrflags,crlists;
	debugpins_isr_set();
	isrflags=READ_REG(huart2.Instance->ISR);
	//crlists=READ_REG(huart2.Instance->CR1);
	if(isrflags&USART_ISR_ORE)
	{
		__HAL_UART_CLEAR_FLAG(&huart2,UART_CLEAR_OREF);
	}
	if((isrflags&USART_ISR_RXNE)!=RESET)
	{
		//clear();//
		uart_rx_isr();
	}
	if((isrflags&USART_ISR_TXE)!=RESET)
	{
		__HAL_UART_CLEAR_FLAG(&huart2, UART_CLEAR_TCF);
		uart_tx_isr();
	}
	__HAL_UART_CLEAR_FLAG(&huart2,0x00000000);
	debugpins_isr_clr();
}
void EXTI15_10_IRQHandler(void)
{
  /* USER CODE BEGIN EXTI15_10_IRQn 0 */
  /* USER CODE END EXTI15_10_IRQn 0 */
	debugpins_isr_set();
	if(__HAL_GPIO_EXTI_GET_IT(GPIO_PIN_RFEXTI3)!=RESET)
	{
		__HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_RFEXTI3);
		radio_isr();
	}

   debugpins_isr_clr();
  //HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_15);
  /* USER CODE BEGIN EXTI15_10_IRQn 1 */

  /* USER CODE END EXTI15_10_IRQn 1 */
}

/**
* @brief This function handles LPTIM1 global interrupt.
*/
void DMA1_Channel1_IRQHandler(void)
{
}
void DMA1_Channel2_IRQHandler(void)
{
}
void DMA1_Channel3_IRQHandler(void)
{
}
void DMA1_Channel6_IRQHandler(void)
{
}
void DMA1_Channel4_IRQHandler(void)
{
  /* USER CODE BEGIN DMA1_Channel4_IRQn 0 */

  /* USER CODE END DMA1_Channel4_IRQn 0 */
  debugpins_isr_set();
  HAL_DMA_IRQHandler(&hdma_usart1_tx);
  debugpins_isr_clr();
  /* USER CODE BEGIN DMA1_Channel4_IRQn 1 */

  /* USER CODE END DMA1_Channel4_IRQn 1 */
}

/**
* @brief This function handles DMA1 channel5 global interrupt.
*/
void DMA1_Channel5_IRQHandler(void)
{
  /* USER CODE BEGIN DMA1_Channel5_IRQn 0 */

  /* USER CODE END DMA1_Channel5_IRQn 0 */
  debugpins_isr_set();
  HAL_DMA_IRQHandler(&hdma_usart1_rx);
  debugpins_isr_clr();
  /* USER CODE BEGIN DMA1_Channel5_IRQn 1 */

  /* USER CODE END DMA1_Channel5_IRQn 1 */
}
/*Debug UART*/
void DMA1_Channel7_IRQHandler(void)
{
	debugpins_isr_set();
	HAL_DMA_IRQHandler(&hdma_usart2_tx);
	debugpins_isr_clr();
}
void LPTIM1_IRQHandler(void)
{
  /* USER CODE BEGIN LPTIM1_IRQn 0 */
  //__HAL_LPTIM_GET_FLAG(&hlptim1, LPTIM_FLAG_CMPM);

  #if 0
	if(__HAL_LPTIM_GET_FLAG(&hlptim1, LPTIM_FLAG_CMPM))
	{
		sctimer_isr();
		//EXTI->SWIER1|=0x01;
	}
	if(__HAL_LPTIM_GET_FLAG(&hlptim1,LPTIM_FLAG_ARRM))
		{
		//HAL_GPIO_TogglePin(GPIO_LED1,GPIO_PIN_LED1);
		}
	#endif
  	//HAL_GPIO_TogglePin(GPIO_LED2, GPIO_PIN_LED2);
  /* USER CODE END LPTIM1_IRQn 0 */
  	debugpins_isr_set();
  	HAL_LPTIM_IRQHandler(&hlptim1);
  	debugpins_isr_clr();
  /* USER CODE BEGIN LPTIM1_IRQn 1 */

  /* USER CODE END LPTIM1_IRQn 1 */
}

/* USER CODE BEGIN 1 */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{


}
void HAL_LPTIM_CompareMatchCallback(LPTIM_HandleTypeDef *hlptim)
{
	//debugpins_fsm_set();  //lcg 20180519 add  20180521 interrupt  in there later 3us then LPTIM1_IRQHandler
	sctimer_isr();
  	//debugpins_fsm_clr(); //lcg 20180519 add  20180521 interrupt  in there early 5us then LPTIM1_IRQHandler
}
void HAL_LPTIM_AutoReloadMatchCallback(LPTIM_HandleTypeDef *hlptim)
{

}


/* USER CODE END 1 */
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
