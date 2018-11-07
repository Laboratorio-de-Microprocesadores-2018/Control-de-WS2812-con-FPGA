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
	config.continuousSlaveSelection = true;
	SPI_MasterInit(SPI_0, &config);

	sysTickInit();
//	LedMatrix_Init();
}

void call(void)
{
	//static uint8_t arr[] = {4,5};
	//SPI_SendFrame(arr,sizeof(arr)/sizeof(arr[0]),0);
}
/** Función que se llama constantemente en un ciclo infinito */
void App_Run (void)
{
	static uint8_t arr[] = {1,2,3};
	static uint64_t time;
	if((millis()-time)>500)
	{
		time = millis();
		SPI_SendFrame(arr,sizeof(arr)/sizeof(arr[0]),call);
	}



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
