#ifndef __ZYX_H
#define __ZYX_H
//#include "stm32l4xx"
#include "stm32l4xx_hal.h"
#include "MCU_Interface.h"
#include "opendefs.h"
//
#include "IEEE802154E.h"
#include "board.h"
#define USE_FLASH_PARAMETER//USE FLASH PARAMETER OR USE DEAD PARAMETER
//#define BOARD_VERSION_V2//V1为最原始版本，V2为64k版本 V3为32k版本,
#define BOARD_VERSION_V3//


#define	UART_COMMUNICATION_HEAD_0x71 0x71//header of the communication
#define	UART_COMMUNICATION_HEAD_0x66 0x65//0x66

static const char SOFT_VERSION[]="RF190409";

typedef enum
{
	UART_READPARA_COMMAND=0x84,
	UART_WRITEPARA_COMMAND=0x88,
	
}UART_COMMUNICATION_COMMAND;
#if 1
typedef struct
{

	uint8_t   FrameH;
	uint8_t   LenH;
	uint8_t   LenL;
	uint8_t   FrameL;
	uint8_t   Contrl;	/*控制域*/
	uint8_t   AdressL;
	uint8_t   AdressH;
	uint8_t   Type; 	/*类型标示*/
	uint8_t   InfoLimit;	/*结构限定词*/
	uint8_t   Reason;
	uint8_t   MAC_ADDR[4];
	uint8_t   IP;
	uint8_t   NUMBERS;
}ParaFrameTypeDef;
#endif
typedef struct
{
	uint8_t   FrameH;
	uint8_t   LenH;
	uint8_t   LenL;
	uint8_t   FrameL;
	uint8_t   Contrl;
	uint8_t   AdressL;
	uint8_t   AdressH;
	uint8_t   Type;
	uint8_t   fuc;
	uint8_t   Reason;


}ParaFrame;
#define RfParaAdress1 0x6000
#define RfParaAdress2 0x6200
#define RfParaAdress3 0x6400
#define RfParaAdress5 0x6600
#define RfParaAdress4 0x7200
#define WriteRfInfoAck 135//
#define WriteRfInfo  136
#define ReadRfInfoAck 133
#define ReadRfInfo  132
#define MacAddrLenth  8
#define FrameHeadLenth  4

#define HOWMANY_BYTES_PARAMETERNUMBER_I_HAVE 0x30 

#define ADDR_PARAMENT_STARTADDRESS	0x0803F800
//#define OFFSETADDR_MYMAC_Host_ADDRESS 0x0000//my DAGROOT MAC address
#define OFFSETADDR_MYMAC_ADDRESS 0x0000//my mac address 8B
#define OFFSETADDR_MYPAN_ADDRESS 0x0008//pan address 2B
#define OFFSETADDR_MYIP_ADDRESS 0x000A//IP address 1B
#define OFFSETADDR_NUMBEROFMOTES 0x000B//how many motes are running in the group
#define OFFSETADDR_MYPARENT_MAC_ADDRESS 0x000C//parent mac address 



//#define	OFFSETADDR_FIRST_WIRELESS_CHANNEL 0X0009//last pan byte means the wireless channel
#define	OFFSETADDR_FIRST_WIRELESS_CHANNEL 0X001f//first wireless channel
#define	OFFSETADDR_WIRELESS_CHANNEL_JUMP	0X0020//frequency jump
#define	OFFSETADDR_TRANSMIT_POWER			0X0021//transmit power
#define	OFFSETADDR_BANDRATE_OVER_AIR		0X0022//bandrate over air
#define	OFFSETADDR_RESET_POWERMODE				0X0023//power mode

#define	OFFSETADDR_RUNNINGMODE_RESET_TO_CALLINGMODE_TIME 0x0024//the mote changes its mode from runnning mode to calling mode after so many seconds,maybe master has double seconds or not;
#define	OFFSETADDR_CALLINGMODE_SLEEPING_TIME 0X0025//every so many seconds the mote opens 3 little 64us slots to listen the preamble,
#define OFFSETADDR_FREQUENCYOFFSET	0x0026
#define	OFFSETADDR_FREQUENCY_BANDWIDTH	0x0027

#define OFFSETADDR_LOWERPOWER_CALLINGON			0x0028
#define OFFSETADDR_CPUSLEEP						0x0029
#define OFFSETADDR_MASTERFORSE_CALLINGON 0x002A

#define	OFFSETADDR_VERSION	0x002B//software version information
#define	OFFSETADDR_PARAMETER_CRC 0x002F

#define BASEADDR_FLASH_PARAMETER		0x0803F800
#define	BSAEADDR_FLASH_PARAMETERBACK	0x0803F000

#define CallingONTime_S 	3
#define FIRSTCallingONPeriod_S	3



#define	GPIO_RFCS	GPIOB
#define	GPIO_PIN_RFCS	GPIO_PIN_12
#define GPIO_RF_SPIMOSI	GPIOB
#define GPIO_PIN_RF_SPIMOSI	GPIO_PIN_15
#define GPIO_RF_SPIMISO	GPIOB
#define	GPIO_PIN_RF_SPIMISO	GPIO_PIN_14
#define GPIO_RF_SPISCK	GPIOB
#define	GPIO_PIN_RF_SPISCK	GPIO_PIN_13
#define GPIO_RFSDN		GPIOA
#define GPIO_PIN_RFSDN		GPIO_PIN_5
#define GPIO_LED0	GPIOA//NONE
#define	GPIO_PIN_LED0	GPIO_PIN_0//
#define GPIO_LED1	GPIOB
#define	GPIO_PIN_LED1	GPIO_PIN_8
#define	GPIO_LED2	GPIOB
#define	GPIO_PIN_LED2	GPIO_PIN_3
#define GPIO_RFEXTI0	GPIOA
#define	GPIO_PIN_RFEXTI0	GPIO_PIN_15
#define	GPIO_RFEXTI1	GPIOB
#define	GPIO_PIN_RFEXTI1	GPIO_PIN_11
#define	GPIO_RFEXTI2	GPIOA
#define	GPIO_PIN_RFEXTI2	GPIO_PIN_12
#define	GPIO_RFEXTI3	GPIOA
#define	GPIO_PIN_RFEXTI3	GPIO_PIN_11
/*ports to exits*/
#ifdef BOARD_VERSION_V2
#define GPIO_SecPulse   GPIOB//20181102zyx
#define GPIO_PIN_SecPulse       GPIO_PIN_9//sec pulse
#endif

#ifdef BOARD_VERSION_V3
#define GPIO_SecPulse   GPIOB
#define GPIO_PIN_SecPulse       GPIO_PIN_8//sec pulse

#endif
/*             */

#define GPIO_LED3	GPIOA
#define GPIO_PIN_LED3	GPIO_PIN_1
#define GPIO_LED4	GPIOA
#define GPIO_PIN_LED4	GPIO_PIN_2
#define GPIO_LED5	GPIOA
#define GPIO_PIN_LED5	GPIO_PIN_3



#define HEADER_WRITE_MASK     0x00 /*!< Write mask for header byte*/
#define HEADER_READ_MASK      0x01 /*!< Read mask for header byte*/
#define HEADER_ADDRESS_MASK   0x00 /*!< Address mask for header byte*/
#define HEADER_COMMAND_MASK   0x80 /*!< Command mask for header byte*/

#define LINEAR_FIFO_ADDRESS 0xFF  /*!< Linear FIFO address*/


#define BUILT_HEADER(add_comm, w_r) (add_comm | w_r)  /*!< macro to build the header byte*/
#define WRITE_HEADER    BUILT_HEADER(HEADER_ADDRESS_MASK, HEADER_WRITE_MASK) /*!< macro to build the write header byte*/
#define READ_HEADER     BUILT_HEADER(HEADER_ADDRESS_MASK, HEADER_READ_MASK)  /*!< macro to build the read header byte*/
#define COMMAND_HEADER  BUILT_HEADER(HEADER_COMMAND_MASK, HEADER_WRITE_MASK) /*!< macro to build the command header byte*/

#define SPI_ENTER_CRITICAL()           __disable_irq()
#define SPI_EXIT_CRITICAL()            __enable_irq()

#define SdkEvalSPICSLow()        HAL_GPIO_WritePin(GPIO_RFCS, GPIO_PIN_RFCS, GPIO_PIN_RESET)
#define SdkEvalSPICSHigh()       HAL_GPIO_WritePin(GPIO_RFCS, GPIO_PIN_RFCS, GPIO_PIN_SET)


void SdkEvalSpiInit(void);
void SdkEvalSpiDeinit(void);

StatusBytes SdkEvalSpiWriteRegisters(uint8_t cRegAddress, uint8_t cNbBytes, uint8_t* pcBuffer);
StatusBytes SdkEvalSpiReadRegisters(uint8_t cRegAddress, uint8_t cNbBytes, uint8_t* pcBuffer);
StatusBytes SdkEvalSpiCommandStrobes(uint8_t cCommandCode);
StatusBytes SdkEvalSpiWriteFifo(uint8_t cNbBytes, uint8_t* pcBuffer);
StatusBytes SdkEvalSpiReadFifo(uint8_t cNbBytes, uint8_t* pcBuffer);

void SdkEvalEnterShutdown(void);
void SdkEvalExitShutdown(void);
SFlagStatus SdkEvalCheckShutdown(void);
int8_t ChkSum(int8_t *buf, int16_t len);
bool ISConfigPacket(uint8_t *buf,int16_t len);
bool AnalyzePacket(uint8_t *buf,int16_t len,uint8_t *answerbuf,uint16_t *answerbuflen);

bool CheckFlashParamisOK(void);
bool Read_FLASH_Paraments(uint8_t *buf);
bool Write_FLASH_Paraments(uint8_t *buf);
void Write_Paramenter_TOFlash(uint8_t *buf,uint16_t len);
void Write_Default_Parameter(void);
int32_t crc32(uint8_t *addr,uint8_t len);




#endif

