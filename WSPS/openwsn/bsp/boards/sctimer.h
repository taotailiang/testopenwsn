#ifndef __SCTIMER_H
#define __SCTIMER_H

/**
\addtogroup BSP
\{
\addtogroup sctimer
\{

\brief A timer module with only a single compare value.

\author Tengfei Chang <tengfei.chang@inria.fr>, April 2017.
*/

#include "stdint.h"
#include "board.h"
#define TIMERLOOP_THRESHOLD          0x4000     // 所有的16位定时器所用TIMERLOOP TIME都是0x4000，都是0.5S
//#define OVERFLOW_THRESHOLD           0x7fffffff   // as openmotestm32 uses 16kHz, the upper timer overflows when timer research to 0x7fffffff
#define MINIMUM_COMPAREVALE_ADVANCE    5*LSE_64K  //3//此处为10,slot断的BUG不容易出20171228zyx   //lcg 20180521 add LSE_64K  12

//=========================== typedef =========================================

typedef void  (*sctimer_cbt)(void);
typedef void  (*sctimer_capture_cbt)(PORT_TIMER_WIDTH timestamp);

//=========================== variables =======================================


//=========================== prototypes ======================================
void	TIM16_Init(void);
uint32_t GetSYSFrequency(void);
void     sctimer_init(void);
void     sctimer_setCompare(PORT_TIMER_WIDTH val);
void     sctimer_set_callback(sctimer_cbt cb);
void     sctimer_setStartFrameCb(sctimer_capture_cbt cb);
void     sctimer_setEndFrameCb(sctimer_capture_cbt cb);
PORT_TIMER_WIDTH sctimer_readCounter(void);
//uint32_t sctimer_readCounter(void);

void     sctimer_enable(void);
void     sctimer_disable(void);

kick_scheduler_t sctimer_isr(void);

/**
\}
\}
*/

#endif
