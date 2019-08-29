/**
\brief openmoteSTM32 definition of the RCC.

\author Chang Tengfei <tengfei.chang@gmail.com>,  July 2012.
*/
//#include "stm32f10x_lib.h"//zyx
#include "stm32l4xx_hal.h"
//=========================== defines =========================================

//=========================== variables =======================================

//=========================== prototypes ======================================

//=========================== public ==========================================
//#define USE_HSI
#define MSI_USE_PLL
void RCC_Configuration(void)
{
    RCC_OscInitTypeDef	RCC_OscInitStruct;
    RCC_ClkInitTypeDef	RCC_ClkInitStruct;
    RCC_PeriphCLKInitTypeDef	PeriphClkInit;
    __HAL_RCC_LSEDRIVE_CONFIG(RCC_LSEDRIVE_MEDIUMHIGH);
	#ifdef USE_HSI
    /*Configure LSE Drive Capability*/
    __HAL_RCC_LSEDRIVE_CONFIG(RCC_LSEDRIVE_LOW);
	/*Initializes the CPU AHB and APB busses clocks*/
    RCC_OscInitStruct.OscillatorType=RCC_OSCILLATORTYPE_HSI|RCC_OSCILLATORTYPE_LSE;
    RCC_OscInitStruct.LSEState=RCC_LSE_ON;
    RCC_OscInitStruct.HSIState=RCC_HSI_ON;
    RCC_OscInitStruct.HSICalibrationValue=16;
    RCC_OscInitStruct.PLL.PLLState=RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource=RCC_PLLSOURCE_HSI;
    RCC_OscInitStruct.PLL.PLLM=1;
    RCC_OscInitStruct.PLL.PLLN=8;
    RCC_OscInitStruct.PLL.PLLP=RCC_PLLP_DIV7;
    RCC_OscInitStruct.PLL.PLLQ=RCC_PLLQ_DIV2;
    RCC_OscInitStruct.PLL.PLLR=RCC_PLLR_DIV4;//主频设置DIV4为32MHz，DIV2为64MHz
    if(HAL_RCC_OscConfig(&RCC_OscInitStruct)!=HAL_OK)
    {
    	//_Error_Handler(__FILE__, __LINE__);
        while(1);
    }
    /*Initializes the CLK*/
    RCC_ClkInitStruct.ClockType=RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK|RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource=RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider=RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider=RCC_HCLK_DIV1;
    RCC_ClkInitStruct.APB2CLKDivider=RCC_HCLK_DIV1;
	if(HAL_RCC_ClockConfig(&RCC_ClkInitStruct,FLASH_LATENCY_4)!=HAL_OK)
	{
		//_Error_Handler(__FILE__, __LINE__);
                while(1);
	}
	PeriphClkInit.PeriphClockSelection=RCC_PERIPHCLK_LPUART1|RCC_PERIPHCLK_LPTIM1|RCC_PERIPHCLK_ADC;
	PeriphClkInit.Lpuart1ClockSelection=RCC_LPUART1CLKSOURCE_SYSCLK;
	PeriphClkInit.Lptim1ClockSelection=RCC_LPTIM1CLKSOURCE_LSE;
	PeriphClkInit.AdcClockSelection=RCC_ADCCLKSOURCE_SYSCLK;
	
	if(HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit)!=HAL_OK)
	{
		//_Error_Handler(__FILE__, __LINE__);
                while(1);
	}
	if(HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1)!=HAL_OK)
	{
		//_Error_Handler(__FILE__, __LINE__);
                while(1);
	}
	//SYSTICK暂时没有配置
	//HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq()/1000);

	/**Configure the Systick 
	*/
	HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);

	/* SysTick_IRQn interrupt configuration */
	//HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);
	
	__HAL_RCC_WAKEUPSTOP_CLK_CONFIG(RCC_STOP_WAKEUPCLOCK_HSI);
	#else
	__HAL_RCC_LSEDRIVE_CONFIG(RCC_LSEDRIVE_LOW);
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSE|RCC_OSCILLATORTYPE_MSI;//|RCC_OSCILLATORTYPE_LSI;
	RCC_OscInitStruct.LSIState=RCC_LSI_ON;
	RCC_OscInitStruct.LSEState = RCC_LSE_BYPASS;//RCC_LSE_ON;  lcg 20180425
	RCC_OscInitStruct.MSIState = RCC_MSI_ON;
	RCC_OscInitStruct.MSICalibrationValue = 32;
	RCC_OscInitStruct.MSIClockRange =RCC_MSIRANGE_10;//RCC_MSIRANGE_10;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
	if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
	{
		//_Error_Handler(__FILE__, __LINE__);
		while(1);
	}
	RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
	                          |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_MSI;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
	RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

	if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
	{
		//_Error_Handler(__FILE__, __LINE__);
		while(1);
	}
	PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USART1|RCC_PERIPHCLK_LPTIM1;
	PeriphClkInit.Usart1ClockSelection = RCC_USART1CLKSOURCE_PCLK2;
	//PeriphClkInit.Usart3ClockSelection=RCC_USART3CLKSOURCE_SYSCLK;//added by zyx 20180622
	PeriphClkInit.Lptim1ClockSelection = RCC_LPTIM1CLKSOURCE_LSE;
	if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
	{
		while(1);	
	}
	if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1) != HAL_OK)
	{
		while(1);
	}
	#ifdef MSI_USE_PLL
	//HAL_RCCEx_EnableMSIPLLMode();
	#endif
	
	__HAL_RCC_WAKEUPSTOP_CLK_CONFIG(RCC_STOP_WAKEUPCLOCK_MSI);//WAKE UP USING MSI
	HAL_RCCEx_EnableMSIPLLMode();//zyx20181117
	#endif
	
}

//when wakeup by alarm, configure rcc
void RCC_Wakeup(void){
//
//
#if 0
	RCC_Configuration();
	if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1) != HAL_OK)
	{
		while(1);
	}
#endif
	//HAL_RCCEx_EnableMSIPLLMode();//zyx20181117
	//while(READ_BIT(RCC->CR,RCC_CR_MSIRDY));
	#ifdef USE_HSI
	__HAL_RCC_PLL_ENABLE();
	__HAL_RCC_PLLCLKOUT_ENABLE(RCC_PLL_SYSCLK);
	while(READ_BIT(RCC->CR, RCC_CR_PLLRDY) == RESET);
	#endif
	//while();
	//while(!__HAL_RCC_GET_FLAG(RCC_FLAG_PLLRDY));
	//HAL_RCC_ClockConfig(RCC_ClkInitTypeDef * RCC_ClkInitStruct, uint32_t FLatency)
	//while(__HAL_RCC_GET_FLAG(__FLAG__));
#if 0//zyx
    //enable PLL
    RCC_PLLCmd(ENABLE);

    // Wait till PLL is ready 
    while(RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET);

    // Select PLL as system clock source 
    RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);
  
    // Wait till PLL is used as system clock source 
    while(RCC_GetSYSCLKSource() != 0x08);
#endif
}
/*void _Error_Handler(char * file, int line)
{
	while(1)
	{
		debugpins_task_toggle();
	}
}*/

