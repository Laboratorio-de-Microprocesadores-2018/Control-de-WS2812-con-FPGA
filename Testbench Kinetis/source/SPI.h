#ifndef SPI_H_
#define SPI_H_
#include "stdint.h"
#include "stdbool.h"

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
}SPI_OutputMode;
typedef enum{
	SPI_PinModeNormal,
	SPI_PinModeInput,
	SPI_PinModeOutput,
}SPI_PinMode;
typedef struct
{
	bool enableMaster;
	bool enableStopInWaitMode;
	SPI_ClockPolarity polarity;
	SPI_ClockPhase phase;
	SPI_ShiftDirection direction;
	SPI_OutputMode outputMode;
	SPI_PinMode pinMode;
	uint32_t baudRate;
}SPI_MasterConfig;

void SPI_MasterGetDefaultConfig(SPI_MasterConfig * config);

void SPI_MasterInit(SPI_MasterConfig * config);

void SPI_SendByte(uint8_t byte);

#endif /* SPI_H_ */
