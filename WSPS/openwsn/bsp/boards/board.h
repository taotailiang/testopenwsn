#ifndef __BOARD_H
#define __BOARD_H


#define OPEN_IWDG
/**
\addtogroup BSP
\{
\addtogroup board
\{

\brief Cross-platform declaration "board" bsp module.

\author Thomas Watteyne <watteyne@eecs.berkeley.edu>, February 2012.
*/
#define  POLY_CRC32	0x04C11DB7//0x04c011bb7

#include "board_info.h"
#include "toolchain_defs.h"
#include "stm32L4xx_hal.h"
#include "S2LP_Config.h"
//V2 use 64k V3 use 32k
//#define BOARD_VERSION_V2//V1为最原始版本，V2为64k版本 V3为32k版本,
#define BOARD_VERSION_V3//

//=========================== define ==========================================
/*
#define	GPIO_RFCS	GPIOB
#define	GPIO_PIN_RFCS	GPIO_PIN_6
#define GPIO_RF_SPIMOSI	GPIOA
#define GPIO_PIN_RF_SPIMOSI_	GPIO_PIN_7
#define GPIO_RF_SPIMISO	GPIOA
#define	GPIO_PIN_RF_SPIMISO	GPIO_PIN_6
#define GPIO_RF_SPISCK	GPIOA
#define	GPIO_PIN_RF_SPISCK	GPIO_PIN_5
#define GPIO_RFSDN		GPIOA
#define GPIO_PIN_RFSDN		GPIO_PIN_8
#define GPIO_DEBUG	GPIOC
#define	GPIO_PIN_DEBUG	GPIO_PIN_2*/
typedef enum {
   DO_NOT_KICK_SCHEDULER,
   KICK_SCHEDULER,
} kick_scheduler_t;

//=========================== typedef =========================================

//=========================== variables =======================================

//=========================== prototypes ======================================

void board_init(void);
void board_sleep(void);
void board_reset(void);

/**
\}
\}
*/

#endif
