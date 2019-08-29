/**
\brief openmoteSTM32 definition of the "board" bsp module.

\author Thomas Watteyne <watteyne@eecs.berkeley.edu>, February 2012.
\author Chang Tengfei <tengfei.chang@gmail.com>,  July 2012.
*/
//#include "stm32f10x_lib.h"zyx
#include "stm32l4xx_hal.h"
#include "board.h"
#include "zyx.h"
// bsp modules
#include "leds.h"
#include "uart.h"
#include "spi.h"
#include "sctimer.h"
#include "radio.h"
#include "rcc.h"
#include "nvic.h"
#include "debugpins.h"
uint8_t SPI_SEND_BUF[20];
uint8_t	SPI_REC_BUF[20];
uint8_t i;
extern	SPI_HandleTypeDef hspi2;
extern  LPTIM_HandleTypeDef hlptim1;
CRC_HandleTypeDef CrcHandle;
uint8_t No_Feed_DOG=0;
#ifdef OPEN_IWDG
IWDG_HandleTypeDef hiwdg;
#endif
//=========================== variable ========================================

//=========================== private =========================================

//Configures the different GPIO ports as Analog Inputs.
void GPIO_Config_ALL_AIN(void);
// configure the hard fault exception
void board_enableHardFaultExceptionHandler(void);
static void CRC_INIT(void);//zyx20190226
#ifdef OPEN_IWDG
static void IWDG_Init(void);
#endif
//=========================== main ============================================


extern int mote_main(void);
int main(void) 
{ 
    //while(1);
     return mote_main();
}

//=========================== public ==========================================

void board_init(void)
{
    uint16_t testtimer=0;
    //Configure rcc
    HAL_Init();
    RCC_Configuration();
    //configure NVIC and Vector Table
    NVIC_Configuration();
    // configure hardfault exception
    board_enableHardFaultExceptionHandler();
    //configure ALL GPIO to AIN to get lowest power
	GPIO_Config_ALL_AIN();
#if 0//5uA
		HAL_GPIO_WritePin(GPIO_RFSDN,GPIO_PIN_RFSDN,GPIO_PIN_SET);
		while(1)
		{
			HAL_PWREx_EnterSTOP2Mode(PWR_STOPENTRY_WFI);
		}
#endif



	CRC_INIT();//zyx20190226
#if 0//5uA
		HAL_GPIO_WriteLPin(GPIO_RFSDN,GPIO_PIN_RFSDN,GPIO_PIN_SET);
		while(1)
		{
			HAL_PWREx_EnterSTOP2Mode(PWR_STOPENTRY_WFI);
		}
#endif
    // initialize board
    leds_init();
    debugpins_init();
  
    uart_init();
#if 0//5uA
	HAL_GPIO_WritePin(GPIO_RFSDN,GPIO_PIN_RFSDN,GPIO_PIN_SET);
    while(1)
    {
    	HAL_PWREx_EnterSTOP2Mode(PWR_STOPENTRY_WFI);
    }
#endif
   printf("I am reset!\n");
    spi_init();
    #if 0//1ma
	HAL_GPIO_WritePin(GPIO_RFSDN,GPIO_PIN_RFSDN,GPIO_PIN_SET);
    while(1)
    {
    	HAL_PWREx_EnterSTOP2Mode(PWR_STOPENTRY_WFI);
    }
    #endif
#ifdef USE_FLASH_PARAMETER
	if(!CheckFlashParamisOK())
	{
		leds_error_toggle();
		printf("FLASH Parameter is ERROR\n");
		if(*(__IO uint8_t *)(BASEADDR_FLASH_PARAMETER)==0xFF&&*(__IO uint8_t *)(BASEADDR_FLASH_PARAMETER+OFFSETADDR_PARAMETER_CRC)==0xFF)
		{
			if(!CheckFlashParamisOK())
			{
				Write_Default_Parameter();
			}
		}
		printf("Write default parameter and I will reset!\n");
		HAL_NVIC_SystemReset();
		
	
	}
#endif
    NVIC_radio();//?????Radio???§Ø?
    #if 0//1ma
	HAL_GPIO_WritePin(GPIO_RFSDN,GPIO_PIN_RFSDN,GPIO_PIN_SET);
    while(1)
    {
    	HAL_PWREx_EnterSTOP2Mode(PWR_STOPENTRY_WFI);
    }
    #endif

    radio_init();//
    //S2LPCmdStrobeSres();
#if 0//1ma
	HAL_GPIO_WritePin(GPIO_RFSDN,GPIO_PIN_RFSDN,GPIO_PIN_SET);
    while(1)
    {
    	HAL_PWREx_EnterSTOP2Mode(PWR_STOPENTRY_WFI);
    }
#endif

    sctimer_init();//
    #ifdef OPEN_IWDG
	IWDG_Init();
	#endif
    #if 0
    HAL_GPIO_WritePin(GPIO_RFSDN,GPIO_PIN_RFSDN,GPIO_PIN_SET);
    while(1)
    {
    	HAL_PWREx_EnterSTOP2Mode(PWR_STOPENTRY_WFI);
    }
	#endif
    /*while(1)
    	{
    	
    	testtimer=HAL_LPTIM_ReadCounter(&hlptim1);
		testtimer+=32768;
		testtimer=__HAL_LPTIM_COMPARE_SET(&hlptim1, testtimer);
    	//HAL_Delay(1000);
		HAL_PWREx_EnterSTOP2Mode(PWR_STOPENTRY_WFE);
		
		
    	}*/
    //enable nvic for the radio
    

}

void board_sleep(void) 
{
	//if (*(uint8_t *)(ADDR_PARAMENT_STARTADDRESS+OFFSETADDR_CPUSLEEP)==0)
	{
		//__disable_irq();
		#ifdef OPEN_IWDG
		if(No_Feed_DOG)
		{
		}else
		{
			HAL_IWDG_Refresh(&hiwdg);
		}
		#endif
		HAL_PWREx_EnterSTOP2Mode(PWR_STOPENTRY_WFI);
		
	}
#if 0//zyx
    DBGMCU_Config(DBGMCU_SLEEP, ENABLE);
    // Enable PWR and BKP clock
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);
    // Desable the SRAM and FLITF clock in sleep mode
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_SRAM | RCC_AHBPeriph_FLITF, DISABLE);
    // enter sleep mode
    __WFI();
#endif
}



void board_reset(void){
#if 0//zyx
    NVIC_GenerateSystemReset();
#endif
	
}

// ========================== private =========================================

/**
  * @brief  Configures the different GPIO ports as Analog Inputs.
  * @param  None
  * @retval : None
  */
  
void GPIO_Config_ALL_AIN(void)
{
	uint8_t i;
	GPIO_InitTypeDef GPIO_InitStruct;
	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_GPIOB_CLK_ENABLE();
	__HAL_RCC_GPIOC_CLK_ENABLE();
	
	/*PA1 UART TO FCI INTERRUPT*/
	GPIO_InitStruct.Pin=GPIO_PIN_1;
	GPIO_InitStruct.Mode=GPIO_MODE_IT_RISING_FALLING;
	GPIO_InitStruct.Pull=GPIO_PULLDOWN;
	HAL_GPIO_Init(GPIOA,&GPIO_InitStruct);
	
	

	/*PA6,PB1 background two testpoints*/
	GPIO_InitStruct.Pin=GPIO_PIN_6|GPIO_PIN_4;//
	GPIO_InitStruct.Mode=GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Speed=GPIO_SPEED_HIGH;//20171124
	GPIO_InitStruct.Pull=GPIO_NOPULL;
	HAL_GPIO_Init(GPIOA,&GPIO_InitStruct);
	
	GPIO_InitStruct.Pin=GPIO_PIN_1;//|GPIO_PIN_4|GPIO_PIN_5;//
	GPIO_InitStruct.Mode=GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Speed=GPIO_SPEED_HIGH;//20171124
	GPIO_InitStruct.Pull=GPIO_NOPULL;
	HAL_GPIO_Init(GPIOB,&GPIO_InitStruct);
	
	GPIO_InitStruct.Pin=GPIO_PIN_8|GPIO_PIN_9;//
	GPIO_InitStruct.Mode=GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Speed=GPIO_SPEED_HIGH;//20171124
	GPIO_InitStruct.Pull=GPIO_NOPULL;
	HAL_GPIO_Init(GPIOB,&GPIO_InitStruct);
	
	#ifdef USE_2_Radio_Interrupt//
	GPIO_InitStruct.Pin = GPIO_PIN_2;
	GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
	GPIO_InitStruct.Pull = GPIO_PULLDOWN;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
	#endif
	
	//SPI ???????????hal_msp?§µ??????????CS??SDN???
	GPIO_InitStruct.Pin=GPIO_PIN_RFCS;
	GPIO_InitStruct.Mode=GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Speed=GPIO_SPEED_LOW;//
	GPIO_InitStruct.Pull=GPIO_NOPULL;
	HAL_GPIO_Init(GPIO_RFCS,&GPIO_InitStruct);
	GPIO_InitStruct.Pin=GPIO_PIN_RFSDN;
	GPIO_InitStruct.Mode=GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Speed=GPIO_SPEED_LOW;//
	GPIO_InitStruct.Pull=GPIO_NOPULL;
	HAL_GPIO_Init(GPIO_RFSDN,&GPIO_InitStruct);

	HAL_GPIO_WritePin(GPIO_RFCS,GPIO_PIN_RFCS,GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIO_RFSDN,GPIO_PIN_RFSDN,GPIO_PIN_RESET);
	GPIO_InitStruct.Pin=GPIO_PIN_4;
	GPIO_InitStruct.Mode=GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull=GPIO_NOPULL;
	HAL_GPIO_Init(GPIOA,&GPIO_InitStruct);
	//PA0 software trig interrupt
	GPIO_InitStruct.Pin = GPIO_PIN_0;
	GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	//PA11----->GPIO3 radio interrupt
	GPIO_InitStruct.Pin = GPIO_PIN_RFEXTI3;
	GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	HAL_GPIO_Init(GPIO_RFEXTI3, &GPIO_InitStruct);
	//PA15-------->GPIO0 NO USE
	GPIO_InitStruct.Pin = GPIO_PIN_RFEXTI0;
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_PULLDOWN;
	HAL_GPIO_Init(GPIO_RFEXTI0, &GPIO_InitStruct);
		/*REEXTI1 used to send packet to wireless&RFEXTI2 no use*/
	GPIO_InitStruct.Pin = GPIO_PIN_RFEXTI1;
	GPIO_InitStruct.Mode =GPIO_MODE_OUTPUT_PP;// GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(GPIO_RFEXTI1, &GPIO_InitStruct);
	GPIO_InitStruct.Pin = GPIO_PIN_RFEXTI2;
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_PULLDOWN;
	HAL_GPIO_Init(GPIO_RFEXTI2, &GPIO_InitStruct);
	//HAL_GPIO_WritePin(GPIO_RFEXTI0,GPIO_PIN_RFEXTI0,GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIO_RFEXTI1,GPIO_PIN_RFEXTI1,GPIO_PIN_RESET);
	//HAL_GPIO_WritePin(GPIO_RFEXTI2,GPIO_PIN_RFEXTI2,GPIO_PIN_RESET);
#if 1
	/*20180912zyx*/
	GPIO_InitStruct.Pin=GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_7|GPIO_PIN_8|GPIO_PIN_10;//
	GPIO_InitStruct.Mode=GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Speed=GPIO_SPEED_LOW;//???????????????????????????????20171124
	GPIO_InitStruct.Pull=GPIO_NOPULL;
	HAL_GPIO_Init(GPIOA,&GPIO_InitStruct);
	HAL_GPIO_WritePin(GPIOA,GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_7|GPIO_PIN_8|GPIO_PIN_10,GPIO_PIN_RESET);
	
	GPIO_InitStruct.Pin=GPIO_PIN_0|GPIO_PIN_3|GPIO_PIN_5|GPIO_PIN_10;
	GPIO_InitStruct.Mode=GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Speed=GPIO_SPEED_LOW;//???????????????????????????????20171124
	GPIO_InitStruct.Pull=GPIO_NOPULL;
	HAL_GPIO_Init(GPIOB,&GPIO_InitStruct);
	HAL_GPIO_WritePin(GPIOB,GPIO_PIN_0|GPIO_PIN_3|GPIO_PIN_5|GPIO_PIN_10,GPIO_PIN_RESET);
	GPIO_InitStruct.Pin=GPIO_PIN_6|GPIO_PIN_7;//
	GPIO_InitStruct.Mode=GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Speed=GPIO_SPEED_HIGH;//???????????????????????????????20171124
	GPIO_InitStruct.Pull=GPIO_NOPULL;
	HAL_GPIO_Init(GPIOB,&GPIO_InitStruct);
	HAL_GPIO_WritePin(GPIOB,GPIO_PIN_6|GPIO_PIN_7,GPIO_PIN_RESET);
#endif
	//HAL_NVIC_SetPriority(EXTI15_10_IRQn, 1, 0);
	//HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);
	HAL_NVIC_SetPriority(EXTI0_IRQn, 1, 1);
	HAL_NVIC_EnableIRQ(EXTI0_IRQn);
	HAL_NVIC_SetPriority(EXTI1_IRQn,2,2);
	HAL_NVIC_EnableIRQ(EXTI1_IRQn);	
}

void board_enableHardFaultExceptionHandler(void)
{
    // Configures:
    //    bit9. stack alignment on exception entry 
    //    bit4. enables faulting
    //    bit3. unaligned access traps
    #if 0//zyx
    SCB->CCR = 0x00000210;
    #endif
}
void CRC_INIT(void)//zyx20190226
{
	
	CrcHandle.Instance=CRC;
	CrcHandle.Init.DefaultPolynomialUse=DEFAULT_POLYNOMIAL_ENABLE;
	//CrcHandle.Init.GeneratingPolynomial=POLY_CRC32;
	CrcHandle.Init.CRCLength=CRC_POLYLENGTH_32B;
	CrcHandle.Init.DefaultInitValueUse=DEFAULT_INIT_VALUE_ENABLE;
	CrcHandle.Init.InputDataInversionMode=CRC_INPUTDATA_INVERSION_NONE;
	CrcHandle.Init.OutputDataInversionMode=CRC_OUTPUTDATA_INVERSION_DISABLE;
	CrcHandle.InputDataFormat=CRC_INPUTDATA_FORMAT_BYTES;
	if(HAL_CRC_Init(&CrcHandle)!=HAL_OK)
	{
		while(1);
	}
}
/*used to Reset System with no feed watch dog */
void SYSTEM_RESET_NO_FEED_DOG(void)
{
	No_Feed_DOG=1;
}
#ifdef OPEN_IWDG
static void IWDG_Init(void)
{
	hiwdg.Instance=IWDG;
	hiwdg.Init.Prescaler=IWDG_PRESCALER_32;
	hiwdg.Init.Window=4095;
	hiwdg.Init.Reload=4095;
	if(HAL_IWDG_Init(&hiwdg)!=HAL_OK)
	{
		while(1);
	}
} 
#endif
