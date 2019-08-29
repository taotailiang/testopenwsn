/**
\brief An example CoAP application.
*/

#include "opendefs.h"
#include "cexample.h"
#include "opencoap.h"
#include "openqueue.h"
#include "packetfunctions.h"
#include "openserial.h"
#include "openrandom.h"
#include "scheduler.h"
//#include "ADC_Channel.h"
#include "idmanager.h"
#include "IEEE802154E.h"
#include "icmpv6rpl.h" //lcg 20180622
#include "idmanager.h"
#include "zyx.h"
//=========================== defines =========================================

/// inter-packet period (in ms)
#define CEXAMPLEPERIOD  500  //lcg 20180504   pri:10000
#define PAYLOADLEN      64   //lcg 20180504   pri:40 25

const uint8_t cexample_path0[] = "ex";

//=========================== variables =======================================

cexample_vars_t cexample_vars;

uint16_t cex_send=0,cex_send_done=0,cex_send_fail=0; //lcg 20180509 add test comm  reliability ;
bool Last_Packet_SendDone=E_SUCCESS;

extern uint8_t Uart_TX_BUF[1024];
extern uint8_t Uart_RX_BUF[1024];
extern uint16_t Uart_Tx_BUF_LEN;




//=========================== prototypes ======================================

owerror_t cexample_receive( OpenQueueEntry_t* msg,
        coap_header_iht*  coap_header,
        coap_option_iht*  coap_incomingOptions,
        coap_option_iht*  coap_outgoingOptions,
        uint8_t*          coap_outgoingOptionsLen);

void    cexample_timer_cb(opentimers_id_t id);
void    cexample_sendDone(OpenQueueEntry_t* msg,
                       owerror_t error);

//=========================== public ==========================================

void cexample_init() {
    // prepare the resource descriptor for the /ex path
    cexample_vars.desc.path0len             = sizeof(cexample_path0)-1;
    cexample_vars.desc.path0val             = (uint8_t*)(&cexample_path0);
    cexample_vars.desc.path1len             = 0;
    cexample_vars.desc.path1val             = NULL;
    cexample_vars.desc.componentID          = COMPONENT_CEXAMPLE;
    cexample_vars.desc.securityContext      = NULL;
    cexample_vars.desc.discoverable         = TRUE;
    cexample_vars.desc.callbackRx           = &cexample_receive;
    cexample_vars.desc.callbackSendDone     = &cexample_sendDone;
    
    
    opencoap_register(&cexample_vars.desc);
    /*cexample_vars.timerId    = opentimers_create();
    opentimers_scheduleIn(
        cexample_vars.timerId, 
        CEXAMPLEPERIOD, 
        TIME_MS, 
        TIMER_PERIODIC,
        cexample_timer_cb
    );*/
}

//=========================== private =========================================

owerror_t cexample_receive( OpenQueueEntry_t* msg,
        coap_header_iht*  coap_header,
        coap_option_iht*  coap_incomingOptions,
        coap_option_iht*  coap_outgoingOptions,
        uint8_t*          coap_outgoingOptionsLen) 
{
	uint16_t i;
	//Uart_Tx_BUF_LEN=msg->length;
		//only master has same couple trig zyx 20181008
		if(idmanager_getIsDAGroot()&&(msg->payload[0]==UART_COMMUNICATION_HEAD_0x71))
		{
			///if(msg->payload[23]==ChkSum(&(msg->payload[4]),18))
			//{
				//如果校验通过就发送数据
				cexample_task_cb_Send_Uart_To_RF(&(msg->payload[0]),msg->length);
			///}else
			//{
				//如果校验不通过，可以输出调试信息
			//}
		}
	/*
	if(msg->payload[0]==UART_COMMUNICATION_HEAD_0x71)
	{//如果是同组触发包，就解析，直接写入open->openqueue
		
	}
	else//如果不是同组触发包，就直接往串口缓存区里扔
	{
		
	}*/
	
	for(i=0;i<msg->length;i++)
	{
	
		Uart_TX_BUF[(Uart_Tx_BUF_LEN++)&0x3FF]=msg->payload[i];
	}
	return E_FAIL;
}

//timer fired, but we don't want to execute task in ISR mode
//instead, push task to scheduler with COAP priority, and let scheduler take care of it
void cexample_timer_cb(opentimers_id_t id){
   //scheduler_push_task(cexample_task_cb_Send_Uart_To_RF,TASKPRIO_COAP);//zyx
}

//lcg 20180625
bool cexample_task_cb_Send_Uart_To_RF(uint8_t *data,uint8_t len)
{
   OpenQueueEntry_t*    pkt;
   owerror_t            outcome;
   uint8_t              i;
   coap_option_iht      options[2];
   
   uint16_t             x_int       = 0;
   uint16_t             sum         = 0;
   uint16_t             avg         = 0;
   uint8_t              N_avg       = 10;
   uint8_t              medtype;

   // don't run if not synch
   if (ieee154e_isSynch() == FALSE) return FALSE;
   
   // don't run on dagroot
   if (idmanager_getIsDAGroot()) {
     // opentimers_destroy(cexample_vars.timerId);
     // return;
   }else
   {
   	
   }
   
   HAL_GPIO_WritePin(GPIOA,GPIO_PIN_3,GPIO_PIN_SET);
   
   // create a CoAP RD packet
   pkt = openqueue_getFreePacketBuffer(COMPONENT_CEXAMPLE);
   if (pkt==NULL) 
   {
      openserial_printError(
         COMPONENT_CEXAMPLE,
         ERR_NO_FREE_PACKET_BUFFER,
         (errorparameter_t)0,
         (errorparameter_t)0
      );
      openqueue_freePacketBuffer(pkt);
      return 0;
   }
   // take ownership over that packet
   pkt->creator                   = COMPONENT_CEXAMPLE;
   pkt->owner                     = COMPONENT_CEXAMPLE;
  ///// #ifdef DAGROOT//主机才做判断 同组触发
   if(data[0]==UART_COMMUNICATION_HEAD_0x71)
   {
   	//pkt->creator=COMPONENT_USER_GROUPTRIG;
   	//pkt->owner=COMPONENT_USER_GROUPTRIG;
   }
  ///// #endif
   // CoAP payload
   packetfunctions_reserveHeaderSize(pkt,len);
   /*for (i=0;i<len;i++)//把串口缓存区的数据搞进去 
   {
      pkt->payload[i]             = data[i];
   }*/
   memcpy( &pkt->payload[0],data,len);
   //avg = openrandom_get16b();
   //pkt->payload[0]                = 0x99;
   //pkt->payload[1]                = (avg>>0)&0xff;
	
   // set location-path option
   options[0].type = COAP_OPTION_NONE;
   options[0].length = 0;
   options[0].pValue = (uint8_t *) cexample_path0;

   // set content-type option
   medtype = COAP_MEDTYPE_APPOCTETSTREAM;
   options[1].type = COAP_OPTION_NUM_CONTENTFORMAT;
   options[1].length = 0;
   options[1].pValue = &medtype;
   
   // metadata
   pkt->l4_destination_port       = WKP_UDP_COAP;//lcg 20180621   pri:WKP_UDP_COAP; WKP_UDP_TEST
   pkt->l3_destinationAdd.type    = ADDR_128B;
   if(1) //(idmanager_getIsDAGroot()==TRUE)//lcg 20180622
	{
	   memcpy(&pkt->l3_destinationAdd.addr_128b[0],&all_routers_multicast,16); // ipAddr_motesEecs
	   #if 0
	   if(idmanager_getIsDAGroot()&&AnyNeighborISlowerPowerRun())//
	   {
	   		pkt->LOWERPOWER_RUN=TRUE;
	   }
	   #endif
	}
   else
	   memcpy(&pkt->l3_destinationAdd.addr_128b[0],&ipAddr_motemaster,16); // ipAddr_motesEecs
   //lcg pkt->l3_destinationAdd.type    = ADDR_16B;
   //lcg memcpy(&pkt->l3_destinationAdd.addr_16b[0],&ipAddr_motemaster[14],2);
   // send
   outcome = opencoap_send(
      pkt,
      COAP_TYPE_NON,
	  COAP_CODE_EMPTY,//COAP_CODE_REQ_PUT,
      1, // token len
      options,
      0, // options len
      &cexample_vars.desc
   );
   
   // avoid overflowing the queue if fails
   if (outcome==E_FAIL) 
   {
      openqueue_freePacketBuffer(pkt);
   }
   else
   {
	   cex_send++;//lcg 20180509 add test comm  reliability ;
   }
   return 1;
}
void cexample_task_cb_bak() {
   OpenQueueEntry_t*    pkt;
   owerror_t            outcome;
   uint8_t              i;
   coap_option_iht      options[2];
   uint16_t             x_int       = 0;
   uint16_t             sum         = 0;
   uint16_t             avg         = 0;
   uint8_t              N_avg       = 10;
   uint8_t              medtype;
   if (ieee154e_isSynch() == FALSE) return;
   if (idmanager_getIsDAGroot()) 
   {
      opentimers_destroy(cexample_vars.timerId);
      return;
   }
   
   for (i = 0; i < N_avg; i++) {
      sum += x_int;
   }
   avg = sum/N_avg;
   
   // create a CoAP RD packet
   pkt = openqueue_getFreePacketBuffer(COMPONENT_CEXAMPLE);
   if (pkt==NULL) {
      openserial_printError(
         COMPONENT_CEXAMPLE,
         ERR_NO_FREE_PACKET_BUFFER,
         (errorparameter_t)0,
         (errorparameter_t)0
      );
      openqueue_freePacketBuffer(pkt);
      return;
   }
   // take ownership over that packet
   pkt->creator                   = COMPONENT_CEXAMPLE;
   pkt->owner                     = COMPONENT_CEXAMPLE;
   // CoAP payload
   packetfunctions_reserveHeaderSize(pkt,PAYLOADLEN);
   for (i=0;i<PAYLOADLEN;i++) {
      pkt->payload[i]             = i;
   }
   avg = openrandom_get16b();
   pkt->payload[0]                = (avg>>8)&0xff;
   pkt->payload[1]                = (avg>>0)&0xff;

   // set location-path option
   options[0].type = COAP_OPTION_NUM_URIPATH;
   options[0].length = sizeof(cexample_path0) - 1;
   options[0].pValue = (uint8_t *) cexample_path0;

   // set content-type option
   medtype = COAP_MEDTYPE_APPOCTETSTREAM;
   options[1].type = COAP_OPTION_NUM_CONTENTFORMAT;
   options[1].length = 1;
   options[1].pValue = &medtype;
   
   // metadata
   pkt->l4_destination_port       = WKP_UDP_COAP;
   pkt->l3_destinationAdd.type    = ADDR_128B;
   memcpy(&pkt->l3_destinationAdd.addr_128b[0],&ipAddr_motesEecs,16);
   
   // send
   outcome = opencoap_send(
      pkt,
      COAP_TYPE_NON,
      COAP_CODE_REQ_PUT,
      1, // token len
      options,
      2, // options len
      &cexample_vars.desc
   );
   
   // avoid overflowing the queue if fails
   if (outcome==E_FAIL) {
      openqueue_freePacketBuffer(pkt);
   }
   else{
	   cex_send++;//lcg 20180509 add test comm  reliability ;
   }
   
   return;
}

void cexample_sendDone(OpenQueueEntry_t* msg, owerror_t error) {
    openqueue_freePacketBuffer(msg);
    HAL_GPIO_WritePin(GPIOA,GPIO_PIN_3,GPIO_PIN_RESET);
    OpenSerialBuf_GetDatatoSend();
    #if 0//zyx20190215
    if(error==E_SUCCESS)
    {
    	Last_Packet_SendDone=E_SUCCESS;
    	cex_send_done++;//lcg 20180509 add test comm  reliability ;
    }else
    {
    	Last_Packet_SendDone=E_SUCCESS;
    }
    if(cex_send!=cex_send_done)
    {
    	cex_send_fail++;
    	openserial_printError(COMPONENT_CEXAMPLE,90,
    	                            (errorparameter_t)cex_send_fail,
    	                            (errorparameter_t)cex_send);

    	cex_send_done=cex_send;
    }
    #endif
}



