#include "zyx.h"
LPTIM_HandleTypeDef	hlptim2;
//ADC_HandleTypeDef hadc1;
void ADC_Timer_Init(void)
{
	hlptim2.Instance=LPTIM2;
	hlptim2.Init.Clock.Source=LPTIM_CLOCKSOURCE_APBCLOCK_LPOSC;
	hlptim2.Init.Clock.Prescaler=LPTIM_PRESCALER_DIV1;
	hlptim2.Init.Trigger.Source=LPTIM_TRIGSOURCE_SOFTWARE;
	hlptim2.Init.OutputPolarity=LPTIM_OUTPUTPOLARITY_HIGH;
	hlptim2.Init.UpdateMode=LPTIM_UPDATE_IMMEDIATE;//LPTIM_UPDATE_ENDOFPERIOD;
	hlptim2.Init.CounterSource=LPTIM_COUNTERSOURCE_INTERNAL;
	hlptim2.Init.Input1Source = LPTIM_INPUT1SOURCE_GPIO;
	hlptim2.Init.Input2Source = LPTIM_INPUT2SOURCE_GPIO;
	if (HAL_LPTIM_Init(&hlptim2) != HAL_OK)
	{
		while(1);
	}
}
void ADC_Mode_Init(void)
{
	
}

