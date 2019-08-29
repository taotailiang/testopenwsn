/**
\brief openmoteSTM32 board information bsp module.

This module simply defines some strings describing the board, which CoAP uses
to return the board's description.

\author Thomas Watteyne <watteyne@eecs.berkeley.edu>, February 2012.
\author Tengfei Chang <tengfei.chang@gmail.com>,  July 2012.
*/

#ifndef __BOARD_INFO_H
#define __BOARD_INFO_H

//#include "stm32f10x_lib.h"//zyx
#include	"stm32l4xx_hal.h"
#include "S2LP_Config.h"
#include "stdint.h"
#include "string.h"

//=========================== defines =========================================

//TODO in case previous declaration fails in certain compilers. Remove this 
//one if it works with GNU GCC
//#define PACK_START  _Pragma("pack(1)")
//#define PACK_END    _Pragma("pack()")

//======clock lcg 20180424  1:32768   2:32768*2
#define LSE_64K  1


#define INTERRUPT_DECLARATION(); //no declaration

#define DISABLE_INTERRUPTS()    __disable_irq();//NVIC_SETPRIMASK();//zyx
#define ENABLE_INTERRUPTS()     __enable_irq();//NVIC_RESETPRIMASK();//zyx

//===== timer

#define PORT_TIMER_WIDTH                    uint16_t//uint32_t
#define PORT_TIMER_WIDTH_L                  uint32_t  //lcg 20180918 add
#define PORT_RADIOTIMER_WIDTH               uint16_t//uint32_t

#define PORT_SIGNED_INT_WIDTH               int16_t//int32_t
#define PORT_SIGNED_INT_WIDTH_L               int32_t //lcg 20180918 add
#define PORT_TICS_PER_MS                    33*LSE_64K //32
#define SCHEDULER_WAKEUP()                  //EXTI->SWIER1|=0x01;//zyx20180308
#define SCHEDULER_ENABLE_INTERRUPT()        //enable in board use EXTI_Line1

// this is a workaround from the fact that the interrupt pin for the GINA radio
// is not connected to a pin on the MSP which allows time capture.
#define CAPTURE_TIME()  //TACCTL2 |=  CCIS0;  \
                        //TACCTL2 &= ~CCIS0;//zyx

//===== pinout
// [P4.7] radio SLP_TR_CNTL//
#define PORT_PIN_RADIO_SLP_TR_CNTL_HIGH()     HAL_GPIO_WritePin(GPIO_RFEXTI0,GPIO_PIN_RFEXTI0,GPIO_PIN_SET);//GPIOB->ODR |= 0X0002;//zyx
#define PORT_PIN_RADIO_SLP_TR_CNTL_LOW()      HAL_GPIO_WritePin(GPIO_RFEXTI0,GPIO_PIN_RFEXTI0,GPIO_PIN_RESET);//GPIOB->ODR &= ~0X0002;//zyx
// radio reset line
// radio /RST//shun down is sample to reset
#define PORT_PIN_RADIO_RESET_HIGH()       HAL_GPIO_WritePin(GPIO_RFSDN,GPIO_PIN_RFSDN,GPIO_PIN_SET);//GPIOC->ODR |= 0X0040;
#define PORT_PIN_RADIO_RESET_LOW()        HAL_GPIO_WritePin(GPIO_RFSDN,GPIO_PIN_RFSDN,GPIO_PIN_RESET);//GPIOC->ODR &= ~0X0040;

//===== IEEE802154E timing

// time-slot related
#define PORT_TsSlotDuration                 512*LSE_64K// counter counts one extra count, see datasheet  491
// execution speed related
#define PORT_maxTxDataPrepare               66*LSE_64K    // 2014us (measured 746us)
#define PORT_maxRxAckPrepare                20*LSE_64K    //  305us (measured  83us)
#define PORT_maxRxDataPrepare               33*LSE_64K    // 1007us (measured  84us)
#define PORT_maxTxAckPrepare                30*LSE_64K    //  305us (measured 219us)
// radio speed related
#define PORT_delayTx                        25//pri17*LSE_64Kzyx20180927//18///10     //  214us (measured 219us)  lcg 20180428   pri:12
#define PORT_delayRx                        0*LSE_64K     //    0us (can not measure)
// radio watchdog

//===== adaptive_sync accuracy

#define SYNC_ACCURACY                       1     // ticks

//=========================== typedef  ========================================

//=========================== variables =======================================

static const uint8_t rreg_uriquery[]        = "h=ucb";
static const uint8_t infoBoardname[]        = "OPENMOTESTM32";
static const uint8_t infouCName[]           = "STM32L431";
static const uint8_t infoRadioName[]        = "S2LP";

//=========================== prototypes ======================================

//=========================== public ==========================================

//=========================== private =========================================

#endif
