#ifndef SPI_H_
#define SPI_H_

#include "stdint.h"
#include "stdbool.h"


typedef enum {SPI_0, SPI_1, SPI_2}SPI_Instance;
typedef enum {SPI_CTAR_0, SPI_CTAR_1}SPI_CTAR;

typedef enum {SPI_PCS_0, SPI_PCS_1, SPI_PCS_2, SPI_PCS_3, SPI_PCS_4, SPI_PCS_5}SPI_PCSignal;

typedef enum {
	SPI_PCSActiveLow,
	SPI_PCSActiveHigh,
}SPI_ChipSelectActiveState;

typedef enum {
	SPI_fourBitsFrame = 3, SPI_fiveBitsFrame, SPI_sixBitsFrame, SPI_sevenBitsFrame, SPI_eightBitsFrame,
	SPI_nineBitsFrame, SPI_tenBitsFrame, SPI_elevenBitsFrame, SPI_twelveBitsFrame, SPI_thirteenBitsFrame,
	SPI_fourteenBitsFrame, SPI_fifteenBitsFrame, SPI_sixteenBitsFrame
}SPI_BitsPerFrame;

typedef enum {
	SPI_onePowerDelay, SPI_twoPowerDelay, SPI_threePowerDelay, SPI_fourPowerDelay, SPI_fivePowerDelay,
	SPI_sixPowerDelay, SPI_sevenPowerDelay, SPI_eightPowerDelay, SPI_ninePowerDelay, SPI_tenPowerDelay
}SPI_CLKDelayScaler;

typedef enum {
	SPI_ClockActiveHigh,
	SPI_ClockActiveLow,
}SPI_ClockPolarity;

typedef enum{
	SPI_ClockPhaseFirstEdge,
	SPI_ClockPhaseSecondEdge,
}SPI_ClockPhase;

typedef enum{
	SPI_FirstMSB,
	SPI_FirstLSB,
}SPI_ShiftDirection;

typedef enum{
	SPI_SlaveSelectAsGPIO,
	SPI_SlaveSelectFaultPin,
	SPI_SlaveSelectAutomaticOutput,
}SPI_OutputMode;	//Not used

typedef enum{
	SPI_PinModeNormal,
	SPI_PinModeInput,
	SPI_PinModeOutput,
}SPI_PinMode;		//Not used

typedef struct
{

	bool enableMaster, enableStopInWaitMode, enableShiftRegister, disableTxFIFO, disableRxFIFO, continuousSerialCLK;
	SPI_CTAR CTARUsed;
	SPI_PCSignal PCSSignalSelect;
	SPI_BitsPerFrame bitsPerFrame;
	SPI_CLKDelayScaler clockDelayScaler;
	SPI_ChipSelectActiveState chipSelectActiveState;
	SPI_ClockPolarity polarity;
	SPI_ClockPhase phase;
	SPI_ShiftDirection direction;
	SPI_OutputMode outputMode;
	SPI_PinMode pinMode;
	uint32_t baudRate;
}SPI_MasterConfig;

typedef void (*SPI_Callback)(void);

void SPI_MasterGetDefaultConfig(SPI_MasterConfig * config);
void SPI_MasterInit(SPI_Instance n, SPI_MasterConfig * config);

void SPI_EnableTxFIFOFillDMARequests(SPI_Instance n);
void SPI_EnableTxFIFOFillInterruptRequests(SPI_Instance n);
void SPI_DisableTxFIFOFillRequests(SPI_Instance n);

uint32_t SPI_GetDataRegisterAddress(SPI_Instance n);

bool SPI_SendByte( uint8_t byte);

int SPI_SendFrame(uint8_t * data, uint8_t length, SPI_Callback callback);

#endif /* SPI_H_ */
