#ifndef __CEXAMPLE_H
#define __CEXAMPLE_H

/**
\addtogroup AppUdp
\{
\addtogroup cexample
\{
*/

#include "opencoap.h"

//=========================== define ==========================================

//=========================== typedef =========================================

typedef struct {
   coap_resource_desc_t   desc;
   opentimers_id_t       timerId;
} cexample_vars_t;

//=========================== variables =======================================

//=========================== prototypes ======================================

void cexample_init(void);
bool cexample_task_cb_Send_Uart_To_RF(uint8_t *data,uint8_t len);
/**
\}
\}
*/

#endif

