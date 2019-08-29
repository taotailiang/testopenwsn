#ifndef __RADIO_H
#define __RADIO_H

/**
\addtogroup BSP
\{
\addtogroup radio
\{

\brief Cross-platform declaration "radio" bsp module.

\author Thomas Watteyne <watteyne@eecs.berkeley.edu>, February 2012.
*/

#include "board.h"

//=========================== define ==========================================

#define LENGTH_CRC 2
#define USE_2_Radio_Interrupt
/**
\brief Current state of the radio.

\note This radio driver is very minimal in that it does not follow a state machine.
      It is up to the MAC layer to ensure that the different radio operations 
      are called in the righr order. The radio keeps a state for debugging purposes only.
*/
typedef enum {
   RADIOSTATE_STOPPED             = 0x00,   ///< Completely stopped.
   RADIOSTATE_RFOFF               = 0x01,   ///< Listening for commands, but RF chain is off.
   RADIOSTATE_SETTING_FREQUENCY   = 0x02,   ///< Configuring the frequency.
   RADIOSTATE_FREQUENCY_SET       = 0x03,   ///< Done configuring the frequency.
   RADIOSTATE_LOADING_PACKET      = 0x04,   ///< Loading packet into the radio's TX buffer.
   RADIOSTATE_PACKET_LOADED       = 0x05,   ///< Packet is fully loaded in the radio's TX buffer.
   RADIOSTATE_ENABLING_TX         = 0x06,   ///< The RF TX chaing is being enabled (includes locking the PLL).
   RADIOSTATE_TX_ENABLED          = 0x07,   ///< Radio ready to transmit.
   RADIOSTATE_TRANSMITTING        = 0x08,   ///< Busy transmitting bytes.
   RADIOSTATE_ENABLING_RX         = 0x09,   ///< The RF RX chain is being enabled (includes locking the PLL).
   RADIOSTATE_LISTENING           = 0x0a,   ///< RF chain is on, listening, but no packet received yet.
   RADIOSTATE_RECEIVING           = 0x0b,   ///< Busy receiving bytes.
   RADIOSTATE_TXRX_DONE           = 0x0c,   ///< Frame has been sent/received completely.
   RADIOSTATE_TURNING_OFF         = 0x0d,   ///< Turning the RF chain off.
} radio_state_t;

typedef enum//zyx add
{
	RFCMD_TX =  ((uint8_t)(0x60)),					 /*!< Start to transmit; valid only from READY */
	RFCMD_RX =  ((uint8_t)(0x61)),					 /*!< Start to receive; valid only from READY */
	RFCMD_READY =  ((uint8_t)(0x62)),				 /*!< Go to READY; valid only from STANDBY or SLEEP or LOCK */
	RFCMD_STANDBY =	((uint8_t)(0x63)),				 /*!< Go to STANDBY; valid only from READY */
	RFCMD_SLEEP = ((uint8_t)(0x64)), 				 /*!< Go to SLEEP; valid only from READY */
	RFCMD_LOCKRX = ((uint8_t)(0x65)),				 /*!< Go to LOCK state by using the RX configuration of the synth; valid only from READY */
	RFCMD_LOCKTX = ((uint8_t)(0x66)),				 /*!< Go to LOCK state by using the TX configuration of the synth; valid only from READY */
	RFCMD_SABORT = ((uint8_t)(0x67)),				 /*!< Force exit form TX or RX states and go to READY state; valid only from TX or RX */
	RFCMD_LDC_RELOAD = ((uint8_t)(0x68)),			 /*!< LDC Mode: Reload the LDC timer with the value stored in the  LDC_PRESCALER / COUNTER	registers; valid from all states  */
	RFCMD_RCO_CALIB =  ((uint8_t)(0x69)),			 /*!< Start (or re-start) the RCO calibration */
	RFCMD_SRES = ((uint8_t)(0x70)),					 /*!< Reset of all digital part, except SPI registers */
	RFCMD_FLUSHRXFIFO = ((uint8_t)(0x71)),			 /*!< Clean the RX FIFO; valid from all states */
	RFCMD_FLUSHTXFIFO = ((uint8_t)(0x72)),			 /*!< Clean the TX FIFO; valid from all states */
	RFCMD_SEQUENCE_UPDATE =	((uint8_t)(0x73)),		 /*!< Autoretransmission: Reload the Packet sequence counter with the value stored in the PROTOCOL[2] register valid from all states */
} S2RFLPCmd;

//=========================== typedef =========================================

typedef void  (*radio_capture_cbt)(PORT_TIMER_WIDTH timestamp);

//=========================== variables =======================================

//=========================== prototypes ======================================

// admin
void                radio_init(void);
void				radio_reinit(void);
void                radio_setStartFrameCb(radio_capture_cbt cb);
void                radio_setEndFrameCb(radio_capture_cbt cb);
// reset
void                radio_reset(void);
// RF admin
void                radio_setFrequency(uint8_t frequency);
void                radio_rfOn(void);
void                radio_rfOff(void);
void				radio_ENABLE_PREAMBLE_INTERRUPT(void);
void				radio_DISABLE_PREAMBLE_INTERRUPT(void);

// TX
void                radio_loadPacket_prepare(uint8_t* packet, uint8_t len);
void                radio_loadPacket(uint8_t* packet, uint16_t len);
void                radio_txEnable(void);
void                radio_txNow(void);
// RX
void                radio_rxPacket_prepare(void);
void                radio_rxEnable(void);
void                radio_rxEnable_scum(void);
void                radio_rxNow(void);
void                radio_getReceivedFrame(uint8_t* bufRead,
                                uint8_t* lenRead,
                                uint8_t  maxBufLen,
                                 int8_t* rssi,
                                uint8_t* lqi,
                                   bool* crc);

// interrupt handlers
kick_scheduler_t    radio_isr(void);
#ifdef USE_2_Radio_Interrupt//If I use a second interrupt to the radio20180428
kick_scheduler_t radio_isr_second(PORT_TIMER_WIDTH capturedTime);
#endif
/**
\}
\}
*/
#define	WRITE_HEADER 0x00
#define READ_HEADER	0x01
#define	COMMAND_HEADER	0x80
#define	LINEAR_FIFO_ADDRESS	0xFF

#endif
