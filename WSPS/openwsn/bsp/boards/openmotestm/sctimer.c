#include "stdint.h"

//#include "stm32f10x_conf.h"//zyx
//#include "stm32l4xx_hal_conf.h"
#include "sctimer.h"
//#include "board.h"
#include "leds.h"
#include "rcc.h"
#include "nvic.h"
#include "stm32l4xx_hal.h"
#include "zyx.h"

// ========================== define ==========================================


// ========================== variable ========================================
uint16_t CurrentCountzyx;//当前定时器计数值current count value
uint16_t CurrentVaulezyx;//当前定时器设定值current set value
__IO uint8_t MARKSOFTINTERRUPTZYX=0;
typedef struct {
    sctimer_cbt sctimer_cb;
    bool        convert;
    bool        convertUnlock;
} sctimer_vars_t;
sctimer_vars_t sctimer_vars;
LPTIM_HandleTypeDef	hlptim1;
TIM_HandleTypeDef htim16;
// ========================== private ==========================================
void sctimer_isr_internal(void);
// ========================== protocol =========================================

/**
\brief Initialization sctimer.
*/
void	sctimer_init(void)
{
	memset(&sctimer_vars,0,sizeof(sctimer_vars_t));
	hlptim1.Instance=LPTIM1;
	hlptim1.Init.Clock.Source=LPTIM_CLOCKSOURCE_APBCLOCK_LPOSC;
	hlptim1.Init.Clock.Prescaler=LPTIM_PRESCALER_DIV1;
	hlptim1.Init.Trigger.Source=LPTIM_TRIGSOURCE_SOFTWARE;
	hlptim1.Init.OutputPolarity=LPTIM_OUTPUTPOLARITY_HIGH;
	hlptim1.Init.UpdateMode=LPTIM_UPDATE_IMMEDIATE;//LPTIM_UPDATE_ENDOFPERIOD;
	hlptim1.Init.CounterSource=LPTIM_COUNTERSOURCE_INTERNAL;
	hlptim1.Init.Input1Source = LPTIM_INPUT1SOURCE_GPIO;
	hlptim1.Init.Input2Source = LPTIM_INPUT2SOURCE_GPIO;
	if (HAL_LPTIM_Init(&hlptim1) != HAL_OK)
	{
		while(1);
	}
	NVIC_sctimer();
	TIM16_Init();
	if(HAL_LPTIM_TimeOut_Start_IT(&hlptim1,65535,65535)!=HAL_OK)
	{
		while(1);
	}
	//__HAL_LPTIM_ENABLE_INTERRUPT(&hlptim1,LPTIM_IT_ARRM);
	// HAL_GPIO_TogglePin(GPIO_LED2,GPIO_PIN_LED2);

}

void TIM16_Init(void)
{
       
	TIM_IC_InitTypeDef sConfigIC;
	htim16.Instance = TIM16;
	htim16.Init.Prescaler = 0;
	htim16.Init.CounterMode = TIM_COUNTERMODE_UP;
	htim16.Init.Period = 0xFFFF;
	htim16.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	htim16.Init.RepetitionCounter = 0;
	htim16.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
	if (HAL_TIM_Base_Init(&htim16) != HAL_OK)
	{
		while(1);
	//_Error_Handler(__FILE__, __LINE__);
	}
	if (HAL_TIM_IC_Init(&htim16) != HAL_OK)
	{
		while(1);
	//_Error_Handler(__FILE__, __LINE__);
	}
	sConfigIC.ICPolarity = TIM_INPUTCHANNELPOLARITY_RISING;
	sConfigIC.ICSelection = TIM_ICSELECTION_DIRECTTI;
	sConfigIC.ICPrescaler = TIM_ICPSC_DIV1;
	sConfigIC.ICFilter = 0;
	if (HAL_TIM_IC_ConfigChannel(&htim16, &sConfigIC, TIM_CHANNEL_1) != HAL_OK)
	{
	//_Error_Handler(__FILE__, __LINE__);
	while(1);
	}
	if (HAL_TIMEx_RemapConfig(&htim16, TIM_TIM16_TI1_LSE) != HAL_OK)
	{
		while(1);
	//_Error_Handler(__FILE__, __LINE__);
	}
}
uint32_t GetSYSFrequency(void)
{
	uint16_t timer1=0;
	uint16_t timer2;
	uint16_t delay;
	uint32_t SYSFrequency;
	HAL_TIM_IC_Start(&htim16,TIM_CHANNEL_1);
	for(delay=0;delay<1000;delay++);
	__disable_irq();
    while(timer1==0)
    {
      timer1=HAL_TIM_ReadCapturedValue(&htim16,TIM_CHANNEL_1);
    }
	do
	{
		timer2=HAL_TIM_ReadCapturedValue(&htim16,TIM_CHANNEL_1);
	}
	while (timer1==timer2);
	__enable_irq();
	HAL_TIM_IC_Stop(&htim16,TIM_CHANNEL_1);
	timer2=timer2-timer1;
	SYSFrequency=timer2;
	SYSFrequency=SYSFrequency<<15;
	return SYSFrequency;
}





void	sctimer_set_callback(sctimer_cbt cb)
{
	sctimer_vars.sctimer_cb=cb;
}
void sctimer_setCompare(PORT_TIMER_WIDTH val)
{
	/*
	do
	{
		firstvalue=(PORT_TIMER_WIDTH)HAL_LPTIM_ReadCounter(&hlptim1);
	}
	while(firstvalue!=(PORT_TIMER_WIDTH)HAL_LPTIM_ReadCounter(&hlptim1));*/
	//IntEnable(INT_SMTIM);
	CurrentVaulezyx=val;
	if (((PORT_TIMER_WIDTH)(sctimer_readCounter()-val)) < TIMERLOOP_THRESHOLD){
	    // the timer is already late, schedule the ISR right now manually 
	    //IntPendSet(INT_SMTIM);
	    //触发中断
	    MARKSOFTINTERRUPTZYX=1;
	    EXTI->SWIER1|=0x01;
	    
		
	    
	} else {
	    if ((PORT_TIMER_WIDTH)(val-sctimer_readCounter())<MINIMUM_COMPAREVALE_ADVANCE){
	        // there is hardware limitation to schedule the timer within TIMERTHRESHOLD ticks
	        // schedule ISR right now manually
	        //IntPendSet(INT_SMTIM);
	        //触发中断
	        MARKSOFTINTERRUPTZYX=1;
	        EXTI->SWIER1|=0x01;
			
			
	    } else {
	        // schedule the timer at val//平时状态执行该条语句
	    	///debugpins_frame_set(); //lcg 20180602  set to clr normal <4us,when two setcompare close to about 30us,then the second long 10us.
	        while(!__HAL_LPTIM_GET_FLAG(&hlptim1,LPTIM_FLAG_CMPOK));
	       __HAL_LPTIM_CLEAR_FLAG(&hlptim1, LPTIM_FLAG_CMPOK);
	       __HAL_LPTIM_COMPARE_SET(&hlptim1, val);
	       ///debugpins_frame_clr(); //lcg 20180602
	    }
	}
}
//uint32_t	sctimer_readCounter(void)//PORT_TIMER_WIDTH
PORT_TIMER_WIDTH sctimer_readCounter(void)
{	
	
	//return *(__IO uint32_t *)(0x40007C1C);
	//return 0//HAL_LPTIM_ReadCounter(&hlptim1)
	//HAL_LPTIM_ReadCounter(&hlptim1);
#if 1
	uint32_t temp;
	do
	{
		temp=HAL_LPTIM_ReadCounter(&hlptim1);
		//debugpins_frame_toggle();  //lcg 20180529
	}while(temp!=HAL_LPTIM_ReadCounter(&hlptim1));   // lcg 20180602 (HAL_LPTIM_ReadCounter(&hlptim1)-(PORT_TIMER_WIDTH)temp)>1  not change yet
	return (PORT_TIMER_WIDTH)temp;
#endif
}
void sctimer_enable(void)
{
	HAL_LPTIM_TimeOut_Start_IT(&hlptim1, 65535,65535);
}
void sctimer_disable(void)
{
	HAL_LPTIM_TimeOut_Stop(&hlptim1);
	//HAL_LPTIM_TimeOut_Stop_IT();
}

// ========================== private =========================================


//=========================== interrupt handlers ==============================
kick_scheduler_t sctimer_isr()
{
	if (sctimer_vars.sctimer_cb!=NULL) 
	{
		// call the callback
		RCC_Wakeup();//忽略此处
		CurrentCountzyx=sctimer_readCounter();//*(__IO uint32_t *)(0x40007C1C);
		#if 0
		   if((testzyx12-testzyx13)<20)
		{
			//testzyx11=testzyx11;
		}else
		{
			HAL_GPIO_TogglePin(GPIO_LED1,GPIO_PIN_LED1);
			
			return DO_NOT_KICK_SCHEDULER;
			//testzyx12=testzyx12;
			MARKzyx=1;
			//printf("setting=%d\n",testzyx11);
			//printf("count=%d\n",*(__IO uint32_t *)(0x40007C1C));
			
		}
		#endif
		if (((PORT_TIMER_WIDTH)(CurrentCountzyx-CurrentVaulezyx)) < TIMERLOOP_THRESHOLD)
		{
		}
		else 
		{
			if ((PORT_TIMER_WIDTH)(CurrentVaulezyx-CurrentCountzyx)<MINIMUM_COMPAREVALE_ADVANCE)
			{
			}
			else
			{
				return DO_NOT_KICK_SCHEDULER;
			}
		}
		debugpins_fsm_set();  //lcg 20180521 add
		sctimer_vars.sctimer_cb();//实际上在调用OPENTIMERS的中断函数
		debugpins_fsm_clr(); //lcg 20180521 add
		// kick the OS
		return KICK_SCHEDULER;
	}
return DO_NOT_KICK_SCHEDULER;
	
}
