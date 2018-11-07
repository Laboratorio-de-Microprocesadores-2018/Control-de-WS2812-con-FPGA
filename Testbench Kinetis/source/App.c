/////////////////////////////////////////////////////////////////////////////////
//                     XXXXXXXX_GENERIC_XXXXXXXXX                              //
//          Grupo 3 - Laboratorio de Microprocesadores - ITBA - 2018           //
//	                                                                           //
/////////////////////////////////////////////////////////////////////////////////

/**
 * @file App.c
 * @brief Generic project.
 */

/////////////////////////////////////////////////////////////////////////////////
//                             Included header files                           //
/////////////////////////////////////////////////////////////////////////////////
//#include "LedMatrix.h"
#include "SysTick.h"
#include "SPI.h"
#include "DMA.h"
/////////////////////////////////////////////////////////////////////////////////
//                       Constants and macro definitions                       //
/////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////
//                    Enumerations, structures and typedefs                    //
/////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////
//                         Global variables definition                         //
/////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////
//                   Local variable definitions ('static')                     //
/////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////
//                   Local function prototypes ('static')                      //
/////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////
//                         Global function prototypes                          //
/////////////////////////////////////////////////////////////////////////////////

/** Función que se llama 1 vez, al comienzo del programa */
void App_Init (void)
{
	SPI_MasterConfig config;
	SPI_MasterGetDefaultConfig(&config);
	config.baudRate = SPI_tenPowerDelay;
	SPI_MasterInit(SPI_0, &config);
	DMA_Config DMAconfig;

//	DMA_GetDefaultConfig(&DMAconfig);
//	DMAconfig.enableDebugMode=false;
//	DMA_Init(&DMAconfig);
//	DMAMUX_Init();

//	LedMatrix_Init();
}

/** Función que se llama constantemente en un ciclo infinito */
void App_Run (void)
{
	uint8_t byte;
	byte = 0xAA;
	SPI_SendByte( byte);
	uint32_t n = 0xFFFFF;
	while(n--);


//	Color c;
//
//	c.R = 255;
//	c.G = 0;
//	c.B = 0;
//	LedMatrix_PlainColor(c);
//	delayMs(1000);
//
//	c.R = 0;
//	c.G = 255;
//	c.B = 0;
//	LedMatrix_PlainColor(c);
//	delayMs(1000);
//
//	c.R = 0;
//	c.G = 0;
//	c.B = 255;
//	LedMatrix_PlainColor(c);
//	delayMs(1000);
//
//	LedMatrix_ColorTest();
//	while(LedMatrix_ColorTestRunning());
//
//	char text[] = "Hello world!";
//
//	Color fontColor = {.R=0,.G=0, .B=0};
//	Color backgroundColor = {.R=0,.G=0,.B=255};
//	LedMatrix_Print(text,sizeof(text)/sizeof(char),fontColor,backgroundColor);
//
//	while(1);

}
