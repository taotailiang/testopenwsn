#ifndef __UART_H
#define __UART_H

/**
\addtogroup BSP
\{
\addtogroup uart
\{

\brief Cross-platform declaration "uart" bsp module.

\author Thomas Watteyne <watteyne@eecs.berkeley.edu>, February 2012.
*/

#include "stdint.h"
#include "board.h"

//=========================== define ==========================================
#define OPEN_DEBUG_USART2
//=========================== typedef =========================================

typedef enum {
   UART_EVENT_THRES,
   UART_EVENT_OVERFLOW,
} uart_event_t;
typedef struct
{
	uint8_t TX_BUF[1000];
	uint8_t RX_BUF[1000];
	uint16_t TX_LEN;
	uint16_t RX_LEN;
}User_Data_TypeDef;
typedef void (*uart_tx_cbt)(void);
typedef void (*uart_rx_cbt)(void);
//typedef struct __FILE FILE;
//=========================== variables =======================================

//=========================== prototypes ======================================

void    uart_init(void);
void    uart_setCallbacks(uart_tx_cbt txCb, uart_rx_cbt rxCb);
void    uart_enableInterrupts(void);
void    uart_disableInterrupts(void);
void    uart_disableTxInterrupts(void);
void    uart_clearRxInterrupts(void);
void    uart_clearTxInterrupts(void);
void    uart_writeByte(uint8_t byteToWrite);
#ifdef FASTSIM
void    uart_writeCircularBuffer_FASTSIM(uint8_t* buffer, uint16_t* outputBufIdxR, uint16_t* outputBufIdxW);
#endif
uint8_t uart_readByte(void);
void HAL_UART_MspInit(UART_HandleTypeDef *huart);

#ifdef __GNUC__
	int __io_putchar(int ch);
#else
	int fputc(int ch, FILE *f);
#endif


// interrupt handlers
kick_scheduler_t uart_tx_isr(void);
kick_scheduler_t uart_rx_isr(void);
/*void UART_DMATransmitCplt(DMA_HandleTypeDef *hdma);*/
//void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart);



/**
\}
\}
*/

#endif
