#include "SPI.h"
#include "hardware.h"
#include "Assert.h"
#include "GPIO.h"
#include "PORT.h"

#define BUFFER_SIZE					100

static SPI_Type * SPIs[] = SPI_BASE_ADDRS;

NEW_CIRCULAR_BUFFER(transmitBuffer,BUFFER_SIZE,sizeof(uint8_t));
NEW_CIRCULAR_BUFFER(recieveBuffer,BUFFER_SIZE,sizeof(uint8_t));


void SPI_MasterGetDefaultConfig(SPI_MasterConfig * config)
{
	config->CTARUsed = SPI_CTAR_0;
	config->PCSSignalSelect = SPI_PCS_0;
	config->baudRate = SPI_twoPowerDelay;
	config->bitsPerFrame = SPI_eightBitsFrame;
	config->chipSelectActiveState = SPI_PCSActiveHigh;
	config->clockDelayScaler = SPI_twoPowerDelay;
	config->direction = SPI_FirstLSB;
	config->enableMaster = true;
	config->enableRxFIFO = true;
	config->enableShiftRegister = true;
//	config->enableStopInWaitMode;
	config->enableTxFIFO = true;
//	config->outputMode;
	config->phase = SPI_ClockPhaseSecondEdge;
//	config->pinMode;
	config->polarity = SPI_ClockActiveHigh;
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

	if((SPIs[n]->SR & SPI_SR_TXRXS_MASK) != SPI_SR_TXRXS_MASK)
	{
		if (config->enableMaster)
		{
		//	Clock and transfer attributes register (CTAR ON MASTER MODE)
			SPIs[n]->CTAR[config->CTARUsed] = SPI_CTAR_FMSZ(config->bitsPerFrame) |
					SPI_CTAR_CPOL(config->polarity) |
					SPI_CTAR_CPHA(config->phase) |
					SPI_CTAR_LSBFE(config->direction) |
					SPI_CTAR_PCSSCK(0) | SPI_CTAR_CSSCK(config->clockDelayScaler) |
					SPI_CTAR_PASC(0) | SPI_CTAR_ASC(config->clockDelayScaler) |
					SPI_CTAR_PDT(1) | SPI_CTAR_DT(config->clockDelayScaler) |
					SPI_CTAR_PBR(0) | SPI_CTAR_BR(config->baudRate);
		}else
		{
		//	Clock and transfer attributes register (CTAR ON SLAVE MODE)
//			SPIs[n]->CTAR_SLAVE
		}
	}


//	Module configuration register (MCR)
//	No estan configurados: FRZ, MTFE, DOZE, SMPL_PT
//	SPIs[n]->MCR = SPI_MCR_MSTR(config->enableMaster)
	SPIs[n]->MCR = SPI_MCR_CONT_SCKE(false) |		//ACA HABILITO O DESHABILITO EL CONTINUOUS CLK
			SPI_MCR_PCSSE(false) |
			SPI_MCR_ROOE(config->enableShiftRegister) |
			SPI_MCR_PCSIS(config->chipSelectActiveState) |
			SPI_MCR_MDIS(false) |
			SPI_MCR_DIS_TXF(config->enableTxFIFO) |
			SPI_MCR_DIS_RXF(config->enableRxFIFO);
	SPIs[n]->MCR = SPI_MCR_MSTR(config->enableMaster);
//	SPIs[n]->MCR &= !SPI_MCR_HALT_MASK;



//	DMA/Interrupt request select and enable register (RSER)
//	SPIs[n]->RSER = ;

//	PUSH Tx FIFO register in master mode
	SPIs[n]->PUSHR = SPI_PUSHR_CONT(false) |
			SPI_PUSHR_CTAS(config->CTARUsed) |
			true << (SPI_PUSHR_PCS_SHIFT + config->PCSSignalSelect);


	pinMode(PORTNUM2PIN(PC,5),OUTPUT);
	PORT_Config portConfig;
	PORT_GetPinDefaultConfig(&portConfig);
	PORT_PinConfig(PORT_D,0, &portConfig);
	PORT_PinConfig(PORT_D, 1, &portConfig);
	PORT_PinConfig(PORT_D, 2, &portConfig);
	PORT_PinConfig(PORT_D, 3, &portConfig);
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
	SPIs[n]->RSER &= !SPI_RSER_TFFF_RE_MASK;
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

bool SPI_ReceiveByte(uint8_t * byte)
{
	return pop(&recieveBuffer, byte);
}


void SPI0_IRQHandler(void)
{
	if((SPIs[0]->SR & SPI_SR_EOQF_MASK) != SPI_SR_EOQF_MASK)
	{
		uint8_t byte;
		if((SPIs[0]->MCR & SPI_MCR_HALT_MASK) == SPI_MCR_HALT_MASK)
		{
			SPIs[0]->MCR &= !SPI_MCR_HALT_MASK;
		}
		if(((SPIs[0]->SR & SPI_SR_TFFF_MASK) == SPI_SR_TFFF_MASK) && (pop(&transmitBuffer, &byte)))
		{
			SPIs[0]->PUSHR = byte;
		}
		SPI_DisableTxFIFOFillRequests(SPI_0);
	}else
		SPIs[0]->SR |= SPI_SR_EOQF_MASK;

}
