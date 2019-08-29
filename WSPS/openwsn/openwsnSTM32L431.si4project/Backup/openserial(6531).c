/**
\brief Definition of the "openserial" driver.

\author Fabien Chraim <chraim@eecs.berkeley.edu>, March 2012.
\author Thomas Watteyne <thomas.watteyne@inria.fr>, August 2016.
*/
#include <stdarg.h>
#include "opendefs.h"
#include "openserial.h"
#include "IEEE802154E.h"
#include "neighbors.h"
#include "sixtop.h"
#include "icmpv6echo.h"
#include "idmanager.h"
#include "openqueue.h"
#include "openbridge.h"
#include "leds.h"
#include "schedule.h"
#include "uart.h"
#include "opentimers.h"
#include "openhdlc.h"
#include "schedule.h"
#include "icmpv6rpl.h"
#include "icmpv6echo.h"
#include "sf0.h"
#include "zyx.h"
#include "board.h"
#include "scheduler.h"
//=========================== variables =======================================

openserial_vars_t openserial_vars;
extern uint16_t ADC_CNT;
extern ieee154e_vars_t    ieee154e_vars;
extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart2;
extern DMA_HandleTypeDef hdma_usart1_rx;
extern DMA_HandleTypeDef hdma_usart1_tx;
extern	uint8_t Uart_TX_BUF[];
extern	uint8_t Uart_RX_BUF[];
extern	uint16_t Uart_Tx_BUF_LEN;
#ifdef OPEN_DEBUG_USART2
#endif

//=========================== prototypes ======================================

// printing
owerror_t openserial_printInfoErrorCritical(
    char             severity,
    uint8_t          calling_component,
    uint8_t          error_code,
    errorparameter_t arg1,
    errorparameter_t arg2
);

// command handlers
void openserial_handleEcho(uint8_t* but, uint8_t bufLen);
void openserial_get6pInfo(uint8_t commandId, uint8_t* code,uint8_t* cellOptions,uint8_t* numCells,cellInfo_ht* celllist_add,cellInfo_ht* celllist_delete,uint8_t* listOffset,uint8_t* maxListLen,uint8_t ptr, uint8_t commandLen);
void openserial_handleCommands(void);

// misc
void openserial_board_reset_cb(opentimers_id_t id);

// HDLC output
void outputHdlcOpen(void);
void outputHdlcWrite(uint8_t b);
void outputHdlcClose(void);

// HDLC input
void inputHdlcOpen(void);
void inputHdlcWrite(uint8_t b);
void inputHdlcClose(void);

// sniffer
void sniffer_setListeningChannel(uint8_t channel);
//zyx add 
void openserialIOEnableInput(void);
void openserialIODisableInput(void);
void openserialIOMarkStartOutput(void);
void openserialIOMarkStopOutput(void);
void UART_Transmit_TASK(void);

//=========================== public ==========================================

//===== admin

void openserial_init() {
    uint16_t crc;
    
    // reset variable
    memset(&openserial_vars,0,sizeof(openserial_vars_t));
    
    // admin
    openserial_vars.mode               = MODE_OFF;
    openserial_vars.debugPrintCounter  = 0;
    
    // input
    openserial_vars.reqFrame[0]        = HDLC_FLAG;
    openserial_vars.reqFrame[1]        = SERFRAME_MOTE2PC_REQUEST;
    crc = HDLC_CRCINIT;
    crc = crcIteration(crc,openserial_vars.reqFrame[1]);
    crc = ~crc;
    openserial_vars.reqFrame[2]        = (crc>>0)&0xff;
    openserial_vars.reqFrame[3]        = (crc>>8)&0xff;
    openserial_vars.reqFrame[4]        = HDLC_FLAG;
    openserial_vars.reqFrameIdx        = 0;
    openserial_vars.lastRxByte         = HDLC_FLAG;
    openserial_vars.busyReceiving      = FALSE;
    openserial_vars.inputEscaping      = FALSE;
    openserial_vars.inputBufFill       = 0;
    
    // ouput
    openserial_vars.outputBufFilled    = FALSE;
    openserial_vars.outputBufIdxR      = 0;
    openserial_vars.outputBufIdxW      = 0;
    
    // set callbacks
    uart_setCallbacks(
        isr_openserial_tx,
        isr_openserial_rx
    );
}

void openserial_register(openserial_rsvpt* rsvp) {
    // FIXME: register multiple commands (linked list)
    openserial_vars.registeredCmd = rsvp;
}

//===== printing

owerror_t openserial_printStatus(
    uint8_t             statusElement,
    uint8_t*            buffer,
    uint8_t             length
) {
    uint8_t i;
    INTERRUPT_DECLARATION();
    
    DISABLE_INTERRUPTS();
    openserial_vars.outputBufFilled  = TRUE;
    outputHdlcOpen();
    outputHdlcWrite(SERFRAME_MOTE2PC_STATUS);
    outputHdlcWrite(idmanager_getMyID(ADDR_16B)->addr_16b[0]);
    outputHdlcWrite(idmanager_getMyID(ADDR_16B)->addr_16b[1]);
    outputHdlcWrite(statusElement);
    for (i=0;i<length;i++){
        outputHdlcWrite(buffer[i]);
    }
    outputHdlcClose();
    ENABLE_INTERRUPTS();
    
    return E_SUCCESS;
}

owerror_t openserial_printInfo(
    uint8_t             calling_component,
    uint8_t             error_code,
    errorparameter_t    arg1,
    errorparameter_t    arg2
) {
    return openserial_printInfoErrorCritical(
        SERFRAME_MOTE2PC_INFO,
        calling_component,
        error_code,
        arg1,
        arg2
    );
}

owerror_t openserial_printError(
    uint8_t             calling_component,
    uint8_t             error_code,
    errorparameter_t    arg1,
    errorparameter_t    arg2
) {
    // toggle error LED
    leds_error_toggle();
    
    return openserial_printInfoErrorCritical(
        SERFRAME_MOTE2PC_ERROR,
        calling_component,
        error_code,
        arg1,
        arg2
    );
}

owerror_t openserial_printCritical(
    uint8_t             calling_component,
    uint8_t             error_code,
    errorparameter_t    arg1,
    errorparameter_t    arg2
) {
    opentimers_id_t id; 
    uint32_t         reference;
    // blink error LED, this is serious
    //leds_error_blink();  //lcg 20180530
    
    // schedule for the mote to reboot in 10s
    id        = opentimers_create();
    reference = opentimers_getValue();
    opentimers_scheduleAbsolute(
        id,                             // timerId
        10000,                          // duration
        reference,                      // reference
        TIME_MS,                        // timetype
        openserial_board_reset_cb       // callback
    );
    return openserial_printInfoErrorCritical(
        SERFRAME_MOTE2PC_CRITICAL,
        calling_component,
        error_code,
        arg1,
        arg2
    );
}
owerror_t openserial_printData(uint8_t* buffer, uint8_t length) {
    uint8_t  i;
    uint8_t  asn[5];
    INTERRUPT_DECLARATION();
    
    // retrieve ASN
    ieee154e_getAsn(asn);
    
    DISABLE_INTERRUPTS();
    openserial_vars.outputBufFilled  = TRUE;
    outputHdlcOpen();
    outputHdlcWrite(SERFRAME_MOTE2PC_DATA);
    outputHdlcWrite(idmanager_getMyID(ADDR_16B)->addr_16b[1]);
    outputHdlcWrite(idmanager_getMyID(ADDR_16B)->addr_16b[0]);
    outputHdlcWrite(asn[0]);
    outputHdlcWrite(asn[1]);
    outputHdlcWrite(asn[2]);
    outputHdlcWrite(asn[3]);
    outputHdlcWrite(asn[4]);
    for (i=0;i<length;i++){
        outputHdlcWrite(buffer[i]);
    }
    outputHdlcClose();
    ENABLE_INTERRUPTS();
    
    return E_SUCCESS;
}

owerror_t openserial_printSniffedPacket(uint8_t* buffer, uint8_t length, uint8_t channel) {
    uint8_t  i;
    INTERRUPT_DECLARATION();
    
    DISABLE_INTERRUPTS();
    openserial_vars.outputBufFilled  = TRUE;
    outputHdlcOpen();
    outputHdlcWrite(SERFRAME_MOTE2PC_SNIFFED_PACKET);
    outputHdlcWrite(idmanager_getMyID(ADDR_16B)->addr_16b[1]);
    outputHdlcWrite(idmanager_getMyID(ADDR_16B)->addr_16b[0]);
    for (i=0;i<length;i++){
       outputHdlcWrite(buffer[i]);
    }
    outputHdlcWrite(channel);
    outputHdlcClose();
    
    ENABLE_INTERRUPTS();
    
    return E_SUCCESS;
}

//===== retrieving inputBuffer

uint8_t openserial_getInputBufferFilllevel() {
    uint8_t inputBufFill;
    INTERRUPT_DECLARATION();
    
    DISABLE_INTERRUPTS();
    inputBufFill = openserial_vars.inputBufFill;
    ENABLE_INTERRUPTS();
    
    return inputBufFill-1; // removing the command byte
}

uint8_t openserial_getInputBuffer(uint8_t* bufferToWrite, uint8_t maxNumBytes) {
    uint8_t numBytesWritten;
    uint8_t inputBufFill;
    INTERRUPT_DECLARATION();
    
    DISABLE_INTERRUPTS();
    inputBufFill = openserial_vars.inputBufFill;
    ENABLE_INTERRUPTS();
     
    if (maxNumBytes<inputBufFill-1) {
        openserial_printError(
            COMPONENT_OPENSERIAL,
            ERR_GETDATA_ASKS_TOO_FEW_BYTES,
            (errorparameter_t)maxNumBytes,
            (errorparameter_t)inputBufFill-1
        );
        numBytesWritten = 0;
    } else {
        numBytesWritten = inputBufFill-1;
        memcpy(bufferToWrite,&(openserial_vars.inputBuf[1]),numBytesWritten);
    }
    
    return numBytesWritten;
}

//===== scheduling

void openserial_startInput() {
	uint64_t a;
	uint16_t index;
	uint16_t i;
	uint16_t delay;
	
	openserialIOEnableInput();
	INTERRUPT_DECLARATION();
	//printc("hello\n");
	#if 0
	printc("hello\n");
	
	openserial_printInfo(
	        COMPONENT_OPENWSN,0x5d,
	        (errorparameter_t)0,
	        (errorparameter_t)0
	  );  //lcg 20180503
	#endif
	/////DISABLE_INTERRUPTS();//zyx20180822
	//////ADC_CNT&=0x3F;
	//ADC_CNT=2;
	if(ADC_CNT==2)
	{
		if(Uart_Tx_BUF_LEN>1023-20)//if I have too many words to send
		{
			leds_error_toggle();
			printc("ADCLOST");
		}else
		{
			
			a=ieee154e_vars.asn.bytes0and1|(ieee154e_vars.asn.bytes2and3<<16)|(ieee154e_vars.asn.byte4<<32);
			a=ieee154e_vars.asn.byte4;
			a=(a<<16)|ieee154e_vars.asn.bytes2and3;
			a=(a<<16)|ieee154e_vars.asn.bytes0and1;
			a=a*200;
			ADC_CNT=(ADC_CNT+1)&0x3F;
			#if 0//0 means put ADC count header ,1 means put ADC count after DATA 20190313 zyx
		
			Uart_TX_BUF[(Uart_Tx_BUF_LEN++)&0x3FF]=UART_COMMUNICATION_HEAD_0x71;
			Uart_TX_BUF[(Uart_Tx_BUF_LEN++)&0x3FF]=0x0c;
			Uart_TX_BUF[(Uart_Tx_BUF_LEN++)&0x3FF]=0x0c;
			Uart_TX_BUF[(Uart_Tx_BUF_LEN++)&0x3FF]=UART_COMMUNICATION_HEAD_0x71;
			index=Uart_Tx_BUF_LEN;
			Uart_TX_BUF[(Uart_Tx_BUF_LEN++)&0x3FF]=0x00;
			Uart_TX_BUF[(Uart_Tx_BUF_LEN++)&0x3FF]=0x00;
			Uart_TX_BUF[(Uart_Tx_BUF_LEN++)&0x3FF]=0x00;
			Uart_TX_BUF[(Uart_Tx_BUF_LEN++)&0x3FF]=0x8e;
			Uart_TX_BUF[(Uart_Tx_BUF_LEN++)&0x3FF]=0x00;
			Uart_TX_BUF[(Uart_Tx_BUF_LEN++)&0x3FF]=0x03;
			Uart_TX_BUF[(Uart_Tx_BUF_LEN++)&0x3FF]=a;
			Uart_TX_BUF[(Uart_Tx_BUF_LEN++)&0x3FF]=a>>8;
			Uart_TX_BUF[(Uart_Tx_BUF_LEN++)&0x3FF]=a>>16;
			Uart_TX_BUF[(Uart_Tx_BUF_LEN++)&0x3FF]=a>>24;
			Uart_TX_BUF[(Uart_Tx_BUF_LEN++)&0x3FF]=a>>32;
			Uart_TX_BUF[(Uart_Tx_BUF_LEN++)&0x3FF]=a>>40;
			Uart_TX_BUF[(Uart_Tx_BUF_LEN++)&0x3FF]=ChkSum(&Uart_TX_BUF[index],12);
			Uart_TX_BUF[(Uart_Tx_BUF_LEN++)&0x3FF]=0x16;
			#else
			
			if(Uart_Tx_BUF_LEN==0)
			{
				Uart_TX_BUF[(Uart_Tx_BUF_LEN++)&0x3FF]=UART_COMMUNICATION_HEAD_0x71;
				Uart_TX_BUF[(Uart_Tx_BUF_LEN++)&0x3FF]=0x0c;
				Uart_TX_BUF[(Uart_Tx_BUF_LEN++)&0x3FF]=0x0c;
				Uart_TX_BUF[(Uart_Tx_BUF_LEN++)&0x3FF]=UART_COMMUNICATION_HEAD_0x71;
				index=Uart_Tx_BUF_LEN;
				Uart_TX_BUF[(Uart_Tx_BUF_LEN++)&0x3FF]=0x00;
				Uart_TX_BUF[(Uart_Tx_BUF_LEN++)&0x3FF]=0x00;
				Uart_TX_BUF[(Uart_Tx_BUF_LEN++)&0x3FF]=0x00;
				Uart_TX_BUF[(Uart_Tx_BUF_LEN++)&0x3FF]=0x8e;
				Uart_TX_BUF[(Uart_Tx_BUF_LEN++)&0x3FF]=0x00;
				Uart_TX_BUF[(Uart_Tx_BUF_LEN++)&0x3FF]=0x03;
				Uart_TX_BUF[(Uart_Tx_BUF_LEN++)&0x3FF]=a;
				Uart_TX_BUF[(Uart_Tx_BUF_LEN++)&0x3FF]=a>>8;
				Uart_TX_BUF[(Uart_Tx_BUF_LEN++)&0x3FF]=a>>16;
				Uart_TX_BUF[(Uart_Tx_BUF_LEN++)&0x3FF]=a>>24;
				Uart_TX_BUF[(Uart_Tx_BUF_LEN++)&0x3FF]=a>>32;
				Uart_TX_BUF[(Uart_Tx_BUF_LEN++)&0x3FF]=a>>40;
				Uart_TX_BUF[(Uart_Tx_BUF_LEN++)&0x3FF]=ChkSum(&Uart_TX_BUF[index],12);
				Uart_TX_BUF[(Uart_Tx_BUF_LEN++)&0x3FF]=0x16;
				//
			}
			else
			{
				//
				for(i=Uart_Tx_BUF_LEN;i>0;i--)
				{
					Uart_TX_BUF[i+18-1]=Uart_TX_BUF[i-1];
				}
				Uart_Tx_BUF_LEN+=18;
				Uart_TX_BUF[0]=UART_COMMUNICATION_HEAD_0x71;
				Uart_TX_BUF[1]=0x0c;
				Uart_TX_BUF[2]=0x0c;
				Uart_TX_BUF[3]=UART_COMMUNICATION_HEAD_0x71;
				index=4;//Uart_Tx_BUF_LEN;
				Uart_TX_BUF[4]=0x00;
				Uart_TX_BUF[5]=0x00;
				Uart_TX_BUF[6]=0x00;
				Uart_TX_BUF[7]=0x8e;
				Uart_TX_BUF[8]=0x00;
				Uart_TX_BUF[9]=0x03;
				Uart_TX_BUF[10]=a;
				Uart_TX_BUF[11]=a>>8;
				Uart_TX_BUF[12]=a>>16;
				Uart_TX_BUF[13]=a>>24;
				Uart_TX_BUF[14]=a>>32;
				Uart_TX_BUF[15]=a>>40;
				Uart_TX_BUF[16]=ChkSum(&Uart_TX_BUF[index],12);
				Uart_TX_BUF[17]=0x16;
			
			}
			#endif
		}
		
		
		/*TXBUF[15]=a>>40;
		TXBUF[14]=a>>32;
		TXBUF[13]=a>>24;
		TXBUF[12]=a>>16;
		TXBUF[11]=a>>8;
		TXBUF[10]=a;
		TXBUF[17]=0x16;
		TXBUF[16]=ChkSum(&TXBUF[4],12); */
		//HAL_UART_Transmit_DMA(&huart1,Uart_TX_BUF,18);
		//huart1.gState=HAL_UART_STATE_READY;
	}
	else 
	{
		
	}
	//Uart_Tx_BUF_LEN=300;
	//Uart_Tx_BUF_LEN=10;
	if(Uart_Tx_BUF_LEN)//
   	{
   		openserialIOMarkStartOutput();
	   	scheduler_push_task(UART_Transmit_TASK,TASKPRIO_MAX);
   	}else
   	{
      	
		
   	}
   	openserial_vars.outputBuf[(openserial_vars.outputBufIdxW++)&OUTPUT_BUFFER_MASK]=0x7E;
   	openserial_vars.outputBuf[(openserial_vars.outputBufIdxW++)&OUTPUT_BUFFER_MASK]=0x52;
	openserial_vars.outputBuf[(openserial_vars.outputBufIdxW++)&OUTPUT_BUFFER_MASK]=0xEF;
	openserial_vars.outputBuf[(openserial_vars.outputBufIdxW++)&OUTPUT_BUFFER_MASK]=0x81;
	openserial_vars.outputBuf[(openserial_vars.outputBufIdxW++)&OUTPUT_BUFFER_MASK]=0x7E;
	openserial_startOutput();
	#ifdef OPEN_DEBUG_USART2
   	if(openserial_vars.outputBufIdxW!=openserial_vars.outputBufIdxR)
   	{
   		HAL_UART_Transmit_DMA(&huart2,&openserial_vars.outputBuf[0],openserial_vars.outputBufIdxW-openserial_vars.outputBufIdxR);
   		openserial_vars.outputBufIdxR=0;
   		openserial_vars.outputBufIdxW=0;
   	}
   	#endif
#if 0//zyx shield 20190122
    if (openserial_vars.inputBufFill>0) {
        openserial_printError(
            COMPONENT_OPENSERIAL,
            ERR_INPUTBUFFER_LENGTH,
            (errorparameter_t)openserial_vars.inputBufFill,
            (errorparameter_t)0
        );
        DISABLE_INTERRUPTS();
        openserial_vars.inputBufFill=0;
        ENABLE_INTERRUPTS();
    }
    
    uart_clearTxInterrupts();
    uart_clearRxInterrupts();      // clear possible pending interrupts
    uart_enableInterrupts();       // Enable USCI_A1 TX & RX interrupt
    
    DISABLE_INTERRUPTS();
    openserial_vars.busyReceiving  = FALSE;
    openserial_vars.mode           = MODE_INPUT;
    openserial_vars.reqFrameIdx    = 0;
#ifdef FASTSIM
    uart_writeBufferByLen_FASTSIM(
        openserial_vars.reqFrame,
        sizeof(openserial_vars.reqFrame)
    );
    openserial_vars.reqFrameIdx = sizeof(openserial_vars.reqFrame);
#else
    uart_writeByte(openserial_vars.reqFrame[openserial_vars.reqFrameIdx]);
#endif
    ENABLE_INTERRUPTS();
#endif

}

void openserial_startOutput() 
{
    uint8_t debugPrintCounter;
    INTERRUPT_DECLARATION();
    
    //=== made modules print debug information

# if 1  //lcg 20180418
    DISABLE_INTERRUPTS();
    openserial_vars.debugPrintCounter = (openserial_vars.debugPrintCounter+1)%(STATUS_MAX);
    debugPrintCounter = openserial_vars.debugPrintCounter;
    ENABLE_INTERRUPTS();

    switch (debugPrintCounter) {
        case STATUS_ISSYNC:
            if (debugPrint_isSync()==TRUE) {
                break;
            }
        case STATUS_ID:
            if (debugPrint_id()==TRUE) {
               break;
            }
        case STATUS_DAGRANK:
            if (debugPrint_myDAGrank()==TRUE) {
                break;
            }
        case STATUS_OUTBUFFERINDEXES:
            if (debugPrint_outBufferIndexes()==TRUE) {
                break;
            }
        case STATUS_ASN:
            if (debugPrint_asn()==TRUE) {
                break;
            }
        case STATUS_MACSTATS:
            if (debugPrint_macStats()==TRUE) {
                break;
            }
        case STATUS_SCHEDULE:
            if(debugPrint_schedule()==TRUE) {
                break;
            }
        case STATUS_BACKOFF:
            if(debugPrint_backoff()==TRUE) {
                break;
            }
        case STATUS_QUEUE:
            if(debugPrint_queue()==TRUE) {
                break;
            }
        case STATUS_NEIGHBORS:
            if (debugPrint_neighbors()==TRUE) {
                break;
            }
        case STATUS_KAPERIOD:
            if (debugPrint_kaPeriod()==TRUE) {
                break;
            }
        case STATUS_JOINED:
            if (debugPrint_joined()==TRUE) {
                break;
            }
        default:
            DISABLE_INTERRUPTS();
            if(openserial_vars.debugPrintCounter>STATUS_MAX)
            	openserial_vars.debugPrintCounter=0;
            ENABLE_INTERRUPTS();
    }
#endif
    //=== flush TX buffer
    
    uart_clearTxInterrupts();
    uart_clearRxInterrupts();          // clear possible pending interrupts
    uart_enableInterrupts();           // Enable USCI_A1 TX & RX interrupt
    
    DISABLE_INTERRUPTS();
    openserial_vars.mode=MODE_OUTPUT;
    if (openserial_vars.outputBufFilled) {
    	//debugpins_frame_set() ;//20180418
    	//debugpins_frame_toggle();
#ifdef FASTSIM
        uart_writeCircularBuffer_FASTSIM(
            openserial_vars.outputBuf,
            &openserial_vars.outputBufIdxR,
            &openserial_vars.outputBufIdxW
        );
#else
        uart_writeByte(openserial_vars.outputBuf[OUTPUT_BUFFER_MASK & (openserial_vars.outputBufIdxR++)]);
        //debugpins_frame_clr();
#endif
    } else {
        openserial_stop();
    }
    ENABLE_INTERRUPTS();
}

void openserial_stop() {
    uint8_t inputBufFill;
    uint8_t cmdByte;
    bool    busyReceiving;
    INTERRUPT_DECLARATION();
	openserialIODisableInput();
	openserialIOMarkStopOutput();
    DISABLE_INTERRUPTS();
    busyReceiving = openserial_vars.busyReceiving;
    inputBufFill  = openserial_vars.inputBufFill;
    ENABLE_INTERRUPTS();

    // disable UART interrupts
    uart_disableInterrupts();

    DISABLE_INTERRUPTS();
    openserial_vars.mode=MODE_OFF;
    ENABLE_INTERRUPTS();
    
    // the inputBuffer has to be reset if it is not reset where the data is read.
    // or the function openserial_getInputBuffer is called (which resets the buffer)
    if (busyReceiving==TRUE) {
        openserial_printError(
            COMPONENT_OPENSERIAL,
            ERR_BUSY_RECEIVING,
            (errorparameter_t)0,
            (errorparameter_t)inputBufFill
        );
    }
    
    if (busyReceiving==FALSE && inputBufFill>0) {
        DISABLE_INTERRUPTS();
        cmdByte = openserial_vars.inputBuf[0];
        ENABLE_INTERRUPTS();
        // call hard-coded commands
        // FIXME: needs to be replaced by registered commands only
        switch (cmdByte) {
            case SERFRAME_PC2MOTE_SETROOT://
                idmanager_triggerAboutRoot();
                break;
            case SERFRAME_PC2MOTE_RESET://
                board_reset();
                break;
            case SERFRAME_PC2MOTE_DATA://
            	//openserial_printInfo(COMPONENT_OPENWSN,0x5c,(errorparameter_t)0, (errorparameter_t)0);//lcg 20180125
                openbridge_triggerData();
                break;
            case SERFRAME_PC2MOTE_TRIGGERSERIALECHO://
                openserial_handleEcho(&openserial_vars.inputBuf[1],inputBufFill-1);
                break;
            case SERFRAME_PC2MOTE_COMMAND://
                openserial_handleCommands();
                break;
        }
        // call registered commands
        if (openserial_vars.registeredCmd!=NULL && openserial_vars.registeredCmd->cmdId==cmdByte) {
            
            openserial_vars.registeredCmd->cb();
        }
    }
    
    DISABLE_INTERRUPTS();
    openserial_vars.inputBufFill  = 0;
    openserial_vars.busyReceiving = FALSE;
    ENABLE_INTERRUPTS();
}

//===== debugprint

/**
\brief Trigger this module to print status information, over serial.

debugPrint_* functions are used by the openserial module to continuously print
status information about several modules in the OpenWSN stack.

\returns TRUE if this function printed something, FALSE otherwise.
*/
bool debugPrint_outBufferIndexes() {
    uint16_t temp_buffer[2];
    INTERRUPT_DECLARATION();
    
    DISABLE_INTERRUPTS();
    temp_buffer[0] = openserial_vars.outputBufIdxW;
    temp_buffer[1] = openserial_vars.outputBufIdxR;
    ENABLE_INTERRUPTS();
    
    openserial_printStatus(
        STATUS_OUTBUFFERINDEXES,
        (uint8_t*)temp_buffer,
        sizeof(temp_buffer)
    );
    
    return TRUE;
}

//=========================== private =========================================

//===== printing

owerror_t openserial_printInfoErrorCritical(
    char             severity,
    uint8_t          calling_component,
    uint8_t          error_code,
    errorparameter_t arg1,
    errorparameter_t arg2
) {
    INTERRUPT_DECLARATION();
    
    DISABLE_INTERRUPTS();
    openserial_vars.outputBufFilled  = TRUE;
    outputHdlcOpen();
    outputHdlcWrite(severity);
    outputHdlcWrite(idmanager_getMyID(ADDR_16B)->addr_16b[0]);
    outputHdlcWrite(idmanager_getMyID(ADDR_16B)->addr_16b[1]);
    outputHdlcWrite(calling_component);
    outputHdlcWrite(error_code);
    outputHdlcWrite((uint8_t)((arg1 & 0xff00)>>8));
    outputHdlcWrite((uint8_t) (arg1 & 0x00ff));
    outputHdlcWrite((uint8_t)((arg2 & 0xff00)>>8));
    outputHdlcWrite((uint8_t) (arg2 & 0x00ff));
    outputHdlcClose();
    ENABLE_INTERRUPTS();
    
    return E_SUCCESS;
}

//===== command handlers

void openserial_handleEcho(uint8_t* buf, uint8_t bufLen){
    // echo back what you received
    openserial_printData(
        buf,
        bufLen
    );
}

void openserial_get6pInfo(uint8_t commandId, uint8_t* code,uint8_t* cellOptions,uint8_t* numCells,cellInfo_ht* celllist_add,cellInfo_ht* celllist_delete,uint8_t* listOffset,uint8_t* maxListLen,uint8_t ptr, uint8_t commandLen){
    uint8_t i; 
    uint8_t celllistLen;
    
    // clear command
    if (commandId == COMMAND_SET_6P_CLEAR){
        *code = IANA_6TOP_CMD_CLEAR;
        return;
    }
    
    *cellOptions  = openserial_vars.inputBuf[ptr];
    ptr          += 1;
    commandLen   -= 1;
    
    // list command
    if (commandId == COMMAND_SET_6P_LIST){
        *code = IANA_6TOP_CMD_LIST;
        *listOffset   = openserial_vars.inputBuf[ptr];
        ptr += 1;
        *maxListLen   = openserial_vars.inputBuf[ptr];
        ptr += 1;
        return;
    }
    
    // count command
    if (commandId == COMMAND_SET_6P_COUNT){
        *code = IANA_6TOP_CMD_COUNT;
        return;
    }
    
    *numCells   = openserial_vars.inputBuf[ptr];
    ptr        += 1;
    commandLen -= 1;
    
    // add command
    if (commandId == COMMAND_SET_6P_ADD){
        *code = IANA_6TOP_CMD_ADD;
        // retrieve cell list
        i = 0;
        celllistLen = commandLen/2;
        while(commandLen>celllistLen){
            celllist_add[i].slotoffset     = openserial_vars.inputBuf[ptr];
            celllist_add[i].channeloffset  = openserial_vars.inputBuf[ptr+celllistLen];
            celllist_add[i].isUsed         = TRUE;
            ptr         += 1;
            commandLen  -= 1;
            i++;
        }
        return;
    }

    // delete command
    if (commandId == COMMAND_SET_6P_DELETE){
        *code = IANA_6TOP_CMD_DELETE;
        // retrieve cell list
        i = 0;
        celllistLen = commandLen/2;
        while(commandLen>celllistLen){
            celllist_delete[i].slotoffset     = openserial_vars.inputBuf[ptr];
            celllist_delete[i].channeloffset  = openserial_vars.inputBuf[ptr+celllistLen];
            celllist_delete[i].isUsed         = TRUE;
            ptr         += 1;
            commandLen  -= 1;
            i++;
        }
        return;
    }
    
    // relocate command
    if (commandId == COMMAND_SET_6P_RELOCATE){
        *code = IANA_6TOP_CMD_RELOCATE;
        // retrieve cell list to be relocated
        i = 0;
        while(i<*numCells){
            celllist_delete[i].slotoffset     = openserial_vars.inputBuf[ptr];
            celllist_delete[i].channeloffset  = openserial_vars.inputBuf[ptr+*numCells];
            celllist_delete[i].isUsed         = TRUE;
            ptr         += 1;
            i++;
        }
        
        commandLen      -= (*numCells) * 2;
        ptr             += *numCells;
        // retrieve cell list to be relocated
        i = 0;
        celllistLen = commandLen/2;
        while(commandLen>celllistLen){
            celllist_add[i].slotoffset     = openserial_vars.inputBuf[ptr];
            celllist_add[i].channeloffset  = openserial_vars.inputBuf[ptr+celllistLen];
            celllist_add[i].isUsed         = TRUE;
            ptr         += 1;
            commandLen  -= 1;
            i++;
        }
        return;
    }
}

void openserial_handleCommands(void){
    uint8_t  input_buffer[20];
    uint8_t  numDataBytes;
    uint8_t  commandId;
    uint8_t  commandLen;
    uint8_t  comandParam_8;
    uint16_t comandParam_16;
    
    uint8_t  code,cellOptions,numCell,listOffset,maxListLen;
    uint8_t  ptr;
    cellInfo_ht celllist_add[CELLLIST_MAX_LEN];
    cellInfo_ht celllist_delete[CELLLIST_MAX_LEN];
   
    open_addr_t neighbor;
    bool        foundNeighbor;
    
    ptr = 0;
    memset(celllist_add,0,CELLLIST_MAX_LEN*sizeof(cellInfo_ht));
    memset(celllist_delete,0,CELLLIST_MAX_LEN*sizeof(cellInfo_ht));
    
    numDataBytes = openserial_getInputBufferFilllevel();
    //copying the buffer
    openserial_getInputBuffer(&(input_buffer[ptr]),numDataBytes);
    ptr++;
    commandId  = openserial_vars.inputBuf[ptr];
    ptr++;
    commandLen = openserial_vars.inputBuf[ptr];
    ptr++;
    
    switch(commandId) {
        case COMMAND_SET_EBPERIOD:
            comandParam_8 = openserial_vars.inputBuf[ptr];
            sixtop_setEBPeriod(comandParam_8); // one byte, in seconds
            break;
        case COMMAND_SET_CHANNEL:
           comandParam_8 = openserial_vars.inputBuf[ptr];
           // set communication channel for protocol stack
           ieee154e_setSingleChannel(comandParam_8); // one byte
           // set listenning channel for sniffer
           sniffer_setListeningChannel(comandParam_8); // one byte
           break;
        case COMMAND_SET_KAPERIOD: // two bytes, in slots
            comandParam_16 = (openserial_vars.inputBuf[ptr] & 0x00ff) | \
                ((openserial_vars.inputBuf[ptr+1]<<8) & 0xff00); 
            sixtop_setKaPeriod(comandParam_16);
            break;
        case COMMAND_SET_DIOPERIOD: // two bytes, in mili-seconds
            comandParam_16 = (openserial_vars.inputBuf[ptr] & 0x00ff) | \
                ((openserial_vars.inputBuf[ptr+1]<<8) & 0xff00); 
            icmpv6rpl_setDIOPeriod(comandParam_16);
            break;
        case COMMAND_SET_DAOPERIOD: // two bytes, in mili-seconds
            comandParam_16 = (openserial_vars.inputBuf[ptr] & 0x00ff) | \
                ((openserial_vars.inputBuf[ptr+1]<<8) & 0xff00); 
            icmpv6rpl_setDAOPeriod(comandParam_16);
            break;
        case COMMAND_SET_DAGRANK: // two bytes
            comandParam_16 = (openserial_vars.inputBuf[ptr] & 0x00ff) | \
                ((openserial_vars.inputBuf[ptr+1]<<8) & 0xff00); 
            icmpv6rpl_setMyDAGrank(comandParam_16);
            break;
        case COMMAND_SET_SECURITY_STATUS: // one byte
            comandParam_8 = openserial_vars.inputBuf[ptr];
            ieee154e_setIsSecurityEnabled(comandParam_8);
            break;
        case COMMAND_SET_SLOTFRAMELENGTH: // two bytes
            comandParam_16 = (openserial_vars.inputBuf[ptr] & 0x00ff) | \
                ((openserial_vars.inputBuf[ptr+1]<<8) & 0xff00); 
            schedule_setFrameLength(comandParam_16);
            break;
        case COMMAND_SET_ACK_STATUS:
            comandParam_8 = openserial_vars.inputBuf[ptr];
            ieee154e_setIsAckEnabled(comandParam_8);
            break;
        case COMMAND_SET_6P_ADD:
        case COMMAND_SET_6P_DELETE:
        case COMMAND_SET_6P_RELOCATE:
        case COMMAND_SET_6P_COUNT:
        case COMMAND_SET_6P_LIST:
        case COMMAND_SET_6P_CLEAR:
            // get preferred parent
            foundNeighbor =icmpv6rpl_getPreferredParentEui64(&neighbor);
            if (foundNeighbor==FALSE) {
                break;
            }
            // the following sequence of bytes are, slotframe, cellOption, numCell, celllist
            openserial_get6pInfo(commandId,&code,&cellOptions,&numCell,celllist_add,celllist_delete,&listOffset,&maxListLen,ptr,commandLen);
            sixtop_request(
                code,              // code
                &neighbor,         // neighbor
                numCell,           // number cells
                cellOptions,       // cellOptions
                celllist_add,      // celllist to add
                celllist_delete,   // celllist to delete (not used)
                sf0_getsfid(),     // sfid
                listOffset,        // list command offset (not used)
                maxListLen         // list command maximum celllist (not used)
            );
            break;
        case COMMAND_SET_SLOTDURATION:
            comandParam_16 = (openserial_vars.inputBuf[ptr] & 0x00ff) | \
                ((openserial_vars.inputBuf[ptr+1]<<8) & 0xff00); 
            ieee154e_setSlotDuration(comandParam_16);
            break;
        case COMMAND_SET_6PRESPONSE:
            comandParam_8 = openserial_vars.inputBuf[ptr];
            if (comandParam_8 ==1) {
                sixtop_setIsResponseEnabled(TRUE);
            } else {
                if (comandParam_8 == 0) {
                    sixtop_setIsResponseEnabled(FALSE);
                } else {
                    // security only can be 1 or 0 
                    break;
                }
            }
            break;
        case COMMAND_SET_UINJECTPERIOD:
            comandParam_8 = openserial_vars.inputBuf[ptr];
            sf0_appPktPeriod(comandParam_8);
            break;
        case COMMAND_SET_ECHO_REPLY_STATUS:
            comandParam_8 = openserial_vars.inputBuf[ptr];
            if (comandParam_8 == 1) {
                icmpv6echo_setIsReplyEnabled(TRUE);
            } else {
                if (comandParam_8 == 0) {
                    icmpv6echo_setIsReplyEnabled(FALSE);
                } else {
                    // ack reply
                    break;
                }
            }
            break;
        case COMMAND_SET_JOIN_KEY:
            if (commandLen != 16) { break; }
            idmanager_setJoinKey(&openserial_vars.inputBuf[ptr]);
            break;
        default:
           // wrong command ID
           break;
   }
}

//===== misc

void openserial_board_reset_cb(opentimers_id_t id) {
    board_reset();
}

//===== hdlc (output)

/**
\brief Start an HDLC frame in the output buffer.
*/
port_INLINE void outputHdlcOpen() {
    // initialize the value of the CRC
    openserial_vars.outputCrc                                        = HDLC_CRCINIT;

    // write the opening HDLC flag
    openserial_vars.outputBuf[OUTPUT_BUFFER_MASK & (openserial_vars.outputBufIdxW++)]       = HDLC_FLAG;
}
/**
\brief Add a byte to the outgoing HDLC frame being built.
*/
port_INLINE void outputHdlcWrite(uint8_t b) {
    
    // iterate through CRC calculator
    openserial_vars.outputCrc = crcIteration(openserial_vars.outputCrc,b);
    
    // add byte to buffer
    if (b==HDLC_FLAG || b==HDLC_ESCAPE) {
        openserial_vars.outputBuf[OUTPUT_BUFFER_MASK & (openserial_vars.outputBufIdxW++)]   = HDLC_ESCAPE;
        b                                                            = b^HDLC_ESCAPE_MASK;
    }
    openserial_vars.outputBuf[OUTPUT_BUFFER_MASK & (openserial_vars.outputBufIdxW++)]       = b;
}
/**
\brief Finalize the outgoing HDLC frame.
*/
port_INLINE void outputHdlcClose() {
    uint16_t   finalCrc;

    // finalize the calculation of the CRC
    finalCrc   = ~openserial_vars.outputCrc;

    // write the CRC value
    outputHdlcWrite((finalCrc>>0)&0xff);
    outputHdlcWrite((finalCrc>>8)&0xff);

    // write the closing HDLC flag
    openserial_vars.outputBuf[OUTPUT_BUFFER_MASK & (openserial_vars.outputBufIdxW++)]       = HDLC_FLAG;
}

//===== hdlc (input)

/**
\brief Start an HDLC frame in the input buffer.
*/
port_INLINE void inputHdlcOpen() {
    // reset the input buffer index
    openserial_vars.inputBufFill                                     = 0;

    // initialize the value of the CRC
    openserial_vars.inputCrc                                         = HDLC_CRCINIT;
}
/**
\brief Add a byte to the incoming HDLC frame.
*/
port_INLINE void inputHdlcWrite(uint8_t b) {
    if (b==HDLC_ESCAPE) {
        openserial_vars.inputEscaping = TRUE;
    } else {
        if (openserial_vars.inputEscaping==TRUE) {
            b                             = b^HDLC_ESCAPE_MASK;
            openserial_vars.inputEscaping = FALSE;
        }
        
        // add byte to input buffer
        openserial_vars.inputBuf[openserial_vars.inputBufFill] = b;
        openserial_vars.inputBufFill++;
        
        // iterate through CRC calculator
        openserial_vars.inputCrc = crcIteration(openserial_vars.inputCrc,b);
    }
}
/**
\brief Finalize the incoming HDLC frame.
*/
port_INLINE void inputHdlcClose() {
    
    // verify the validity of the frame
    if (openserial_vars.inputCrc==HDLC_CRCGOOD) {
        // the CRC is correct
        
        // remove the CRC from the input buffer
        openserial_vars.inputBufFill    -= 2;
    } else {
        // the CRC is incorrect
        
        // drop the incoming fram
        openserial_vars.inputBufFill     = 0;
    }
}
void printc(const char *fmt,...)
{
    char msg[512];
    va_list varg;
    va_start(varg,fmt);
    vsprintf(msg,fmt,varg);//19KROM
    va_end(varg);
    openserial_printBuf(msg,strlen(msg));
}
void openserial_printBuf(uint8_t *buf,uint16_t len)
{
	uint8_t i;
	INTERRUPT_DECLARATION();
	DISABLE_INTERRUPTS();
	if(len+openserial_vars.outputBufIdxW>=1024)
	{
		
	}else
	{
		openserial_vars.outputBufFilled  = TRUE;
		outputHdlcOpen();
		outputHdlcWrite(SERFRAME_MOTE2PC_ERROR);
		outputHdlcWrite(idmanager_getMyID(ADDR_16B)->addr_16b[0]);
		outputHdlcWrite(idmanager_getMyID(ADDR_16B)->addr_16b[1]);
		outputHdlcWrite(0x55);
		outputHdlcWrite(0xaa);
		for (i=0;i<len;i++)
		{
			outputHdlcWrite(buf[i]);
		}
		outputHdlcClose();
		//memcpy(&openserial_vars.outputBuf[openserial_vars.outputBufIdxW],buf,len);
		//openserial_vars.outputBufIdxW+=len;
	}
	ENABLE_INTERRUPTS();
	
}
//=========================== interrupt handlers ==============================

// executed in ISR, called from scheduler.c
void isr_openserial_tx() {
    switch (openserial_vars.mode) {
        case MODE_INPUT:
            openserial_vars.reqFrameIdx++;
            if (openserial_vars.reqFrameIdx<sizeof(openserial_vars.reqFrame)) {
                uart_writeByte(openserial_vars.reqFrame[openserial_vars.reqFrameIdx]);
            }
            else
            {
            	uart_disableTxInterrupts();  //  lcg 20180427    after add comment

            }
            break;
        case MODE_OUTPUT:
            if ((openserial_vars.outputBufIdxW)==(openserial_vars.outputBufIdxR)) {
                openserial_vars.outputBufFilled = FALSE;
                uart_disableInterrupts();//  lcg 20180427    after add comment
            }
            if (openserial_vars.outputBufFilled) {
                uart_writeByte(openserial_vars.outputBuf[OUTPUT_BUFFER_MASK & (openserial_vars.outputBufIdxR++)]);
            }
            break;
        case MODE_OFF:
        	//debugpins_isr_set();  lcg 20180418
            default:
            break;
    }
}

// executed in ISR, called from scheduler.c
void isr_openserial_rx() {
    uint8_t rxbyte;
    uint8_t inputBufFill;

    // stop if I'm not in input mode
    if (openserial_vars.mode!=MODE_INPUT) {
        return;
    }

    // read byte just received
    rxbyte = uart_readByte();
    // keep length
    inputBufFill=openserial_vars.inputBufFill;
    
    if (
        openserial_vars.busyReceiving==FALSE  &&
        openserial_vars.lastRxByte==HDLC_FLAG &&
        rxbyte!=HDLC_FLAG
    ) {
        // start of frame

        // I'm now receiving
        openserial_vars.busyReceiving         = TRUE;

        // create the HDLC frame
        inputHdlcOpen();

        // add the byte just received
        inputHdlcWrite(rxbyte);
    } else if (
        openserial_vars.busyReceiving==TRUE   &&
        rxbyte!=HDLC_FLAG
    ) {
        // middle of frame

        // add the byte just received
        inputHdlcWrite(rxbyte);
        if (openserial_vars.inputBufFill+1>SERIAL_INPUT_BUFFER_SIZE){
            // input buffer overflow
            openserial_printError(
                COMPONENT_OPENSERIAL,
                ERR_INPUT_BUFFER_OVERFLOW,
                (errorparameter_t)0,
                (errorparameter_t)0
            );
            openserial_vars.inputBufFill       = 0;
            openserial_vars.busyReceiving      = FALSE;
            openserial_stop();
        }
    } 
    else if (
        openserial_vars.busyReceiving==TRUE   &&
        rxbyte==HDLC_FLAG
    ) {
        // end of frame
        
        // finalize the HDLC frame
        inputHdlcClose();
        
        if (openserial_vars.inputBufFill==0){
            // invalid HDLC frame
            openserial_printError(
                COMPONENT_OPENSERIAL,
                ERR_WRONG_CRC_INPUT,
                (errorparameter_t)inputBufFill,
                (errorparameter_t)0
            );
        }
         
        openserial_vars.busyReceiving      = FALSE;
        openserial_stop();
    }
    
    openserial_vars.lastRxByte = rxbyte;
}
/*zyxadded*/

void openserialIOEnableInput(void)
{
	HAL_NVIC_EnableIRQ(EXTI1_IRQn);
	#ifdef BOARD_VERSION_V3
		HAL_GPIO_WritePin(GPIOA,GPIO_PIN_9,GPIO_PIN_SET);
	#endif
	#ifdef BOARD_VERSION_V2
		HAL_GPIO_WritePin(GPIOA,GPIO_PIN_2,GPIO_PIN_SET);
	#endif
}
void openserialIODisableInput(void)
{
		HAL_NVIC_DisableIRQ(EXTI1_IRQn);
#ifdef BOARD_VERSION_V3
		HAL_GPIO_WritePin(GPIOA,GPIO_PIN_9,GPIO_PIN_RESET);
#endif
#ifdef BOARD_VERSION_V2
		HAL_GPIO_WritePin(GPIOA,GPIO_PIN_2,GPIO_PIN_RESET);
#endif

}
void openserialIOMarkStartOutput(void)
{
	#ifdef BOARD_VERSION_V3
		HAL_GPIO_WritePin(GPIOB,GPIO_PIN_5,GPIO_PIN_SET);
	#endif
	#ifdef BOARD_VERSION_V2
		HAL_GPIO_WritePin(GPIOB,GPIO_PIN_8,GPIO_PIN_SET);
	#endif
}
void openserialIOMarkStopOutput(void)
{
	#ifdef BOARD_VERSION_V3
		HAL_GPIO_WritePin(GPIOB,GPIO_PIN_5,GPIO_PIN_RESET);
	#endif
	#ifdef BOARD_VERSION_V2
		HAL_GPIO_WritePin(GPIOB,GPIO_PIN_8,GPIO_PIN_RESET);
	#endif
}
void UART_Transmit_TASK(void)
{
	uint16_t a;
	uint32_t Frequency;
	for(a=0;a<10000;a++);//delay
	if(!UARTDMA_IS_BUSY())
	{
		Frequency=GetSYSFrequency();
		if(Frequency<30000000||Frequency>34000000)
		{
			printc("system Frequency=%d",Frequency);
		}
		Frequency=Frequency/115200;
		
		if(Frequency!=huart1.Instance->BRR)
		{
			
			if(Frequency==0)
			{
				Frequency=32000000/115200;
			}
			//Frequency=32000000/9600;
			//printf("hello\n");
			__disable_irq();
			CLEAR_BIT(huart1.Instance->CR1, USART_CR1_UE);
			huart1.Instance->BRR=Frequency;
			SET_BIT(huart1.Instance->CR1,USART_CR1_UE);
			
			CLEAR_BIT(huart2.Instance->CR1, USART_CR1_UE);
			huart2.Instance->BRR=Frequency;
			SET_BIT(huart2.Instance->CR1,USART_CR1_UE);
			__enable_irq();
		}
	}
	HAL_UART_Transmit_DMA(&huart1,Uart_TX_BUF,Uart_Tx_BUF_LEN);
	
	huart1.gState=HAL_UART_STATE_READY;
	Uart_Tx_BUF_LEN=0;
}
void Debug_UartOUTPUT_DMA(uint8_t *data,uint8_t len)
{
	HAL_UART_Transmit_DMA(&huart1,data,len);
	huart1.gState=HAL_UART_STATE_READY;
}
void OpenSerialBuf_GetDatatoSend(void)
{
	uint8_t data[64];
	uint8_t data_len=0;
	if(openserial_vars.inputBufIdxR!=openserial_vars.inputBufIdxW)
	{
		for(data_len=0;data_len<64;data_len++)
		{
			data[data_len]=openserial_vars.inputBuf[openserial_vars.inputBufIdxR];
			openserial_vars.inputBufIdxR=(openserial_vars.inputBufIdxR+1)&INPUT_BUFFER_MASK;//0x3FF
			if(openserial_vars.inputBufIdxR==openserial_vars.inputBufIdxW)
			{
				openserial_vars.inputBufIdxR=0;
				openserial_vars.inputBufIdxW=0;
				data_len=data_len+1;
				break;
			}
		}
        
	cexample_task_cb_Send_Uart_To_RF(data,data_len);
	}
	
}
void Put_Uartdata_to_Openserialinputbuf(uint8_t *buf,uint16_t len)
{
	//1,copy the data received to the buf
	uint16_t i;
	uint16_t Openserialinputbuf_len;
	if(!idmanager_getIsDAGroot())//zyx20190304 deal the bad thing
	{
		openserial_vars.inputBufIdxW=0;
		openserial_vars.inputBufIdxR=0;
		
	}
	for(i=0;i<len;i++)
	{
		openserial_vars.inputBuf[openserial_vars.inputBufIdxW]=buf[i];
		openserial_vars.inputBufIdxW=(openserial_vars.inputBufIdxW+1)&INPUT_BUFFER_MASK;
	}

	//2,put one data to a queue;
	if(openqueue_sixtopGetCoapSentPacket()==NULL)//check if there is a coap packet in the queue
	{
		OpenSerialBuf_GetDatatoSend();
	}else
	{
		leds_error_toggle();
		printc("there is a coap packet in the queue");
		//there is a coap packet in the queues
	}
}



