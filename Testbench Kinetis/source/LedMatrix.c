#include "LedMatrix.h"
#include "SPI.h"
#include "DMA.h"
#include "SysTick.h"
#include "Assert.h"
#include "string.h"

#define MAX_BUFFER_LEN 40


static char TEXT[MAX_BUFFER_LEN];

static uint8_t SCREEN[MATRIX_WIDTH * MATRIX_HEIGHT * 3];
static uint8_t screenBufferSize;
static uint8_t numberOfLeds;
static bool screenBusy;
static bool colorTestRunning;

void LedMatrix_Init()
{
	sysTickInit();

	numberOfLeds = MATRIX_WIDTH * MATRIX_HEIGHT;
	screenBufferSize =  numberOfLeds * 3;

}
void LedMatrix_Clear()
{

	//SPI_StartTransfer();
}
void LedMatrix_PlainColor(Color c)
{
	for(int i=0; i<numberOfLeds; i++)
	{
		SCREEN[i]   = c.G;
		SCREEN[i+1] = c.R;
		SCREEN[i+2] = c.B;
	}
	//SPI_StartTransfer();
}
void LedMatrix_ColorTest()
{

}

bool LedMatrix_ColorTestRunning()
{
	return colorTestRunning;
}

void LedMatrix_Print(char * c, uint8_t len,Color fontColor, Color backgroundColor)
{
	ASSERT(len < MAX_BUFFER_LEN);
	memcpy(c,TEXT,len);

}

