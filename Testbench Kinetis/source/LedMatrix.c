#include "LedMatrix.h"
#include "SPI.h"
#include "DMA.h"
#include "DMAMUX.h"
#include "SysTick.h"
#include "Assert.h"
#include "string.h"

/////////////////////////////////////////////////////////////////////////////////
//                       Constants and macro definitions                       //
/////////////////////////////////////////////////////////////////////////////////

#define TEXT_BUFFER_LEN 40
#define NUMBER_OF_LEDS (MATRIX_WIDTH * MATRIX_HEIGHT)
#define SCREEN_BUFFER_SIZE  (NUMBER_OF_LEDS * 3)

/////////////////////////////////////////////////////////////////////////////////
//                    Enumerations, structures and typedefs                    //
/////////////////////////////////////////////////////////////////////////////////
typedef enum {
	WRITE_MEMORY    = 0x07,
	UPDATE_NORMAL   = 0x10,
	UPDATE_MIRRORED = 0x11
}Command;

/////////////////////////////////////////////////////////////////////////////////
//                   Local variable definitions ('static')                     //
/////////////////////////////////////////////////////////////////////////////////

static char textBuffer[TEXT_BUFFER_LEN];
static Color screenBuffer[NUMBER_OF_LEDS];
static Command updateMode;
static bool screenBusy;
static bool colorTestRunning;
static DMA_TransferConfig DMATransfer;
static void LedMatrix_WriteMemory();
static void LedMatrix_PartialWriteMemory(uint8_t startAddress, uint8_t length);
static void LedMatrix_SendUpdate();

/////////////////////////////////////////////////////////////////////////////////
//                          Global Function Definitions                        //
/////////////////////////////////////////////////////////////////////////////////

void LedMatrix_Init(void)
{
	sysTickInit();

	DMA_Config DMAconfig;
	DMA_GetDefaultConfig(&DMAconfig);
	DMAconfig.enableDebugMode=false;
	DMA_Init(&DMAconfig);
	DMAMUX_Init();

	// Configure DMA0 to copy from screenBuffer to SPI transmitter
	DMAMUX_SetSource(0,DMAMUX_SPI0_TX);
	DMAMUX_EnableChannel(0,true);


	DMATransfer.sourceAddress = (uint32_t)screenBuffer;
	DMATransfer.sourceOffset = 1;
	DMATransfer.sourceTransferSize = DMA_TransferSize1Bytes;

	DMATransfer.destinationAddress = (uint32_t)0;//SPI_GetFIFOAddress();
	DMATransfer.destinationOffset = 0;
	DMATransfer.destinationTransferSize = DMA_TransferSize1Bytes;

	DMATransfer.majorLoopCounts = sizeof(screenBuffer)/sizeof(uint8_t);
	DMATransfer.minorLoopBytes = 1;
	DMATransfer.sourceLastAdjust = -1*sizeof(screenBuffer)/sizeof(uint8_t);
	DMATransfer.destinationLastAdjust = 0;
	DMA_SetTransferConfig(0,&DMATransfer);
	DMA_EnableChannelRequest (0);

	DMA_SetCallback(0,LedMatrix_SendUpdate);




	SPI_MasterConfig SPIConfig; // Con DMA deshabilitado
	SPI_MasterGetDefaultConfig(&SPIConfig);
	SPI_MasterInit(&SPIConfig);

}
void LedMatrix_Clear(void)
{

	//SPI_StartTransfer();
}
void LedMatrix_PlainColor(Color c)
{
	for(int i=0; i<NUMBER_OF_LEDS; i+=3)
	{
		screenBuffer[i]   = c;
	}
	LedMatrix_WriteMemory();
}
void LedMatrix_ColorTest(void)
{
	colorTestRunning = true;

	LedMatrix_WriteMemory();
}

bool LedMatrix_ColorTestRunning(void)
{
	return colorTestRunning;
}

void LedMatrix_Print(char * c, uint8_t len,Color fontColor, Color backgroundColor)
{
	ASSERT(len < TEXT_BUFFER_LEN);
	memcpy(c,textBuffer,len);

}

static void LedMatrix_WriteMemory()
{

	LedMatrix_PartialWriteMemory(0,NUMBER_OF_LEDS);
	// SPI_EnableDMAReqests();
}

static void LedMatrix_PartialWriteMemory(uint8_t startAddress, uint8_t length)
{
	ASSERT(startAddress < NUMBER_OF_LEDS);
	ASSERT(length < NUMBER_OF_LEDS - startAddress);

	SPI_SendByte(WRITE_MEMORY);
	SPI_SendByte(startAddress);

	DMATransfer.sourceAddress = screenBuffer + startAddress;
	DMATransfer.majorLoopCounts = length * 3;
	DMA_SetTransferConfig(0,&DMATransfer);
	// SPI_EnableDMAReqests();
}

static void LedMatrix_SendUpdate()
{
	//SPI_DisableDMAReqests();
	SPI_SendByte(updateMode);
}


