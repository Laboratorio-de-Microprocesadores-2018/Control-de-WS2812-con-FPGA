#include "SPI.h"
#include "hardware.h"
#include "Assert.h"
#include "GPIO.h"
#include "PORT.h"
#include "CircularBuffer.h"
#include "stdlib.h"
#include "CPUTimeMeasurement.h"
#define BUFFER_SIZE					256

static SPI_Type * SPIs[] = SPI_BASE_ADDRS;


// Bytes left in current transfer
static uint8_t bytesLeft;

static SPI_Callback transferCallback;

NEW_CIRCULAR_BUFFER(transmitBuffer,BUFFER_SIZE,sizeof(uint8_t));
NEW_CIRCULAR_BUFFER(recieveBuffer,BUFFER_SIZE,sizeof(uint8_t));


void SPI_ClearTxFIFO(SPI_Instance n);
void SPI_StartCountingTxFIFO(SPI_Instance n);
void SPI_HaltModule(SPI_Instance n);
void SPI_RunModule(SPI_Instance n);
void SPI_SetEOQ(SPI_Instance n);
void SPI_ClearEOQ(SPI_Instance n);



void SPI_MasterGetDefaultConfig(SPI_MasterConfig * config)
{
//	MCR config
	config->enableRxFIFOverflowOverwrite = false;
	config->disableTxFIFO = false;
	config->disableRxFIFO = false;
	config->chipSelectActiveState = SPI_PCSActiveLow;
	config->enableMaster = true;
	config->delayAfterTransferPreScale = SPI_DelayAfterTransferPreScaleOne;
	config->delayAfterTransfer = SPI_eightPowerDelay;
	config->continuousSerialCLK = false;		//CON ESTE HABILITO EL CONTINUOUS CLK


//	CTAR config
	config->bitsPerFrame = SPI_eightBitsFrame;
	config->polarity = SPI_ClockActiveHigh;
	config->phase = SPI_ClockPhaseSecondEdge;
	config->direction = SPI_FirstLSB;
	config->clockDelayScaler = SPI_eightPowerDelay;
	config->baudRate = SPI_twoPowerDelay;
	config->chipSelectToClkDelay = SPI_eightPowerDelay;


//	config->enableStopInWaitMode;
//	config->outputMode;
//	config->pinMode;

}

void SPI_MasterInit(SPI_Instance n, SPI_MasterConfig * config)
{
	ASSERT(n<FSL_FEATURE_SOC_DSPI_COUNT);
	ASSERT((config->CTARUsed)<FSL_FEATURE_DSPI_CTAR_COUNT);

	// Enable clock gating and NVIC
	if(n==SPI_0)
	{
		SIM->SCGC6 |= SIM_SCGC6_SPI0_MASK;
		NVIC_EnableIRQ(SPI0_IRQn);
	}
	else if (n==SPI_1)
	{
		SIM->SCGC6 |= SIM_SCGC6_SPI1_MASK;
		NVIC_EnableIRQ(SPI1_IRQn);
	}
	else if (n==SPI_2)
	{
		SIM->SCGC3 |= SIM_SCGC3_SPI2_MASK;
		NVIC_EnableIRQ(SPI2_IRQn);
	}
	// Check if the module is in stop state
	ASSERT((SPIs[n]->SR & SPI_SR_TXRXS_MASK) != SPI_SR_TXRXS_MASK);


	if (config->enableMaster)
	{
	//	Clock and transfer attributes register (CTAR ON MASTER MODE)
		SPIs[n]->CTAR[config->CTARUsed] =

				SPI_CTAR_FMSZ(config->bitsPerFrame) |
				SPI_CTAR_CPOL(config->polarity) |
				SPI_CTAR_CPHA(config->phase) |
				SPI_CTAR_LSBFE(config->direction) |

				SPI_CTAR_PCSSCK(1) |
				SPI_CTAR_CSSCK(config->chipSelectToClkDelay) |  // PCS to SCK Delay Scaler: t CSC = (1/fP ) x PCSSCK x CSSCK.

				SPI_CTAR_PASC(1) |
				SPI_CTAR_ASC(config->clockDelayScaler) |		//After SCK Delay Scaler: tASC = (1/fP) x PASC x ASC

				SPI_CTAR_PDT(3)|//config->delayAfterTransferPreScale) |
				SPI_CTAR_DT(config->delayAfterTransfer) |  //Delay After Transfer Scaler: tDT = (1/fP ) x PDT x DT

				SPI_CTAR_DBR(0)|
				SPI_CTAR_PBR(0) |
				SPI_CTAR_BR(config->baudRate);  // Baud Rate Scaler: SCK baud rate = (fP /PBR) x [(1+DBR)/BR]
	}else
	{
	//	Clock and transfer attributes register (CTAR ON SLAVE MODE)
	//	SPIs[n]->CTAR_SLAVE
	}

	//SPI_HaltModule(n);
//	Module configuration register (MCR)
//	No estan configurados: MTFE, DOZE, SMPL_PT
	// Enable the module clocks
	SPIs[n]->MCR &= ~SPI_MCR_MDIS_MASK;
	SPIs[n]->MCR =
			SPI_MCR_MSTR(1) |
			SPI_MCR_CONT_SCKE(config->continuousSerialCLK) |		//ACA HABILITO O DESHABILITO EL CONTINUOUS CLK
			SPI_MCR_DCONF(0) |
			SPI_MCR_FRZ(1) |
			SPI_MCR_MTFE(0) |
			SPI_MCR_PCSSE(0) |
			SPI_MCR_ROOE(config->enableRxFIFOverflowOverwrite) |
			SPI_MCR_PCSIS(config->chipSelectActiveState) |
			SPI_MCR_DOZE(0) |
			SPI_MCR_MDIS(0) |
			SPI_MCR_DIS_TXF(config->disableTxFIFO) |
			SPI_MCR_DIS_RXF(config->disableRxFIFO) |
			SPI_MCR_HALT(1);

	//SPI_RunModule(n);
//	PUSH Tx FIFO register in master mode
	//SPIs[n]->PUSHR = SPI_PUSHR_CONT(config->continuousChipSelect) | // Return CS signal to inactive state between transfers.
	//		SPI_PUSHR_CTAS(config->CTARUsed) |
//			SPI_PUSHR_PCS(1<<config->PCSSignalSelect);

	//SPIs[n]->MCR |= SPI_MCR_MSTR(config->enableMaster);

	pinMode(PORTNUM2PIN(PC,5),OUTPUT);
	PORT_Config portConfig;
	PORT_GetPinDefaultConfig(&portConfig);
	PORT_PinConfig(PORT_D, 0, &portConfig); // CS
	PORT_PinConfig(PORT_D, 1, &portConfig); // SCK
	PORT_PinConfig(PORT_D, 2, &portConfig); // SOUT
	PORT_PinConfig(PORT_D, 3, &portConfig); // SIN
	PORT_PinMux(PORT_D, 0, PORT_MuxAlt2);
	PORT_PinMux(PORT_D, 1, PORT_MuxAlt2);
	PORT_PinMux(PORT_D, 2, PORT_MuxAlt2);
	PORT_PinMux(PORT_D, 3, PORT_MuxAlt2);
}

void SPI_EnableTxFIFOFillDMARequests(SPI_Instance n)
{
	SPIs[n]->RSER |= SPI_RSER_TFFF_RE_MASK | SPI_RSER_TFFF_DIRS_MASK;
}
void SPI_EnableTxFIFOFillInterruptRequests(SPI_Instance n)
{
	SPIs[n]->RSER |= SPI_RSER_TFFF_RE_MASK;
	SPIs[n]->RSER &= ~SPI_RSER_TFFF_DIRS_MASK;
}
void SPI_DisableTxFIFOFillRequests(SPI_Instance n)
{
	SPIs[n]->RSER &= ~SPI_RSER_TFFF_RE_MASK;
}

void SPI_EnableEOQInterruptRequests(SPI_Instance n)
{
	SPIs[n]->RSER |= SPI_RSER_EOQF_RE_MASK;
}
void SPI_DisableEOQInterruptRequests(SPI_Instance n)
{
	SPIs[n]->RSER &= ~SPI_RSER_EOQF_RE_MASK;
}

void SPI_HaltModule(SPI_Instance n)
{
	SPIs[n]->MCR |= SPI_MCR_HALT_MASK;
}
void SPI_RunModule(SPI_Instance n)
{
	SPIs[n]->MCR &= ~SPI_MCR_HALT_MASK;
}
void SPI_ClearTxFIFO(SPI_Instance n)
{
	SPIs[n]->MCR |= SPI_MCR_CLR_TXF_MASK;
}
void SPI_StartCountingTxFIFO(SPI_Instance n)
{
	SPIs[n]->MCR &= ~SPI_MCR_CLR_TXF_MASK;
}
void SPI_SetEOQ(SPI_Instance n)
{
	SPIs[n]->MCR |= SPI_PUSHR_EOQ_MASK;
}
void SPI_ClearEOQ(SPI_Instance n)
{
	SPIs[n]->MCR &= ~SPI_PUSHR_EOQ_MASK;
}


uint32_t SPI_GetDataRegisterAddress(SPI_Instance n)
{
	return SPIs[n]->PUSHR;
}

bool SPI_SendByte( uint8_t byte)
{
	if(push(&transmitBuffer, &byte))
	{
		// Enable interrupts
		SPI_EnableTxFIFOFillInterruptRequests(SPI_0);
		return true;
	}
	else return false;
}



int SPI_SendFrame(uint8_t * data, uint8_t length, SPI_Callback callback)
{


	ASSERT(data!=NULL);
	ASSERT(length<spaceLeft(&transmitBuffer));

	int bytesSent = 0;
	for(int i=0; i< length; i++)
		if(push(&transmitBuffer, data+i)==false)
		{
			bytesSent = i;
			break;
		}

	if(bytesSent == 0) // If didn't break..
		bytesSent = length;

	// Store bytes left
	bytesLeft = bytesSent;

	transferCallback = callback;

	// Enable interrupts to start copying bytes from circular buffer to SPI module
	SPI_ClearTxFIFO(0);
	SPI_EnableTxFIFOFillInterruptRequests(SPI_0);
	return bytesSent;
}

bool SPI_ReceiveByte(uint8_t * byte)
{
	return pop(&recieveBuffer, byte);
}


void SPI0_IRQHandler(void)
{
	/// If EOQF bit is set, transmission has ended
	if((SPIs[0]->SR & SPI_SR_EOQF_MASK) == SPI_SR_EOQF_MASK)
	{SET_TEST_PIN;
		//SPIs[0]->MCR |= SPI_MCR_HALT_MASK;
		SPIs[0]->SR |= SPI_SR_EOQF_MASK; // Clear EOQF flag


		// Call user callback
		if(transferCallback!=NULL)
			transferCallback();
		CLEAR_TEST_PIN;

	}
	// If TFFF bit is set there is space in Tx FIFO
	if((SPIs[0]->SR & SPI_SR_TFFF_MASK) == SPI_SR_TFFF_MASK)
	{
		SPIs[0]->SR |= SPI_SR_TFFF_MASK;
		// If HALT bit is set, clear it to start transfer
		if((SPIs[0]->MCR & SPI_MCR_HALT_MASK) == SPI_MCR_HALT_MASK)
		{
			SPIs[0]->MCR &= ~SPI_MCR_HALT_MASK;
		}

		// Fill FIFO
		while(((SPIs[0]->SR&SPI_SR_TXCTR_MASK)>>SPI_SR_TXCTR_SHIFT) < 4 && numel(&transmitBuffer)>0 )
		{
			uint8_t byte;
			if(pop(&transmitBuffer, &byte))
			{
				int b=0;
				if(byte == 5)
					b=5;
				// If its last byte, set end of queue bit
				if(isEmpty(&transmitBuffer))
				{
					SPIs[0]->PUSHR = SPI_PUSHR_CONT(0) |
								SPI_PUSHR_CTAS(0) |
								SPI_PUSHR_EOQ(1) |
								SPI_PUSHR_PCS(1) |
								SPI_PUSHR_TXDATA(byte);
					SPI_DisableTxFIFOFillRequests(SPI_0);
				}
				else
					SPIs[0]->PUSHR = SPI_PUSHR_CONT(1) |
								SPI_PUSHR_CTAS(0) |
								SPI_PUSHR_EOQ(0) |
								SPI_PUSHR_PCS(1) |
								SPI_PUSHR_TXDATA(byte);
			}
		}

	}

}
