#include "opendefs.h"
#include "IEEE802154E.h"
#include "radio.h"
#include "IEEE802154.h"
#include "IEEE802154_security.h"
#include "openqueue.h"
#include "idmanager.h"
#include "openserial.h"
#include "schedule.h"
#include "packetfunctions.h"
#include "scheduler.h"
#include "leds.h"
#include "neighbors.h"
#include "debugpins.h"
#include "sixtop.h"
#include "adaptive_sync.h"
#include "sctimer.h"
#include "openrandom.h"
#include "zyx.h"

//=========================== variables =======================================

ieee154e_vars_t    ieee154e_vars;
ieee154e_stats_t   ieee154e_stats;
ieee154e_dbg_t     ieee154e_dbg;
extern CRC_HandleTypeDef CrcHandle;
extern	uint8_t Uart_TX_BUF[1024];
extern	uint16_t Uart_Tx_BUF_LEN;
uint8_t FCI_LETME_LOWERPOWERON=0;

//=========================== prototypes ======================================
void ieee154e_prepare_calling_package(OpenQueueEntry_t* eb);

// SYNCHRONIZING
void     activity_synchronize_new_listen_Preamble_Slot(void);
void     activity_synchronize_newSlot(void);
void     activity_synchronize_startOfFrame(PORT_TIMER_WIDTH capturedTime);
void     activity_synchronize_endOfFrame(PORT_TIMER_WIDTH capturedTime);
// TX
void     activity_ti1ORri1(void);
void     activity_ti2(void);
void     activity_tie1(void);
void     activity_ti3(void);
void     activity_tie2(void);
void     activity_ti4(PORT_TIMER_WIDTH capturedTime);
void     activity_tie3(void);
void     activity_ti5(PORT_TIMER_WIDTH capturedTime);
void     activity_ti6(void);
void     activity_tie4(void);
void     activity_ti7(void);
void     activity_tie5(void);
void     activity_ti8(PORT_TIMER_WIDTH capturedTime);
void     activity_tie6(void);
void     activity_ti9(PORT_TIMER_WIDTH capturedTime);
// RX
void     activity_ri2(void);
void     activity_rie1(void);
void     activity_ri3(void);
void     activity_rie2(void);
void     activity_ri4(PORT_TIMER_WIDTH capturedTime);
void     activity_rie3(void);
void     activity_ri5(PORT_TIMER_WIDTH capturedTime);
void     activity_ri6(void);
void     activity_rie4(void);
void     activity_ri7(void);
void     activity_rie5(void);
void     activity_ri8(PORT_TIMER_WIDTH capturedTime);
void     activity_rie6(void);
void     activity_ri9(PORT_TIMER_WIDTH capturedTime);
void    activity_ci1(void);
void	activity_ci2(void);
void	activity_ci3(PORT_TIMER_WIDTH capturedTime);
void	activity_ci4(PORT_TIMER_WIDTH capturedTime);
void	activity_cie1(void);
void	activity_si1(void);
void 	activity_si2(void);
void 	activity_sie1(void);



// frame validity check
bool     isValidRxFrame(ieee802154_header_iht* ieee802514_header);
bool     isValidAck(ieee802154_header_iht*     ieee802514_header,
                    OpenQueueEntry_t*          packetSent);
bool     isValidJoin(OpenQueueEntry_t* eb, ieee802154_header_iht *parsedHeader); 
bool     isValidEbFormat(OpenQueueEntry_t* pkt, uint16_t* lenIE);
// IEs Handling
bool     ieee154e_processIEs(OpenQueueEntry_t* pkt, uint16_t* lenIE);
void     timeslotTemplateIDStoreFromEB(uint8_t id);
void     channelhoppingTemplateIDStoreFromEB(uint8_t id);
// ASN handling
void     incrementAsnOffset(void);
void     ieee154e_resetAsn(void);
void     ieee154e_syncSlotOffset(void);
void     asnStoreFromEB(uint8_t* asn);
void     joinPriorityStoreFromEB(uint8_t jp);
// synchronization
void 	 synchronizecallingPacket(PORT_TIMER_WIDTH timeReceived);
void     synchronizePacket(PORT_TIMER_WIDTH timeReceived);
void     synchronizeAck(PORT_SIGNED_INT_WIDTH timeCorrection);
void     changeIsSync(bool newIsSync);
// notifying upper layer
void     notif_sendDone(OpenQueueEntry_t* packetSent, owerror_t error);
void     notif_receive(OpenQueueEntry_t* packetReceived);
// statistics
void     resetStats(void);
void     updateStats(PORT_SIGNED_INT_WIDTH timeCorrection);
// misc
uint8_t  calculateFrequency(uint8_t channelOffset);
void     changeState(ieee154e_state_t newstate);
void     endSlot(void);
bool     debugPrint_asn(void);
bool     debugPrint_isSync(void);
// interrupts
void     isr_ieee154e_newSlot(opentimers_id_t id);
void     isr_ieee154e_timer(opentimers_id_t id);

//=========================== admin ===========================================

/**
\brief This function initializes this module.

Call this function once before any other function in this module, possibly
during boot-up.
*/
#ifdef USE_FLASH_PARAMETER
void ieee154e_init() 
{
   // initialize variables
   memset(&ieee154e_vars,0,sizeof(ieee154e_vars_t));
   memset(&ieee154e_dbg,0,sizeof(ieee154e_dbg_t));
   if(*(__IO uint8_t *)(ADDR_PARAMENT_STARTADDRESS+OFFSETADDR_LOWERPOWER_CALLINGON)==1)
   {
   		ieee154e_vars.Mode=LOWERPOWER_CALLINGON;//LOWERPOWER_RUN;//
   }else
   {
   		ieee154e_vars.Mode=HIGHPOWER_CALLINGOFF;
   }
   ieee154e_vars.singleChannel     = 0; // 0 means channel hopping 020180115
   //ieee154e_vars.singleChannel     = 26;//zyx20171218
   ieee154e_vars.isAckEnabled      = TRUE;
   ieee154e_vars.isSecurityEnabled = FALSE;
   ieee154e_vars.slotDuration      = TsSlotDuration;
   ieee154e_vars.numOfSleepSlots   = 1;
   
   // default hopping template
   /*
   memcpy(
       &(ieee154e_vars.chTemplate[0]),
       chTemplate_default,
       sizeof(ieee154e_vars.chTemplate)
   );*/
    memcpy(
       &(ieee154e_vars.chTemplate[0]),
       &chTemplate_zyx[(*(__IO uint8_t *)(ADDR_PARAMENT_STARTADDRESS+OFFSETADDR_WIRELESS_CHANNEL_JUMP))&0x0F][0],
       sizeof(ieee154e_vars.chTemplate)
   );
   if (idmanager_getIsDAGroot()==TRUE) 
   {
      changeIsSync(TRUE);
   } else {
      changeIsSync(FALSE);
      if(ieee154e_vars.Mode==LOWERPOWER_CALLINGON)
      {
      	radio_ENABLE_PREAMBLE_INTERRUPT();
      }
   }
   ieee154e_prepare_calling_package(&ieee154e_vars.CallingQueue);
   //ieee154e_changemode(HIGHPOWER_CALLINGOFF);
   resetStats();
   ieee154e_stats.numDeSync                 = 0;
   // switch radio on
   radio_rfOn();//reset it zyx
   
   // set callback functions for the radio
   //radiotimer_setOverflowCb(isr_ieee154e_newSlot);
   //radiotimer_setCompareCb(isr_ieee154e_timer);
   radio_setStartFrameCb(ieee154e_startOfFrame);
   radio_setEndFrameCb(ieee154e_endOfFrame);
   // have the radio start its timer
   ieee154e_vars.timerId = opentimers_create();
   // assign ieee802154e timer with highest priority
   opentimers_setPriority(ieee154e_vars.timerId,0);
   opentimers_scheduleAbsolute(
        ieee154e_vars.timerId,          // timerId
        ieee154e_vars.slotDuration,     // duration
        sctimer_readCounter(),          // reference
        TIME_TICS,                      // timetype
        isr_ieee154e_newSlot            // callback
   );
   // radiotimer_start(ieee154e_vars.slotDuration);
   //
   IEEE802154_security_init();
}
#else
void ieee154e_init() {
   
   // initialize variables
   memset(&ieee154e_vars,0,sizeof(ieee154e_vars_t));
   memset(&ieee154e_dbg,0,sizeof(ieee154e_dbg_t));
   ieee154e_vars.Mode=HIGHPOWER_CALLINGOFF;//LOWERPOWER_RUN;//
   ieee154e_vars.singleChannel     = 0; // 0 means channel hopping 0是跳频20180115
   //ieee154e_vars.singleChannel     = 26;//zyx20171218
   ieee154e_vars.isAckEnabled      = TRUE;
   ieee154e_vars.isSecurityEnabled = FALSE;
   ieee154e_vars.slotDuration      = TsSlotDuration;
   ieee154e_vars.numOfSleepSlots   = 1;
   
   // default hopping template
   memcpy(
       &(ieee154e_vars.chTemplate[0]),
       chTemplate_default,
       sizeof(ieee154e_vars.chTemplate)
   );
   
   if (idmanager_getIsDAGroot()==TRUE) 
   {
      changeIsSync(TRUE);
   } else {
      changeIsSync(FALSE);
      //move it to the radio init
      /*
      if(ieee154e_vars.Mode==LOWERPOWER_CALLINGON)
      {
      	radio_ENABLE_PREAMBLE_INTERRUPT();
      }*/
   }
   ieee154e_prepare_calling_package(&ieee154e_vars.CallingQueue);
   ieee154e_changemode(HIGHPOWER_CALLINGOFF);
   resetStats();
   ieee154e_stats.numDeSync                 = 0;
   // switch radio on
   radio_rfOn();//reset it zyx
   
   // set callback functions for the radio
   //radiotimer_setOverflowCb(isr_ieee154e_newSlot);
   //radiotimer_setCompareCb(isr_ieee154e_timer);
   radio_setStartFrameCb(ieee154e_startOfFrame);
   radio_setEndFrameCb(ieee154e_endOfFrame);
   // have the radio start its timer
   ieee154e_vars.timerId = opentimers_create();
   // assign ieee802154e timer with highest priority
   opentimers_setPriority(ieee154e_vars.timerId,0);
   opentimers_scheduleAbsolute(
        ieee154e_vars.timerId,          // timerId
        ieee154e_vars.slotDuration,     // duration
        sctimer_readCounter(),          // reference
        TIME_TICS,                      // timetype
        isr_ieee154e_newSlot            // callback
   );
   // radiotimer_start(ieee154e_vars.slotDuration);
   //
   IEEE802154_security_init();
}

#endif
void ieee154e_prepare_calling_package(OpenQueueEntry_t* eb)
{
    uint8_t     i;
    uint8_t     eb_len;
    uint16_t    temp16b;
   openqueue_reset_entry(eb);
    eb->creator = COMPONENT_SIXTOP;//just put the calling packet here this type zyx
    eb->owner   = COMPONENT_SIXTOP;
    
    // in case we none default number of shared cells defined in minimal configuration
    #if 1
    if (ebIEsBytestream[EB_SLOTFRAME_NUMLINK_OFFSET]>1)//follow the type of EB zyx
    {
        for (i=ebIEsBytestream[EB_SLOTFRAME_NUMLINK_OFFSET]-1;i>0;i--)
        {
            packetfunctions_reserveHeaderSize(eb,5);
            eb->payload[0]   = i;    // slot offset
            eb->payload[1]   = 0x00;
            eb->payload[2]   = 0x00; // channel offset
            eb->payload[3]   = 0x00;
            eb->payload[4]   = 0x0F; // link options
        }
    }
    #endif
    // reserve space for EB IEs
    packetfunctions_reserveHeaderSize(eb,EB_IE_LEN);
    for (i=0;i<EB_IE_LEN;i++){
        eb->payload[i]   = ebIEsBytestream[i];
    }
    
    if (ebIEsBytestream[EB_SLOTFRAME_NUMLINK_OFFSET]>1)
    {
        // reconstruct the MLME IE header since length changed 
        eb_len = EB_IE_LEN-2+5*(ebIEsBytestream[EB_SLOTFRAME_NUMLINK_OFFSET]-1);
        temp16b = eb_len | IEEE802154E_PAYLOAD_DESC_GROUP_ID_MLME | IEEE802154E_PAYLOAD_DESC_TYPE_MLME;
        eb->payload[0] = (uint8_t)(temp16b & 0x00ff);
        eb->payload[1] = (uint8_t)((temp16b & 0xff00)>>8);
    }
    
    // Keep a pointer to where the ASN will be
    // Note: the actual value of the current ASN and JP will be written by the
    //    IEEE802.15.4e when transmitting
    eb->l2_ASNpayload               = &eb->payload[EB_ASN0_OFFSET]; 
  
    // some l2 information about this packet
    eb->l2_frameType                     = IEEE154_TYPE_CALLING;//add framType ACK or NOACK
    eb->l2_nextORpreviousHop.type        = ADDR_16B;//ADDR_CALLING;
    eb->l2_nextORpreviousHop.addr_16b[0] = 0xFF;
    eb->l2_nextORpreviousHop.addr_16b[1] = 0xFF;
    //I has an IE in my payload
    eb->l2_payloadIEpresent = TRUE;

    // set l2-security attributes
    eb->l2_securityLevel   = IEEE802154_SECURITY_LEVEL_BEACON;
    eb->l2_keyIdMode       = IEEE802154_SECURITY_KEYIDMODE;
    eb->l2_keyIndex        = IEEE802154_security_getBeaconKeyIndex();

    // put in queue for MAC to handle
    // record this packet's dsn (for matching the ACK)
    eb->l2_dsn =eb->l2_dsn++; //sixtop_vars.dsn++;//pri zyx20180820
    // this is a new packet which I never attempted to send
    eb->l2_numTxAttempts = 0;
    // transmit with the default TX power
    eb->l1_txPower = TX_POWER;
    // add a IEEE802.15.4 header
    ieee802154_prependHeader(
        eb,
        eb->l2_frameType,
       	eb->l2_payloadIEpresent,
        eb->l2_dsn,
        &(eb->l2_nextORpreviousHop)
    );
    // change owner to IEEE802154E fetches it from queue
    eb->owner  = COMPONENT_SIXTOP_TO_IEEE802154E;
   
}
//=========================== public ==========================================

/**
/brief Difference between some older ASN and the current ASN.

\param[in] someASN some ASN to compare to the current

\returns The ASN difference, or 0xffff if more than 65535 different
*/
PORT_TIMER_WIDTH ieee154e_asnDiff(asn_t* someASN) {
   PORT_TIMER_WIDTH diff;
   INTERRUPT_DECLARATION();
   DISABLE_INTERRUPTS();
   if (ieee154e_vars.asn.byte4 != someASN->byte4) {
      ENABLE_INTERRUPTS();
      return (PORT_TIMER_WIDTH)0xFFFFFFFF;;
   }
   
   diff = 0;
   if (ieee154e_vars.asn.bytes2and3 == someASN->bytes2and3) {
      ENABLE_INTERRUPTS();
      return ieee154e_vars.asn.bytes0and1-someASN->bytes0and1;
   } else if (ieee154e_vars.asn.bytes2and3-someASN->bytes2and3==1) {
      diff  = ieee154e_vars.asn.bytes0and1;
      diff += 0xffff-someASN->bytes0and1;
      diff += 1;
   } else {
      diff = (PORT_TIMER_WIDTH)0xFFFFFFFF;;
   }
   ENABLE_INTERRUPTS();
   return diff;
}

#ifdef DEADLINE_OPTION_ENABLED
/**
/brief Difference between two ASN values

\param[in] h_asn bigger ASN value
\param[in] l_asn smaller ASN value

\returns The ASN difference, or 0xffff if more than 65535 different
*/
int16_t ieee154e_computeAsnDiff(asn_t* h_asn, asn_t* l_asn) {
   int16_t diff;

   if (h_asn->byte4 != l_asn->byte4) {
      return (int16_t)0xFFFFFFFF;
   }
   
   diff = 0;
   if (h_asn->bytes2and3 == l_asn->bytes2and3) {
      return h_asn->bytes0and1-l_asn->bytes0and1;
   } else if (h_asn->bytes2and3-l_asn->bytes2and3==1) {
      diff  = h_asn->bytes0and1;
      diff += 0xffff-l_asn->bytes0and1;
      diff += 1;
   } else {
      diff = (int16_t)0xFFFFFFFF;
   }
   return diff;
}

/**
/brief Determine Expiration Time in ASN

\param[in]  max_delay Maximum permissible delay before which 
            packet is expected to reach destination

\param[out] et_asn bigger ASN value
*/
void ieee154e_calculateExpTime(uint16_t max_delay, uint8_t* et_asn) {
   uint8_t delay_array[5];
   uint8_t i =0, carry = 0,slot_time = 0;
   uint16_t sum = 0, delay_in_asn =0;
	
   memset(&delay_array[0],0,5);
   
   //Slot time = (Duration in ticks * Time equivalent ticks w.r.t 32kHz) in ms
   slot_time = (ieee154e_getSlotDuration()*305)/10000;  
   delay_in_asn = max_delay / slot_time; 
		
   delay_array[0]         = (delay_in_asn     & 0xff);
   delay_array[1]         = (delay_in_asn/256 & 0xff);
   
   ieee154e_getAsn(&et_asn[0]);
   for(i=0; i<5; i++) {
      sum = et_asn[i] + delay_array[i] + carry;  
      et_asn[i] = sum & 0xFF; 
      carry = ((sum >> 8) & 0xFF);
   }
}

/**
/brief Format asn to asn_t structure

\param[in]  in  asn value represented in array format

\param[out] val_asn   asn value represented in asn_t format
*/
void ieee154e_orderToASNStructure(uint8_t* in,asn_t* val_asn) {
   val_asn->bytes0and1   =     in[0] + 256*in[1];
   val_asn->bytes2and3   =     in[2] + 256*in[3];
   val_asn->byte4        =     in[4];
}
#endif
//======= events

/**
\brief Indicates a new slot has just started.

This function executes in ISR mode, when the new slot timer fires.
*/

void isr_ieee154e_newSlot(opentimers_id_t id) //
{
    ieee154e_vars.startOfSlotReference = opentimers_getCurrentTimeout();
    uint32_t	settime;
    uint8_t i;
    static uint8_t sleep_time=0;
    uint32_t    GLOBLE_ASN=0;

    /*not sync lowpowercalling I am not master*/
    
    if(ieee154e_vars.isSync==FALSE&&ieee154e_vars.Mode==LOWERPOWER_CALLINGON&&idmanager_getIsDAGroot()==FALSE)
    {//when it is LOWERPOWER_CALLING ON MODE
    	//settime=sleepduration;
    	settime=200;
    	sleep_time++;
    	incrementAsnOffset();
		if(sleep_time!=WAKEDURATION_S)//just change here to change the wake up time when it is working on the lower power calling on mode 
		{
			/*before return I must set the time*/
			openserial_stop();//added by zyx 20181113
			 opentimers_scheduleAbsolute
	    	(
	        ieee154e_vars.timerId,                  // timerId
	        settime,//TsSlotDuration,               // duration
	        ieee154e_vars.startOfSlotReference,     // reference
	        TIME_TICS,                              // timetype
	        isr_ieee154e_newSlot                    // callback
	    	);
	    	/*I can reset the openserial here*/
	    	openserial_startInput();//added by zyx 20181113
			return;
			
		}else
		{
              
			sleep_time=0;
		}
    }
    else if(ieee154e_vars.Mode==LOWERPOWER_RUN&&idmanager_getIsDAGroot()==FALSE)
    {//when it is LOWERPOWER_RUN mode
    	GLOBLE_ASN=ieee154e_vars.asn.byte4;
    	GLOBLE_ASN=(GLOBLE_ASN<<16)|ieee154e_vars.asn.bytes2and3;
    	GLOBLE_ASN=GLOBLE_ASN%(SLOTFRAME_LENGTH*(SLEEP_SUPERFRAME_LENGTH+1));
    	GLOBLE_ASN=(GLOBLE_ASN<<16)|ieee154e_vars.asn.bytes0and1;
		//GLOBLE_ASN=GLOBLE_ASN<<32|ieee154e_vars.asn.bytes2and3<<16|ieee154e_vars.asn.bytes0and1;
		if(GLOBLE_ASN%(SLOTFRAME_LENGTH*(SLEEP_SUPERFRAME_LENGTH+1))==11)
		//if(ieee154e_vars.slotOffset==0)
		{
			//printf("%d\n",GLOBLE_ASN);
			
			settime=TsSlotDuration*SLOTFRAME_LENGTH*SLEEP_SUPERFRAME_LENGTH;//512*12*9 sleep time
			opentimers_scheduleAbsolute
			(
			    ieee154e_vars.timerId,                  // timerId
			    settime,//TsSlotDuration,               // duration
			    ieee154e_vars.startOfSlotReference,     // reference
			    TIME_TICS,                              // timetype
			    isr_ieee154e_newSlot                    // callback
			);
	    	for(i=0;i<SLOTFRAME_LENGTH*SLEEP_SUPERFRAME_LENGTH;i++)//add the sleep slots
	    	{
	    		incrementAsnOffset();
	    	}
	    	if(ieee154e_vars.deSyncTimeout>SLOTFRAME_LENGTH*SLEEP_SUPERFRAME_LENGTH)
	    	{
	    		ieee154e_vars.deSyncTimeout-=SLOTFRAME_LENGTH*SLEEP_SUPERFRAME_LENGTH;
	    	}else
	    	{
	    		ieee154e_vars.deSyncTimeout=0;
	    	}
    		return;
		}
		else
		{
			settime=TsSlotDuration;
		}
    }
    else
    {//when it is HIGHPOWER other mode
		settime=TsSlotDuration;
		if(ieee154e_vars.Mode==LOWERPOWER_RUN_MORE_TIME&&idmanager_getIsDAGroot()==FALSE)//
		{
			ieee154e_vars.lowpowerrunmoreslots--;
			if(ieee154e_vars.lowpowerrunmoreslots==0)
			{
				ieee154e_vars.Mode=LOWERPOWER_RUN;
			}
		}
    }
    /*I will arrive here*/
	 opentimers_scheduleAbsolute
	    (
	        ieee154e_vars.timerId,                  // timerId
	        settime,//TsSlotDuration,               // duration
	        ieee154e_vars.startOfSlotReference,     // reference
	        TIME_TICS,                              // timetype
	        isr_ieee154e_newSlot                    // callback
	    );
    ieee154e_vars.slotDuration          = TsSlotDuration;
    // radiotimer_setPeriod(ieee154e_vars.slotDuration);
   if (ieee154e_vars.isSync==FALSE) 
   {
      if (idmanager_getIsDAGroot()==TRUE) 
      {
         changeIsSync(TRUE);
         ieee154e_resetAsn();
         ieee154e_vars.nextActiveSlotOffset = schedule_getNextActiveSlotOffset();
      } else 
      {
      	if(ieee154e_vars.Mode==LOWERPOWER_CALLINGON)
      	{
			activity_synchronize_new_listen_Preamble_Slot();//when I am in LOWERPOWER_CALLINGON MODE I Will arrive here zyx added; 
        }else
        {
        	activity_synchronize_newSlot();/*the enterance of openwsn MCU*/
        }
      }
   }
   else 
   {
#ifdef ADAPTIVE_SYNC
     // adaptive synchronization
      adaptive_sync_countCompensationTimeout();
#endif
      
      activity_ti1ORri1();//
   }
   ieee154e_dbg.num_newSlot++;
}

/**
\brief Indicates the FSM timer has fired.

This function executes in ISR mode, when the FSM timer fires.
*/
void isr_ieee154e_timer(opentimers_id_t id) {
   switch (ieee154e_vars.state) {
      case S_TXDATAOFFSET:
         activity_ti2();
         break;
      case S_TXDATAPREPARE:
         activity_tie1();
         break;
      case S_TXDATAREADY:
         activity_ti3();
         break;
      case S_TXDATADELAY:
         activity_tie2();
         break;
      case S_TXDATA:
         activity_tie3();
         break;
      case S_RXACKOFFSET:
         activity_ti6();
         break;
      case S_RXACKPREPARE:
         activity_tie4();
         break;
      case S_RXACKREADY:
         activity_ti7();
         break;
      case S_RXACKLISTEN:
         activity_tie5();
         break;
      case S_RXACK:
         activity_tie6();
         break;
      case S_RXDATAOFFSET:
         activity_ri2(); 
         break;
      case S_RXDATAPREPARE:
         activity_rie1();
         break;
      case S_RXDATAREADY:
         activity_ri3();
         break;
      case S_RXDATALISTEN:
         activity_rie2();
         break;
      case S_RXDATA:
         activity_rie3();
         break;
      case S_TXACKOFFSET: 
         activity_ri6();
         break;
      case S_TXACKPREPARE:
         activity_rie4();
         break;
      case S_TXACKREADY:
         activity_ri7();
         break;
      case S_TXACKDELAY:
         activity_rie5();
         break;
      case S_TXACK:
         activity_rie6();
         break;
         /*this mode is added by zyx 20180823*/
      case S_TXCALLINGDATAOFFSET:
      	 activity_ci1();
      	 break;
      case S_TXCALLINGDATAREADY:
      	 activity_ci2();//tx calling package
         break;
      case S_TXCALLINGDATA:
      	activity_cie1();
      	 break;
      case S_LONGTIMESLEEP:
      	activity_si1();
      	break;
      case S_RXPREAMBLELISTEN:
      	activity_sie1();
      	break;
      /*case S_RXSYNCLISTEN:
      	activity_si2();
      	break;*/

      case 0:
      	break;
      default:
         // log the error
         openserial_printError(COMPONENT_IEEE802154E,ERR_WRONG_STATE_IN_TIMERFIRES,
                               (errorparameter_t)ieee154e_vars.state,
                               (errorparameter_t)ieee154e_vars.slotOffset);
         // abort
         endSlot();
         break;
   }
   ieee154e_dbg.num_timer++;
}

/**
\brief Indicates the radio just received the first byte of a packet.

This function executes in ISR mode.
*/
void ieee154e_startOfFrame(PORT_TIMER_WIDTH capturedTime) {
   PORT_TIMER_WIDTH referenceTime = capturedTime - ieee154e_vars.startOfSlotReference;
   if (ieee154e_vars.isSync==FALSE) {
   		if(ieee154e_vars.Mode==LOWERPOWER_CALLINGON)
   		{
			 opentimers_scheduleAbsolute(
	  		ieee154e_vars.timerId,                            // timerId
	  		PORT_TsSlotDuration*2/3,              // duration
	  		sctimer_readCounter(),//ieee154e_vars.startOfSlotReference,  // reference
	  		TIME_TICS,                                        // timetype
	  		isr_ieee154e_timer                                // callback
			);
   		}
     	activity_synchronize_startOfFrame(referenceTime);
   } else {
      switch (ieee154e_vars.state) {
      	 /*case S_RXPREAMBLELISTEN:
      	 	activity_si2();//only make long the receive time
         	break;*/
         case S_TXDATADELAY:   
            activity_ti4(referenceTime);
            break;
         case S_RXACKREADY:
            /*
            It is possible to receive in this state for radio where there is no
            way of differentiated between "ready to listen" and "listening"
            (e.g. CC2420). We must therefore expect to the start of a packet in
            this "ready" state.
            */
            // no break!
         case S_RXACKLISTEN:
            activity_ti8(referenceTime);
            break;
         case S_RXDATAREADY:
            /*
            Similarly as above.
            */
            // no break!
         case S_RXDATALISTEN:
            activity_ri4(referenceTime);
            break;
         case S_TXACKDELAY:
            activity_ri8(referenceTime);
            break;
         case S_TXCALLINGDATADELAY:
         	/*just break*/
         	activity_ci3(referenceTime);
         	break;
         default:
            // log the error
        	 //debugpins_frame_toggle(); //lcg 20180523
            openserial_printError(COMPONENT_IEEE802154E,ERR_WRONG_STATE_IN_NEWSLOT,
                                  (errorparameter_t)ieee154e_vars.state,
                                  (errorparameter_t)ieee154e_vars.slotOffset);
            // abort
            endSlot();
            break;
      }
   }
   ieee154e_dbg.num_startOfFrame++;
}

/**
\brief Indicates the radio just received the last byte of a packet.

This function executes in ISR mode.
*/
void ieee154e_endOfFrame(PORT_TIMER_WIDTH capturedTime) 
{
   PORT_TIMER_WIDTH referenceTime = capturedTime - ieee154e_vars.startOfSlotReference;
   if (ieee154e_vars.isSync==FALSE) 
   {
   		
      	activity_synchronize_endOfFrame(referenceTime);
      	
   } else {
      switch (ieee154e_vars.state) {
         case S_TXDATA:
            activity_ti5(referenceTime);
            break;
         case S_RXACK:
            activity_ti9(referenceTime);
            break;
         case S_RXDATA:
            activity_ri5(referenceTime);
            break;
         case S_TXACK:
            activity_ri9(referenceTime);
            break;
         case S_TXCALLINGDATA://zyx
         	activity_ci4(referenceTime);
         	break;
         default:
            // log the error
            openserial_printError(COMPONENT_IEEE802154E,ERR_WRONG_STATE_IN_ENDOFFRAME,
                                  (errorparameter_t)ieee154e_vars.state,
                                  (errorparameter_t)ieee154e_vars.slotOffset);
            // abort
            endSlot();
            break;
      }
   }
   ieee154e_dbg.num_endOfFrame++;
}

//======= misc

/**
\brief Trigger this module to print status information, over serial.

debugPrint_* functions are used by the openserial module to continuously print
status information about several modules in the OpenWSN stack.

\returns TRUE if this function printed something, FALSE otherwise.
*/
bool debugPrint_asn() {
   asn_t output;
   output.byte4         =  ieee154e_vars.asn.byte4;
   output.bytes2and3    =  ieee154e_vars.asn.bytes2and3;
   output.bytes0and1    =  ieee154e_vars.asn.bytes0and1;
   openserial_printStatus(STATUS_ASN,(uint8_t*)&output,sizeof(output));
   return TRUE;
}

/**
\brief Trigger this module to print status information, over serial.

debugPrint_* functions are used by the openserial module to continuously print
status information about several modules in the OpenWSN stack.

\returns TRUE if this function printed something, FALSE otherwise.
*/
bool debugPrint_isSync() {
   uint8_t output=0;
   output = ieee154e_vars.isSync;
   openserial_printStatus(STATUS_ISSYNC,(uint8_t*)&output,sizeof(uint8_t));
   return TRUE;
}

/**
\brief Trigger this module to print status information, over serial.

debugPrint_* functions are used by the openserial module to continuously print
status information about several modules in the OpenWSN stack.

\returns TRUE if this function printed something, FALSE otherwise.
*/
bool debugPrint_macStats() {
   // send current stats over serial
   openserial_printStatus(STATUS_MACSTATS,(uint8_t*)&ieee154e_stats,sizeof(ieee154e_stats_t));
   return TRUE;
}

//=========================== private =========================================

//======= SYNCHRONIZING
port_INLINE	void activity_synchronize_new_listen_Preamble_Slot()
{
	ieee154e_vars.startOfSlotReference = opentimers_getCurrentTimeout();
	activity_si1();
}
port_INLINE void activity_synchronize_newSlot()/**/ 
{
    // I'm in the middle of receiving a packet
    if (ieee154e_vars.state==S_SYNCRX) 
    {
    	// lcg 20180517 when in this state,occassionaly not recieve the complete packet,then can not out this state. to be done.
    	//debugpins_fsm_toggle();  //lcg 20180516 add
    	/*time over and reset the radio zyx adedd20181115*/
		if((ieee154e_vars.asn.bytes0and1&0x0FFF)==0x0FFF)
		{
			changeState(S_SYNCLISTEN);
			radio_rfOff();
			//ieee154e_vars.freq = (openrandom_get16b()&0x0F) + 11;
			ieee154e_vars.freq =0;// (openrandom_get16b()&0x0F) + 11;
			radio_rxEnable();
			radio_setFrequency(ieee154e_vars.freq);
			radio_rxNow();
		}
        //return;
    }
    
    ieee154e_vars.radioOnInit=sctimer_readCounter();
    ieee154e_vars.radioOnThisSlot=TRUE;
    
    // if this is the first time I call this function while not synchronized,
    // switch on the radio in Rx mode
    //if (ieee154e_vars.state!=S_SYNCLISTEN) {
    if(ieee154e_vars.state!=S_SYNCLISTEN&&ieee154e_vars.state!=S_SYNCRX){//zyxadd20181211 I test the programe will radio off here when it receive a syncword,it may not do this
        // change state
        changeState(S_SYNCLISTEN);
        // turn off the radio (in case it wasn't yet)
        radio_rfOff();
        
        // update record of current channel
        ieee154e_vars.freq = (openrandom_get16b()&0x0F) + 11;
        
        // configure the radio to listen to the default synchronizing channel
        
        
        // switch on the radio in Rx mode.
        radio_rxEnable();
        radio_setFrequency(ieee154e_vars.freq);
        radio_rxNow();
    } else {
        // I'm listening last slot
        ieee154e_stats.numTicsOn    += ieee154e_vars.slotDuration;
        ieee154e_stats.numTicsTotal += ieee154e_vars.slotDuration;
    }
    
    // if I'm already in S_SYNCLISTEN, while not synchronized,
    // but the synchronizing channel has been changed,
    // change the synchronizing channel
    if ((ieee154e_vars.state==S_SYNCLISTEN) && (ieee154e_vars.singleChannelChanged == TRUE)) {
        // turn off the radio (in case it wasn't yet)
        radio_rfOff();
        
        // update record of current channel
        ieee154e_vars.freq = calculateFrequency(ieee154e_vars.singleChannel);
        
        // configure the radio to listen to the default synchronizing channel
        
        // switch on the radio in Rx mode.
        radio_rxEnable();
        radio_setFrequency(ieee154e_vars.freq);
        radio_rxNow();
        ieee154e_vars.singleChannelChanged = FALSE;
    }
    //radio_rxEnable();//lcg 20180503 doesnt exist originaly ///////////////////////////////////////////////
    // increment ASN (used only to schedule serial activity)
    incrementAsnOffset();
    
    // to be able to receive and transmist serial even when not synchronized
    // take turns every 8 slots sending and receiving
    if ((ieee154e_vars.asn.bytes0and1&0x000f)==0x0000) {
        openserial_stop();
        ////openserial_startOutput();//zyx20181210
    } else if ((ieee154e_vars.asn.bytes0and1&0x000f)==0x0008) {
        openserial_stop();
        openserial_startInput();
    }
}

port_INLINE void activity_synchronize_startOfFrame(PORT_TIMER_WIDTH capturedTime) {
   
   // don't care about packet if I'm not listening//20190123zyx add bug
   if (ieee154e_vars.state!=S_SYNCLISTEN) {
      return;
   }
   
   // change state
   changeState(S_SYNCRX);
   
   // stop the serial
   openserial_stop();
   
   // record the captured time 
   ieee154e_vars.lastCapturedTime = capturedTime;
   
   // record the captured time (for sync)
   ieee154e_vars.syncCapturedTime = capturedTime;
}
port_INLINE void activity_synchronize_endOfFrame(PORT_TIMER_WIDTH capturedTime) {
   ieee802154_header_iht ieee802514_header;
   uint16_t              lenIE;
   
   // check state
   if (ieee154e_vars.state!=S_SYNCRX) 
   {
      // log the error
      openserial_printError(COMPONENT_IEEE802154E,ERR_WRONG_STATE_IN_ENDFRAME_SYNC,
                            (errorparameter_t)ieee154e_vars.state,
                            (errorparameter_t)0);
      // abort
      endSlot();
   }
   
   // change state
   changeState(S_SYNCPROC);
   
   // get a buffer to put the (received) frame in
   ieee154e_vars.dataReceived = openqueue_getFreePacketBuffer(COMPONENT_IEEE802154E);
   if (ieee154e_vars.dataReceived==NULL) 
   {
      // log the error
      openserial_printError(COMPONENT_IEEE802154E,ERR_NO_FREE_PACKET_BUFFER,
                            (errorparameter_t)0,
                            (errorparameter_t)0);
      // abort
      endSlot();
      return;
   }
   
   // declare ownership over that packet
   ieee154e_vars.dataReceived->creator = COMPONENT_IEEE802154E;
   ieee154e_vars.dataReceived->owner   = COMPONENT_IEEE802154E;
   
   /*
   The do-while loop that follows is a little parsing trick.
   Because it contains a while(0) condition, it gets executed only once.
   The behavior is:
   - if a break occurs inside the do{} body, the error code below the loop
     gets executed. This indicates something is wrong with the packet being 
     parsed.
   - if a return occurs inside the do{} body, the error code below the loop
     does not get executed. This indicates the received packet is correct.
   */
   do { // this "loop" is only executed once
      
      // retrieve the received data frame from the radio's Rx buffer
      ieee154e_vars.dataReceived->payload = &(ieee154e_vars.dataReceived->packet[FIRST_FRAME_BYTE]);
      radio_getReceivedFrame(       ieee154e_vars.dataReceived->payload,
                                   &ieee154e_vars.dataReceived->length,
                             sizeof(ieee154e_vars.dataReceived->packet),
                                   &ieee154e_vars.dataReceived->l1_rssi,
                                   &ieee154e_vars.dataReceived->l1_lqi,
                                   &ieee154e_vars.dataReceived->l1_crc);
      
      // break if packet too short
      if (ieee154e_vars.dataReceived->length<LENGTH_CRC || ieee154e_vars.dataReceived->length>LENGTH_IEEE154_MAX) {
         // break from the do-while loop and execute abort code below
          openserial_printError(COMPONENT_IEEE802154E,ERR_INVALIDPACKETFROMRADIO,
                            (errorparameter_t)0,
                            ieee154e_vars.dataReceived->length);
          // lcg 20180504 occasionally when startup print this and dont stop.
          //radio_init();  // lcg 20180508
         break;
      }
      
      // toss CRC (2 last bytes)
      packetfunctions_tossFooter(   ieee154e_vars.dataReceived, LENGTH_CRC);
      
      // break if invalid CRC
      if (ieee154e_vars.dataReceived->l1_crc==FALSE) {
         // break from the do-while loop and execute abort code below
    	  openserial_printInfo(
    	  	    	            COMPONENT_OPENWSN,0x5d,
    	  	    	            (errorparameter_t)0,
    	  	    	            (errorparameter_t)0
    	  	    	      );  //lcg 20180503
         break;
      }
      
      // parse the IEEE802.15.4 header (synchronize, end of frame)
      ieee802154_retrieveHeader(ieee154e_vars.dataReceived,&ieee802514_header);
      
      // break if invalid IEEE802.15.4 header
      if (ieee802514_header.valid==FALSE) {
         // break from the do-while loop and execute the clean-up code below
    	  openserial_printInfo(
    	      	  	    	            COMPONENT_OPENWSN,0x5e,
    	      	  	    	            (errorparameter_t)0,
    	      	  	    	            (errorparameter_t)0
    	      	  	    	      );  //lcg 20180503
         break;
      }
      
      // store header details in packet buffer
      ieee154e_vars.dataReceived->l2_frameType = ieee802514_header.frameType;
      ieee154e_vars.dataReceived->l2_dsn       = ieee802514_header.dsn;
      memcpy(&(ieee154e_vars.dataReceived->l2_nextORpreviousHop),&(ieee802514_header.src),sizeof(open_addr_t));

      // verify that incoming security level is acceptable
      if (IEEE802154_security_acceptableLevel(ieee154e_vars.dataReceived, &ieee802514_header) == FALSE) {
    	  openserial_printInfo(
    	      	  	    	            COMPONENT_OPENWSN,0x5f,
    	      	  	    	            (errorparameter_t)0,
    	      	  	    	            (errorparameter_t)0
    	      	  	    	      );  //lcg 20180503
            break;
      }

      if (ieee154e_vars.dataReceived->l2_securityLevel != IEEE154_ASH_SLF_TYPE_NOSEC) 
      {
         // If we are not synced, we need to parse IEs and retrieve the ASN
         // before authenticating the beacon, because nonce is created from the ASN
         if (!ieee154e_vars.isSync && ieee802514_header.frameType == IEEE154_TYPE_BEACON) 
         {
            if (!isValidJoin(ieee154e_vars.dataReceived, &ieee802514_header)) 
            {
            	openserial_printInfo(
            	    	  	    	            COMPONENT_OPENWSN,0x60,
            	    	  	    	            (errorparameter_t)0,
            	    	  	    	            (errorparameter_t)0
            	    	  	    	      );  //lcg 20180503
               break;
            }
         }
         else { // discard other frames as we cannot decrypt without being synced
        	 openserial_printInfo(
        	     	  	    	            COMPONENT_OPENWSN,0x61,
        	     	  	    	            (errorparameter_t)0,
        	     	  	    	            (errorparameter_t)0
        	     	  	    	      );  //lcg 20180503
            break;
         }
      }

      // toss the IEEE802.15.4 header -- this does not include IEs as they are processed
      // next.
      packetfunctions_tossHeader(ieee154e_vars.dataReceived,ieee802514_header.headerLength);
     
      // process IEs
      lenIE = 0;
      if (
            (
               //ieee154e_vars.Mode!=LOWERPOWER_CALLINGON											   &&//zyx add
               ieee802514_header.valid==TRUE                                                       &&
               ieee802514_header.ieListPresent==TRUE                                               &&
               (ieee802514_header.frameType==IEEE154_TYPE_BEACON||ieee802514_header.frameType==IEEE154_TYPE_CALLING)&&//ADD frameType by zyx20180929
               packetfunctions_sameAddress(&ieee802514_header.panid,idmanager_getMyID(ADDR_PANID)) &&
               ieee154e_processIEs(ieee154e_vars.dataReceived,&lenIE)//active the slot here and schedule the syncSlotOffset
            )==FALSE) {
         // break from the do-while loop and execute the clean-up code below
    	 // openserial_printInfo(COMPONENT_OPENWSN,0x62,(errorparameter_t)ieee802514_header.ieListPresent,(errorparameter_t)ieee802514_header.frameType);  //lcg 20180503
         break;
      }
    
      // turn off the radio
      //radio_rfOff();
      
      // compute radio duty cycle
      //ieee154e_vars.radioOnTics += (sctimer_readCounter()-ieee154e_vars.radioOnInit);

      // toss the IEs
      packetfunctions_tossHeader(ieee154e_vars.dataReceived,lenIE);
      
      // synchronize (for the first time) to the sender's EB
      if(ieee154e_vars.Mode==LOWERPOWER_CALLINGON&&ieee154e_vars.dataReceived->packet[1]==0x46)//add the last by zyx20181114
      {
      		if(FCI_LETME_LOWERPOWERON)
      		{
      			ieee154e_vars.Mode=LOWERPOWER_RUN;
      		}else
      		{
      			ieee154e_vars.Mode=HIGHPOWER_CALLINGOFF;
      		}
            
            changeIsSync(TRUE);
      		radio_DISABLE_PREAMBLE_INTERRUPT();
      		synchronizecallingPacket(ieee154e_vars.syncCapturedTime);//zyx20180903
      		//changeIsSync(TRUE);
			//ieee154e_vars.dataReceived = NULL;
			//return;
      		//sixtop_sendSepecialKA();//I can send special KA in the notif_receive packet or the upper layers
			//send KA at UP layer
      }else
      {
      	synchronizePacket(ieee154e_vars.syncCapturedTime);//
      }
      // turn off the radio
      radio_rfOff();//zyx20190401
      // compute radio duty cycle
      ieee154e_vars.radioOnTics += (sctimer_readCounter()-ieee154e_vars.radioOnInit);
      
      // declare synchronized
      changeIsSync(TRUE);
      // log the info
      openserial_printInfo(COMPONENT_IEEE802154E,ERR_SYNCHRONIZED,
                            (errorparameter_t)ieee154e_vars.slotOffset,
                            (errorparameter_t)0);
      
      // send received EB up the stack so RES can update statistics (synchronizing)
      notif_receive(ieee154e_vars.dataReceived);//add mac address and so on
      
      // clear local variable
      ieee154e_vars.dataReceived = NULL;
      
      // official end of synchronization
      endSlot();
      
      // everything went well, return here not to execute the error code below
      return;
      
   } while(0);
   // free the (invalid) received data buffer so RAM memory can be recycled
   openqueue_freePacketBuffer(ieee154e_vars.dataReceived);
   
   // clear local variable
   ieee154e_vars.dataReceived = NULL;
   
   // return to listening state
   //changeState(S_SYNCLISTEN);  //lcg 20180503 fix can not sync again

}

port_INLINE bool ieee154e_processIEs(OpenQueueEntry_t* pkt, uint16_t* lenIE) {
    uint8_t i;
    if (isValidEbFormat(pkt,lenIE)==TRUE){
        // at this point, ASN and frame length are known
        // the current slotoffset can be inferred
        ieee154e_syncSlotOffset();
        schedule_syncSlotOffset(ieee154e_vars.slotOffset);
        /*I will get off the not used slots zyx20180923*/
        ieee154e_vars.nextActiveSlotOffset = schedule_getNextActiveSlotOffset();
        /* 
        infer the asnOffset based on the fact that
        ieee154e_vars.freq = 11 + (asnOffset + channelOffset)%16 
        */
        for (i=0;i<NUM_CHANNELS;i++)
        {
            if ((ieee154e_vars.freq - 11)==ieee154e_vars.chTemplate[i])
            {
                break;
            }
        }
        ieee154e_vars.asnOffset = i - schedule_getChannelOffset();
        return TRUE;
    } else {
        // wrong eb format
        openserial_printError(COMPONENT_IEEE802154E,ERR_UNSUPPORTED_FORMAT,3,0);
        return FALSE;
    }
}

//======= TX
port_INLINE void activity_ti1ORri1() {
   cellType_t  cellType;
   open_addr_t neighbor;
   uint8_t     i;
   uint8_t     asn[5];
   uint8_t     join_priority;
   bool        changeToRX=FALSE;
   bool        couldSendEB=FALSE;
	uint32_t    GLOBLE_ASN;
	static bool Master_has_AnySlave_Is_Lowerpowerrunmoretime=FALSE;
   // increment ASN (do this first so debug pins are in sync)
   incrementAsnOffset();
   
   // wiggle debug pins
   debugpins_slot_toggle();
   //printf("%d\n",*(__IO uint32_t *)(0x40007C1C));
   if (ieee154e_vars.slotOffset==0) 
   {
      debugpins_frame_toggle();   //lcg 20180413
   }
   
   // desynchronize if needed
   if (idmanager_getIsDAGroot()==FALSE)//if I am a slave zyx20180816
   {
      if(ieee154e_vars.deSyncTimeout > ieee154e_vars.numOfSleepSlots)
      {
         ieee154e_vars.deSyncTimeout -= ieee154e_vars.numOfSleepSlots;
      }
      else
      {
            // Reset sleep slots
            ieee154e_vars.numOfSleepSlots = 1;
        
            // declare myself desynchronized
            
            changeIsSync(FALSE);
#ifdef USE_FLASH_PARAMETER//
            if(*(__IO uint8_t *)(ADDR_PARAMENT_STARTADDRESS+OFFSETADDR_LOWERPOWER_CALLINGON)==1)
		   {
		   		ieee154e_vars.Mode=LOWERPOWER_CALLINGON;//LOWEROWER_RUN;//
 		   		radio_reinit();//zyx20181120
		   		//HAL_NVIC_SystemReset();//add system reset 20190320
		   }
		   else
		   {
		   		ieee154e_vars.Mode=HIGHPOWER_CALLINGOFF;
 		   }
#endif
            //debugpins_frame_toggle(); //lcg 20180508
            // log the error
            openserial_printError(COMPONENT_IEEE802154E,ERR_DESYNCHRONIZED,
                                  (errorparameter_t)ieee154e_vars.slotOffset,
                                  (errorparameter_t)ieee154e_vars.numOfSleepSlots);  //lcg 20180427  par add
            
            // update the statistics
            ieee154e_stats.numDeSync++;
               
            // abort
            endSlot();
            return;
      }
   }
   else
   {
  		
   }
 
   
   // if the previous slot took too long, we will not be in the right state
   if (ieee154e_vars.state!=S_SLEEP) {
      // log the error
      openserial_printError(COMPONENT_IEEE802154E,ERR_WRONG_STATE_IN_STARTSLOT,
                            (errorparameter_t)ieee154e_vars.state,
                            (errorparameter_t)ieee154e_vars.slotOffset);
      // abort
      endSlot();
      return;
   }
   
   // Reset sleep slots
   ieee154e_vars.numOfSleepSlots = 1;
   
   if (ieee154e_vars.slotOffset==ieee154e_vars.nextActiveSlotOffset) 
   {
      // this is the next active slot
      
      // advance the schedule schedule get the next state zyx20180923
      schedule_advanceSlot();
      
      // calculate the frequency to transmit on
      ieee154e_vars.freq = calculateFrequency(schedule_getChannelOffset()); 
      
      // find the next one
      ieee154e_vars.nextActiveSlotOffset = schedule_getNextActiveSlotOffset();
      if (idmanager_getIsSlotSkip() && idmanager_getIsDAGroot()==FALSE)
      {
          if (ieee154e_vars.nextActiveSlotOffset>ieee154e_vars.slotOffset) 
          {
               ieee154e_vars.numOfSleepSlots = ieee154e_vars.nextActiveSlotOffset-ieee154e_vars.slotOffset;
          } else {
               ieee154e_vars.numOfSleepSlots = schedule_getFrameLength()+ieee154e_vars.nextActiveSlotOffset-ieee154e_vars.slotOffset; 
          }
          /*here is the sleep mode*/
          opentimers_scheduleAbsolute(
              ieee154e_vars.timerId,                            // timerId
              TsSlotDuration*(ieee154e_vars.numOfSleepSlots),   // duration
              ieee154e_vars.startOfSlotReference,               // reference
              TIME_TICS,                                        // timetype
              isr_ieee154e_newSlot                              // callback
          );
          ieee154e_vars.slotDuration = TsSlotDuration*(ieee154e_vars.numOfSleepSlots);
          // radiotimer_setPeriod(TsSlotDuration*(ieee154e_vars.numOfSleepSlots));
           
          //increase ASN by numOfSleepSlots-1 slots as at this slot is already incremented by 1
          for (i=0;i<ieee154e_vars.numOfSleepSlots-1;i++)
          {
             incrementAsnOffset();
          }
      }  
      ieee154e_vars.nextActiveSlotOffset = schedule_getNextActiveSlotOffset();
      openserial_stop();
   } 
   else {
      // this is NOT the next active slot, abort
      // stop using serial
      openserial_stop();
      // abort the slot
      endSlot();
      //start outputing serial
      openserial_startOutput();//zyx20181210 zyx20190131
      return;
   }
   
   // check the schedule to see what type of slot this is
   
   
    if(idmanager_getIsDAGroot()==TRUE&&ieee154e_vars.Mode==LOWERPOWER_CALLINGON)//or I am master I will check whether I am in the MODE calling on zyx added20180815
   {
   		cellType=CELLTYPE_CALLING;
   }else
   {
   		cellType = schedule_getType();
   }
   switch (cellType) 
   {
      case CELLTYPE_TXRX:
      case CELLTYPE_TX:
         // stop using serial
         openserial_stop();
         // assuming that there is nothing to send
         ieee154e_vars.dataToSend = NULL;//remove it up 7lines zyx 20180907
         // check whether we can send
         if (schedule_getOkToSend()) 
         {
			schedule_getNeighbor(&neighbor);
			if(idmanager_getIsDAGroot()&&AnyNeighborISlowerPowerRun())//low Power slot only send lower powercket
			{
				if(Master_has_AnySlave_Is_Lowerpowerrunmoretime==FALSE)
				{
					//calculate ASN if it is sleep now
					GLOBLE_ASN=ieee154e_vars.asn.byte4;
					GLOBLE_ASN=(GLOBLE_ASN<<16)|ieee154e_vars.asn.bytes2and3;
					GLOBLE_ASN=GLOBLE_ASN%(SLOTFRAME_LENGTH*(SLEEP_SUPERFRAME_LENGTH+1));
					GLOBLE_ASN=(GLOBLE_ASN<<16)|ieee154e_vars.asn.bytes0and1;
                    if(GLOBLE_ASN%(SLOTFRAME_LENGTH*(SLEEP_SUPERFRAME_LENGTH+1))<SLOTFRAME_LENGTH)//SLOTFRAME_LENGTH*SLEEP_SUPERFRAME_LENGTH
                    {
                            //LOWERPOWER time is arrived zyx20181107
                            //////HAL_GPIO_TogglePin(GPIOA,GPIO_PIN_3);
                            ieee154e_vars.dataToSend = openqueue_macGetDataPacket(&neighbor);//to send data
                            //ieee154e_vars.dataToSend = openqueue_macGetLowerPowerDataPacket(&neighbor);//to send data
                            if(ieee154e_vars.dataToSend!=NULL)//MASTER LOWERPOWER RUN MORE TIME MODE
                            {
                                    Master_has_AnySlave_Is_Lowerpowerrunmoretime=TRUE;
                            }else
                            {
                                    Master_has_AnySlave_Is_Lowerpowerrunmoretime=FALSE;
                            }
                    }else
                    {
                            //ieee154e_vars.dataToSend = openqueue_macGetDataPacket(&neighbor);//to send data
                    }
				}
				else
				{
					//Any Slave is in Lowerpowerrunmoretime mode
					ieee154e_vars.dataToSend=openqueue_macGetDataPacket(&neighbor);
					if(ieee154e_vars.dataToSend==NULL)
					{
						Master_has_AnySlave_Is_Lowerpowerrunmoretime=FALSE;//EXIT the 
					}
				}
			}
			else//
			{
				ieee154e_vars.dataToSend=openqueue_macGetDataPacket(&neighbor);
			}
            if ((ieee154e_vars.dataToSend==NULL) && (cellType==CELLTYPE_TXRX)) 
            {
               couldSendEB=TRUE;
               // look for an EB packet in the queue
               ieee154e_vars.dataToSend = openqueue_macGetEBPacket();//if there is nodata I will look for a EB packet 
            }
            
         }
         if (ieee154e_vars.dataToSend==NULL) 
         {
            if (cellType==CELLTYPE_TX) 
            {
               // abort
               endSlot();
               break;
            } else 
            {
               changeToRX=TRUE;
            }
         }
         else 
         {
            // change state
                     
            changeState(S_TXDATAOFFSET);
            // change owner
            ieee154e_vars.dataToSend->owner = COMPONENT_IEEE802154E;
            if (couldSendEB==TRUE) 
            {        // I will be sending an EB
               //copy synch IE  -- should be Little endian???
               // fill in the ASN field of the EB
               ieee154e_getAsn(asn);
               join_priority = (icmpv6rpl_getMyDAGrank()/MINHOPRANKINCREASE)-1; //poipoi -- use dagrank(rank)-1
               memcpy(ieee154e_vars.dataToSend->l2_ASNpayload,&asn[0],sizeof(asn_t));
               memcpy(ieee154e_vars.dataToSend->l2_ASNpayload+sizeof(asn_t),&join_priority,sizeof(uint8_t));
            }
            // record that I attempt to transmit this packet
            ieee154e_vars.dataToSend->l2_numTxAttempts++;
#ifdef SLOT_FSM_IMPLEMENTATION_MULTIPLE_TIMER_INTERRUPT
            // 1. schedule timer for loading packet
            radiotimer_schedule(ACTION_LOAD_PACKET,      DURATION_tt1);
            // prepare the packet for load packet action at DURATION_tt1
            // make a local copy of the frame
            packetfunctions_duplicatePacket(&ieee154e_vars.localCopyForTransmission, ieee154e_vars.dataToSend);

            // check if packet needs to be encrypted/authenticated before transmission 
            if (ieee154e_vars.localCopyForTransmission.l2_securityLevel != IEEE154_ASH_SLF_TYPE_NOSEC) { // security enabled
                // encrypt in a local copy
                if (IEEE802154_security_outgoingFrameSecurity(&ieee154e_vars.localCopyForTransmission) != E_SUCCESS) {
                    // keep the frame in the OpenQueue in order to retry later
                    endSlot(); // abort
                    return;
                }
            }
            // add 2 CRC bytes only to the local copy as we end up here for each retransmission
            packetfunctions_reserveFooterSize(&ieee154e_vars.localCopyForTransmission, 2);
            // set the tx buffer address and length register.(packet is NOT loaded at this moment)
            radio_loadPacket_prepare(ieee154e_vars.localCopyForTransmission.payload,
                                     ieee154e_vars.localCopyForTransmission.length);
            // 2. schedule timer for sending packet
            radiotimer_schedule(ACTION_SEND_PACKET,  DURATION_tt2);
            // 3. schedule timer radio tx watchdog
            radiotimer_schedule(ACTION_NORMAL_TIMER, DURATION_tt3);
            // 4. set capture interrupt for Tx SFD senddone and packet senddone
            radiotimer_setCapture(ACTION_TX_SFD_DONE);
            radiotimer_setCapture(ACTION_TX_SEND_DONE);
#else
            // arm tt1
            opentimers_scheduleAbsolute(
                ieee154e_vars.timerId,                            // timerId
                DURATION_tt1,                                     // duration
                ieee154e_vars.startOfSlotReference,               // reference
                TIME_TICS,                                        // timetype
                isr_ieee154e_timer                                // callback
            );
            // radiotimer_schedule(DURATION_tt1);
#endif
            break;
         }
      case CELLTYPE_CALLING:
      //debug
      case CELLTYPE_RX:
         if (changeToRX==FALSE) {
            // stop using serial
            openserial_stop();
         }
         // change state
         changeState(S_RXDATAOFFSET);
#ifdef SLOT_FSM_IMPLEMENTATION_MULTIPLE_TIMER_INTERRUPT
         // arm rt1
         radiotimer_schedule(ACTION_RADIORX_ENABLE,DURATION_rt1);
         radio_rxPacket_prepare();
         // 2. schedule timer for starting 
         radiotimer_schedule(ACTION_NORMAL_TIMER,DURATION_rt2);
         // 3.  set capture interrupt for Rx SFD done and receiving packet done
         radiotimer_setCapture(ACTION_RX_SFD_DONE);
         radiotimer_setCapture(ACTION_RX_DONE);
#else
         // arm rt1
         //ieee154e_vars.startOfSlotReference+=1;
         opentimers_scheduleAbsolute(
              ieee154e_vars.timerId,                            // timerId
              DURATION_rt1,                                     // duration
              ieee154e_vars.startOfSlotReference,               // reference
              TIME_TICS,                                        // timetype
              isr_ieee154e_timer                                // callback
         );
         // radiotimer_schedule(DURATION_rt1);
#endif
         break;
      case CELLTYPE_SERIALRX:
         // stop using serial
         openserial_stop();
         // abort the slot
         endSlot();
         //start inputting serial data
         openserial_startOutput();//zyx 20190318
         openserial_startInput();
         //this is to emulate a set of serial input slots without having the slotted structure.

         //skip the serial rx slots
         ieee154e_vars.numOfSleepSlots = NUMSERIALRX;
         
         //increase ASN by NUMSERIALRX-1 slots as at this slot is already incremented by 1
         for (i=0;i<NUMSERIALRX-1;i++){
            incrementAsnOffset();
            // advance the schedule
            schedule_advanceSlot();
            // find the next one
            ieee154e_vars.nextActiveSlotOffset = schedule_getNextActiveSlotOffset();
         }
         // possibly skip additional slots if enabled
         if (idmanager_getIsSlotSkip() && idmanager_getIsDAGroot()==FALSE) {
             if (ieee154e_vars.nextActiveSlotOffset>ieee154e_vars.slotOffset) {
                 ieee154e_vars.numOfSleepSlots = ieee154e_vars.nextActiveSlotOffset-ieee154e_vars.slotOffset+NUMSERIALRX-1;
             } else 
             {
                 ieee154e_vars.numOfSleepSlots = schedule_getFrameLength()+ieee154e_vars.nextActiveSlotOffset-ieee154e_vars.slotOffset+NUMSERIALRX-1; 
             }
              
             //only increase ASN by numOfSleepSlots-NUMSERIALRX
             for (i=0;i<ieee154e_vars.numOfSleepSlots-NUMSERIALRX;i++){
                incrementAsnOffset();
             }
         }
         // set the timer based on calcualted number of slots to skip
         opentimers_scheduleAbsolute(
              ieee154e_vars.timerId,                            // timerId
              TsSlotDuration*(ieee154e_vars.numOfSleepSlots),   // duration
              ieee154e_vars.startOfSlotReference,               // reference
              TIME_TICS,                                        // timetype
              isr_ieee154e_newSlot                              // callback
         );
         ieee154e_vars.slotDuration = TsSlotDuration*(ieee154e_vars.numOfSleepSlots);
         // radiotimer_setPeriod(TsSlotDuration*(ieee154e_vars.numOfSleepSlots));
         
#ifdef ADAPTIVE_SYNC
         // deal with the case when schedule multi slots
         adaptive_sync_countCompensationTimeout_compoundSlots(NUMSERIALRX-1);
#endif
         break;
      case CELLTYPE_MORESERIALRX:
         // do nothing (not even endSlot())
         break;
      default:
         // stop using serial
         openserial_stop();
         // log the error
         openserial_printCritical(COMPONENT_IEEE802154E,ERR_WRONG_CELLTYPE,
                               (errorparameter_t)cellType,
                               (errorparameter_t)ieee154e_vars.slotOffset);
         // abort
         endSlot();
         break;
   }
}

port_INLINE void activity_ti2() 
{
   
    // change state
    changeState(S_TXDATAPREPARE);
    
#ifdef SLOT_FSM_IMPLEMENTATION_MULTIPLE_TIMER_INTERRUPT
#else
    // arm tt2
    opentimers_scheduleAbsolute(
          ieee154e_vars.timerId,                            // timerId
          DURATION_tt2,                                     // duration
          ieee154e_vars.startOfSlotReference,               // reference
          TIME_TICS,                                        // timetype
          isr_ieee154e_timer                                // callback
    );
    // radiotimer_schedule(DURATION_tt2);

    // make a local copy of the frame
    packetfunctions_duplicatePacket(&ieee154e_vars.localCopyForTransmission, ieee154e_vars.dataToSend);

    // check if packet needs to be encrypted/authenticated before transmission 
    if (ieee154e_vars.localCopyForTransmission.l2_securityLevel != IEEE154_ASH_SLF_TYPE_NOSEC) { // security enabled
        // encrypt in a local copy
        if (IEEE802154_security_outgoingFrameSecurity(&ieee154e_vars.localCopyForTransmission) != E_SUCCESS) {
            // keep the frame in the OpenQueue in order to retry later
            endSlot(); // abort
            return;
        }
    }
   
    // add 2 CRC bytes only to the local copy as we end up here for each retransmission
    packetfunctions_reserveFooterSize(&ieee154e_vars.localCopyForTransmission, 2);
#endif
   	radio_txEnable();
    // configure the radio for that frequency
	radio_setPreamble_len(Communicate_Preamble_LEN);//zyx added
    radio_setFrequency(ieee154e_vars.freq);
	
    // load the packet in the radio's Tx buffer
    radio_loadPacket(ieee154e_vars.localCopyForTransmission.payload,
                     ieee154e_vars.localCopyForTransmission.length);
    // enable the radio in Tx mode. This does not send the packet.


    ieee154e_vars.radioOnInit=sctimer_readCounter();
    ieee154e_vars.radioOnThisSlot=TRUE;
    // change state
    changeState(S_TXDATAREADY);
#ifdef SLOT_FSM_IMPLEMENTATION_MULTIPLE_TIMER_INTERRUPT
    // update state in advance
    changeState(S_TXDATADELAY);
#endif
}

port_INLINE void activity_tie1() {
   // log the error
   openserial_printError(COMPONENT_IEEE802154E,ERR_MAXTXDATAPREPARE_OVERFLOW,
                         (errorparameter_t)ieee154e_vars.state,
                         (errorparameter_t)ieee154e_vars.slotOffset);
   
   // abort
   endSlot();
}

port_INLINE void activity_ti3() {
    // change state
    changeState(S_TXDATADELAY);
#ifdef SLOT_FSM_IMPLEMENTATION_MULTIPLE_TIMER_INTERRUPT
#else
    // arm tt3
    opentimers_scheduleAbsolute(
        ieee154e_vars.timerId,                            // timerId
        DURATION_tt3,                                     // duration
        ieee154e_vars.startOfSlotReference,               // reference
        TIME_TICS,                                        // timetype
        isr_ieee154e_timer                                // callback
    );
    // radiotimer_schedule(DURATION_tt3);
    
    // give the 'go' to transmit
    radio_txNow();
    //ieee154e_startOfFrame(sctimer_readCounter());//20180125zyx
    
#endif
}

port_INLINE void activity_tie2() {
    // log the error
   openserial_printError(COMPONENT_IEEE802154E,ERR_WDRADIO_OVERFLOWS,
                         (errorparameter_t)ieee154e_vars.state,
                         (errorparameter_t)ieee154e_vars.slotOffset);
   
    // abort
    endSlot();
}

//start of frame interrupt
port_INLINE void activity_ti4(PORT_TIMER_WIDTH capturedTime) {
    // change state
    changeState(S_TXDATA);
#ifdef SLOT_FSM_IMPLEMENTATION_MULTIPLE_TIMER_INTERRUPT
    // cancel tt3
    radiotimer_cancel(ACTION_NORMAL_TIMER);
#else
    // cancel tt3
    opentimers_scheduleAbsolute(
        ieee154e_vars.timerId,                            // timerId
        ieee154e_vars.slotDuration,                       // duration
        ieee154e_vars.startOfSlotReference,               // reference
        TIME_TICS,                                        // timetype
        isr_ieee154e_newSlot                              // callback
    );
    // radiotimer_cancel();
#endif
    // record the captured time
    ieee154e_vars.lastCapturedTime = capturedTime;
#ifdef SLOT_FSM_IMPLEMENTATION_MULTIPLE_TIMER_INTERRUPT
    // arm tt4
    radiotimer_schedule(ACTION_NORMAL_TIMER,DURATION_tt4);
#else
    // arm tt4
    opentimers_scheduleAbsolute(
        ieee154e_vars.timerId,                            // timerId
        DURATION_tt4,                                     // duration
        ieee154e_vars.startOfSlotReference,               // reference
        TIME_TICS,                                        // timetype
        isr_ieee154e_timer                                // callback
    );
    // radiotimer_schedule(DURATION_tt4);
#endif
}

port_INLINE void activity_tie3() {
    // log the error
	//debugpins_frame_toggle(); //lcg 20180523
    openserial_printError(COMPONENT_IEEE802154E,ERR_WDDATADURATION_OVERFLOWS,
                         (errorparameter_t)ieee154e_vars.state,
                         (errorparameter_t)ieee154e_vars.slotOffset);
   
    // abort
    endSlot();
}

port_INLINE void activity_ti5(PORT_TIMER_WIDTH capturedTime) {
    bool listenForAck;
    
    // change state
    changeState(S_RXACKOFFSET);
#ifdef SLOT_FSM_IMPLEMENTATION_MULTIPLE_TIMER_INTERRUPT
    // cancel tt4
    radiotimer_cancel(ACTION_NORMAL_TIMER);
#else
    // cancel tt4
    opentimers_scheduleAbsolute(
        ieee154e_vars.timerId,                            // timerId
        ieee154e_vars.slotDuration,                       // duration
        ieee154e_vars.startOfSlotReference,               // reference
        TIME_TICS,                                        // timetype
        isr_ieee154e_newSlot                              // callback
    );
    // radiotimer_cancel();
#endif
    // turn off the radio
    radio_rfOff();
    ieee154e_vars.radioOnTics+=(sctimer_readCounter()-ieee154e_vars.radioOnInit);
   
    // record the captured time
    ieee154e_vars.lastCapturedTime = capturedTime;
   
    // decides whether to listen for an ACK
    if (packetfunctions_isBroadcastMulticast(&ieee154e_vars.dataToSend->l2_nextORpreviousHop)==TRUE) {
        listenForAck = FALSE;
    } else {
        listenForAck = TRUE;
    }
    
    if (listenForAck==TRUE) {
#ifdef SLOT_FSM_IMPLEMENTATION_MULTIPLE_TIMER_INTERRUPT
        // 1. schedule timer for enabling receiving
        // arm tt5
        radiotimer_schedule(ACTION_RADIORX_ENABLE,DURATION_tt5);
        // set receiving buffer address (radio is NOT enabled at this moment)
        radio_rxPacket_prepare();
        // 2. schedule timer for starting receiving
        radiotimer_schedule(ACTION_NORMAL_TIMER,DURATION_tt6);
        // 3. set capture for receiving SFD and packet receiving done
        radiotimer_setCapture(ACTION_RX_SFD_DONE);
        radiotimer_setCapture(ACTION_RX_DONE);
#else
        // arm tt5
        opentimers_scheduleAbsolute(
            ieee154e_vars.timerId,                            // timerId
            DURATION_tt5,                                     // duration
            ieee154e_vars.startOfSlotReference,               // reference
            TIME_TICS,                                        // timetype
            isr_ieee154e_timer                                // callback
        );
        // radiotimer_schedule(DURATION_tt5);
#endif
    }
    else
    {
        // indicate succesful Tx to schedule to keep statistics
        schedule_indicateTx(&ieee154e_vars.asn,TRUE);
        // indicate to upper later the packet was sent successfully
        notif_sendDone(ieee154e_vars.dataToSend,E_SUCCESS);
        // reset local variable
        ieee154e_vars.dataToSend = NULL;
        // abort
        endSlot();
    }
}

port_INLINE void activity_ti6() {
    // change state
    changeState(S_RXACKPREPARE);
#ifdef SLOT_FSM_IMPLEMENTATION_MULTIPLE_TIMER_INTERRUPT
#else
    // arm tt6
    opentimers_scheduleAbsolute(
        ieee154e_vars.timerId,                            // timerId
        DURATION_tt6,                                     // duration
        ieee154e_vars.startOfSlotReference,               // reference
        TIME_TICS,                                        // timetype
        isr_ieee154e_timer                                // callback
    );
    // radiotimer_schedule(DURATION_tt6);
#endif   
   
    // configure the radio for that frequency
    
   
#ifdef SLOT_FSM_IMPLEMENTATION_MULTIPLE_TIMER_INTERRUPT
    // enable the radio in Rx mode. The radio is not actively listening yet.
    radio_rxEnable_scum();
#else
    radio_rxEnable();
#endif
    radio_setFrequency(ieee154e_vars.freq);
    //caputre init of radio for duty cycle calculation
    ieee154e_vars.radioOnInit=sctimer_readCounter();
    ieee154e_vars.radioOnThisSlot=TRUE;
   
    // change state
    changeState(S_RXACKREADY);
}

port_INLINE void activity_tie4() {
   // log the error
   openserial_printError(COMPONENT_IEEE802154E,ERR_MAXRXACKPREPARE_OVERFLOWS,
                         (errorparameter_t)ieee154e_vars.state,
                         (errorparameter_t)ieee154e_vars.slotOffset);
   
   // abort
   
   endSlot();
}

port_INLINE void activity_ti7() {
   // change state
   changeState(S_RXACKLISTEN);
   
   // start listening
   radio_rxNow();
#ifdef SLOT_FSM_IMPLEMENTATION_MULTIPLE_TIMER_INTERRUPT
   // arm tt7
   radiotimer_schedule(ACTION_NORMAL_TIMER,DURATION_tt7);
#else
   // arm tt7
   opentimers_scheduleAbsolute(
        ieee154e_vars.timerId,                            // timerId
        DURATION_tt7,                                     // duration
        ieee154e_vars.startOfSlotReference,               // reference
        TIME_TICS,                                        // timetype
        isr_ieee154e_timer                                // callback
    );
   // radiotimer_schedule(DURATION_tt7);
#endif
}

port_INLINE void activity_tie5() {
    // indicate transmit failed to schedule to keep stats
    schedule_indicateTx(&ieee154e_vars.asn,FALSE);
   
    // decrement transmits left counter
    ieee154e_vars.dataToSend->l2_retriesLeft--;
   
    if (ieee154e_vars.dataToSend->l2_retriesLeft==0) {
        // indicate tx fail if no more retries left
        notif_sendDone(ieee154e_vars.dataToSend,E_FAIL);
    } else {
        // return packet to the virtual COMPONENT_SIXTOP_TO_IEEE802154E component
        ieee154e_vars.dataToSend->owner = COMPONENT_SIXTOP_TO_IEEE802154E;
    }
   
    // reset local variable
    ieee154e_vars.dataToSend = NULL;
   
    // abort
    endSlot();
}

port_INLINE void activity_ti8(PORT_TIMER_WIDTH capturedTime) {
    // change state
    changeState(S_RXACK);
#ifdef SLOT_FSM_IMPLEMENTATION_MULTIPLE_TIMER_INTERRUPT
    // cancel tt7
    radiotimer_cancel(ACTION_NORMAL_TIMER);
#else
    // cancel tt7
    opentimers_scheduleAbsolute(
        ieee154e_vars.timerId,                            // timerId
        ieee154e_vars.slotDuration,                       // duration
        ieee154e_vars.startOfSlotReference,               // reference
        TIME_TICS,                                        // timetype
        isr_ieee154e_newSlot                              // callback
    );
    // radiotimer_cancel();
#endif
    // record the captured time
    ieee154e_vars.lastCapturedTime = capturedTime;
#ifdef SLOT_FSM_IMPLEMENTATION_MULTIPLE_TIMER_INTERRUPT
    // arm tt8
    radiotimer_schedule(ACTION_NORMAL_TIMER,DURATION_tt8);
#else
    // arm tt8
   opentimers_scheduleAbsolute(
        ieee154e_vars.timerId,                            // timerId
        DURATION_tt8,                                     // duration
        ieee154e_vars.startOfSlotReference,               // reference
        TIME_TICS,                                        // timetype
        isr_ieee154e_timer                                // callback
    );
    // radiotimer_schedule(DURATION_tt8);
#endif
}

port_INLINE void activity_tie6() {
    // log the error  // lcg 20180602 this occur 3 times, just didnot wait for the radio complete sig,but this time interrput happened delay.
	/*
	> 16:28:20 ERROR 4b00 [IEEE802154E] wdAckDuration overflows while at state 13 in slotOffset 1
	16:28:23 ERROR 4b00 [CEXAMPLE] unknown error 90 arg1=1 arg2=11417
	16:28:23 ERROR 4b00 [IEEE802154E] got desynchronized at slotOffset 8,arg 1
	16:28:24 INFO 4b00 [IEEE802154E] synchronized at slotOffset 0
	16:28:24 ERROR 4b00 [CEXAMPLE] unknown error 90 arg1=2 arg2=11417
	16:28:25 ERROR 4b00 [FORWARDING] no next hop
	16:42:20 ERROR 4b00 [IEEE802154E] wdAckDuration overflows while at state 13 in slotOffset 1
	16:42:21 ERROR 4b00 [CEXAMPLE] unknown error 90 arg1=3 arg2=11832
	16:42:23 ERROR 4b00 [IEEE802154E] got desynchronized at slotOffset 8,arg 1
	16:42:23 INFO 4b00 [IEEE802154E] synchronized at slotOffset 0
	16:42:23 ERROR 4b00 [CEXAMPLE] unknown error 90 arg1=4 arg2=11832
	17:20:00 ERROR 4b00 [IEEE802154E] wdAckDuration overflows while at state 13 in slotOffset 1
	17:20:01 ERROR 4b00 [CEXAMPLE] unknown error 90 arg1=5 arg2=12953
	17:20:02 ERROR 4b00 [IEEE802154E] got desynchronized at slotOffset 8,arg 1
	17:20:04 INFO 4b00 [IEEE802154E] synchronized at slotOffset 0
	17:20:04 ERROR 4b00 [CEXAMPLE] unknown error 90 arg1=6 arg2=12953*/
    openserial_printError(COMPONENT_IEEE802154E,ERR_WDACKDURATION_OVERFLOWS,
                         (errorparameter_t)ieee154e_vars.state,
                         (errorparameter_t)ieee154e_vars.slotOffset);
    // abort
    endSlot();
}

port_INLINE void activity_ti9(PORT_TIMER_WIDTH capturedTime) 
{
    ieee802154_header_iht     ieee802514_header;
    
    // change state
    changeState(S_TXPROC);
#ifdef SLOT_FSM_IMPLEMENTATION_MULTIPLE_TIMER_INTERRUPT
    // cancel tt8
    radiotimer_cancel(ACTION_NORMAL_TIMER);
#else
    // cancel tt8
    opentimers_scheduleAbsolute(
        ieee154e_vars.timerId,                            // timerId
        ieee154e_vars.slotDuration,                       // duration
        ieee154e_vars.startOfSlotReference,               // reference
        TIME_TICS,                                        // timetype
        isr_ieee154e_newSlot                              // callback
    );
    // radiotimer_cancel();
#endif
    // turn off the radio
    radio_rfOff();
    //compute tics radio on.
    ieee154e_vars.radioOnTics+=(sctimer_readCounter()-ieee154e_vars.radioOnInit);
   
    // record the captured time
    ieee154e_vars.lastCapturedTime = capturedTime;
   
    // get a buffer to put the (received) ACK in
    ieee154e_vars.ackReceived = openqueue_getFreePacketBuffer(COMPONENT_IEEE802154E);
    if (ieee154e_vars.ackReceived==NULL) {
        // log the error
        openserial_printError(COMPONENT_IEEE802154E,ERR_NO_FREE_PACKET_BUFFER,
                            (errorparameter_t)0,
                            (errorparameter_t)0);
        // abort
        endSlot();
        return;
    }
      
    // declare ownership over that packet
    ieee154e_vars.ackReceived->creator = COMPONENT_IEEE802154E;
    ieee154e_vars.ackReceived->owner   = COMPONENT_IEEE802154E;
    
    /*
    The do-while loop that follows is a little parsing trick.
    Because it contains a while(0) condition, it gets executed only once.
    Below the do-while loop is some code to cleans up the ack variable.
    Anywhere in the do-while loop, a break statement can be called to jump to
    the clean up code early. If the loop ends without a break, the received
    packet was correct. If it got aborted early (through a break), the packet
    was faulty.
    */
    do { // this "loop" is only executed once
        
        // retrieve the received ack frame from the radio's Rx buffer
        ieee154e_vars.ackReceived->payload = &(ieee154e_vars.ackReceived->packet[FIRST_FRAME_BYTE]);
        radio_getReceivedFrame(       ieee154e_vars.ackReceived->payload,
                                   &ieee154e_vars.ackReceived->length,
                             sizeof(ieee154e_vars.ackReceived->packet),
                                   &ieee154e_vars.ackReceived->l1_rssi,
                                   &ieee154e_vars.ackReceived->l1_lqi,
                                   &ieee154e_vars.ackReceived->l1_crc);
      
        // break if wrong length
        if (ieee154e_vars.ackReceived->length<LENGTH_CRC || ieee154e_vars.ackReceived->length>LENGTH_IEEE154_MAX) {
            // break from the do-while loop and execute the clean-up code below
            openserial_printError(COMPONENT_IEEE802154E,ERR_INVALIDPACKETFROMRADIO,
                            (errorparameter_t)1,
                            ieee154e_vars.ackReceived->length);
        
            break;
        }
      
        // toss CRC (2 last bytes)
        packetfunctions_tossFooter(   ieee154e_vars.ackReceived, LENGTH_CRC);
   
        // break if invalid CRC
        if (ieee154e_vars.ackReceived->l1_crc==FALSE) {
            // break from the do-while loop and execute the clean-up code below
            break;
        }
      
        // parse the IEEE802.15.4 header (RX ACK)
        ieee802154_retrieveHeader(ieee154e_vars.ackReceived,&ieee802514_header);
      
        // break if invalid IEEE802.15.4 header
        if (ieee802514_header.valid==FALSE) {
            // break from the do-while loop and execute the clean-up code below
            break;
        }

        // store header details in packet buffer
        ieee154e_vars.ackReceived->l2_frameType  = ieee802514_header.frameType;
        ieee154e_vars.ackReceived->l2_dsn        = ieee802514_header.dsn;
        memcpy(&(ieee154e_vars.ackReceived->l2_nextORpreviousHop),&(ieee802514_header.src),sizeof(open_addr_t));
        
        // verify that incoming security level is acceptable
        if (IEEE802154_security_acceptableLevel(ieee154e_vars.ackReceived, &ieee802514_header) == FALSE) {
            break;
        }
 
        // check the security level of the ACK frame and decrypt/authenticate
        if (ieee154e_vars.ackReceived->l2_securityLevel != IEEE154_ASH_SLF_TYPE_NOSEC) {
            if (IEEE802154_security_incomingFrame(ieee154e_vars.ackReceived) != E_SUCCESS) {
                break;
            }
        } 
    
        // toss the IEEE802.15.4 header
        packetfunctions_tossHeader(ieee154e_vars.ackReceived,ieee802514_header.headerLength);
         
        // break if invalid ACK
        if (isValidAck(&ieee802514_header,ieee154e_vars.dataToSend)==FALSE) {
            // break from the do-while loop and execute the clean-up code below
            break;
        }
        
        if (
            idmanager_getIsDAGroot()==FALSE &&
            icmpv6rpl_isPreferredParent(&(ieee154e_vars.ackReceived->l2_nextORpreviousHop))
        ) {
            synchronizeAck(ieee802514_header.timeCorrection); 
        }
      
        // inform schedule of successful transmission
        schedule_indicateTx(&ieee154e_vars.asn,TRUE);
      
        // inform upper layer
        notif_sendDone(ieee154e_vars.dataToSend,E_SUCCESS);
        ieee154e_vars.dataToSend = NULL;
      
        // in any case, execute the clean-up code below (processing of ACK done)
    } while (0);
   
    // free the received ack so corresponding RAM memory can be recycled
    openqueue_freePacketBuffer(ieee154e_vars.ackReceived);
   
    // clear local variable
    ieee154e_vars.ackReceived = NULL;
   
    // official end of Tx slot
    endSlot();
}

//======= RX

port_INLINE void activity_ri2() {
    // change state
    changeState(S_RXDATAPREPARE);
#ifdef SLOT_FSM_IMPLEMENTATION_MULTIPLE_TIMER_INTERRUPT
#else
    // arm rt2
   opentimers_scheduleAbsolute(
        ieee154e_vars.timerId,                            // timerId
        DURATION_rt2,                                     // duration
        ieee154e_vars.startOfSlotReference,               // reference
        TIME_TICS,                                        // timetype
        isr_ieee154e_timer                                // callback
    );
    // radiotimer_schedule(DURATION_rt2);
#endif
   

#ifdef SLOT_FSM_IMPLEMENTATION_MULTIPLE_TIMER_INTERRUPT
    radio_rxEnable_scum();
#else
    // enable the radio in Rx mode. The radio does not actively listen yet.
    radio_rxEnable();
        // configure the radio for that frequency
    radio_setFrequency(ieee154e_vars.freq);
#endif
    ieee154e_vars.radioOnInit=sctimer_readCounter();
    ieee154e_vars.radioOnThisSlot=TRUE;
       
    // change state
    changeState(S_RXDATAREADY);
}

port_INLINE void activity_rie1() {
   // log the error
   openserial_printError(COMPONENT_IEEE802154E,ERR_MAXRXDATAPREPARE_OVERFLOWS,
                         (errorparameter_t)ieee154e_vars.state,
                         (errorparameter_t)ieee154e_vars.slotOffset);
   
   // abort
   endSlot();
}

port_INLINE void activity_ri3() {
    // change state
    changeState(S_RXDATALISTEN);
    
    // give the 'go' to receive
    radio_rxNow();
#ifdef SLOT_FSM_IMPLEMENTATION_MULTIPLE_TIMER_INTERRUPT
    // arm rt3
    radiotimer_schedule(ACTION_NORMAL_TIMER,DURATION_rt3);
#else
    // arm rt3 
    opentimers_scheduleAbsolute(
        ieee154e_vars.timerId,                            // timerId
        DURATION_rt3,                                     // duration
        ieee154e_vars.startOfSlotReference,               // reference
        TIME_TICS,                                        // timetype
        isr_ieee154e_timer                                // callback
    );
    // radiotimer_schedule(DURATION_rt3);
#endif
}

port_INLINE void activity_rie2() 
{
   // abort
	if(ieee154e_vars.Mode==LOWERPOWER_CALLINGON&&idmanager_getIsDAGroot()==TRUE)//when I am in calling mode zyx 20180823
	{
		radio_rfOff();
		changeState(S_TXCALLINGDATAOFFSET);
		//Prepare the calling packets and then send it
		opentimers_scheduleAbsolute(
	    ieee154e_vars.timerId,                            // timerId
	    DURATION_ct9,                                     // duration
	    ieee154e_vars.startOfSlotReference,               // reference
	    TIME_TICS,                                        // timetype
	    isr_ieee154e_timer                                // callback
	);
	}else
	{
		endSlot();
	}
}
//////char strbuf[512];
port_INLINE void activity_ri4(PORT_TIMER_WIDTH capturedTime) {

   // change state
   changeState(S_RXDATA);
   
#ifdef SLOT_FSM_IMPLEMENTATION_MULTIPLE_TIMER_INTERRUPT
   // cancel rt3
   radiotimer_cancel(ACTION_NORMAL_TIMER);
#else
   // cancel rt3
    opentimers_scheduleAbsolute(
        ieee154e_vars.timerId,                            // timerId
        ieee154e_vars.slotDuration,                       // duration
        ieee154e_vars.startOfSlotReference,               // reference
        TIME_TICS,                                        // timetype
        isr_ieee154e_newSlot                              // callback
    );//屏蔽此处，同步字中断收到，接收中断不能收到的问题解决20180108
    // radiotimer_cancel();
#endif
   // record the captured time
   ieee154e_vars.lastCapturedTime = capturedTime;
   
   // record the captured time to sync
   ieee154e_vars.syncCapturedTime = capturedTime;
#ifdef SLOT_FSM_IMPLEMENTATION_MULTIPLE_TIMER_INTERRUPT
   radiotimer_schedule(ACTION_NORMAL_TIMER,DURATION_rt4);
#else
    opentimers_scheduleAbsolute(
        ieee154e_vars.timerId,                            // timerId
        DURATION_rt4,                                     // duration
        ieee154e_vars.startOfSlotReference,               // reference
        TIME_TICS,                                        // timetype
        isr_ieee154e_timer                                // callback
    );
    // radiotimer_schedule(DURATION_rt4);
#endif
}

port_INLINE void activity_rie3() {
    // log the error
	//debugpins_frame_toggle();  //lcg 20180530
    //openserial_printError(COMPONENT_IEEE802154E,ERR_WDDATADURATION_OVERFLOWS,   //lcg 20180530  ,some case,there are more then one sender send frame simultaneously ,this is normoal.
     //                    (errorparameter_t)ieee154e_vars.state,
      //                   (errorparameter_t)ieee154e_vars.slotOffset);
   
    // abort
    ///printf("ieee154e_vars.lastCapturedTime+wdDataDuration=%d\n",ieee154e_vars.lastCapturedTime+wdDataDuration);//20180108
    printc("rie3break");
    endSlot();
}
uint8_t CRC_CHIP[4];
uint32_t CRC_CALCULATE;
port_INLINE void activity_ri5(PORT_TIMER_WIDTH capturedTime) {
    ieee802154_header_iht ieee802514_header;
    uint16_t lenIE=0,i,len;
   
    // change state
    changeState(S_TXACKOFFSET);
#ifdef SLOT_FSM_IMPLEMENTATION_MULTIPLE_TIMER_INTERRUPT
    // cancel rt4
    radiotimer_cancel(ACTION_NORMAL_TIMER);
#else
    // cancel rt4////20180111
    
    opentimers_scheduleAbsolute(
        ieee154e_vars.timerId,                            // timerId
        ieee154e_vars.slotDuration,                       // duration
        ieee154e_vars.startOfSlotReference,               // reference
        TIME_TICS,                                        // timetype
        isr_ieee154e_newSlot                              // callback
    );
    // radiotimer_cancel();
#endif
    // turn off the radio
    //radio_rfOff();
    ieee154e_vars.radioOnTics+=sctimer_readCounter()-ieee154e_vars.radioOnInit;
    // get a buffer to put the (received) data in
    ieee154e_vars.dataReceived = openqueue_getFreePacketBuffer(COMPONENT_IEEE802154E);
    if (ieee154e_vars.dataReceived==NULL) {
        // log the error
        openserial_printError(COMPONENT_IEEE802154E,ERR_NO_FREE_PACKET_BUFFER,
                            (errorparameter_t)0,
                            (errorparameter_t)0);
        // abort
        endSlot();
        return;
    }
    
    // declare ownership over that packet
    ieee154e_vars.dataReceived->creator = COMPONENT_IEEE802154E;
    ieee154e_vars.dataReceived->owner   = COMPONENT_IEEE802154E;

    /*
    The do-while loop that follows is a little parsing trick.
    Because it contains a while(0) condition, it gets executed only once.
    The behavior is:
    - if a break occurs inside the do{} body, the error code below the loop
        gets executed. This indicates something is wrong with the packet being 
        parsed.
    - if a return occurs inside the do{} body, the error code below the loop
        does not get executed. This indicates the received packet is correct.
    */
    do { // this "loop" is only executed once
      
        // retrieve the received data frame from the radio's Rx buffer
        ieee154e_vars.dataReceived->payload = &(ieee154e_vars.dataReceived->packet[FIRST_FRAME_BYTE]);
        radio_getReceivedFrame(
            ieee154e_vars.dataReceived->payload,
            &ieee154e_vars.dataReceived->length,
            sizeof(ieee154e_vars.dataReceived->packet),
            &ieee154e_vars.dataReceived->l1_rssi,
            &ieee154e_vars.dataReceived->l1_lqi,
            &ieee154e_vars.dataReceived->l1_crc
        );
        /////////////////////printc("RecRSSI=%d\n",ieee154e_vars.dataReceived->l1_rssi);
        // break if wrong length
#if 1
        S2LPSpiReadRegisters(0xA6, 4, CRC_CHIP);
        // turn off the radio
   		radio_rfOff();
        //CRC_CALCULATE=crc32(&ieee154e_vars.dataReceived->packet[1],ieee154e_vars.dataReceived->length);
        //printc("CRC_CLA=%x",CRC_CALCULATE);
        ieee154e_vars.dataReceived->packet[0]=ieee154e_vars.dataReceived->length;
        CRC_CALCULATE=HAL_CRC_Calculate(&CrcHandle,(uint32_t*)&ieee154e_vars.dataReceived->packet[0],(ieee154e_vars.dataReceived->length)+1);
		if(((CRC_CALCULATE>>24)&0xFF)==CRC_CHIP[0]&&((CRC_CALCULATE>>16)&0xFF)==CRC_CHIP[1]&&((CRC_CALCULATE>>8)&0xFF)==CRC_CHIP[2]&&((CRC_CALCULATE>>0)&0xFF)==CRC_CHIP[3])
		{
			
		}else
		{
			leds_error_toggle();
			leds_error_toggle();
   			leds_error_toggle();
   			leds_error_toggle();
   			leds_error_toggle();

			printc("CRC_CHIP=%x",CRC_CHIP[0]);
	        printc("CRC_CHIP=%x",CRC_CHIP[1]);
	        printc("CRC_CHIP=%x",CRC_CHIP[2]);
	        printc("CRC_CHIP=%x",CRC_CHIP[3]);
	        printc("CRC_CLACPU=%x",CRC_CALCULATE);
	        Uart_TX_BUF[(Uart_Tx_BUF_LEN++)&0x3F]=0x71;
	        Uart_TX_BUF[(Uart_Tx_BUF_LEN++)&0x3F]=CRC_CHIP[0];
	        Uart_TX_BUF[(Uart_Tx_BUF_LEN++)&0x3F]=CRC_CHIP[1];
	        Uart_TX_BUF[(Uart_Tx_BUF_LEN++)&0x3F]=CRC_CHIP[2];
	        Uart_TX_BUF[(Uart_Tx_BUF_LEN++)&0x3F]=CRC_CHIP[3];
	        Uart_TX_BUF[(Uart_Tx_BUF_LEN++)&0x3F]=(CRC_CALCULATE>>24)&0xFF;
	        Uart_TX_BUF[(Uart_Tx_BUF_LEN++)&0x3F]=(CRC_CALCULATE>>16)&0xFF;
	        Uart_TX_BUF[(Uart_Tx_BUF_LEN++)&0x3F]=(CRC_CALCULATE>>8)&0xFF;
	        Uart_TX_BUF[(Uart_Tx_BUF_LEN++)&0x3F]=(CRC_CALCULATE>>0)&0xFF;
	        for(i=0;i<(ieee154e_vars.dataReceived->length)+1;i++)
	        {
	        	Uart_TX_BUF[(Uart_Tx_BUF_LEN++)]=ieee154e_vars.dataReceived->packet[i];
	        }
	        Uart_TX_BUF[(Uart_Tx_BUF_LEN++)&0x3F]=0x71;
	        
	        
	        break;//20190312
		}
        ieee154e_vars.dataReceived->packet[0]=0;
#endif
        if (ieee154e_vars.dataReceived->length<LENGTH_CRC || ieee154e_vars.dataReceived->length>LENGTH_IEEE154_MAX ) {
            // jump to the error code below this do-while loop
            openserial_printError(COMPONENT_IEEE802154E,ERR_INVALIDPACKETFROMRADIO,
                            (errorparameter_t)2,
                            ieee154e_vars.dataReceived->length);
            break;
        }
      
        // toss CRC (2 last bytes)
        packetfunctions_tossFooter(ieee154e_vars.dataReceived, LENGTH_CRC);

      	
        // if CRC doesn't check, stop
        if (ieee154e_vars.dataReceived->l1_crc==FALSE) {
            // jump to the error code below this do-while loop
        	leds_error_toggle();
        	printc("Receive a packet but crc is FALSE,ri5 break\n");
        	
           // break;
        }
      
        // parse the IEEE802.15.4 header (RX DATA)
        
        ieee802154_retrieveHeader(ieee154e_vars.dataReceived,&ieee802514_header);
      
        // break if invalid IEEE802.15.4 header
        if (ieee802514_header.valid==FALSE) 
        {
            // break from the do-while loop and execute the clean-up code below
            leds_error_toggle();
            printc("Receive a packet 802154Header valid is FALSE,ri5 break\n");
            #if 0
            for(i=0;i<23;i++)
            	len+=sprintf(&strbuf[len],"%02x ",ieee154e_vars.dataReceived->packet[i]);
            printc("data:%s\n",strbuf);
            #endif
            break;
        }

        // store header details in packet buffer
        ieee154e_vars.dataReceived->l2_frameType      = ieee802514_header.frameType;
        ieee154e_vars.dataReceived->l2_dsn            = ieee802514_header.dsn;
        ieee154e_vars.dataReceived->l2_IEListPresent  = ieee802514_header.ieListPresent;
        memcpy(&(ieee154e_vars.dataReceived->l2_nextORpreviousHop),&(ieee802514_header.src),sizeof(open_addr_t));

        // verify that incoming security level is acceptable
        if (IEEE802154_security_acceptableLevel(ieee154e_vars.dataReceived, &ieee802514_header) == FALSE) {
            break;
        }
   
        // if security is active and configured need to decrypt the frame
        if (ieee154e_vars.dataReceived->l2_securityLevel != IEEE154_ASH_SLF_TYPE_NOSEC) {
            if (IEEE802154_security_isConfigured()) {
                if (IEEE802154_security_incomingFrame(ieee154e_vars.dataReceived) != E_SUCCESS) {
                    break;
                }
            }
            // bypass authentication of beacons during join process
            else if(ieee154e_vars.dataReceived->l2_frameType == IEEE154_TYPE_BEACON) { // not joined yet
                packetfunctions_tossFooter(ieee154e_vars.dataReceived, ieee154e_vars.dataReceived->l2_authenticationLength);
            }
            else {
                break;
            }
        }
        // toss the IEEE802.15.4 header
        packetfunctions_tossHeader(ieee154e_vars.dataReceived,ieee802514_header.headerLength);
        if (
            ieee802514_header.frameType       == IEEE154_TYPE_BEACON                             && // if it is not a beacon and have ie, the ie will be processed in sixtop
            ieee802514_header.ieListPresent   == TRUE                                            && 
            packetfunctions_sameAddress(&ieee802514_header.panid,idmanager_getMyID(ADDR_PANID))
        ) 
        {
            if (ieee154e_processIEs(ieee154e_vars.dataReceived,&lenIE)==FALSE)
            {
                // retrieve EB IE failed, break the do-while loop and execute the clean up code below
                break;
            }
        }
      
        // toss the IEs including Synch
        packetfunctions_tossHeader(ieee154e_vars.dataReceived,lenIE);
            
        // record the captured time
        ieee154e_vars.lastCapturedTime = capturedTime;
      
        // if I just received an invalid frame, stop
        if (isValidRxFrame(&ieee802514_header)==FALSE) 
        {
            // jump to the error code below this do-while loop
            break;
        }
      
        // record the timeCorrection and print out at end of slot
        ieee154e_vars.dataReceived->l2_timeCorrection = (PORT_SIGNED_INT_WIDTH)((PORT_SIGNED_INT_WIDTH)TsTxOffset-(PORT_SIGNED_INT_WIDTH)ieee154e_vars.syncCapturedTime);
      
        // check if ack requested
        if (ieee802514_header.ackRequested==1 && ieee154e_vars.isAckEnabled == TRUE) 
        {
#ifdef SLOT_FSM_IMPLEMENTATION_MULTIPLE_TIMER_INTERRUPT
            // 1. schedule timer for loading packet
            radiotimer_schedule(ACTION_LOAD_PACKET,DURATION_rt5);
            // get a buffer to put the ack to send in
            ieee154e_vars.ackToSend = openqueue_getFreePacketBuffer(COMPONENT_IEEE802154E);
            if (ieee154e_vars.ackToSend==NULL) {
                // log the error
                openserial_printError(
                    COMPONENT_IEEE802154E,ERR_NO_FREE_PACKET_BUFFER,
                    (errorparameter_t)0,
                    (errorparameter_t)0
                );
                // indicate we received a packet anyway (we don't want to loose any)
                notif_receive(ieee154e_vars.dataReceived);
                // free local variable
                ieee154e_vars.dataReceived = NULL;
                // abort
                endSlot();
                return;
            }
           
            // declare ownership over that packet
            ieee154e_vars.ackToSend->creator = COMPONENT_IEEE802154E;
            ieee154e_vars.ackToSend->owner   = COMPONENT_IEEE802154E;
            
            // calculate the time timeCorrection (this is the time the sender is off w.r.t to this node. A negative number means
            // the sender is too late.
            ieee154e_vars.timeCorrection = (PORT_SIGNED_INT_WIDTH)((PORT_SIGNED_INT_WIDTH)TsTxOffset-(PORT_SIGNED_INT_WIDTH)ieee154e_vars.syncCapturedTime);
           
            // prepend the IEEE802.15.4 header to the ACK
            ieee154e_vars.ackToSend->l2_frameType = IEEE154_TYPE_ACK;
            ieee154e_vars.ackToSend->l2_dsn       = ieee154e_vars.dataReceived->l2_dsn;

            // To send ACK, we use the same security level (including NOSEC) and keys
            // that were present in the DATA packet.
            ieee154e_vars.ackToSend->l2_securityLevel = ieee154e_vars.dataReceived->l2_securityLevel;
            ieee154e_vars.ackToSend->l2_keyIdMode     = ieee154e_vars.dataReceived->l2_keyIdMode;
            ieee154e_vars.ackToSend->l2_keyIndex      = ieee154e_vars.dataReceived->l2_keyIndex;

            ieee802154_prependHeader(
                ieee154e_vars.ackToSend,
                ieee154e_vars.ackToSend->l2_frameType,
                FALSE,//no payloadIE in ack
                ieee154e_vars.dataReceived->l2_dsn,
                &(ieee154e_vars.dataReceived->l2_nextORpreviousHop)
            );
           
            // if security is enabled, encrypt directly in OpenQueue as there are no retransmissions for ACKs
            if (ieee154e_vars.ackToSend->l2_securityLevel != IEEE154_ASH_SLF_TYPE_NOSEC) {
                if (IEEE802154_security_outgoingFrameSecurity(ieee154e_vars.ackToSend) != E_SUCCESS) {
                    openqueue_freePacketBuffer(ieee154e_vars.ackToSend);
                    endSlot();
                    return;
                }
            }
            // space for 2-byte CRC
            packetfunctions_reserveFooterSize(ieee154e_vars.ackToSend,2);
            // set tx buffer address and length to prepare loading packet (packet is NOT loaded at this moment)
            radio_loadPacket_prepare(ieee154e_vars.ackToSend->payload,
                                    ieee154e_vars.ackToSend->length);
            radiotimer_schedule(ACTION_SEND_PACKET,DURATION_rt6);
            // 2. schedule timer for radio tx watchdog
            radiotimer_schedule(ACTION_NORMAL_TIMER,DURATION_rt7);
            // 3. set capture for SFD senddone and Tx send done
            radiotimer_setCapture(ACTION_TX_SFD_DONE);
            radiotimer_setCapture(ACTION_TX_SEND_DONE);
#else
            // arm rt5
            opentimers_scheduleAbsolute(
                ieee154e_vars.timerId,                            // timerId
                DURATION_rt5,                                     // duration
                ieee154e_vars.startOfSlotReference,               // reference
                TIME_TICS,                                        // timetype
                isr_ieee154e_timer                                // callback
            );
            // radiotimer_schedule(DURATION_rt5);
#endif
        } 
        else 
        {
            // synchronize to the received packet if I'm not a DAGroot and this is my preferred parent
            // or in case I'm in the middle of the join process when parent is not yet selected
            /*if ((idmanager_getIsDAGroot()==FALSE && 
                icmpv6rpl_isPreferredParent(&(ieee154e_vars.dataReceived->l2_nextORpreviousHop))) ||
                IEEE802154_security_isConfigured() == FALSE) */
            if (idmanager_getIsDAGroot()==FALSE ||
            IEEE802154_security_isConfigured() == FALSE)//lcg 20180602
            {
            	if(ieee154e_vars.dataReceived->packet[1]==0x46)
            	{
            		synchronizecallingPacket(ieee154e_vars.syncCapturedTime);//it is a calling queue zyx
					break;
            	}
            	else
            	{
            		synchronizePacket(ieee154e_vars.syncCapturedTime);
            		//check if it is LSSS Mode
            		if(ieee154e_vars.Mode==LOWERPOWER_RUN&&ieee154e_vars.dataReceived->packet[1]==0x41)
            		{
            			ieee154e_changemode(LOWERPOWER_RUN_MORE_TIME);
            			
            		}
            		
            	}
        		
        	}
        	else
        	{
        		if(ieee154e_vars.dataReceived->packet[1]==0x46)//I am a master I received a calling queue
        		{
        			break;
        		}
        	}
            // indicate reception to upper layer (no ACK asked)
            
            notif_receive(ieee154e_vars.dataReceived);
            
            // reset local variable
            ieee154e_vars.dataReceived = NULL;
            // abort
            endSlot();
        }
      
        // everything went well, return here not to execute the error code below
        return;
      
    } while(0);
    
    // free the (invalid) received data so RAM memory can be recycled
    openqueue_freePacketBuffer(ieee154e_vars.dataReceived);
   
    // clear local variable
    ieee154e_vars.dataReceived = NULL;
   
    // abort
    endSlot();
}

port_INLINE void activity_ri6() {
   
    // change state
    changeState(S_TXACKPREPARE);
#ifdef SLOT_FSM_IMPLEMENTATION_MULTIPLE_TIMER_INTERRUPT
#else
    // arm rt6
    opentimers_scheduleAbsolute(
        ieee154e_vars.timerId,                            // timerId
        DURATION_rt6,                                     // duration
        ieee154e_vars.startOfSlotReference,               // reference
        TIME_TICS,                                        // timetype
        isr_ieee154e_timer                                // callback
    );
    // radiotimer_schedule(DURATION_rt6);
#endif
#ifdef SLOT_FSM_IMPLEMENTATION_MULTIPLE_TIMER_INTERRUPT
#else
    // get a buffer to put the ack to send in
    ieee154e_vars.ackToSend = openqueue_getFreePacketBuffer(COMPONENT_IEEE802154E);
    if (ieee154e_vars.ackToSend==NULL) {
        // log the error
        openserial_printError(COMPONENT_IEEE802154E,ERR_NO_FREE_PACKET_BUFFER,
                            (errorparameter_t)0,
                            (errorparameter_t)0);
        // indicate we received a packet anyway (we don't want to loose any)
        notif_receive(ieee154e_vars.dataReceived);
        // free local variable
        ieee154e_vars.dataReceived = NULL;
        // abort
        endSlot();
        return;
    }
   
    // declare ownership over that packet
    ieee154e_vars.ackToSend->creator = COMPONENT_IEEE802154E;
    ieee154e_vars.ackToSend->owner   = COMPONENT_IEEE802154E;
   
    // calculate the time timeCorrection (this is the time the sender is off w.r.t to this node. A negative number means
    // the sender is too late.
    ieee154e_vars.timeCorrection = (PORT_SIGNED_INT_WIDTH)((PORT_SIGNED_INT_WIDTH)TsTxOffset-(PORT_SIGNED_INT_WIDTH)ieee154e_vars.syncCapturedTime);
   
    // prepend the IEEE802.15.4 header to the ACK
    ieee154e_vars.ackToSend->l2_frameType = IEEE154_TYPE_ACK;
    ieee154e_vars.ackToSend->l2_dsn       = ieee154e_vars.dataReceived->l2_dsn;

    // To send ACK, we use the same security level (including NOSEC) and keys
    // that were present in the DATA packet.
    ieee154e_vars.ackToSend->l2_securityLevel = ieee154e_vars.dataReceived->l2_securityLevel;
    ieee154e_vars.ackToSend->l2_keyIdMode     = ieee154e_vars.dataReceived->l2_keyIdMode;
    ieee154e_vars.ackToSend->l2_keyIndex      = ieee154e_vars.dataReceived->l2_keyIndex;

    ieee802154_prependHeader(ieee154e_vars.ackToSend,
                            ieee154e_vars.ackToSend->l2_frameType,
                            FALSE,//no payloadIE in ack
                            ieee154e_vars.dataReceived->l2_dsn,
                            &(ieee154e_vars.dataReceived->l2_nextORpreviousHop)
                            );
   
    // if security is enabled, encrypt directly in OpenQueue as there are no retransmissions for ACKs
    if (ieee154e_vars.ackToSend->l2_securityLevel != IEEE154_ASH_SLF_TYPE_NOSEC) {
        if (IEEE802154_security_outgoingFrameSecurity(ieee154e_vars.ackToSend) != E_SUCCESS) {
            openqueue_freePacketBuffer(ieee154e_vars.ackToSend);
            endSlot();
            return;
        }
    }
    // space for 2-byte CRC
   packetfunctions_reserveFooterSize(ieee154e_vars.ackToSend,2);
#endif
   // configure the radio for that frequency
    radio_txEnable();
   if(idmanager_getIsDAGroot()&&ieee154e_vars.Mode==LOWERPOWER_CALLINGON)//if I am master and I am in calling mode I will change the Frame length for the data type zyx added
	{
		radio_setPreamble_len(Communicate_Preamble_LEN);
	}
   radio_setFrequency(ieee154e_vars.freq);
   
   // load the packet in the radio's Tx buffer
   radio_loadPacket(ieee154e_vars.ackToSend->payload,
                    ieee154e_vars.ackToSend->length);
   
    // enable the radio in Tx mode. This does not send that packet.

    ieee154e_vars.radioOnInit=sctimer_readCounter();
    ieee154e_vars.radioOnThisSlot=TRUE;
    // change state
    changeState(S_TXACKREADY);
#ifdef SLOT_FSM_IMPLEMENTATION_MULTIPLE_TIMER_INTERRUPT
    changeState(S_TXACKDELAY);
#endif
}

port_INLINE void activity_rie4() 
{
    // log the error
    openserial_printError(COMPONENT_IEEE802154E,ERR_MAXTXACKPREPARE_OVERFLOWS,
                         (errorparameter_t)ieee154e_vars.state,
                         (errorparameter_t)ieee154e_vars.slotOffset);
   
    // abort
    endSlot();
}

port_INLINE void activity_ri7() {
    // change state
    changeState(S_TXACKDELAY);
#ifdef SLOT_FSM_IMPLEMENTATION_MULTIPLE_TIMER_INTERRUPT
#else
    // arm rt7
    opentimers_scheduleAbsolute(
        ieee154e_vars.timerId,                            // timerId
        DURATION_rt7,                                     // duration
        ieee154e_vars.startOfSlotReference,               // reference
        TIME_TICS,                                        // timetype
        isr_ieee154e_timer                                // callback
    );
    // radiotimer_schedule(DURATION_rt7);
    
    // give the 'go' to transmit
    radio_txNow(); 
#endif
}

port_INLINE void activity_rie5() {
   // log the error
   openserial_printError(COMPONENT_IEEE802154E,ERR_WDRADIOTX_OVERFLOWS,
                         (errorparameter_t)ieee154e_vars.state,
                         (errorparameter_t)ieee154e_vars.slotOffset);
   
   // abort
   endSlot();
}

port_INLINE void activity_ri8(PORT_TIMER_WIDTH capturedTime) {
    // change state
    changeState(S_TXACK);
#ifdef SLOT_FSM_IMPLEMENTATION_MULTIPLE_TIMER_INTERRUPT
    // cancel rt7
    radiotimer_cancel(ACTION_NORMAL_TIMER);
#else
    // cancel rt7
    opentimers_scheduleAbsolute(
        ieee154e_vars.timerId,                            // timerId
        ieee154e_vars.slotDuration,                       // duration
        ieee154e_vars.startOfSlotReference,               // reference
        TIME_TICS,                                        // timetype
        isr_ieee154e_newSlot                              // callback
    );
    // radiotimer_cancel();
#endif
    // record the captured time
    ieee154e_vars.lastCapturedTime = capturedTime;
#ifdef SLOT_FSM_IMPLEMENTATION_MULTIPLE_TIMER_INTERRUPT
    // arm rt8
    radiotimer_schedule(ACTION_NORMAL_TIMER,DURATION_rt8);
#else
    // arm rt8
    opentimers_scheduleAbsolute(
        ieee154e_vars.timerId,                            // timerId
        DURATION_rt8,                                     // duration
        ieee154e_vars.startOfSlotReference,               // reference
        TIME_TICS,                                        // timetype
        isr_ieee154e_timer                                // callback
    );
    // radiotimer_schedule(DURATION_rt8);
#endif
}

port_INLINE void activity_rie6() {
   // log the error
   openserial_printError(COMPONENT_IEEE802154E,ERR_WDACKDURATION_OVERFLOWS,
                         (errorparameter_t)ieee154e_vars.state,
                         (errorparameter_t)ieee154e_vars.slotOffset);
   
   // abort
   endSlot();
}

port_INLINE void activity_ri9(PORT_TIMER_WIDTH capturedTime) 
{
   // change state
   changeState(S_RXPROC);
#ifdef SLOT_FSM_IMPLEMENTATION_MULTIPLE_TIMER_INTERRUPT
   // cancel rt8
   radiotimer_cancel(ACTION_NORMAL_TIMER);
#else
   // cancel rt8
    opentimers_scheduleAbsolute(
        ieee154e_vars.timerId,                            // timerId
        ieee154e_vars.slotDuration,                       // duration
        ieee154e_vars.startOfSlotReference,               // reference
        TIME_TICS,                                        // timetype
        isr_ieee154e_newSlot                              // callback
    );
    // radiotimer_cancel();
#endif
   // record the captured time
   ieee154e_vars.lastCapturedTime = capturedTime;
   
   // free the ack we just sent so corresponding RAM memory can be recycled
   openqueue_freePacketBuffer(ieee154e_vars.ackToSend);
   
   // clear local variable
   ieee154e_vars.ackToSend = NULL;
   
   // inform upper layer of reception (after ACK sent)
   notif_receive(ieee154e_vars.dataReceived);
   
   // clear local variable
   ieee154e_vars.dataReceived = NULL;
   
   // official end of Rx slot
   endSlot();
}
port_INLINE void  activity_ci1()
{
   	uint8_t     asn[5];
    // change state
    changeState(S_TXCALLINGDATAPREPARE);
#ifdef SLOT_FSM_IMPLEMENTATION_MULTIPLE_TIMER_INTERRUPT
#else
    // arm tt2
    opentimers_scheduleAbsolute(
          ieee154e_vars.timerId,                            // timerId
          DURATION_ct10,                                     // duration
          ieee154e_vars.startOfSlotReference,               // reference
          TIME_TICS,                                        // timetype
          isr_ieee154e_timer                                // callback
    );
    // radiotimer_schedule(DURATION_tt2);

    // make a local copy of the frame
    
    ////ieee154e_prepare_calling_package(&ieee154e_vars.CallingQueue);//I will prepare the calling Queue every calling time
	ieee154e_vars.dataToSend=&ieee154e_vars.CallingQueue;
    ieee154e_getAsn(asn);
    memcpy(ieee154e_vars.dataToSend->l2_ASNpayload,&asn[0],sizeof(asn_t));
    // memcpy(ieee154e_vars.dataToSend->l2_ASNpayload+sizeof(asn_t),&join_priority,sizeof(uint8_t));//原来是0现在暂时忽略
    ieee154e_vars.dataToSend->l2_numTxAttempts++;
    //packetfunctions_duplicatePacket(&ieee154e_vars.localCopyForTransmission,&ieee154e_vars.CallingQueue);
    // add 2 CRC bytes only to the local copy as we end up here for each retransmission
    //packetfunctions_reserveFooterSize(&ieee154e_vars.localCopyForTransmission, 2);
#endif
   
    // configure the radio for that frequency
    radio_txEnable();
   	radio_setPreamble_len(Calling_Preamble_LEN);
    radio_setFrequency(ieee154e_vars.callingfreq);
	
    // load the packet in the radio's Tx buffer
    //radio_loadPacket(asn,5);//to test
    radio_loadPacket(ieee154e_vars.dataToSend->payload,
                     ieee154e_vars.dataToSend->length);
    // enable the radio in Tx mode. This does not send the packet.
    

    ieee154e_vars.radioOnInit=sctimer_readCounter();
    ieee154e_vars.radioOnThisSlot=TRUE;
    // change state
    changeState(S_TXCALLINGDATAREADY);
#ifdef SLOT_FSM_IMPLEMENTATION_MULTIPLE_TIMER_INTERRUPT
    // update state in advance
    changeState(S_TXCALLINGDATAREADY);
#endif
}
port_INLINE void activity_ci2() //txnow
{
    // change state
    changeState(S_TXCALLINGDATADELAY);
#ifdef SLOT_FSM_IMPLEMENTATION_MULTIPLE_TIMER_INTERRUPT
#else
    // arm tt3
    opentimers_scheduleAbsolute(
        ieee154e_vars.timerId,                            // timerId
        DURATION_ct11,                                     // duration
        ieee154e_vars.startOfSlotReference,               // reference
        TIME_TICS,                                        // timetype
        isr_ieee154e_timer                                // callback
    );
    // radiotimer_schedule(DURATION_tt3);
    
    // give the 'go' to transmit
    radio_txNow();
    //ieee154e_startOfFrame(sctimer_readCounter());//20180125zyx
    
#endif
}
port_INLINE void activity_ci3(PORT_TIMER_WIDTH capturedTime)/*START of FRAM*/
{
	changeState(S_TXCALLINGDATA);
#ifdef SLOT_FSM_IMPLEMENTATION_MULTIPLE_TIMER_INTERRUPT
    // cancel ct3
    radiotimer_cancel(ACTION_NORMAL_TIMER);
#else
    // cancel ct3
    opentimers_scheduleAbsolute(
        ieee154e_vars.timerId,                            // timerId
        ieee154e_vars.slotDuration,                       // duration
        ieee154e_vars.startOfSlotReference,               // reference
        TIME_TICS,                                        // timetype
        isr_ieee154e_newSlot                              // callback
    );
    // radiotimer_cancel();
#endif
    // record the captured time
    ieee154e_vars.lastCapturedTime = capturedTime;
#ifdef SLOT_FSM_IMPLEMENTATION_MULTIPLE_TIMER_INTERRUPT
    // arm tt4
    radiotimer_schedule(ACTION_NORMAL_TIMER,DURATION_ct12);
#else
    // arm tt4
    opentimers_scheduleAbsolute(
        ieee154e_vars.timerId,                            // timerId
        DURATION_ct12,                                     // duration
        ieee154e_vars.startOfSlotReference,               // reference
        TIME_TICS,                                        // timetype
        isr_ieee154e_timer                                // callback
    );
    // radiotimer_schedule(DURATION_tt4);
#endif
}
port_INLINE	void activity_ci4(PORT_TIMER_WIDTH capturedTime)/*end of FRAME*/
{
    // change state no listen for ack
 
    changeState(S_TXCALLINGDATADONE);
#ifdef SLOT_FSM_IMPLEMENTATION_MULTIPLE_TIMER_INTERRUPT
    // cancel ci4
    radiotimer_cancel(ACTION_NORMAL_TIMER);
#else
    // cancel ci4
    opentimers_scheduleAbsolute(
        ieee154e_vars.timerId,                            // timerId
        ieee154e_vars.slotDuration,                       // duration
        ieee154e_vars.startOfSlotReference,               // reference
        TIME_TICS,                                        // timetype
        isr_ieee154e_newSlot                              // callback
    );
    // radiotimer_cancel();
#endif
    // turn off the radio
    ieee154e_vars.radioOnTics+=(sctimer_readCounter()-ieee154e_vars.radioOnInit);
    ieee154e_vars.dataToSend = NULL;
    // abort
    endSlot();

}

port_INLINE void activity_cie1()
{
	leds_error_toggle();
	printc("MASTER CIE1 break!");
	//endSlot();
}

port_INLINE void activity_si1()
{
	 changeState(S_RXPREAMBLELISTEN);
	 opentimers_scheduleAbsolute(
	  ieee154e_vars.timerId,                            // timerId
	  listenpreamableDuration,              // duration
	  sctimer_readCounter(),//ieee154e_vars.startOfSlotReference,  // reference
	  TIME_TICS,                                        // timetype
	  isr_ieee154e_timer                                // callback
	);
	debugpins_radio_set();
   	leds_radio_on();
	radio_rxEnable();
	radio_setFrequency(ieee154e_vars.callingfreq);
	radio_rxNow();
	
	/**/
}/*when it receive a preamble interrupt it will arrive here or si1 will go to sie1*/
port_INLINE void activity_si2()
{
	//changeState(S_RXDATALISTEN);
	if(ieee154e_vars.Mode!=LOWERPOWER_CALLINGON)
	{
		radio_DISABLE_PREAMBLE_INTERRUPT();
		return;
	}
	changeState(S_SYNCLISTEN);
	opentimers_scheduleAbsolute(
	    ieee154e_vars.timerId,                            // timerId
	    2*PORT_TsSlotDuration/3,//*listenpreamablePeriod,//ieee154e_vars.slotDuration,                       // duration
	    sctimer_readCounter(),               // reference
	    TIME_TICS,                                        // timetype
	    isr_ieee154e_timer                              // callback
		);
	/**/
}
port_INLINE void activity_sie1()
{
	static uint8_t Func_time=1;
	uint32_t	settime=0;
	
	changeState(S_LONGTIMESLEEP);
	radio_rfOff();
	if(Func_time==LOWPOWER_LISTENING_Preamble_Times_ONESLOT)
	{//the third time it will set a long time to go to a new slot 
		if(ieee154e_vars.asn.bytes0and1<10&&ieee154e_vars.asn.bytes2and3==0)
		{
			settime=32765/3;// the slave will wake up 5times every 1s the first 5 secs 
		}else
		{
			settime=sleepduration;
		}
		Func_time=1;
		opentimers_scheduleAbsolute(
	    ieee154e_vars.timerId,                            // timerId
	    settime,//ieee154e_vars.slotDuration,// duration
	    sctimer_readCounter(),               // reference
	    TIME_TICS,                                        // timetype
	    isr_ieee154e_newSlot                                // callback
		);
	}else
	{
		opentimers_scheduleAbsolute(
	    ieee154e_vars.timerId,                            // timerId
	    listenpreamablePeriod,//ieee154e_vars.slotDuration,                       // duration
	    sctimer_readCounter(),               // reference
	    TIME_TICS,                                        // timetype
	    isr_ieee154e_timer                              // callback
		);
		Func_time++;
	}
	
   	/**/
}






//======= frame validity check

/**
\brief Decides whether the packet I just received is valid received frame.

A valid Rx frame satisfies the following constraints:
- its IEEE802.15.4 header is well formatted
- it's a DATA of BEACON frame (i.e. not ACK and not COMMAND)
- it's sent on the same PANid as mine
- it's for me (unicast or broadcast)

\param[in] ieee802514_header IEEE802.15.4 header of the packet I just received

\returns TRUE if packet is valid received frame, FALSE otherwise
*/
port_INLINE bool isValidRxFrame(ieee802154_header_iht* ieee802514_header) 
{
	uint8_t result;
   result= ieee802514_header->valid==TRUE                                                           && \
          (
             ieee802514_header->frameType==IEEE154_TYPE_DATA                   ||
             ieee802514_header->frameType==IEEE154_TYPE_BEACON
          )                                                                                        && \
          packetfunctions_sameAddress(&ieee802514_header->panid,idmanager_getMyID(ADDR_PANID))     && \
          (
             idmanager_isMyAddress(&ieee802514_header->dest)                   ||
             packetfunctions_isBroadcastMulticast(&ieee802514_header->dest)
          );
    return result;
}

/**
\brief Decides whether the packet I just received is a valid ACK.

A packet is a valid ACK if it satisfies the following conditions:
- the IEEE802.15.4 header is valid
- the frame type is 'ACK'
- the sequence number in the ACK matches the sequence number of the packet sent
- the ACK contains my PANid
- the packet is unicast to me
- the packet comes from the neighbor I sent the data to

\param[in] ieee802514_header IEEE802.15.4 header of the packet I just received
\param[in] packetSent points to the packet I just sent

\returns TRUE if packet is a valid ACK, FALSE otherwise.
*/
port_INLINE bool isValidAck(ieee802154_header_iht* ieee802514_header, OpenQueueEntry_t* packetSent) {
   /*
   return ieee802514_header->valid==TRUE                                                           && \
          ieee802514_header->frameType==IEEE154_TYPE_ACK                                           && \
          ieee802514_header->dsn==packetSent->l2_dsn                                               && \
          packetfunctions_sameAddress(&ieee802514_header->panid,idmanager_getMyID(ADDR_PANID))     && \
          idmanager_isMyAddress(&ieee802514_header->dest)                                          && \
          packetfunctions_sameAddress(&ieee802514_header->src,&packetSent->l2_nextORpreviousHop);
   */
   // poipoi don't check for seq num
   return ieee802514_header->valid==TRUE                                                           && \
          ieee802514_header->frameType==IEEE154_TYPE_ACK                                           && \
          packetfunctions_sameAddress(&ieee802514_header->panid,idmanager_getMyID(ADDR_PANID))     && \
          idmanager_isMyAddress(&ieee802514_header->dest)                                          && \
          packetfunctions_sameAddress(&ieee802514_header->src,&packetSent->l2_nextORpreviousHop);
}

//======= ASN handling

port_INLINE void incrementAsnOffset() 
{
   frameLength_t frameLength;
   
   // increment the asn
   ieee154e_vars.asn.bytes0and1++;
   if (ieee154e_vars.asn.bytes0and1==0) {
      ieee154e_vars.asn.bytes2and3++;
      if (ieee154e_vars.asn.bytes2and3==0) {
         ieee154e_vars.asn.byte4++;
      }
   }
   
   // increment the offsets
   frameLength = schedule_getFrameLength();
   if (frameLength == 0) 
   {
      ieee154e_vars.slotOffset++;
   } else {
      ieee154e_vars.slotOffset  = (ieee154e_vars.slotOffset+1)%frameLength;
   }
   ieee154e_vars.asnOffset   = (ieee154e_vars.asnOffset+1)%16;
}

port_INLINE void ieee154e_resetAsn(){
    // reset slotoffset
    ieee154e_vars.slotOffset     = 0;
    ieee154e_vars.asnOffset      = 0;
    // reset asn
    ieee154e_vars.asn.byte4      = 0;
    ieee154e_vars.asn.bytes2and3 = 0;
    ieee154e_vars.asn.bytes0and1 = 0;
}

//from upper layer that want to send the ASN to compute timing or latency
port_INLINE void ieee154e_getAsn(uint8_t* array) {
   array[0]         = (ieee154e_vars.asn.bytes0and1     & 0xff);
   array[1]         = (ieee154e_vars.asn.bytes0and1/256 & 0xff);
   array[2]         = (ieee154e_vars.asn.bytes2and3     & 0xff);
   array[3]         = (ieee154e_vars.asn.bytes2and3/256 & 0xff);
   array[4]         =  ieee154e_vars.asn.byte4;
}

port_INLINE uint16_t ieee154e_getTimeCorrection() {
    int16_t returnVal;
    
    returnVal = (uint16_t)(ieee154e_vars.timeCorrection);
    
    return returnVal;
}

port_INLINE void joinPriorityStoreFromEB(uint8_t jp){
  ieee154e_vars.dataReceived->l2_joinPriority = jp;
  ieee154e_vars.dataReceived->l2_joinPriorityPresent = TRUE;     
}

// This function parses IEs from an EB to get to the ASN before security
// processing is invoked. It should be called *only* when a node has no/lost sync.
// This way, we can authenticate EBs and reject unwanted ones.
bool isValidJoin(OpenQueueEntry_t* eb, ieee802154_header_iht *parsedHeader) {
   uint16_t              lenIE;

   // toss the header in order to get to IEs
   packetfunctions_tossHeader(eb, parsedHeader->headerLength);
     
   // process IEs
   // at this stage, this can work only if EB is authenticated but not encrypted
   lenIE = 0;
   if (
         (
            parsedHeader->valid==TRUE                                                       &&
            parsedHeader->ieListPresent==TRUE                                               &&
            parsedHeader->frameType==IEEE154_TYPE_BEACON                                    &&
            packetfunctions_sameAddress(&parsedHeader->panid,idmanager_getMyID(ADDR_PANID)) &&
            ieee154e_processIEs(eb,&lenIE)
         )==FALSE) {
      return FALSE;
   }
   
   // put everything back in place in order to invoke security-incoming on the
   // correct frame length and correct pointers (first byte of the frame)
   packetfunctions_reserveHeaderSize(eb, parsedHeader->headerLength);

   // verify EB's authentication tag if keys are configured
   if (IEEE802154_security_isConfigured()) {
      if (IEEE802154_security_incomingFrame(eb) == E_SUCCESS) {
         return TRUE;
      }
   }
   else {
       // bypass authentication check for beacons if security is not configured
        packetfunctions_tossFooter(eb,eb->l2_authenticationLength);
        return TRUE;
   }

   return FALSE;
}
/*Parsing data packet and config the slots zyx*/
#ifdef USE_FLASH_PARAMETER
bool isValidEbFormat(OpenQueueEntry_t* pkt, uint16_t* lenIE){
    
    bool    chTemplate_checkPass;
    bool    tsTemplate_checkpass;
    bool    sync_ie_checkPass;
    bool    slotframelink_ie_checkPass;
    
    uint8_t ptr;
    uint16_t temp16b;
    bool    mlme_ie_found;
    uint8_t mlme_ie_content_offset;
    uint8_t ielen;
    
    uint8_t subtype;
    uint8_t sublen;
    uint8_t subid;
    
    uint8_t  i;
    uint8_t  oldFrameLength;
    uint8_t  numlinks;
    open_addr_t temp_neighbor;
    uint16_t slotoffset;
    uint16_t channeloffset;
    uint8_t myoffsetaddress;//zyx added
    chTemplate_checkPass        = FALSE;
    tsTemplate_checkpass        = FALSE;
    sync_ie_checkPass           = FALSE;
    slotframelink_ie_checkPass  = FALSE; 
    
    ptr = 0;
    mlme_ie_found = FALSE;
    myoffsetaddress=idmanager_getmy_offsetaddress();//added by zyx
    while (ptr<pkt->length){
    
        temp16b  = *((uint8_t*)(pkt->payload)+ptr);
        temp16b |= (*((uint8_t*)(pkt->payload)+ptr+1))<<8;
        ptr += 2;
        
        ielen = temp16b & IEEE802154E_DESC_LEN_PAYLOAD_IE_MASK;
        
        if (
            (temp16b & IEEE802154E_DESC_GROUPID_PAYLOAD_IE_MASK)>>IEEE802154E_DESC_GROUPID_PAYLOAD_IE_SHIFT != IEEE802154E_MLME_IE_GROUPID || 
            (temp16b & IEEE802154E_DESC_TYPE_LONG) == 0
        ){
            // this is not MLME IE
            ptr += ielen;
        } else {
            // found the MLME payload IE
            mlme_ie_found = TRUE;
            mlme_ie_content_offset = ptr;
            break;
        }
    }
    
    if (mlme_ie_found==FALSE){
        // didn't find the MLME payload IE
        return FALSE;
    }
    
    while(
        ptr<mlme_ie_content_offset+ielen &&
        (
            chTemplate_checkPass        == FALSE || 
            tsTemplate_checkpass        == FALSE ||
            sync_ie_checkPass           == FALSE ||
            slotframelink_ie_checkPass  == FALSE 
        )
    ){
        // subID
        temp16b  = *((uint8_t*)(pkt->payload)+ptr);
        temp16b |= (*((uint8_t*)(pkt->payload)+ptr+1))<<8;
        ptr += 2;
        
        subtype = (temp16b & IEEE802154E_DESC_TYPE_IE_MASK)>>IEEE802154E_DESC_TYPE_IE_SHIFT;
        if (subtype == 1) {
            // this is long type subID
            subid  = (temp16b & IEEE802154E_DESC_SUBID_LONG_MLME_IE_MASK)>>IEEE802154E_DESC_SUBID_LONG_MLME_IE_SHIFT;
            sublen = (temp16b & IEEE802154E_DESC_LEN_LONG_MLME_IE_MASK);
            switch(subid){
            case IEEE802154E_MLME_CHANNELHOPPING_IE_SUBID:
                channelhoppingTemplateIDStoreFromEB(*((uint8_t*)(pkt->payload+ptr)));
                chTemplate_checkPass = TRUE;
                break;
            default:
                // unsupported IE type, skip the ie
              break;
            }
        } else {
            // this is short type subID
            subid  = (temp16b & IEEE802154E_DESC_SUBID_SHORT_MLME_IE_MASK)>>IEEE802154E_DESC_SUBID_SHORT_MLME_IE_SHIFT;
            sublen = (temp16b & IEEE802154E_DESC_LEN_SHORT_MLME_IE_MASK);
            switch(subid){
            case IEEE802154E_MLME_SYNC_IE_SUBID:
                asnStoreFromEB((uint8_t*)(pkt->payload+ptr));
                joinPriorityStoreFromEB(*((uint8_t*)(pkt->payload)+ptr+5));
                sync_ie_checkPass    = TRUE;
                break;
            case IEEE802154E_MLME_TIMESLOT_IE_SUBID:
                timeslotTemplateIDStoreFromEB(*((uint8_t*)(pkt->payload)+ptr));
                tsTemplate_checkpass = TRUE;
                break;
            case IEEE802154E_MLME_SLOTFRAME_LINK_IE_SUBID:
                schedule_setFrameNumber(*((uint8_t*)(pkt->payload)+ptr));       // number of slotframes
                schedule_setFrameHandle(*((uint8_t*)(pkt->payload)+ptr+1));     // slotframe id
                oldFrameLength = schedule_getFrameLength();
                if (oldFrameLength==0){
                    temp16b  = *((uint8_t*)(pkt->payload+ptr+2));               // slotframes length
                    temp16b |= *((uint8_t*)(pkt->payload+ptr+3))<<8;
                    schedule_setFrameLength(temp16b);
                    numlinks = *((uint8_t*)(pkt->payload+ptr+4));               // number of links

                    // shared TXRX anycast slot(s)
                    memset(&temp_neighbor,0,sizeof(temp_neighbor));
                    temp_neighbor.type             = ADDR_ANYCAST;
                    
                    for (i=0;i<numlinks;i++)
                    {
                        slotoffset     = *((uint8_t*)(pkt->payload+ptr+5+5*i));   // slotframes length
                        slotoffset    |= *((uint8_t*)(pkt->payload+ptr+5+5*i+1))<<8;
                        
                        channeloffset  = *((uint8_t*)(pkt->payload+ptr+5+5*i+2));   // slotframes length
                        channeloffset |= *((uint8_t*)(pkt->payload+ptr+5+5*i+3))<<8;
                        if(i==9)  //lcg  20180418//zyx	20180726
                        {
							schedule_addActiveSlot(
								slotoffset,    // slot offset
								CELLTYPE_RX, // type of slot
								TRUE,          // shared?
								channeloffset, // channel offset
								&temp_neighbor // neighbor
                        	);
                    	}
	                    else if((i==3*myoffsetaddress-1)||(i==3*myoffsetaddress-2)||(i==3*myoffsetaddress-3))
	                    //else if((i==SLAVENUMBER-1)||(i==SLAVENUMBER+3-1)||(i==SLAVENUMBER+6-1))//20180628zyx add 3slot20180726zyx
						//else if(i==SLAVENUMBER)
	                    {
	                            schedule_addActiveSlot(
	                                slotoffset,    // slot offset
	                                CELLTYPE_TX, // type of slot
	                                TRUE,          // shared?
	                                channeloffset, // channel offset
	                                &temp_neighbor // neighbor
	                            );
	                    }else
	                    {
	                    		schedule_addActiveSlot(
	                                slotoffset,    // slot offset
	                                CELLTYPE_MORESERIALRX, // type of slot
	                                TRUE,          // shared?
	                                channeloffset, // channel offset
	                                &temp_neighbor // neighbor
	                            );
	                    }
	               }
                }
                slotframelink_ie_checkPass = TRUE;
                break;
            default:
                // unsupported IE type, skip the ie
                break;
            }
        }
        ptr += sublen;
    }
    
    if (
        chTemplate_checkPass     && 
        tsTemplate_checkpass     && 
        sync_ie_checkPass        && 
        slotframelink_ie_checkPass
    ) {
        *lenIE = pkt->length;
        return TRUE;
    } else {
        return FALSE;
    }
}

#else
bool isValidEbFormat(OpenQueueEntry_t* pkt, uint16_t* lenIE){
    
    bool    chTemplate_checkPass;
    bool    tsTemplate_checkpass;
    bool    sync_ie_checkPass;
    bool    slotframelink_ie_checkPass;
    
    uint8_t ptr;
    uint16_t temp16b;
    bool    mlme_ie_found;
    uint8_t mlme_ie_content_offset;
    uint8_t ielen;
    
    uint8_t subtype;
    uint8_t sublen;
    uint8_t subid;
    
    uint8_t  i;
    uint8_t  oldFrameLength;
    uint8_t  numlinks;
    open_addr_t temp_neighbor;
    uint16_t slotoffset;
    uint16_t channeloffset;
    
    chTemplate_checkPass        = FALSE;
    tsTemplate_checkpass        = FALSE;
    sync_ie_checkPass           = FALSE;
    slotframelink_ie_checkPass  = FALSE; 
    
    ptr = 0;
    mlme_ie_found = FALSE;
    
    while (ptr<pkt->length){
    
        temp16b  = *((uint8_t*)(pkt->payload)+ptr);
        temp16b |= (*((uint8_t*)(pkt->payload)+ptr+1))<<8;
        ptr += 2;
        
        ielen = temp16b & IEEE802154E_DESC_LEN_PAYLOAD_IE_MASK;
        
        if (
            (temp16b & IEEE802154E_DESC_GROUPID_PAYLOAD_IE_MASK)>>IEEE802154E_DESC_GROUPID_PAYLOAD_IE_SHIFT != IEEE802154E_MLME_IE_GROUPID || 
            (temp16b & IEEE802154E_DESC_TYPE_LONG) == 0
        ){
            // this is not MLME IE
            ptr += ielen;
        } else {
            // found the MLME payload IE
            mlme_ie_found = TRUE;
            mlme_ie_content_offset = ptr;
            break;
        }
    }
    
    if (mlme_ie_found==FALSE){
        // didn't find the MLME payload IE
        return FALSE;
    }
    
    while(
        ptr<mlme_ie_content_offset+ielen &&
        (
            chTemplate_checkPass        == FALSE || 
            tsTemplate_checkpass        == FALSE ||
            sync_ie_checkPass           == FALSE ||
            slotframelink_ie_checkPass  == FALSE 
        )
    ){
        // subID
        temp16b  = *((uint8_t*)(pkt->payload)+ptr);
        temp16b |= (*((uint8_t*)(pkt->payload)+ptr+1))<<8;
        ptr += 2;
        
        subtype = (temp16b & IEEE802154E_DESC_TYPE_IE_MASK)>>IEEE802154E_DESC_TYPE_IE_SHIFT;
        if (subtype == 1) {
            // this is long type subID
            subid  = (temp16b & IEEE802154E_DESC_SUBID_LONG_MLME_IE_MASK)>>IEEE802154E_DESC_SUBID_LONG_MLME_IE_SHIFT;
            sublen = (temp16b & IEEE802154E_DESC_LEN_LONG_MLME_IE_MASK);
            switch(subid){
            case IEEE802154E_MLME_CHANNELHOPPING_IE_SUBID:
                channelhoppingTemplateIDStoreFromEB(*((uint8_t*)(pkt->payload+ptr)));
                chTemplate_checkPass = TRUE;
                break;
            default:
                // unsupported IE type, skip the ie
              break;
            }
        } else {
            // this is short type subID
            subid  = (temp16b & IEEE802154E_DESC_SUBID_SHORT_MLME_IE_MASK)>>IEEE802154E_DESC_SUBID_SHORT_MLME_IE_SHIFT;
            sublen = (temp16b & IEEE802154E_DESC_LEN_SHORT_MLME_IE_MASK);
            switch(subid){
            case IEEE802154E_MLME_SYNC_IE_SUBID:
                asnStoreFromEB((uint8_t*)(pkt->payload+ptr));
                joinPriorityStoreFromEB(*((uint8_t*)(pkt->payload)+ptr+5));
                sync_ie_checkPass    = TRUE;
                break;
            case IEEE802154E_MLME_TIMESLOT_IE_SUBID:
                timeslotTemplateIDStoreFromEB(*((uint8_t*)(pkt->payload)+ptr));
                tsTemplate_checkpass = TRUE;
                break;
            case IEEE802154E_MLME_SLOTFRAME_LINK_IE_SUBID:
                schedule_setFrameNumber(*((uint8_t*)(pkt->payload)+ptr));       // number of slotframes
                schedule_setFrameHandle(*((uint8_t*)(pkt->payload)+ptr+1));     // slotframe id
                oldFrameLength = schedule_getFrameLength();
                if (oldFrameLength==0){
                    temp16b  = *((uint8_t*)(pkt->payload+ptr+2));               // slotframes length
                    temp16b |= *((uint8_t*)(pkt->payload+ptr+3))<<8;
                    schedule_setFrameLength(temp16b);
                    numlinks = *((uint8_t*)(pkt->payload+ptr+4));               // number of links

                    // shared TXRX anycast slot(s)
                    memset(&temp_neighbor,0,sizeof(temp_neighbor));
                    temp_neighbor.type             = ADDR_ANYCAST;
                    
                    for (i=0;i<numlinks;i++)
                    {
                        slotoffset     = *((uint8_t*)(pkt->payload+ptr+5+5*i));   // slotframes length
                        slotoffset    |= *((uint8_t*)(pkt->payload+ptr+5+5*i+1))<<8;
                        
                        channeloffset  = *((uint8_t*)(pkt->payload+ptr+5+5*i+2));   // slotframes length
                        channeloffset |= *((uint8_t*)(pkt->payload+ptr+5+5*i+3))<<8;
                        if(i==9)  //lcg  20180418//zyx	20180726
                        {
							schedule_addActiveSlot(
								slotoffset,    // slot offset
								CELLTYPE_RX, // type of slot
								TRUE,          // shared?
								channeloffset, // channel offset
								&temp_neighbor // neighbor
                        	);
                    	}
	                    else if((i==3*SLAVENUMBER-1)||(i==3*SLAVENUMBER-2)||(i==3*SLAVENUMBER-3))
	                    //else if((i==SLAVENUMBER-1)||(i==SLAVENUMBER+3-1)||(i==SLAVENUMBER+6-1))//20180628zyx add 3slot20180726zyx
						//else if(i==SLAVENUMBER)
	                    {
	                            schedule_addActiveSlot(
	                                slotoffset,    // slot offset
	                                CELLTYPE_TX, // type of slot
	                                TRUE,          // shared?
	                                channeloffset, // channel offset
	                                &temp_neighbor // neighbor
	                            );
	                    }else
	                    {
	                    		schedule_addActiveSlot(
	                                slotoffset,    // slot offset
	                                CELLTYPE_MORESERIALRX, // type of slot
	                                TRUE,          // shared?
	                                channeloffset, // channel offset
	                                &temp_neighbor // neighbor
	                            );
	                    }
	               }
                }
                slotframelink_ie_checkPass = TRUE;
                break;
            default:
                // unsupported IE type, skip the ie
                break;
            }
        }
        ptr += sublen;
    }
    
    if (
        chTemplate_checkPass     && 
        tsTemplate_checkpass     && 
        sync_ie_checkPass        && 
        slotframelink_ie_checkPass
    ) {
        *lenIE = pkt->length;
        return TRUE;
    } else {
        return FALSE;
    }
}
#endif
port_INLINE void asnStoreFromEB(uint8_t* asn) {
   
   // store the ASN
   ieee154e_vars.asn.bytes0and1   =     asn[0]+
                                    256*asn[1];
   ieee154e_vars.asn.bytes2and3   =     asn[2]+
                                    256*asn[3];
   ieee154e_vars.asn.byte4        =     asn[4];
}

port_INLINE void ieee154e_syncSlotOffset() {
    frameLength_t frameLength;
    uint32_t slotOffset;
    uint8_t i;
   
    frameLength = schedule_getFrameLength();
   
    // determine the current slotOffset
    slotOffset = ieee154e_vars.asn.byte4;
    slotOffset = slotOffset % frameLength;
    slotOffset = slotOffset << 16;
    slotOffset = slotOffset + ieee154e_vars.asn.bytes2and3;
    slotOffset = slotOffset % frameLength;
    slotOffset = slotOffset << 16;
    slotOffset = slotOffset + ieee154e_vars.asn.bytes0and1;
    slotOffset = slotOffset % frameLength;
   
    ieee154e_vars.slotOffset       = (slotOffset_t) slotOffset;
   
    schedule_syncSlotOffset(ieee154e_vars.slotOffset);
    ieee154e_vars.nextActiveSlotOffset = schedule_getNextActiveSlotOffset();
    /* 
    infer the asnOffset based on the fact that
    ieee154e_vars.freq = 11 + (asnOffset + channelOffset)%16 
    */
    for (i=0;i<NUM_CHANNELS;i++){
        if ((ieee154e_vars.freq - 11)==ieee154e_vars.chTemplate[i]){
            break;
        }
    }
    ieee154e_vars.asnOffset = i - schedule_getChannelOffset();
}

void ieee154e_setIsAckEnabled(bool isEnabled){
    ieee154e_vars.isAckEnabled = isEnabled;
}

void ieee154e_setSingleChannel(uint8_t channel){
    if (
        (channel < 11 || channel > 26) &&
         channel != 0   // channel == 0 means channel hopping is enabled
    ) {
        // log wrong channel, should be  : (0, or 11~26)
        return;
    }
    ieee154e_vars.singleChannel = channel;
    ieee154e_vars.singleChannelChanged = TRUE;
}

void ieee154e_setIsSecurityEnabled(bool isEnabled){
    ieee154e_vars.isSecurityEnabled = isEnabled;
}

void ieee154e_setSlotDuration(uint16_t duration){
    ieee154e_vars.slotDuration = duration;
}

uint16_t ieee154e_getSlotDuration(){
    return ieee154e_vars.slotDuration;
}

// timeslot template handling
port_INLINE void timeslotTemplateIDStoreFromEB(uint8_t id){
    ieee154e_vars.tsTemplateId = id;
}

// channelhopping template handling
port_INLINE void channelhoppingTemplateIDStoreFromEB(uint8_t id){
    ieee154e_vars.chTemplateId = id;
}
//======= synchronization
void synchronizecallingPacket(PORT_TIMER_WIDTH timeReceived) 
{
   PORT_SIGNED_INT_WIDTH timeCorrection;
   PORT_TIMER_WIDTH newPeriod;
   PORT_TIMER_WIDTH currentPeriod;
   PORT_TIMER_WIDTH currentValue;
   
   // record the current timer value and period
   currentValue                   =  opentimers_getValue()-ieee154e_vars.startOfSlotReference;
   currentPeriod                  =  ieee154e_vars.slotDuration;
   
   // calculate new period
   timeCorrection                 =  (PORT_SIGNED_INT_WIDTH)((PORT_SIGNED_INT_WIDTH)timeReceived - (PORT_SIGNED_INT_WIDTH)(377));//+131+25+30+30+CallingTxoffset_to_rie2+Callingdelay
	////timeCorrection+=100;
   // The interrupt beginning a new slot can either occur after the packet has been
   // or while it is being received, possibly because the mote is not yet synchronized.
   // In the former case we simply take the usual slotLength and correct it.
   // In the latter case the timer did already roll over and
   // currentValue < timeReceived. slotLength did then already pass which is why
   // we need the new slot to end after the remaining time which is timeCorrection
   // and in this constellation is guaranteed to be positive.
   if (currentValue < timeReceived) 
   {
       newPeriod = (PORT_TIMER_WIDTH)timeCorrection;//next slot zyx
       
   } else 
   {
       newPeriod =  (PORT_TIMER_WIDTH)((PORT_SIGNED_INT_WIDTH)currentPeriod + timeCorrection);//next slot plus one slot length zyx
		//incrementAsnOffset();//zyxadded20190124
   }
   // detect whether I'm too close to the edge of the slot, in that case,
   // skip a slot and increase the temporary slot length to be 2 slots long								//zyx changed 5or 10 to 100
   if ((PORT_SIGNED_INT_WIDTH)((PORT_SIGNED_INT_WIDTH)newPeriod - (PORT_SIGNED_INT_WIDTH)currentValue) < (PORT_SIGNED_INT_WIDTH)100) {
      newPeriod                  +=  TsSlotDuration;
      incrementAsnOffset();//I do not think mingbai it 
      
   }
   // resynchronize by applying the new period
    opentimers_scheduleAbsolute(
        ieee154e_vars.timerId,                            // timerId
        newPeriod,                                        // duration
        ieee154e_vars.startOfSlotReference,               // reference
        TIME_TICS,                                        // timetype
        isr_ieee154e_newSlot                              // callback
    );//20180111
    ieee154e_vars.slotDuration = newPeriod;
    // radiotimer_setPeriod(newPeriod);
#ifdef ADAPTIVE_SYNC
   // indicate time correction to adaptive sync module
   adaptive_sync_indicateTimeCorrection(timeCorrection,ieee154e_vars.dataReceived->l2_nextORpreviousHop);
#endif
   // reset the de-synchronization timeout
   ieee154e_vars.deSyncTimeout    = DESYNCTIMEOUT;
   //zyx20190216
/*
   // log a large timeCorrection
   if (
         ieee154e_vars.isSync==TRUE
      ) {
	   if((timeCorrection>1)||(timeCorrection<-1))
	   {
		   //debugpins_frame_toggle();  //lcg 20180529
	   }
	   
      openserial_printError(COMPONENT_IEEE802154E,ERR_LARGE_TIMECORRECTION,
                            (errorparameter_t)timeCorrection,
                            (errorparameter_t)0);
   }
*/
   // update the stats
   ieee154e_stats.numSyncPkt++;
   updateStats(timeCorrection);
   
#ifdef OPENSIM
   debugpins_syncPacket_set();
   debugpins_syncPacket_clr();
#endif
}

void synchronizePacket(PORT_TIMER_WIDTH timeReceived) {
   PORT_SIGNED_INT_WIDTH timeCorrection;
   PORT_TIMER_WIDTH newPeriod;
   PORT_TIMER_WIDTH currentPeriod;
   PORT_TIMER_WIDTH currentValue;
   
   // record the current timer value and period
   currentValue                   =  opentimers_getValue()-ieee154e_vars.startOfSlotReference;
   currentPeriod                  =  ieee154e_vars.slotDuration;
   
   // calculate new period
   timeCorrection                 =  (PORT_SIGNED_INT_WIDTH)((PORT_SIGNED_INT_WIDTH)timeReceived - (PORT_SIGNED_INT_WIDTH)TsTxOffset);

   // The interrupt beginning a new slot can either occur after the packet has been
   // or while it is being received, possibly because the mote is not yet synchronized.
   // In the former case we simply take the usual slotLength and correct it.
   // In the latter case the timer did already roll over and
   // currentValue < timeReceived. slotLength did then already pass which is why
   // we need the new slot to end after the remaining time which is timeCorrection
   // and in this constellation is guaranteed to be positive.
   if (currentValue < timeReceived) {
       newPeriod = (PORT_TIMER_WIDTH)timeCorrection;
   } else {
       newPeriod =  (PORT_TIMER_WIDTH)((PORT_SIGNED_INT_WIDTH)currentPeriod + timeCorrection);
   }
   
   // detect whether I'm too close to the edge of the slot, in that case,
   // skip a slot and increase the temporary slot length to be 2 slots long
   if ((PORT_SIGNED_INT_WIDTH)((PORT_SIGNED_INT_WIDTH)newPeriod - (PORT_SIGNED_INT_WIDTH)currentValue) < (PORT_SIGNED_INT_WIDTH)RESYNCHRONIZATIONGUARD) {
      newPeriod                  +=  TsSlotDuration;
      incrementAsnOffset();
   }
   
   // resynchronize by applying the new period
    opentimers_scheduleAbsolute(
        ieee154e_vars.timerId,                            // timerId
        newPeriod,                                        // duration
        ieee154e_vars.startOfSlotReference,               // reference
        TIME_TICS,                                        // timetype
        isr_ieee154e_newSlot                              // callback
    );//20180111
    ieee154e_vars.slotDuration = newPeriod;
    // radiotimer_setPeriod(newPeriod);
#ifdef ADAPTIVE_SYNC
   // indicate time correction to adaptive sync module
   adaptive_sync_indicateTimeCorrection(timeCorrection,ieee154e_vars.dataReceived->l2_nextORpreviousHop);
#endif
   // reset the de-synchronization timeout
   ieee154e_vars.deSyncTimeout    = DESYNCTIMEOUT;
//zyx20190216
/*
   // log a large timeCorrection
   if (
         ieee154e_vars.isSync==TRUE
      ) {
	   if((timeCorrection>1)||(timeCorrection<-1))
	   {
		   //debugpins_frame_toggle();  //lcg 20180529
	   }
		printc("hello\n");
      openserial_printError(COMPONENT_IEEE802154E,ERR_LARGE_TIMECORRECTION,
                            (errorparameter_t)timeCorrection,
                            (errorparameter_t)0);
   }
*/
   // update the stats
   ieee154e_stats.numSyncPkt++;
   updateStats(timeCorrection);
   
#ifdef OPENSIM
   debugpins_syncPacket_set();
   debugpins_syncPacket_clr();
#endif
}

void synchronizeAck(PORT_SIGNED_INT_WIDTH timeCorrection) {
   PORT_TIMER_WIDTH newPeriod;
   PORT_TIMER_WIDTH currentPeriod;
   
   // calculate new period
   currentPeriod                  =  ieee154e_vars.slotDuration;
   newPeriod                      =  (PORT_TIMER_WIDTH)((PORT_SIGNED_INT_WIDTH)currentPeriod+timeCorrection);

   // resynchronize by applying the new period
    opentimers_scheduleAbsolute(
        ieee154e_vars.timerId,                            // timerId
        newPeriod,                                        // duration
        ieee154e_vars.startOfSlotReference,               // reference
        TIME_TICS,                                        // timetype
        isr_ieee154e_newSlot                              // callback
    );
    ieee154e_vars.slotDuration = newPeriod;
    // radiotimer_setPeriod(newPeriod);
   
   // reset the de-synchronization timeout
   ieee154e_vars.deSyncTimeout    = DESYNCTIMEOUT;
#ifdef ADAPTIVE_SYNC
   // indicate time correction to adaptive sync module
   adaptive_sync_indicateTimeCorrection((-timeCorrection),ieee154e_vars.ackReceived->l2_nextORpreviousHop);
#endif
//zyx20190216
/*
   // log a large timeCorrection
   if (
         ieee154e_vars.isSync==TRUE
      ) {
      printc("hello\n");
      openserial_printError(COMPONENT_IEEE802154E,ERR_LARGE_TIMECORRECTION,
                            (errorparameter_t)timeCorrection,
                            (errorparameter_t)1);
   }
*/
   // update the stats
   ieee154e_stats.numSyncAck++;
   updateStats(timeCorrection);
   
#ifdef OPENSIM
   debugpins_syncAck_set();
   debugpins_syncAck_clr();
#endif
}

void changeIsSync(bool newIsSync) {
   ieee154e_vars.isSync = newIsSync;
   
   if (ieee154e_vars.isSync==TRUE) {
      leds_sync_on();
      resetStats();
   } else {
      leds_sync_off();
      schedule_resetBackoff();
   }
}

//======= notifying upper layer

void notif_sendDone(OpenQueueEntry_t* packetSent, owerror_t error) {
   // record the outcome of the trasmission attempt
   packetSent->l2_sendDoneError   = error;
   // record the current ASN
   memcpy(&packetSent->l2_asn,&ieee154e_vars.asn,sizeof(asn_t));
   // associate this packet with the virtual component
   // COMPONENT_IEEE802154E_TO_RES so RES can knows it's for it
   packetSent->owner              = COMPONENT_IEEE802154E_TO_SIXTOP;
   // post RES's sendDone task
   scheduler_push_task(task_sixtopNotifSendDone,TASKPRIO_SIXTOP_NOTIF_TXDONE);
   // wake up the scheduler
   SCHEDULER_WAKEUP();
}

void notif_receive(OpenQueueEntry_t* packetReceived) {
   // record the current ASN
   memcpy(&packetReceived->l2_asn, &ieee154e_vars.asn, sizeof(asn_t));
   // indicate reception to the schedule, to keep statistics
   schedule_indicateRx(&packetReceived->l2_asn);
   
   // associate this packet with the virtual component
   // COMPONENT_IEEE802154E_TO_SIXTOP so sixtop can knows it's for it
   packetReceived->owner          = COMPONENT_IEEE802154E_TO_SIXTOP;
   // post RES's Receive task
   scheduler_push_task(task_sixtopNotifReceive,TASKPRIO_SIXTOP_NOTIF_RX);
   // wake up the scheduler
   SCHEDULER_WAKEUP();
}

//======= stats

port_INLINE void resetStats() {
   ieee154e_stats.numSyncPkt      =    0;
   ieee154e_stats.numSyncAck      =    0;
   ieee154e_stats.minCorrection   =  127;
   ieee154e_stats.maxCorrection   = -127;
   ieee154e_stats.numTicsOn       =    0;
   ieee154e_stats.numTicsTotal    =    0;
   // do not reset the number of de-synchronizations
}

void updateStats(PORT_SIGNED_INT_WIDTH timeCorrection) {
   // update minCorrection
   if (timeCorrection<ieee154e_stats.minCorrection) {
     ieee154e_stats.minCorrection = timeCorrection;
   }
   // update maxConnection
   if(timeCorrection>ieee154e_stats.maxCorrection) {
     ieee154e_stats.maxCorrection = timeCorrection;
   }
}

//======= misc

/**
\brief Calculates the frequency channel to transmit on, based on the 
absolute slot number and the channel offset of the requested slot.

During normal operation, the frequency used is a function of the 
channelOffset indicating in the schedule, and of the ASN of the
slot. This ensures channel hopping, consecutive packets sent in the same slot
in the schedule are done on a difference frequency channel.

During development, you can force single channel operation by having this
function return a constant channel number (between 11 and 26). This allows you
to use a single-channel sniffer; but you can not schedule two links on two
different channel offsets in the same slot.

\param[in] channelOffset channel offset for the current slot

\returns The calculated frequency channel, an integer between 11 and 26.
*/
#if 0
port_INLINE uint8_t calculateFrequency(uint8_t channelOffset) {
    if (ieee154e_vars.singleChannel >= 11 && ieee154e_vars.singleChannel <= 26 ) {
        return ieee154e_vars.singleChannel; // single channel
    } else {
        // channel hopping enabled, use the channel depending on hopping template
        return 11 + ieee154e_vars.chTemplate[(ieee154e_vars.asnOffset+channelOffset)%NUM_CHANNELS];
    }
    //return 11+(ieee154e_vars.asnOffset+channelOffset)%16; //channel hopping
}
#endif
port_INLINE uint8_t calculateFrequency(uint8_t channelOffset) 
{
#if 0
    if (ieee154e_vars.singleChannel >= 11 && ieee154e_vars.singleChannel <= 26 ) {
        return ieee154e_vars.singleChannel; // single channel
    } else {
        // channel hopping enabled, use the channel depending on hopping template
        return 11 + ieee154e_vars.chTemplate[(ieee154e_vars.asnOffset+channelOffset)%NUM_CHANNELS];
    }
    //return 11+(ieee154e_vars.asnOffset+channelOffset)%16; //channel hopping
#endif
	return ieee154e_vars.chTemplate[(ieee154e_vars.asnOffset+channelOffset)%NUM_CHANNELS];

}

/**
\brief Changes the state of the IEEE802.15.4e FSM.

Besides simply updating the state global variable,
this function toggles the FSM debug pin.

\param[in] newstate The state the IEEE802.15.4e FSM is now in.
*/
void changeState(ieee154e_state_t newstate) {
   // update the state
   ieee154e_vars.state = newstate;
   // wiggle the FSM debug pin
   switch (ieee154e_vars.state) {
      case S_SYNCLISTEN:
      case S_TXDATAOFFSET:
         //debugpins_fsm_set();  //lcg 20180516 del
         break;
      case S_SLEEP:
      case S_RXDATAOFFSET:
         //debugpins_fsm_clr(); //lcg 20180516 del
         break;
      case S_SYNCRX:
      case S_SYNCPROC:
      case S_TXDATAPREPARE:
      case S_TXDATAREADY:
      case S_TXDATADELAY:
      case S_TXDATA:
      case S_RXACKOFFSET:
      case S_RXACKPREPARE:
      case S_RXACKREADY:
      case S_RXACKLISTEN:
      case S_RXACK:
      case S_TXPROC:
      case S_RXDATAPREPARE:
      case S_RXDATAREADY:
      case S_RXDATALISTEN:
      case S_RXDATA:
      case S_TXACKOFFSET:
      case S_TXACKPREPARE:
      case S_TXACKREADY:
      case S_TXACKDELAY:
      case S_TXACK:
      case S_RXPROC:
         //debugpins_fsm_toggle();  //lcg 20180516 del
         break;
   }
}

/**
\brief Housekeeping tasks to do at the end of each slot.

This functions is called once in each slot, when there is nothing more
to do. This might be when an error occured, or when everything went well.
This function resets the state of the FSM so it is ready for the next slot.

Note that by the time this function is called, any received packet should already
have been sent to the upper layer. Similarly, in a Tx slot, the sendDone
function should already have been done. If this is not the case, this function
will do that for you, but assume that something went wrong.
*/
void endSlot() {
#if 0
	uint8_t slotasn;
	slotasn=ieee154e_vars.asn.bytes0and1;
	Debug_UartOUTPUT_DMA(&slotasn,1);
#endif
   // turn off the radio
   radio_rfOff();
   if(!HAL_GPIO_ReadPin(GPIO_RFEXTI3,GPIO_PIN_RFEXTI3))//added zyx 20181218
   {
		radio_reinit();
		if(*(__IO uint8_t *)(ADDR_PARAMENT_STARTADDRESS+OFFSETADDR_LOWERPOWER_CALLINGON)==1)
		{
			radio_ENABLE_PREAMBLE_INTERRUPT();
		}
   }
   // compute the duty cycle if radio has been turned on
   
   if (ieee154e_vars.radioOnThisSlot==TRUE){  
      ieee154e_vars.radioOnTics+=(sctimer_readCounter()-ieee154e_vars.radioOnInit);
   }
#ifdef SLOT_FSM_IMPLEMENTATION_MULTIPLE_TIMER_INTERRUPT
   radiotimer_cancel(ACTION_ALL_RADIOTIMER_INTERRUPT);
#else
   // clear any pending timer
    opentimers_scheduleAbsolute(
        ieee154e_vars.timerId,                            // timerId
        ieee154e_vars.slotDuration,                       // duration
        ieee154e_vars.startOfSlotReference,               // reference
        TIME_TICS,                                        // timetype
        isr_ieee154e_newSlot                              // callback
    );
    // radiotimer_cancel();
#endif
   // reset capturedTimes
   ieee154e_vars.lastCapturedTime = 0;
   ieee154e_vars.syncCapturedTime = 0;
   
   //computing duty cycle.
   ieee154e_stats.numTicsOn+=ieee154e_vars.radioOnTics;//accumulate and tics the radio is on for that window
   ieee154e_stats.numTicsTotal+=ieee154e_vars.slotDuration;//increment total tics by timer period.

   if (ieee154e_stats.numTicsTotal>DUTY_CYCLE_WINDOW_LIMIT){
      ieee154e_stats.numTicsTotal = ieee154e_stats.numTicsTotal>>1;
      ieee154e_stats.numTicsOn    = ieee154e_stats.numTicsOn>>1;
   }

   //clear vars for duty cycle on this slot   
   ieee154e_vars.radioOnTics=0;
   ieee154e_vars.radioOnThisSlot=FALSE;
   
   // clean up dataToSend
   if (ieee154e_vars.dataToSend!=NULL) {
      // if everything went well, dataToSend was set to NULL in ti9
      // getting here means transmit failed
      
      // indicate Tx fail to schedule to update stats
      schedule_indicateTx(&ieee154e_vars.asn,FALSE);
      
      //decrement transmits left counter
      ieee154e_vars.dataToSend->l2_retriesLeft--;
      
      if (ieee154e_vars.dataToSend->l2_retriesLeft==0) 
      {
         // indicate tx fail if no more retries left
         notif_sendDone(ieee154e_vars.dataToSend,E_FAIL);
      } else 
      {
         // return packet to the virtual COMPONENT_SIXTOP_TO_IEEE802154E component
         ieee154e_vars.dataToSend->owner = COMPONENT_SIXTOP_TO_IEEE802154E;
      }
      
      // reset local variable
      ieee154e_vars.dataToSend = NULL;
   }
   
   // clean up dataReceived
   if (ieee154e_vars.dataReceived!=NULL) {
      // assume something went wrong. If everything went well, dataReceived
      // would have been set to NULL in ri9.
      // indicate  "received packet" to upper layer since we don't want to loose packets
      notif_receive(ieee154e_vars.dataReceived);
      // reset local variable
      ieee154e_vars.dataReceived = NULL;
   }
   
   // clean up ackToSend
   if (ieee154e_vars.ackToSend!=NULL) {
      // free ackToSend so corresponding RAM memory can be recycled
      openqueue_freePacketBuffer(ieee154e_vars.ackToSend);
      // reset local variable
      ieee154e_vars.ackToSend = NULL;
   }
   
   // clean up ackReceived
   if (ieee154e_vars.ackReceived!=NULL) {
      // free ackReceived so corresponding RAM memory can be recycled
      openqueue_freePacketBuffer(ieee154e_vars.ackReceived);
      // reset local variable
      ieee154e_vars.ackReceived = NULL;
   }
   
   
   // change state
   changeState(S_SLEEP);
}

bool ieee154e_isSynch(){
   return ieee154e_vars.isSync;
}
void ieee154e_changemode(ieee154e_power_mode_t newmode)
{
	if(newmode==LOWERPOWER_RUN_MORE_TIME)
	{
		ieee154e_vars.lowpowerrunmoreslots=(SLOTFRAME_LENGTH-1)*(SLEEP_SUPERFRAME_LENGTH-1);
	}
	ieee154e_vars.Mode=newmode;
}
bool ieee154e_islowpower(void)
{
	if(ieee154e_vars.Mode==LOWERPOWER_RUN||ieee154e_vars.Mode==LOWERPOWER_RUN_MORE_TIME)
	{
		return TRUE;
	}else
	{
		return FALSE;
	}
}

