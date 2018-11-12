#include "LedMatrix.h"
#include "SPI.h"
#include "DMA.h"
#include "DMAMUX.h"
#include "SysTick.h"
#include "Assert.h"
#include "string.h"
#include "GPIO.h"
#include "Board.h"
#include "Font.h"


/////////////////////////////////////////////////////////////////////////////////
//                       Constants and macro definitions                       //
/////////////////////////////////////////////////////////////////////////////////

#define TEXT_BUFFER_LEN 40
#define NUMBER_OF_LEDS (MATRIX_WIDTH * MATRIX_HEIGHT)
#define SCREEN_BUFFER_SIZE  (NUMBER_OF_LEDS * 3)
#define SCROLL_PERIOD 0.4

#define RESET_PIN PORTNUM2PIN(PC,3)
#define BUSY_PIN PORTNUM2PIN(PA,2)

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

static char textBuffer[TEXT_BUFFER_LEN+1];
static uint16_t textLength;
static uint16_t screenTextBufferLength;
static Color screenBuffer[NUMBER_OF_LEDS];
static Color screenTextBuffer[MATRIX_HEIGHT][(FONT_CHAR_WIDTH + FONT_SEPARATION)*(TEXT_BUFFER_LEN+1)];


static Command updateMode;


static bool colorTestRunning;
//static DMA_TransferConfig DMATransfer;
static ScrollMode scrollMode;
static bool scrolling;
static uint16_t scrollIndex;


// SPI transfer to write memory: |WRITE_MEMORY|STAR_ADDRESS| data... |
typedef struct{
	Command command;
	uint8_t startAddress;
	Color data[NUMBER_OF_LEDS];
}__attribute__((__packed__)) SPIFrame;

static uint8_t frame[3*NUMBER_OF_LEDS+2];


static void LedMatrix_WriteMemory();
static void LedMatrix_PartialWriteMemory(uint8_t startAddress, uint8_t length);
static void LedMatrix_SendUpdate();
static void LedMatrix_ScrollText();

/////////////////////////////////////////////////////////////////////////////////
//                          Global Function Definitions                        //
/////////////////////////////////////////////////////////////////////////////////

void LedMatrix_Init(void)
{
	sysTickInit();



	SPI_MasterConfig config;
	SPI_MasterGetDefaultConfig(&config);
	config.baudRate = SPI_fivePowerDelay;
	config.enableRxFIFOverflowOverwrite = false;
	config.disableTxFIFO = false;
	config.disableRxFIFO = false;
	config.chipSelectActiveState = SPI_PCSActiveLow;
	config.enableMaster = true;
	config.delayAfterTransferPreScale = SPI_DelayAfterTransferPreScaleOne;
	config.delayAfterTransfer = SPI_twoPowerDelay;
	config.continuousSerialCLK = false;		//CON ESTE HABILITO EL CONTINUOUS CLK
	config.bitsPerFrame = SPI_eightBitsFrame;
	config.polarity = SPI_ClockActiveHigh;
	config.phase = SPI_ClockPhaseFirstEdge;
	config.direction = SPI_FirstMSB;
	config.clockDelayScaler = SPI_twoPowerDelay;
	config.chipSelectToClkDelay = SPI_twoPowerDelay;


	SPI_MasterInit(SPI_0, &config);
	SPI_EnableEOQInterruptRequests(SPI_0);


	// Configure RESET and BUSY pins
	pinMode(RESET_PIN,OUTPUT);
	pinMode(BUSY_PIN,INPUT_PULLUP);

	// Reset pulse
	digitalWrite(RESET_PIN,1);
	uint16_t n=0xFFFF;
	while(n--);
	digitalWrite(RESET_PIN,0);



	updateMode = UPDATE_NORMAL;
	scrollIndex = 0;
	scrolling=false;
	sysTickAddCallback(LedMatrix_ScrollText,SCROLL_PERIOD);






/*
	DMA_Config DMAconfig;
	DMA_GetDefaultConfig(&DMAconfig);
	DMAconfig.enableDebugMode = true;
	DMA_Init(&DMAconfig);
	DMAMUX_Init();

	// Configure DMA0 to copy from screenBuffer to SPI transmitter
	DMAMUX_SetSource(0,DMAMUX_SPI0_TX);
	DMAMUX_EnableChannel(0,DMAMUX_Normal);

	DMATransfer.sourceAddress = (uint32_t)screenBuffer;
	DMATransfer.sourceOffset = 1;
	DMATransfer.sourceTransferSize = DMA_TransferSize1Bytes;

	DMATransfer.destinationAddress = (uint32_t)SPI_GetDataRegisterAddress(SPI_0);
	DMATransfer.destinationOffset = 0;
	DMATransfer.destinationTransferSize = DMA_TransferSize1Bytes;

	DMATransfer.majorLoopCounts = sizeof(screenBuffer)/sizeof(uint8_t);
	DMATransfer.minorLoopBytes = 1;
	DMATransfer.sourceLastAdjust = -1*sizeof(screenBuffer)/sizeof(uint8_t);
	DMATransfer.destinationLastAdjust = 0;
	DMA_SetTransferConfig(0,&DMATransfer);
*/

	//DMA_EnableChannelRequest (0);

	//DMA_SetCallback(0,LedMatrix_SendUpdate);

}

void LedMatrix_Mirror(bool b)
{
	if(b==true)
		updateMode = UPDATE_MIRRORED;
	else
		updateMode = UPDATE_NORMAL;
}

void LedMatrix_Clear(void)
{
	Color c = {0,0,0};
	for(int i=0; i<NUMBER_OF_LEDS; i++)
	{
		screenBuffer[i]   = c;
	}
	LedMatrix_WriteMemory();
}

bool LedMatrix_IsBusy()
{
	if(digitalRead(BUSY_PIN) == 1)
		return false;
	else
		return true;
}

void LedMatrix_PrintLed(uint8_t row, uint8_t col, Color c)
{
	uint8_t index = row*MATRIX_WIDTH + col;
	screenBuffer[index] = c;
	LedMatrix_PartialWriteMemory(index,1);

}

void LedMatrix_PrintLedInvalid(uint8_t row, uint8_t col, Color c)
{
	// Build frame
	frame[0] = WRITE_MEMORY;
	frame[1] = row*MATRIX_WIDTH + col;
	frame[2] = c.R;
	frame[3] = c.G;
	//frame[4] = c.B;

	SPI_SendFrame(frame,4,LedMatrix_SendUpdate);
}

void LedMatrix_PrintScreen(const Color * screen)
{
	for(int row=0; row<MATRIX_HEIGHT; row++)
	{
		for(int col=0; col<MATRIX_WIDTH; col++)
		{
			screenBuffer[col+row*MATRIX_WIDTH] = screen[col+row*MATRIX_WIDTH];
		}
	}
	LedMatrix_WriteMemory();
}

void LedMatrix_PlainColor(Color c)
{
	for(int i=0; i<NUMBER_OF_LEDS; i++)
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

void LedMatrix_Print(char * c, uint8_t len,Color fontColor, Color backgroundColor, ScrollMode mode)
{
	ASSERT(len < TEXT_BUFFER_LEN);

	if(mode == SCROLL_ONCE)
	{
		memcpy(textBuffer+1,c,len);

		len++;
		textBuffer[len] = ' ';
		len++;
		textBuffer[0] = ' ';
	}
	else
		memcpy(textBuffer,c,len);


	textLength = len;
	screenTextBufferLength = textLength*(FONT_CHAR_WIDTH+FONT_SEPARATION);
	scrollMode = mode;
	scrollIndex = 0;
	// Build  screen memory
	for(int i=0; i<len; i++)
	{
		for(int row=0; row<MATRIX_WIDTH; row++)
		{
			for(int col=0; col< FONT_CHAR_WIDTH; col++)
			{
				if(FONT_BIT(textBuffer[i], col, row) == true)
					screenTextBuffer[row][i*(FONT_CHAR_WIDTH + FONT_SEPARATION)+col]=fontColor;
				else
					screenTextBuffer[row][i*(FONT_CHAR_WIDTH + FONT_SEPARATION)+col]=backgroundColor;
			}

			for(int col= FONT_CHAR_WIDTH; col<FONT_CHAR_WIDTH + FONT_SEPARATION; col++)
				screenTextBuffer[row][i*(FONT_CHAR_WIDTH + FONT_SEPARATION)+col] = backgroundColor;
		}
	}
	scrolling=true;
}

static void LedMatrix_ScrollText()
{

	if(scrolling)
	{
		for(int row=0; row<MATRIX_HEIGHT; row++)
		{
			for(int col=0; col<MATRIX_WIDTH; col++)
			{
				screenBuffer[col+row*MATRIX_WIDTH] = screenTextBuffer[row][(scrollIndex+col)%screenTextBufferLength];
			}
		}
		LedMatrix_WriteMemory();


		scrollIndex = scrollIndex + 1;

		if(scrollMode == SCROLL_ONCE)
		{
			if(scrollIndex == screenTextBufferLength-8)
			{
				scrolling = false;
				scrollIndex = 0;
			}
		}
		else if(scrollMode == SCROLL_CONTINUOUS)
		{
			scrollIndex %= screenTextBufferLength;
		}


	}
}

void LedMatrix_StopScrolling()
{
	scrolling=false;
}
void LedMatrix_StartScrolling()
{
	scrolling=true;
}

static void LedMatrix_WriteMemory()
{
	LedMatrix_PartialWriteMemory(0,NUMBER_OF_LEDS);
}


static void LedMatrix_PartialWriteMemory(uint8_t startAddress, uint8_t length)
{
	ASSERT(startAddress < NUMBER_OF_LEDS);
	ASSERT(length <= NUMBER_OF_LEDS - startAddress);

	// Build frame
	frame[0] = WRITE_MEMORY;
	frame[1] = startAddress;
	for(int i=0; i<3*length-2;i+=3)
	{
		frame[2+i+0] = screenBuffer[startAddress+i/3].R;
		frame[2+i+1] = screenBuffer[startAddress+i/3].G;
		frame[2+i+2] = screenBuffer[startAddress+i/3].B;
	}

	SPI_SendFrame(frame,2 + length * 3,LedMatrix_SendUpdate);
	/*
	// Configure DMA transfer
	DMATransfer.sourceAddress = (uint32_t)(&frame);
	DMATransfer.majorLoopCounts = 2 + length * 3;
	DMA_SetTransferConfig(0,&DMATransfer);

	// Enable request to start transfer
	DMA_EnableChannelRequest(0);*/
}

static void LedMatrix_SendUpdate()
{
	// Build frame
	frame[0] = updateMode;
	SPI_SendFrame(frame,1,0);
}


