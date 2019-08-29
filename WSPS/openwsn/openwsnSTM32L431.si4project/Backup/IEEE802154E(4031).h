#ifndef __IEEE802154E_H
#define __IEEE802154E_H

/**
\addtogroup MAClow
\{
\addtogroup IEEE802154E
\{
*/

#include "opendefs.h"
#include "board.h"
#include "opentimers.h"
#include "schedule.h"

//=========================== debug define ====================================

//=========================== static ==========================================
static const uint8_t chTemplate_default[] = {
   5,6,12,7,15,4,14,11,8,0,1,2,13,3,9,10
};
static const uint8_t chTemplate_zyx[16][5]=
{
	{0,0,0,0,0},//0
	{2,1,0,3,4},//1
	{0,4,2,3,1},//2
	{4,1,0,2,3},//3
	{0,3,2,4,1},//4
	{0,4,2,1,3},//5
	{1,2,4,0,3},//6
	{3,0,4,1,2},//7
	{0,2,3,1,4},//8
	{2,3,4,0,1},//9
	{3,4,0,1,2},//A
	{1,1,1,1,1},//B
	{2,2,2,2,2},//C
	{3,3,3,3,3},//D
	{4,4,4,4,4},//E
};
// refer to RFC8180: https://tools.ietf.org/html/rfc8180#appendix-A.1
// ASN and join Metric are replaced later when sending an EB
static const uint8_t ebIEsBytestream[] = {
    0x1A,0x88,0x06,0x1A,0x00,0x00,            0x00,0x00,                                0x00,0x00,0x01,0x1C,0x00,0x01,
    0xC8,0x00,0x0A,0x1B,0x01,0x00,SLOTFRAME_LENGTH,0x00,SCHEDULE_MINIMAL_6TISCH_ACTIVE_CELLS,0x00,0x00,0x00,0x00,0x0F
};
//=========================== define ==========================================
#define WAKEDURATION_S    2
#define LOWPOWER_LISTENING_Preamble_Times_ONESLOT 3//每次起来听三次
#define Calling_Preamble_LEN 		628/4//628/4//564/4	//141字节//uints bytes zyx20180823 5ms前导码对应157字节，设置的长度为157*4=628
#define	Communicate_Preamble_LEN	64/4	//uints bytes  zyx20180823


#define EB_ASN0_OFFSET               4
#define EB_JP_OFFSET                 9
#define EB_SLOTFRAME_TS_ID_OFFSET   12
#define EB_SLOTFRAME_CH_ID_OFFSET   15
#define EB_SLOTFRAME_LEN_OFFSET     20
#define EB_SLOTFRAME_NUMLINK_OFFSET 22

#define EB_IE_LEN                   28

#define NUM_CHANNELS                 7//16 // number of channels to channel hop on
#define TXRETRIES                    0//不重发20180725zyx1 // number of MAC retries before declaring failed  //lcg 20180625 pri:3 ,change 1 for transparent transmission .
#define TX_POWER                    31 // 1=-25dBm, 31=0dBm (max value)
#define RESYNCHRONIZATIONGUARD       10//5 // in 32kHz ticks. min distance to the end of the slot to successfully synchronize
#define US_PER_TICK                 30/LSE_64K // number of us per 32kHz clock tick
#define EB_PORTION                   1//3 // The portion of minimal cells that used for sending EB.
#define MAXKAPERIOD               500 // in slots: @15ms per slot -> ~30 seconds. Max value used by adaptive synchronization.
#define DESYNCTIMEOUT             1000//2333//1000 //2333 in slots: @15ms per slot -> ~35 seconds. A larger DESYNCTIMEOUT is needed if using a larger KATIMEOUT.
#define LIMITLARGETIMECORRECTION     5 // threshold number of ticks to declare a timeCorrection "large"
#define LENGTH_IEEE154_MAX         128 // max length of a valid radio packet  
#define DUTY_CYCLE_WINDOW_LIMIT    (0xFFFFFFFF>>1) // limit of the dutycycle window

//15.4e information elements related
#define IEEE802154E_PAYLOAD_DESC_LEN_SHIFT                 0x04
#define IEEE802154E_PAYLOAD_DESC_GROUP_ID_MLME             (1<<11)
#define IEEE802154E_PAYLOAD_DESC_TYPE_MLME                 (1<<15)
#define IEEE802154E_DESC_TYPE_LONG                         ((uint16_t)(1<<15))
#define IEEE802154E_DESC_TYPE_SHORT                        ((uint16_t)(0<<15))

// GROUP_ID changed to 5 (IETF IE) https://openwsn.atlassian.net/browse/FW-569
#define IANA_IETF_IE_GROUP_ID                              (5<<11)
#define IANA_IETF_IE_TYPE                                  (1<<15)
#define IEEE802154E_DESC_LEN_PAYLOAD_ID_TYPE_MASK          0xF800

#define IEEE802154E_DESC_TYPE_HEADER_IE                    0x0000
#define IEEE802154E_DESC_TYPE_PAYLOAD_IE                   0x8000
//len field on PAYLOAD/HEADER DESC
#define IEEE802154E_DESC_LEN_HEADER_IE_MASK                0x007F
#define IEEE802154E_DESC_LEN_PAYLOAD_IE_MASK               0x07FF

//groupID/elementID field on PAYLOAD/HEADER DESC
#define IEEE802154E_DESC_ELEMENTID_HEADER_IE_MASK          0x7F80
#define IEEE802154E_DESC_GROUPID_PAYLOAD_IE_MASK           0x7800

#define IEEE802154E_DESC_ELEMENTID_HEADER_IE_SHIFT         7
#define IEEE802154E_DESC_GROUPID_PAYLOAD_IE_SHIFT          11

//type field on PAYLOAD/HEADER DESC
#define IEEE802154E_DESC_TYPE_IE_MASK                      0x8000

#define IEEE802154E_DESC_TYPE_IE_SHIFT                     15

//MLME Sub IE LONG page 83
#define IEEE802154E_DESC_LEN_LONG_MLME_IE_MASK             0x07FF
#define IEEE802154E_DESC_SUBID_LONG_MLME_IE_MASK           0x7800

#define IEEE802154E_DESC_SUBID_LONG_MLME_IE_SHIFT          11

//MLME Sub IE SHORT page 82
#define IEEE802154E_DESC_LEN_SHORT_MLME_IE_MASK            0x00FF
#define IEEE802154E_DESC_SUBID_SHORT_MLME_IE_MASK          0x7F00

#define IEEE802154E_DESC_SUBID_SHORT_MLME_IE_SHIFT         8

#define IEEE802154E_MLME_SYNC_IE_SUBID                     0x1A
#define IEEE802154E_MLME_SYNC_IE_SUBID_SHIFT               8
#define IEEE802154E_MLME_SLOTFRAME_LINK_IE_SUBID           0x1B
#define IEEE802154E_MLME_SLOTFRAME_LINK_IE_SUBID_SHIFT     8
#define IEEE802154E_MLME_TIMESLOT_IE_SUBID                 0x1c
#define IEEE802154E_MLME_TIMESLOT_IE_SUBID_SHIFT           8
#define IEEE802154E_MLME_CHANNELHOPPING_IE_SUBID           0x09
#define IEEE802154E_MLME_CHANNELHOPPING_IE_SUBID_SHIFT     11

#define IEEE802154E_MLME_IE_GROUPID                        0x01
#define IEEE802154E_ACK_NACK_TIMECORRECTION_ELEMENTID      0x1E

/**
When a packet is received, it is written inside the OpenQueueEntry_t->packet
buffer, starting at the byte defined below. When a packet is relayed, it
traverses the stack in which the MAC and IPHC headers are parsed and stripped
off, then put on again. During that process, the IPv6 hop limit field is
decremented. Depending on the new value of the hop limit, the IPHC header
compression algorithm might not be able to compress it, and hence has to carry
it inline, adding a byte to the header. To avoid having to move bytes around
inside OpenQueueEntry_t->packet buffer, we start writing the received packet a
bit after the start of the packet.
*/
#define FIRST_FRAME_BYTE             1

// the different states of the IEEE802.15.4e state machine
typedef enum {
   S_SLEEP                   = 0x00,   // ready for next slot
   // synchronizing
   S_SYNCLISTEN              = 0x01,   // listened for packet to synchronize to network
   S_SYNCRX                  = 0x02,   // receiving packet to synchronize to network
   S_SYNCPROC                = 0x03,   // processing packet just received
   // TX
   S_TXDATAOFFSET            = 0x04,   // waiting to prepare for Tx data
   S_TXDATAPREPARE           = 0x05,   // preparing for Tx data
   S_TXDATAREADY             = 0x06,   // ready to Tx data, waiting for 'go'
   S_TXDATADELAY             = 0x07,   // 'go' signal given, waiting for SFD Tx data
   S_TXDATA                  = 0x08,   // Tx data SFD received, sending bytes
   S_RXACKOFFSET             = 0x09,   // Tx data done, waiting to prepare for Rx ACK
   S_RXACKPREPARE            = 0x0a,   // preparing for Rx ACK
   S_RXACKREADY              = 0x0b,   // ready to Rx ACK, waiting for 'go'
   S_RXACKLISTEN             = 0x0c,   // idle listening for ACK
   S_RXACK                   = 0x0d,   // Rx ACK SFD received, receiving bytes
   S_TXPROC                  = 0x0e,   // processing sent data
   // RX
   S_RXDATAOFFSET            = 0x0f,   // waiting to prepare for Rx data
   S_RXDATAPREPARE           = 0x10,   // preparing for Rx data
   S_RXDATAREADY             = 0x11,   // ready to Rx data, waiting for 'go'
   S_RXDATALISTEN            = 0x12,   // idle listening for data
   S_RXDATA                  = 0x13,   // data SFD received, receiving more bytes
   S_TXACKOFFSET             = 0x14,   // waiting to prepare for Tx ACK
   S_TXACKPREPARE            = 0x15,   // preparing for Tx ACK
   S_TXACKREADY              = 0x16,   // Tx ACK ready, waiting for 'go'
   S_TXACKDELAY              = 0x17,   // 'go' signal given, waiting for SFD Tx ACK
   S_TXACK                   = 0x18,   // Tx ACK SFD received, sending bytes
   S_RXPROC                  = 0x19,   // processing received data

   /*	ONLY the master has calling mode												*/
   S_TXCALLINGDATAOFFSET	=0x20,//zyx added 3 mode 20180823
   S_TXCALLINGDATAPREPARE	=0x21,
   S_TXCALLINGDATAREADY		=0x22,
   S_TXCALLINGDATADELAY		=0x23,
   S_TXCALLINGDATA			=0x24,
   S_TXCALLINGDATADONE		=0x25,

   /*slave has this mode only*/
   S_LONGTIMESLEEP			=0x26,
   S_RXPREAMBLELISTEN		=0x27,
   S_RXSYNCLISTEN			=0x28,
   
} ieee154e_state_t;

#define  TIMESLOT_TEMPLATE_ID         0x00
#define  CHANNELHOPPING_TEMPLATE_ID   0x00

// Atomic durations
// expressed in 32kHz ticks:
//    - ticks = duration_in_seconds * 32768
//    - duration_in_seconds = ticks / 32768
enum ieee154e_atomicdurations_enum 
{
   // time-slot related
#ifdef SLOTDURATION_10MS
   TsTxOffset                =   70,                  //  2120us
   TsLongGT                  =   36,                  //  1100us
   TsTxAckDelay              =   33,                  //  1000us
   TsShortGT                 =    9,                  //   500us, The standardlized value for this is 400/2=200us(7ticks). Currectly 7 doesn't work for short packet, change it back to 7 when found the problem.
#else
   TsTxOffset                =  131*LSE_64K,                  //  4000us
   TsLongGT                  =   25*LSE_64K,                  // 43: 1300us   lcg 20180508
   TsTxAckDelay              =  151*LSE_64K,                  //  4606us
   TsShortGT                 =   20*LSE_64K,                  //   500us    lcg 20180523 pri:16 fix bug ERR_WRONG_STATE_IN_NEWSLOT.
#endif
   TsSlotDuration            =  PORT_TsSlotDuration,  // 10000us
   // execution speed related
   maxTxDataPrepare          =  PORT_maxTxDataPrepare,
   maxRxAckPrepare           =  PORT_maxRxAckPrepare,
   maxRxDataPrepare          =  PORT_maxRxDataPrepare,
   maxTxAckPrepare           =  PORT_maxTxAckPrepare,
   // radio speed related
   delayTx                   =  PORT_delayTx,         // between GO signal and SFD
   delayRx                   =  PORT_delayRx,         // between GO signal and start listening
   // radio watchdog
   wdRadioTx                 =   33*LSE_64K,                  //  1000us (needs to be >delayTx)
   wdDataDuration            =  180*LSE_64K,                  //  5000us (measured 4280us with max payload)  lcg 20180426 pri 164
#ifdef SLOTDURATION_10MS
   wdAckDuration             =   80,                  //  2400us (measured 1000us)
#else
   wdAckDuration             =   63*LSE_64K,                  //  3000us (measured 1000us)  // lcg 20180602 pri:98,it is exceed 15ms. now 1.9ms (27+24)=1.632ms.
#endif
	CallingTxoffset_to_rie2	 =	30*LSE_64K,//zyx added 3ms after tie2 20180823,915us after rie
	Callingdelay			 =	0,//30*LSE_64K,//zyx added calling data send after prepare 915us 20180823
	wdCallingDataDuration	 =	328*LSE_64K,//calling data len 10ms
	sleepduration			 =65535,//max 32766 one circle zyx follow the softsetting
	listenpreamableDuration	 =30*LSE_64K,//25*LSE_64K,//25*LSE_64K,//763us
	listenpreamablePeriod	  =PORT_TsSlotDuration/3-listenpreamableDuration,//每个通道呼叫三次
};

//shift of bytes in the linkOption bitmap: draft-ietf-6tisch-minimal-10.txt: page 6
enum ieee154e_linkOption_enum {
   FLAG_TX_S                 = 0,
   FLAG_RX_S                 = 1,
   FLAG_SHARED_S             = 2,
   FLAG_TIMEKEEPING_S        = 3,   
};
typedef enum
{
	HIGHPOWER_CALLINGOFF=0,
	HIGHPOWER_CALLINGON=1,
	LOWERPOWER_CALLINGON=2,
	LOWERPOWER_RUN=3,
	LOWERPOWER_RUN_MORE_TIME=4,
}ieee154e_power_mode_t;
// FSM timer durations (combinations of atomic durations)
// TX
#define DURATION_tt1 ieee154e_vars.lastCapturedTime+TsTxOffset-delayTx-maxTxDataPrepare
#define DURATION_tt2 ieee154e_vars.lastCapturedTime+TsTxOffset-delayTx
#define DURATION_tt3 ieee154e_vars.lastCapturedTime+TsTxOffset-delayTx+wdRadioTx

#define DURATION_tt4 ieee154e_vars.lastCapturedTime+wdDataDuration//+180//zyx
#define DURATION_tt5 ieee154e_vars.lastCapturedTime+TsTxAckDelay-TsShortGT-delayRx-maxRxAckPrepare
#define DURATION_tt6 ieee154e_vars.lastCapturedTime+TsTxAckDelay-TsShortGT-delayRx-3*5//20190322zyx
#define DURATION_tt7 ieee154e_vars.lastCapturedTime+TsTxAckDelay+TsShortGT
#define DURATION_tt8 ieee154e_vars.lastCapturedTime+wdAckDuration

#define DURATION_ct9 ieee154e_vars.lastCapturedTime+TsTxOffset+TsLongGT+CallingTxoffset_to_rie2//zyx added 20180823
#define DURATION_ct10 ieee154e_vars.lastCapturedTime+TsTxOffset+TsLongGT+CallingTxoffset_to_rie2+Callingdelay
#define DURATION_ct11 ieee154e_vars.lastCapturedTime+TsTxOffset+TsLongGT+CallingTxoffset_to_rie2+Callingdelay+wdCallingDataDuration/2
#define DURATION_ct12 ieee154e_vars.lastCapturedTime+wdCallingDataDuration//zyx
// RX
#define DURATION_rt1 ieee154e_vars.lastCapturedTime+TsTxOffset-TsLongGT-delayRx-maxRxDataPrepare
#define DURATION_rt2 ieee154e_vars.lastCapturedTime+TsTxOffset-TsLongGT-delayRx-3*5//500us 20190322zyx
#define DURATION_rt3 ieee154e_vars.lastCapturedTime+TsTxOffset+TsLongGT
#define DURATION_rt4 ieee154e_vars.lastCapturedTime+wdDataDuration
#define DURATION_rt5 ieee154e_vars.lastCapturedTime+TsTxAckDelay-delayTx-maxTxAckPrepare
#define DURATION_rt6 ieee154e_vars.lastCapturedTime+TsTxAckDelay-delayTx
#define DURATION_rt7 ieee154e_vars.lastCapturedTime+TsTxAckDelay-delayTx+wdRadioTx
#define DURATION_rt8 ieee154e_vars.lastCapturedTime+wdAckDuration
#define DURATION_st1 ieee154e_vars.lastCapturedTime+listenpreamableDuration
//#define DURATION_st2 ieee154e_vars.lastCapturedTime+listenpreamablePeriod

//=========================== typedef =========================================

// IEEE802.15.4E acknowledgement (ACK)
typedef struct {
   PORT_SIGNED_INT_WIDTH timeCorrection;
} IEEE802154E_ACK_ht;

//=========================== module variables ================================

typedef struct {
   // misc
   asn_t                     asn;                     // current absolute slot number
   slotOffset_t              slotOffset;              // current slot offset
   slotOffset_t              nextActiveSlotOffset;    // next active slot offset
   PORT_TIMER_WIDTH          deSyncTimeout;           // how many slots left before looses sync
   bool                      isSync;                  // TRUE iff mote is synchronized to network
   OpenQueueEntry_t          localCopyForTransmission;// copy of the frame used for current TX
   OpenQueueEntry_t			 CallingQueue;//zyx added
   PORT_TIMER_WIDTH          numOfSleepSlots;         // number of slots to sleep between active slots
   // as shown on the chronogram
   ieee154e_state_t          state;                   // state of the FSM
   OpenQueueEntry_t*         dataToSend;              // pointer to the data to send
   OpenQueueEntry_t*         dataReceived;            // pointer to the data received
   OpenQueueEntry_t*         ackToSend;               // pointer to the ack to send
   OpenQueueEntry_t*         ackReceived;             // pointer to the ack received
   PORT_TIMER_WIDTH          lastCapturedTime;        // last captured time
   PORT_TIMER_WIDTH          syncCapturedTime;        // captured time used to sync
   // channel hopping
   uint8_t                   freq;                    // frequency of the current slot
   uint8_t					 callingfreq;			  //frequency of calling mode added by zyx20180823
   uint8_t                   asnOffset;               // offset inside the frame
   uint8_t                   singleChannel;           // the single channel used for transmission
   bool                      singleChannelChanged;    // detect id singleChannelChanged
   uint8_t                   chTemplate[NUM_CHANNELS];// storing the template of hopping sequence
   // template ID
   uint8_t                   tsTemplateId;            // timeslot template id
   uint8_t                   chTemplateId;            // channel hopping tempalte id
   
   PORT_TIMER_WIDTH          radioOnInit;             // when within the slot the radio turns on
   PORT_TIMER_WIDTH          radioOnTics;             // how many tics within the slot the radio is on
   bool                      radioOnThisSlot;         // to control if the radio has been turned on in a slot.
   
   //control
   bool                      isAckEnabled;            // whether reply for ack, used for synchronization test
   bool                      isSecurityEnabled;       // whether security is applied
   // time correction
   int16_t                   timeCorrection;          // store the timeCorrection, prepend and retrieve it inside of frame header
   
   uint16_t                  slotDuration;            // duration of slot
   opentimers_id_t           timerId;                 // id of timer used for implementing TSCH slot FSM 
   uint32_t                  startOfSlotReference;    // the time refer to the beginning of slot
   //user added zyx
   ieee154e_power_mode_t	Mode;					  //4 mode Calling mode	zyx20180815 
   PORT_TIMER_WIDTH     	lowpowerrunmoreslots;  //zyx added
} ieee154e_vars_t;

BEGIN_PACK
typedef struct {
   uint8_t                   numSyncPkt;              // how many times synchronized on a non-ACK packet
   uint8_t                   numSyncAck;              // how many times synchronized on an ACK
   int16_t                   minCorrection;           // minimum time correction
   int16_t                   maxCorrection;           // maximum time correction
   uint8_t                   numDeSync;               // number of times a desync happened
   uint32_t                  numTicsOn;               // mac dutyCycle
   uint32_t                  numTicsTotal;            // total tics for which the dutycycle is computed
} ieee154e_stats_t;
END_PACK

typedef struct {
   PORT_TIMER_WIDTH          num_newSlot;
   PORT_TIMER_WIDTH          num_timer;
   PORT_TIMER_WIDTH          num_startOfFrame;
   PORT_TIMER_WIDTH          num_endOfFrame;
} ieee154e_dbg_t;

//=========================== prototypes ======================================

// admin
void               ieee154e_init(void);
// public
PORT_TIMER_WIDTH   ieee154e_asnDiff(asn_t* someASN);
#ifdef DEADLINE_OPTION_ENABLED
int16_t            ieee154e_computeAsnDiff(asn_t* h_asn, asn_t* l_asn);
void               ieee154e_calculateExpTime(uint16_t max_delay, uint8_t* et_asn);
void               ieee154e_orderToASNStructure(uint8_t* in,asn_t* val_asn);
#endif
bool               ieee154e_isSynch(void);
void               ieee154e_getAsn(uint8_t* array);
void               ieee154e_setIsAckEnabled(bool isEnabled);
void               ieee154e_setSingleChannel(uint8_t channel);
void               ieee154e_setIsSecurityEnabled(bool isEnabled);
void               ieee154e_setSlotDuration(uint16_t duration);
uint16_t           ieee154e_getSlotDuration(void);

uint16_t           ieee154e_getTimeCorrection(void);
// events
void               ieee154e_startOfFrame(PORT_TIMER_WIDTH capturedTime);
void               ieee154e_endOfFrame(PORT_TIMER_WIDTH capturedTime);
// misc
bool               debugPrint_asn(void);
bool               debugPrint_isSync(void);
bool               debugPrint_macStats(void);
void ieee154e_changemode(ieee154e_power_mode_t newmode);
bool ieee154e_islowpower(void);
/**
\}
\}
*/

#endif

