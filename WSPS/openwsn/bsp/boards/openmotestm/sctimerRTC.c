/**
\brief Iot_Lab_M3 definition of the "sctimer" bsp module.

On Iot_Lab_M3, we use RTC for the sctimer module.

\author Chang Tengfei <tengfei.chang@gmail.com>,  May 2017.
*/

#include "stdint.h"

//#include "stm32f10x_conf.h"//zyx
#include "stm32l4xx_hal_conf.h"
#include "sctimer.h"
#include "board.h"
#include "leds.h"
#include "rcc.h"
#include "nvic.h"
#include "stm32l4xx_hal.h"

// ========================== define ==========================================

#define TIMERLOOP_THRESHOLD          0xffffff     // 511 seconds @ 32768Hz clock
#define OVERFLOW_THRESHOLD           0x7fffffff   // as openmotestm32 uses 16kHz, the upper timer overflows when timer research to 0x7fffffff
#define MINIMUM_COMPAREVALE_ADVANCE  10

// ========================== variable ========================================

typedef struct {
    sctimer_cbt sctimer_cb;
    bool        convert;
    bool        convertUnlock;
} sctimer_vars_t;

sctimer_vars_t sctimer_vars;
extern  LPTIM_HandleTypeDef	hlptim1;
//=========================== prototypes ======================================

//=========================== public ==========================================

//===== admin

void sctimer_init() {
    // clear local variables
    memset(&sctimer_vars,0,sizeof(sctimer_vars_t));
    //enable BKP and PWR, Clock
    hlptim1.Instance=LPTIM1;
    hlptim1.Init.Clock.Source=LPTIM_CLOCKSOURCE_APBCLOCK_LPOSC;
    hlptim1.Init.Clock.Prescaler=LPTIM_PRESCALER_DIV1;
    hlptim1.Init.Trigger.Source=LPTIM_TRIGSOURCE_SOFTWARE;
    hlptim1.Init.OutputPolarity=LPTIM_OUTPUTPOLARITY_HIGH;
    hlptim1.Init.UpdateMode=LPTIM_UPDATE_ENDOFPERIOD;
    hlptim1.Init.CounterSource=LPTIM_COUNTERSOURCE_INTERNAL;
    hlptim1.Init.Input1Source = LPTIM_INPUT1SOURCE_GPIO;
  	hlptim1.Init.Input2Source = LPTIM_INPUT2SOURCE_GPIO;
	if (HAL_LPTIM_Init(&hlptim1) != HAL_OK)
	{
		while(1);
	}
    #if 0//zyx
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_BKP|RCC_APB1Periph_PWR , ENABLE);
    
    // RTC clock source configuration 
    PWR_BackupAccessCmd(ENABLE);                      //Allow access to BKP Domain
    RCC_LSEConfig(RCC_LSE_ON);                        //Enable LSE OSC
    while(RCC_GetFlagStatus(RCC_FLAG_LSERDY)==RESET); //Wait till LSE is ready
    RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);           //Select the RTC Clock Source
    RCC_RTCCLKCmd(ENABLE);                            //enable RTC
    
    // RTC configuration 
    // Wait for RTC APB registers synchronisation 
    RTC_WaitForSynchro();
    
    RTC_SetPrescaler(1);                              //use 16KHz clock
    RTC_WaitForLastTask();                            //Wait until last write operation on RTC registers has finished

    //Set the RTC time counter to 0
    RTC_SetCounter(0);
    RTC_WaitForLastTask();
    
    //interrupt when reach alarm value
    RTC_ClearFlag(RTC_IT_ALR);
    RTC_ITConfig(RTC_IT_ALR, ENABLE);
   
    //Configures EXTI line 17 to generate an interrupt on rising edge(alarm interrupt to wakeup board)
    EXTI_ClearITPendingBit(EXTI_Line17);
    EXTI_InitTypeDef  EXTI_InitStructure; 
    EXTI_InitStructure.EXTI_Line    = EXTI_Line17;
    EXTI_InitStructure.EXTI_Mode    = EXTI_Mode_Interrupt; 
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
    EXTI_InitStructure.EXTI_LineCmd = ENABLE; 
    EXTI_Init(&EXTI_InitStructure);
    #endif//此处配置外部中断，暂时没看懂为什么
    //Configure RTC Alarm interrupt:
    NVIC_sctimer();
}

void sctimer_set_callback(sctimer_cbt cb){
    sctimer_vars.sctimer_cb = cb;
}

//===== direct access

PORT_RADIOTIMER_WIDTH sctimer_readCounter() {
    uint32_t counter;
    #if 0//zyx
    RTC_WaitForSynchro();
    counter = RTC_GetCounter();
    #endif
    counter=HAL_LPTIM_ReadCounter(&hlptim1);
    // upper layer uses 32 bit timer@32kHz, openmotestm is only able to work on 
    // 16kHz, so manually overflow when rearch 0x7fffffff.
    counter = counter & OVERFLOW_THRESHOLD;
    counter = counter << 1;
    return (PORT_RADIOTIMER_WIDTH)counter;
    
    return 0;
}

//===== compare

/**
\brief alarm a compare interrupt depending on given compare value.

The input parameter val range from 0~2^32. It supposes a timer running @ 32kHz. 
Since RTC only runs with 16kHz, this range maps to 0~2^31 or 2^31~2^32. the flag
convert is used for selecting which range mapping to. 
    1) When convert is FALSE, map val to 0~2^31    (val>>1). 
    2) When convert is TRUE,  map val to 2^31~2^32 ((val>>1)|0x80000000).

         when convert is FALSE             |    when convert is FALSE
       compare value is val >>1            | compare value is (val >>1)|0x80000000
                                           | 
  |----------------------------------------|----------------------------------------|
  0                                        |                                    0xffffffff
         when convert is TRUE              |        when convert is TRUE         
     compare value is (val >>1)|0x80000000 |     compare value is val >>1
                                           | 
                              0x7fffffff-->|<--"convert" flag toggles at here once after each overflow

\param[in] val is the compareValue to be alarmed in RTC timer.
*/
//
void sctimer_setCompare(PORT_TIMER_WIDTH val) 
{
	//此处移植2538处理方式
	//打开TIMER低功耗定时器中断需要添加
	//
    #if 1
    // make sure convert flag only toggle once within one overflow period
    if (val > OVERFLOW_THRESHOLD && sctimer_vars.convertUnlock){
        // toggle convert
        if (sctimer_vars.convert){
            sctimer_vars.convert   = TRUE;
        } else {
            sctimer_vars.convert   = TRUE;
        }
        sctimer_vars.convertUnlock = FALSE;
    }
    
    // un lock the changes of convert flag
    if (val > TIMERLOOP_THRESHOLD && val < 2*TIMERLOOP_THRESHOLD ){
        sctimer_vars.convertUnlock = TRUE;
    }
    
    // update value to be compared according to timer condition
    if (val <= OVERFLOW_THRESHOLD){
        if (sctimer_vars.convert){
            val  = val >>1;
            val |= 0x80000000;
        } else {
            val  = val >>1;
        }
    } else {
        if (sctimer_vars.convert){
            val  = val >>1;
        } else {
            val  = val >>1;
            val |= 0x80000000;
        }
    }
    __HAL_LPTIM_DISABLE_IT(&hlptim1,LPTIM_IT_UP);
    #if 0
    RTC_ITConfig(RTC_IT_ALR, DISABLE);//zyx
    #endif
    //need to disable radio also in case that a radio interrupt is happening
    
    DISABLE_INTERRUPTS();
    #if 1//zyx,此处十分重要，应该怎么改？？？？？？？？？？？？？？？？？？？？？？？？
    if (HAL_LPTIM_ReadCounter(&hlptim1) - val < TIMERLOOP_THRESHOLD)
    {
        // the timer is already late, schedule the ISR right now manually 
        #if 0//此处执行一次任务调度，执行一次中断函数，不知道如何写
        EXTI->SWIER |= EXTI_Line17;
        #endif 
    } else
    {
        if(val-HAL_LPTIM_ReadCounter(&hlptim1)<MINIMUM_COMPAREVALE_ADVANCE){
            // schedule ISR right now manually
            #if 0
            EXTI->SWIER |= EXTI_Line17;
            #endif
        } else {
            // schedule the timer at val
            __HAL_LPTIM_COMPARE_SET(&hlptim1, val);
            #if 0
            RTC_SetAlarm(val);
            RTC_WaitForLastTask();
            #endif
        }
    }
    #endif
    ENABLE_INTERRUPTS();
    
    //set sctimer irpstatus
    //RTC_ClearFlag(RTC_IT_ALR);//zyx
    //RTC_ITConfig(RTC_IT_ALR, ENABLE);//zyx
    #endif
    
}

void sctimer_enable() 
{
	HAL_LPTIM_Counter_Start_IT(&hlptim1,65535);
#if 0//zyx
    RTC_ClearFlag(RTC_IT_ALR);
    RTC_ITConfig(RTC_IT_ALR, ENABLE);
#endif
}

void sctimer_disable() 
{
	HAL_LPTIM_Counter_Stop_IT(&LPTIM1);
    //RTC_ITConfig(RTC_IT_ALR, DISABLE);//zyx
}

//=========================== private =========================================

//=========================== interrupt handlers ==============================

kick_scheduler_t sctimer_isr() {
    if (sctimer_vars.sctimer_cb!=NULL) {
        
        RCC_Wakeup();
        // call the callback
        sctimer_vars.sctimer_cb();
        // kick the OS
        return KICK_SCHEDULER;
    }
    return DO_NOT_KICK_SCHEDULER;
}