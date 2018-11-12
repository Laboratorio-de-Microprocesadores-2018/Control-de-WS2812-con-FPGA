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
#define SCROLL_PERIOD 0.17

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

// Back buffer of screen
static Color screenBuffer[NUMBER_OF_LEDS];


// Text buffer
static char textBuffer[TEXT_BUFFER_LEN+1];
// Length of text
static uint16_t textLength;


// Rendered text buffer
static Color screenTextBuffer[MATRIX_HEIGHT][(FONT_CHAR_WIDTH + FONT_SEPARATION)*(TEXT_BUFFER_LEN+1)];
// Rendered text buffer length
static uint16_t screenTextBufferLength;


// Update mode: normal or flipped
static Command updateMode;

// Scroll mode: once or continuous
static ScrollMode scrollMode;
// Flag of scrolling text
static bool scrolling;
// Index to rendered buffer
static uint16_t scrollIndex;


// SPI frame to send: |WRITE_MEMORY|STAR_ADDRESS| data... |
static uint8_t frame[3*NUMBER_OF_LEDS+2];

/////////////////////////////////////////////////////////////////////////////////
//                          Local functions declarations                       //
/////////////////////////////////////////////////////////////////////////////////

// Update entire memory
static void LedMatrix_WriteMemory();
// Update only a part of memory
static void LedMatrix_PartialWriteMemory(uint8_t startAddress, uint8_t length);
// Send update command
static void LedMatrix_SendUpdate();
// Periodic interrupt to scroll text
static void LedMatrix_ScrollText();

/////////////////////////////////////////////////////////////////////////////////
//                          Global Function Definitions                        //
/////////////////////////////////////////////////////////////////////////////////

void LedMatrix_Init(void)
{
	sysTickInit();

	// Configure SPI module
	SPI_MasterConfig config;
	SPI_MasterGetDefaultConfig(&config);
	config.baudRate = SPI_onePowerDelay;
	config.enableRxFIFOverflowOverwrite = false;
	config.disableTxFIFO = false;
	config.disableRxFIFO = false;
	config.chipSelectActiveState = SPI_PCSActiveLow;
	config.enableMaster = true;
	config.delayAfterTransferPreScale = SPI_DelayAfterTransferPreScaleOne;
	config.delayAfterTransfer = SPI_twoPowerDelay;
	config.continuousSerialCLK = false;
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

	// Generate reset pulse!!
	digitalWrite(RESET_PIN,1);
	uint16_t n=0xFFFF;
	while(n--);
	digitalWrite(RESET_PIN,0);

	updateMode = UPDATE_NORMAL;
	scrollIndex = 0;
	scrolling=false;

	// Add to systick periodic callback to scroll text
	sysTickAddCallback(LedMatrix_ScrollText,SCROLL_PERIOD);

}

void LedMatrix_Mirror(bool b)
{
	if(b==true)
		updateMode = UPDATE_MIRRORED;
	else
		updateMode = UPDATE_NORMAL;
	LedMatrix_WriteMemory();
}

void LedMatrix_Clear(void)
{
	Color c = {0,0,0};
	for(int i=0; i<NUMBER_OF_LEDS; i++)
		screenBuffer[i]   = c;
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

void LedMatrix_PrintScreen(Color * screen)
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
	////////////////////////////////////////////////////////////////////////////
	//  Colores estaticos
	static Color R = {10,0,0};
	static Color G = {0,10,0};
	static Color B = {0,0,10};
	static Color W = {100,100,100};

	////////////////////////////////////////////////////////////////////////////
	// Prueba led por led
	Color colors[3] = {R,G,B};
	LedMatrix_Clear();
	delayMs(100);
	for(int i=0; i<MATRIX_HEIGHT; i++)
	{
		for(int j=0; j<MATRIX_WIDTH; j++)
		{
			LedMatrix_PrintLed(i,j,colors[i%3]);
			delayMs(100);
		}
	}
	////////////////////////////////////////////////////////////////////////////
	// Prueba de colores y brillo creciente
	delayMs(1);
	for(int i=0; i<25; i++)
	{
		LedMatrix_PlainColor(R);
		delayMs(200);
		LedMatrix_PlainColor(G);
		delayMs(200);
		LedMatrix_PlainColor(B);
		delayMs(200);

		R.R +=2;
		G.G +=2;
		B.B +=2;
	}
	LedMatrix_PlainColor(W);
	delayMs(4000);
	////////////////////////////////////////////////////////////////////////////
	// Prueba de texto
	Color fontColor = {0,0,10};
	Color backgroundColor = {10,10,10};
	LedMatrix_Print("ITBA ", 5,fontColor,backgroundColor, SCROLL_CONTINUOUS);
	delayMs(20000);
	LedMatrix_StopScrolling();

	////////////////////////////////////////////////////////////////////////////

	// Prueba de texto
	LedMatrix_Mirror(true);
	LedMatrix_Print("ITBA ", 5,fontColor,backgroundColor, SCROLL_CONTINUOUS);
	delayMs(20000);
	LedMatrix_StopScrolling();
	LedMatrix_Mirror(false);
		////////////////////////////////////////////////////////////////////////////

	R.R = 20;
	G.G = 20;
	B.B = 20;

	Color screen1[8][8] ={{R,R,G,G,B,B,W,W},
								{R,R,G,G,B,B,W,W},
								{R,R,G,G,B,B,W,W},
								{R,R,G,G,B,B,W,W},
								{R,R,G,G,B,B,W,W},
								{R,R,G,G,B,B,W,W},
								{R,R,G,G,B,B,W,W},
								{R,R,G,G,B,B,W,W}};
	LedMatrix_PrintScreen(screen1[0]);
	delayMs(4000);

	Color screen2[8][8] ={{R,R,R,R,R,R,R,R},
								{R,R,R,R,R,R,R,R},
								{G,G,G,G,G,G,G,G},
								{G,G,G,G,G,G,G,G},
								{B,B,B,B,B,B,B,B},
								{B,B,B,B,B,B,B,B},
								{W,W,W,W,W,W,W,W},
								{W,W,W,W,W,W,W,W}};
	LedMatrix_PrintScreen(screen2[0]);
	delayMs(4000);


	////////////////////////////////////////////////////////////////////////////
	// Prueba de que comando invalido no actualiza la memoria
	// Imprimo todo blanco
	Color c = {10,10,10};
	LedMatrix_PlainColor(c);
	delayMs(1000);
	c.B=0;
	c.G=0;
	// Mando un comando invalido
	LedMatrix_PrintLedInvalid(3,3,c);

	delayMs(1000);
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
			scrollIndex %= screenTextBufferLength;
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
	if(LedMatrix_IsBusy()==false)
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
	}
}

static void LedMatrix_SendUpdate()
{
	// Build frame
	frame[0] = updateMode;
	SPI_SendFrame(frame,1,0);
}


