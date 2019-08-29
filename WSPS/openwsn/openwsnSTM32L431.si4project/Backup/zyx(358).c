#include"zyx.h"

extern uint8_t Uart_TX_BUF[1024];
extern uint16_t Uart_Tx_BUF_LEN;
extern SPI_HandleTypeDef hspi2;
extern uint16_t ADC_CNT;

void SdkEvalSpiInit(void)
{
	//NOP
}
void SdkEvalSpiDeinit(void)
{
	//NOP
}
StatusBytes SdkEvalSpiWriteRegisters(uint8_t cRegAddress, uint8_t cNbBytes, uint8_t* pcBuffer)
{
	//
	uint8_t tx_buff[2]={WRITE_HEADER,cRegAddress};
	uint8_t rx_buff[255];
	StatusBytes status;

	SPI_ENTER_CRITICAL();

	/* Puts the SPI chip select low to start the transaction */
	SdkEvalSPICSLow();
	
	HAL_SPI_TransmitReceive(&hspi2, tx_buff, rx_buff, 2, 1000);
	HAL_SPI_TransmitReceive(&hspi2, pcBuffer, &rx_buff[2], cNbBytes, 1000);

	/* Puts the SPI chip select high to end the transaction */
	SdkEvalSPICSHigh();

	SPI_EXIT_CRITICAL();

	((uint8_t*)&status)[1]=rx_buff[0];
	((uint8_t*)&status)[0]=rx_buff[1];

	return status;
}
StatusBytes SdkEvalSpiReadRegisters(uint8_t cRegAddress, uint8_t cNbBytes, uint8_t* pcBuffer)
{
	uint8_t tx_buff[255]={READ_HEADER,cRegAddress};
	uint8_t rx_buff[2];
	StatusBytes status;

	SPI_ENTER_CRITICAL();
	SdkEvalSPICSLow();
	HAL_SPI_TransmitReceive(&hspi2, tx_buff, rx_buff, 2, 1000);
	HAL_SPI_TransmitReceive(&hspi2, tx_buff, pcBuffer, cNbBytes, 1000);
	SdkEvalSPICSHigh();
	SPI_EXIT_CRITICAL();

	((uint8_t*)&status)[1]=rx_buff[0];
	((uint8_t*)&status)[0]=rx_buff[1];  

	return status;
	
}

StatusBytes SdkEvalSpiCommandStrobes(uint8_t cCommandCode)
{
	uint8_t tx_buff[2]={COMMAND_HEADER,cCommandCode};
	uint8_t rx_buff[2];
	StatusBytes status;

	SPI_ENTER_CRITICAL();
	SdkEvalSPICSLow();
	//HAL_GPIO_TogglePin(GPIOB,GPIO_PIN_3);
	if(cCommandCode==CMD_READY)
	{
		for(volatile uint8_t i=0;i<0x50;i++);
	}
	//HAL_GPIO_TogglePin(GPIOB,GPIO_PIN_3);
	HAL_SPI_TransmitReceive(&hspi2, tx_buff, rx_buff, 2, 1000);
	SdkEvalSPICSHigh();
	SPI_EXIT_CRITICAL();

	((uint8_t*)&status)[1]=rx_buff[0];
	((uint8_t*)&status)[0]=rx_buff[1];

	return status;
}
StatusBytes SdkEvalSpiWriteFifo(uint8_t cNbBytes, uint8_t* pcBuffer)
{
	uint8_t tx_buff[2]={WRITE_HEADER,LINEAR_FIFO_ADDRESS};
	uint8_t rx_buff[130];
	StatusBytes status;

	SPI_ENTER_CRITICAL();
	SdkEvalSPICSLow();
	HAL_SPI_TransmitReceive(&hspi2, tx_buff, rx_buff, 2, 1000);
	HAL_SPI_TransmitReceive(&hspi2, pcBuffer, &rx_buff[2], cNbBytes, 1000);
	SdkEvalSPICSHigh();
	SPI_EXIT_CRITICAL();

	((uint8_t*)&status)[1]=rx_buff[0];
	((uint8_t*)&status)[0]=rx_buff[1];

	return status;

}
StatusBytes SdkEvalSpiReadFifo(uint8_t cNbBytes, uint8_t* pcBuffer)
{
	uint8_t tx_buff[130]={READ_HEADER,LINEAR_FIFO_ADDRESS};
	uint8_t rx_buff[2];
	StatusBytes status;

	SPI_ENTER_CRITICAL();
	SdkEvalSPICSLow();
	HAL_SPI_TransmitReceive(&hspi2, tx_buff, rx_buff, 2, 1000);
	HAL_SPI_TransmitReceive(&hspi2, tx_buff, pcBuffer, cNbBytes, 1000);
	SdkEvalSPICSHigh();
	SPI_EXIT_CRITICAL();

	((uint8_t*)&status)[1]=rx_buff[0];
	((uint8_t*)&status)[0]=rx_buff[1];


	return status;
	
}

void SdkEvalEnterShutdown(void)
{
	HAL_GPIO_WritePin(GPIO_RFSDN,GPIO_PIN_RFSDN,GPIO_PIN_SET);
}
void SdkEvalExitShutdown(void)
{
	HAL_GPIO_WritePin(GPIO_RFSDN,GPIO_PIN_RFSDN,GPIO_PIN_RESET);
}
SFlagStatus SdkEvalCheckShutdown(void)
{
	return	(SFlagStatus)HAL_GPIO_ReadPin(GPIO_RFSDN,GPIO_PIN_RFSDN);
}
int8_t ChkSum(int8_t *buf, int16_t len)
{
    int16_t checksum, i;

    checksum = 0;
    for (i = 0; i < len; i++)
    {
        checksum = checksum + *buf;
        buf ++;
    }
    return (int8_t)(checksum & 0xff);
}

bool ISConfigPacket(uint8_t *buf,int16_t len)
{
	if(buf[0]==UART_COMMUNICATION_HEAD_0x66&&buf[3]==UART_COMMUNICATION_HEAD_0x66)//added by zyx 20190223
	{
		#if 0
		if(buf[7]==0x91)
		{
			return FALSE;//same group trig packet
		}else
		{
			return TRUE;//71config packet
		}
		#endif
		return TRUE;
	}
	else
	{
		return FALSE;//
	}
}
/*return READ BUF of the memory*/
void ReadRfParaAck(ParaFrame* UartParaFrame,uint8_t* Pdate )
{
	uint8_t AddressL;
	uint8_t AddressH;
	uint16_t Address;
	uint16_t BaseAddress;
	uint8_t i;
	uint8_t ELementNum;
	uint8_t ELementLen;
	uint32_t ADDR_FLASH;
	uint8_t PARAMENTER_WRITEFLASH_BUF[HOWMANY_BYTES_PARAMETERNUMBER_I_HAVE];
	uint8_t  Sum;
	uint8_t* BufPtr;
	uint16_t AddressLenth = 2;
	uint16_t MacIpAddr=0;
	uint8_t Mac[4];
	uint8_t Ip;
	Address=0;
	ELementLen=0;
	if( (UartParaFrame->fuc & 0x01) ==0x01)
	{
		for(i=0;i<4;i++)
			Mac[i] = * (Pdate ++);
		MacIpAddr=4;
	}
	if( (UartParaFrame->fuc & 0x02) == 0x02)
	{
		Ip = * (Pdate ++);
		MacIpAddr++;
	}
	ELementNum = * (Pdate ++);
	if(ELementNum>HOWMANY_BYTES_PARAMETERNUMBER_I_HAVE)
		return;
	ADDR_FLASH=BASEADDR_FLASH_PARAMETER;
	for(i=0;i<HOWMANY_BYTES_PARAMETERNUMBER_I_HAVE;i++)/*把FLASH里的参数读到内存*/
	{
		PARAMENTER_WRITEFLASH_BUF[i]=*(__IO uint8_t *)ADDR_FLASH;
		ADDR_FLASH++;
	}
	UartParaFrame->Reason = 0x0A;

	UartParaFrame->Type= ReadRfInfoAck;
	UartParaFrame->LenL = ( sizeof(ParaFrame) - FrameHeadLenth + 1   );
	UartParaFrame->LenH = UartParaFrame->LenL;
	UartParaFrame->fuc|=0x80;
	BufPtr = Uart_TX_BUF;
	memcpy( BufPtr , UartParaFrame,sizeof(ParaFrame) );
	BufPtr+= sizeof(ParaFrame);
	if( (UartParaFrame->fuc & 0x01) ==0x01)
		{
			for(i=0;i<4;i++)
			* (BufPtr ++) = Mac[i];
		}
	if( (UartParaFrame->fuc & 0x02) == 0x02)
			* (BufPtr ++) = Ip;
	*(BufPtr++ ) = ELementNum;
	for(i=0;i<ELementNum;i++)
	{
			AddressL = *(Pdate++);
			AddressH = *(Pdate++);
			Address = AddressH;
			Address = Address<<8;
			Address += AddressL;
			if(Address >= RfParaAdress4)
				BaseAddress = RfParaAdress4;
			else if(Address >= RfParaAdress3)
				BaseAddress = RfParaAdress3 - OFFSETADDR_MYMAC_ADDRESS;
			else if(Address >= RfParaAdress2)
				BaseAddress = RfParaAdress2 - OFFSETADDR_FIRST_WIRELESS_CHANNEL;
			else
				BaseAddress = RfParaAdress1;
			*(BufPtr++ ) = AddressL;
			*(BufPtr++ ) = AddressH;
			switch(Address)
			{
				case 0x6000:
					*(BufPtr++ ) =(ieee154e_islowpower()==TRUE)?0:1;
					ELementLen +=( AddressLenth+1);
					break;
				case 0x6400:
					memcpy( BufPtr , PARAMENTER_WRITEFLASH_BUF,MacAddrLenth );
					BufPtr+=MacAddrLenth;
					ELementLen += MacAddrLenth;
					ELementLen += AddressLenth;
					break;
				case 0x6408:
					memcpy( BufPtr ,PARAMENTER_WRITEFLASH_BUF + OFFSETADDR_MYPAN_ADDRESS,2 );
					BufPtr+=2;
					ELementLen += 2;
					ELementLen += AddressLenth;
					break;
				case 0x640C:
					memcpy( BufPtr ,PARAMENTER_WRITEFLASH_BUF + OFFSETADDR_MYPARENT_MAC_ADDRESS,MacAddrLenth );
					BufPtr+=MacAddrLenth;
					ELementLen += MacAddrLenth;
					ELementLen += AddressLenth;
					break;
				case 0x6600:
					*(BufPtr++ ) =getSlavenums();
					ELementLen +=( AddressLenth+1);
					break;
				case 0x6601:
					*(BufPtr++ ) =getSlaveMAC(0x00);
					ELementLen +=( AddressLenth+1);
					break;
				case 0x6602:
					*(BufPtr++ ) =getSlaveLOWERPOWERMODE(0x00);
					ELementLen +=( AddressLenth+1);
					break;
				case 0x6603:
					*(BufPtr++ ) =getSlaveRSSI(0x00);
					ELementLen +=( AddressLenth+1);
					break;
				case 0x6604:
					*(BufPtr++ ) =getSlaveMAC(0x01);
					ELementLen +=( AddressLenth+1);
					break;
				case 0x6605:
					*(BufPtr++ ) =getSlaveLOWERPOWERMODE(0x01);
					ELementLen +=( AddressLenth+1);
					break;
				case 0x6606:
					*(BufPtr++ ) =getSlaveRSSI(0x01);
					ELementLen +=( AddressLenth+1);
					break;
				case 0x6607:
					*(BufPtr++ ) =getSlaveMAC(0x02);
					ELementLen +=( AddressLenth+1);
					break;
				case 0x6608:
					*(BufPtr++ ) =getSlaveLOWERPOWERMODE(0x02);
					ELementLen +=( AddressLenth+1);
					break;
				case 0x6609:
					*(BufPtr++ ) =getSlaveRSSI(0x02);
					ELementLen +=( AddressLenth+1);
					break;
				case 0x7200:
				case 0x7201:
					memcpy( BufPtr ,SOFT_VERSION,sizeof(SOFT_VERSION) );
					BufPtr+=sizeof(SOFT_VERSION);
					ELementLen += sizeof(SOFT_VERSION);
					ELementLen += AddressLenth;
					break;
		
				default:
					*(BufPtr++ ) = PARAMENTER_WRITEFLASH_BUF[Address-BaseAddress];
					ELementLen +=( AddressLenth+1);
					break;
				}
		}
	UartParaFrame->LenL = ( sizeof(ParaFrame) -FrameHeadLenth + 1 + ELementLen +MacIpAddr);
	UartParaFrame->LenH = UartParaFrame->LenL;
	memcpy( Uart_TX_BUF , UartParaFrame,sizeof(ParaFrame) );
	Sum = ChkSum(Uart_TX_BUF+FrameHeadLenth, UartParaFrame->LenL);
	*(BufPtr++ ) = Sum;
	*BufPtr = 0x16;
	Uart_Tx_BUF_LEN = UartParaFrame->LenL + FrameHeadLenth + 2;
}

void WrRfParaAck(ParaFrame* UartParaFrame,uint8_t* Pdate)
{
	unsigned char  i,Sum;
	uint8_t* BufPtr;
	uint8_t MacIpAddr=0;
	UartParaFrame->Reason = 0x0A;

	UartParaFrame->Type= WriteRfInfoAck;
	UartParaFrame->LenL = sizeof(ParaFrame) - FrameHeadLenth +1;
	UartParaFrame->LenH = UartParaFrame->LenL;
	UartParaFrame->fuc|=0x80;
	BufPtr = Uart_TX_BUF;
	memcpy( BufPtr , UartParaFrame,sizeof(ParaFrame) );
	BufPtr+= sizeof(ParaFrame);
	if( (UartParaFrame->fuc & 0x01) ==0x01)
			{
				for(i=0;i<4;i++)
					*(BufPtr++) = * (Pdate ++);
				MacIpAddr=4;
			}
		if( (UartParaFrame->fuc & 0x02) == 0x02)
		{
			*(BufPtr++) = * (Pdate ++);
		MacIpAddr++;
		}
		*(BufPtr++) = * (Pdate ++);
		UartParaFrame->LenL += MacIpAddr;
		UartParaFrame->LenH = UartParaFrame->LenL;
	memcpy( Uart_TX_BUF , UartParaFrame,sizeof(ParaFrame) );
	Sum = ChkSum(Uart_TX_BUF+FrameHeadLenth, UartParaFrame->LenL);
	*(BufPtr++ ) = Sum;
	*BufPtr = 0x16;
	Uart_Tx_BUF_LEN = UartParaFrame->LenL + FrameHeadLenth + 2;
}
void SetCommand(uint16_t Address,uint8_t value)
{
	switch(Address)
	{
		case 0x6000:
			if(value==0)
			{
				if(ieee154e_isSynch()&&(!idmanager_getIsDAGroot()))
				{
					ieee154e_changemode(LOWERPOWER_RUN);
					ADC_CNT=1;
				}else
				{
					FCI_LETME_LOWERPOWERON=1;
				}
				
			}
			else
			{
				if(ieee154e_isSynch())
				{
					ieee154e_changemode(HIGHPOWER_CALLINGOFF);
					ADC_CNT=1;
				}
				FCI_LETME_LOWERPOWERON=0;
			}
			break;
		case 0x6001:
			if(value==0)
				sixtop_change_calling_mode(OFF);
			else
				sixtop_change_calling_mode(ON);
			break;
		case 0x6002:
			if(value)
				HAL_NVIC_SystemReset();
			break;
		case 0x6003:
			if(value)
				radio_reinit();
			break;
		case 0x6004://ADC counter set zero
			if(value==1)
			{
				ADC_CNT=1;
			}
			break;
		case 0x6005:
			break;
		case 0x6006:
			break;
		case 0x6007:
			break;
		 default:
			 break;
	}
}
void WriteRfPara(ParaFrame* UartParaFrame,uint8_t* Pdate)
{
	uint8_t AddressL;
	uint8_t AddressH;
	uint8_t NoSetFlash=0;
	uint16_t Address;
	uint8_t i;
	uint8_t ELementNum;
	uint32_t ADDR_FLASH;
	uint8_t PARAMENTER_WRITEFLASH_BUF[0x30];
	uint8_t Mac[4];
	uint8_t Ip;
	uint8_t value;
	Address=0;
	if( (UartParaFrame->fuc & 0x01) ==0x01)
		{
			for(i=0;i<4;i++)
				Mac[i] = * (Pdate ++);
		}
	if( (UartParaFrame->fuc & 0x02) == 0x02)
			Ip = * (Pdate ++);
	ELementNum = * (Pdate ++);
	if(ELementNum>HOWMANY_BYTES_PARAMETERNUMBER_I_HAVE)
		return;
	if(ELementNum == 0)
		return;
	ADDR_FLASH=BASEADDR_FLASH_PARAMETER;
	for(i=0;i<HOWMANY_BYTES_PARAMETERNUMBER_I_HAVE;i++)/*把FLASH里的参数读到内存*/
	{
		PARAMENTER_WRITEFLASH_BUF[i]=*(__IO uint8_t *)ADDR_FLASH;
		ADDR_FLASH++;
	}
	for(i=0;i<ELementNum;i++)
	{
		AddressL = *(Pdate++);
		AddressH = *(Pdate++);
		Address = AddressH;
		Address = Address<<8;
		Address += AddressL;
		if(( Address>=RfParaAdress1) && (Address < RfParaAdress2) )
		{
			value = *(Pdate++);
			SetCommand(Address,value);
			NoSetFlash = 1;
		}
		else if(( Address>=RfParaAdress2) && (Address < RfParaAdress3) )/*无线参数*/
		{
			Address = Address - RfParaAdress2;
			if(Address <  HOWMANY_BYTES_PARAMETERNUMBER_I_HAVE )
			PARAMENTER_WRITEFLASH_BUF[OFFSETADDR_FIRST_WIRELESS_CHANNEL + Address] = *(Pdate++);
		}
		else if(( Address>=RfParaAdress3) && (Address < RfParaAdress4) )/*设备参数*/
		{
			if((Address == 0x6400)||(Address == 0x640C)){
				Address = Address - RfParaAdress3;
				memcpy(PARAMENTER_WRITEFLASH_BUF+Address,Pdate,MacAddrLenth);
				Pdate+=MacAddrLenth;
			}
			else if(Address == 0x6408){
				Address = Address - RfParaAdress3;
				memcpy(PARAMENTER_WRITEFLASH_BUF+Address,Pdate,2);
				Pdate+=2;
			}
			else{
				Address = Address - RfParaAdress3;
				PARAMENTER_WRITEFLASH_BUF[Address] = *(Pdate++);
			}
		}
		else
			(Pdate++);
	}
	if(NoSetFlash)
	{
		
	}
	else
	{
		PARAMENTER_WRITEFLASH_BUF[HOWMANY_BYTES_PARAMETERNUMBER_I_HAVE-1]=ChkSum(PARAMENTER_WRITEFLASH_BUF,HOWMANY_BYTES_PARAMETERNUMBER_I_HAVE-1);//30个参数的CRC
		Write_Paramenter_TOFlash(PARAMENTER_WRITEFLASH_BUF,HOWMANY_BYTES_PARAMETERNUMBER_I_HAVE/8);
		#ifdef OPEN_IWDG
		SYSTEM_RESET_NO_FEED_DOG();
		#else
		HAL_NVIC_SystemReset();
		#endif
	}
}
/*analyzePacket and answer to the */
bool AnalyzePacket(uint8_t *buf,int16_t len,uint8_t *answerbuf,uint16_t *answerbuflen)
{
	ParaFrameTypeDef *HEADER;

	uint16_t i,j;
	uint16_t Calculate_CRC_START_Count;
	uint32_t ADDR_FLASH=BASEADDR_FLASH_PARAMETER;
	uint8_t PARAMENTER_WRITEFLASH_BUF[HOWMANY_BYTES_PARAMETERNUMBER_I_HAVE];
	uint8_t CRC1;
	ParaFrame* UartParaFrame;
	uint8_t CntSum;
	/*I test a error and I will check it out*/
	
	if( (buf[0] == UART_COMMUNICATION_HEAD_0x66 ) && (buf[3] == UART_COMMUNICATION_HEAD_0x66 ) && (buf[1] == buf[2] ) )
	{
		UartParaFrame = (ParaFrame* )(buf );
		/*                                */
		if(( len  ) < UartParaFrame->LenH )
			return FALSE;
		/*                                   */
		CntSum = ChkSum(buf + 4 , UartParaFrame->LenH);
		/*                             */
		if(CntSum != buf[4+UartParaFrame->LenH])
			return FALSE;
		if(0x16 != buf[4+UartParaFrame->LenH +1 ])/*                */
			return FALSE;
		/*                                    */
		if(UartParaFrame->Type == WriteRfInfo)//WriteRfInfo
		{
			WriteRfPara( UartParaFrame,buf + sizeof(ParaFrame) );
			WrRfParaAck( UartParaFrame,buf + sizeof(ParaFrame) );

			return TRUE;

		}
		if(UartParaFrame->Type ==ReadRfInfo)// ReadDeviceInfoAck)
		{

			ReadRfParaAck( UartParaFrame,buf + sizeof(ParaFrame) );
			return TRUE;

		}
	}
	else
	{
		//zyx private process
		#if 1
		if((buf[0]==UART_COMMUNICATION_HEAD_0x66||buf[0]==0xFF)&&buf[1]==UART_COMMUNICATION_HEAD_0x66)
		{
			for(i=0;i<len-1;i++)
			{
				buf[i]=buf[i+1];
			}
			len=len-1;
		}
		HEADER=(ParaFrameTypeDef *)buf;
		if(HEADER->FrameH!=HEADER->FrameL||HEADER->FrameH!=UART_COMMUNICATION_HEAD_0x66)
		{
			printf("Data Header Error!\n");
			for(i=0;i<len;i++)
			{
				printf("%x\n",buf[i]);
			}
			return FALSE;
		}
	
		
		//right packet 20181105
		if(HEADER->LenH==0xFF&&HEADER->LenL==0xFF)//config packet zyx
		{
			
			for(i=0;i<0x30;i++)
			{
				PARAMENTER_WRITEFLASH_BUF[i]=buf[i+4];
			}
			PARAMENTER_WRITEFLASH_BUF[0x2F]=ChkSum(PARAMENTER_WRITEFLASH_BUF,HOWMANY_BYTES_PARAMETERNUMBER_I_HAVE-1);
			Write_Paramenter_TOFlash(PARAMENTER_WRITEFLASH_BUF,HOWMANY_BYTES_PARAMETERNUMBER_I_HAVE/8);
			printf("Parameters are writed to FLASH !\n");
			printf("I will reset now!\n");
			HAL_NVIC_SystemReset();
			while(1);
		}
		else if(HEADER->LenH==0x00&&HEADER->LenL==0x01)//debug interface lower_power_mode
		{
			ieee154e_changemode(LOWERPOWER_RUN);
			return FALSE;
		}
		else if(HEADER->LenH==0x02&&HEADER->LenL==0x03)//debug interface mode
		{
			ieee154e_changemode(HIGHPOWER_CALLINGOFF);
			ADC_CNT=1;
			return FALSE;
		}
		else if(HEADER->LenH==0x04&&HEADER->LenL==0x05)//debug interface read all flash parameters
		{
			ADDR_FLASH=ADDR_PARAMENT_STARTADDRESS;
			for(i=0;i<HOWMANY_BYTES_PARAMETERNUMBER_I_HAVE;i++)
			{
				answerbuf[(*answerbuflen)++]=*(uint8_t *)ADDR_FLASH;
				ADDR_FLASH++;
			}
			return FALSE;
		}else if(HEADER->LenH==0x06&&HEADER->LenL==0x07)
		{
			sixtop_change_calling_mode(ON);
			return FALSE;
		}else if(HEADER->LenH==0x08&&HEADER->LenL==0x09)
		{
			sixtop_change_calling_mode(OFF);
			return FALSE;
		}else if(HEADER->LenH==0x0A&&HEADER->LenL==0x0B)
		{
			return FALSE;
		}else if(HEADER->LenH==0x0C&&HEADER->LenL==0x0D)
		{
			HAL_NVIC_SystemReset();
			return FALSE;
		}else if(HEADER->LenH==0x0E&&HEADER->LenL==0x0F)
		{
			radio_reinit();
			printf("radio_reinit!\n");
			return FALSE;
		}else if(HEADER->LenH==0x10&&HEADER->LenL==0x11)
		{
			ADDR_FLASH=buf[4];
			ADDR_FLASH=ADDR_FLASH<<8|buf[5];
			ADDR_FLASH=ADDR_FLASH<<8|buf[6];
			ADDR_FLASH=ADDR_FLASH<<8|buf[7];
			answerbuf[(*answerbuflen)++]=UART_COMMUNICATION_HEAD_0x66;
			memcpy(&answerbuf[(*answerbuflen)],(void const*)ADDR_FLASH,buf[8]);
			(*answerbuflen)+=buf[8];
			answerbuf[(*answerbuflen)++]=UART_COMMUNICATION_HEAD_0x66;
			return FALSE;
		}
		#endif
		return FALSE;
	}
}

/*校验存在FLASH里面的参数是不是正确*/
uint8_t testCRC;
bool CheckFlashParamisOK(void)
{
	uint8_t i;
	uint8_t sum_flash=0;
	uint16_t len=0;
	
	len=HOWMANY_BYTES_PARAMETERNUMBER_I_HAVE;
	
	if(len==0)
	{
		return FALSE;
	}else if(len>HOWMANY_BYTES_PARAMETERNUMBER_I_HAVE)
	{
		return FALSE;
	}
	//len=len<<2;
	sum_flash=*(__IO uint8_t*)(ADDR_PARAMENT_STARTADDRESS+OFFSETADDR_PARAMETER_CRC);
	testCRC=ChkSum(( uint8_t *)ADDR_PARAMENT_STARTADDRESS,len-1);
	if(sum_flash==testCRC)
	{
		return TRUE;
	}else
	{
		return FALSE;
	}

}

bool Check_FLASH_Paraments_IS_ALL_0xFF(void)
{
	
}
bool Write_FLASH_Paraments(uint8_t *buf)
{
	uint8_t parameter[HOWMANY_BYTES_PARAMETERNUMBER_I_HAVE];
	uint8_t i;
	
}
void Write_Paramenter_TOFlash(uint8_t *buf,uint16_t len)
{
	FLASH_EraseInitTypeDef	EraseInitStruct;
	uint32_t SECTORError=0;
	uint16_t i;
	uint32_t FIRSTADDRESS=BASEADDR_FLASH_PARAMETER;
	__disable_irq();
	HAL_FLASH_Unlock();
	__HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_OPTVERR);//需要清一下该标志，否则擦除无�?zyx20180518
	__HAL_FLASH_CLEAR_FLAG(0xFFFFFFFF);
	EraseInitStruct.TypeErase=FLASH_TYPEERASE_PAGES;
	EraseInitStruct.Banks=1;
	EraseInitStruct.Page=127;
	EraseInitStruct.NbPages=1;
	while(HAL_FLASHEx_Erase(&EraseInitStruct,&SECTORError));
	for(i=0;i<len;i++)
	{
		HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD,FIRSTADDRESS,*(uint64_t *)buf);
		//HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD,FIRSTADDRESS,i);
		FIRSTADDRESS+=8;
		buf+=8;
	}
	__enable_irq();
}
void Write_Default_Parameter(void)
{
	//uint8_t Parameter_buf[0x30];
        uint8_t i;
    uint8_t     Parameter_buf[0x30]=
         {
			 0x00,
			 0x01,
			 0x02,
			 0x03,
			 0x04,
			 0x05,
			 0x06,
			 0x07,
			 0x00,
			 0x00,
			 0x01,
			 0x04,
			 0x20,
			 0x21,
			 0x22,
			 0x23,
			 0x24,
			 0x25,
			 0x26,
			 0x27,
			 0x14,
			 0x15,
			 0x16,
			 0x17,
			 0x18,
			 0x19,
			 0x1A,
			 0x1B,
			 0x1C,
			 0x1D,
			 0x1E,
			 0x00,
			 0x00,
			 0x21,
			 0x22,
			 0x23,
			 0x24,
			 0x25,
			 0x26,
			 0x27,
			 0x01,
			 0x29,
			 0x01,
			 0x2B
         };
     #if 0
	for(i=0;i<HOWMANY_BYTES_PARAMETERNUMBER_I_HAVE;i++)
	{
		Parameter_buf[i]=i;
	}		
	Parameter_buf[0x0A]=0x01;//slave
	Parameter_buf[0x0B]=0x04;
	Parameter_buf[0x0C]=0xFF;
	Parameter_buf[0x1D]=0x01;
	Parameter_buf[0x1E]=0x00;
	Parameter_buf[0x1F]=0x01;
	#endif
	Parameter_buf[0x2F]=ChkSum(Parameter_buf,HOWMANY_BYTES_PARAMETERNUMBER_I_HAVE-1);
	Write_Paramenter_TOFlash(Parameter_buf,HOWMANY_BYTES_PARAMETERNUMBER_I_HAVE/8);
}
#if 0
int32_t crc32(uint8_t *addr,uint8_t len)
{
	uint16_t i;
	uint32_t crc;
	crc=0xFFFFFFFF;
	for(i=0;i<len;i++)
	{
		crc=crc32tab[(crc^addr[i])&0xff]^(crc>>8);
	}
	return crc;
}
#endif
int32_t crc32(uint8_t *addr,uint8_t len)
{
	int k;
	uint32_t crc=0;
	 crc = ~crc;
	 while (len--) {
		 crc ^= *addr++;
		 for (k = 0; k < 8; k++)
			 crc = crc & 1 ? (crc >> 1) ^ 0x04c011bb7 : crc >> 1;
	 }
	 return ~crc;

}

