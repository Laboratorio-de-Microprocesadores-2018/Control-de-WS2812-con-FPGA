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
#include "LedMatrix.h"
#include "SysTick.h"
#include "SPI.h"
#include "DMA.h"
#include "CPUTimeMeasurement.h"
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
	// Initialization of test pin
	#ifdef MEASURE_CPU_TIME
		MEASURE_CPU_TIME_PORT->PCR[MEASURE_CPU_TIME_PIN] = PORT_PCR_MUX(1);
		MEASURE_CPU_TIME_GPIO->PDDR |= (1<<MEASURE_CPU_TIME_PIN);
		MEASURE_CPU_TIME_GPIO->PDOR &= ~(1<<MEASURE_CPU_TIME_PIN);
	#endif

	/*
	SPI_MasterConfig config;
	SPI_MasterGetDefaultConfig(&config);
	config.baudRate = SPI_tenPowerDelay;
	SPI_MasterInit(SPI_0, &config);
	SPI_EnableEOQInterruptRequests(SPI_0);
	*/
	sysTickInit();
	LedMatrix_Init();
}
void sendUpdate()
{
	static uint8_t arr[] = {4,5};
	SPI_SendFrame(arr,sizeof(arr)/sizeof(arr[0]),0);
}


/** Función que se llama constantemente en un ciclo infinito */
void App_Run (void)
{
	static const Color R = {10,0,0};
	static const Color G = {0,10,0};
	static const Color B = {0,0,10};
	static const Color W = {10,10,10};
	const Color screen[8][8] ={ 	{W,G,G,G,G,G,G,W},
									{R,G,G,G,G,G,G,B},
									{R,G,G,G,G,G,G,B},
									{R,G,G,G,G,G,G,B},
									{R,G,G,G,G,G,G,B},
									{R,G,G,G,G,G,G,B},
									{R,G,G,G,G,G,G,B},
									{W,G,G,G,G,G,G,W}
								};
	//LedMatrix_Mirror(true);
	LedMatrix_PrintScreen(screen[0]);
	delayMs(40);
	//LedMatrix_PrintLed(0,7,R);
	delayMs(40);
	//LedMatrix_PrintLed(7,7,G);
	while(1);


	Color fontColor = {30,30,30};
	Color backgroundColor ={0,0,10};

	//LedMatrix_Mirror(true);
	LedMatrix_Print("ITBA", 4,fontColor,backgroundColor, SCROLL_CONTINUOUS);

	while(1);

/*
	Color c;

	c.R = 255;
	c.G = 0;
	c.B = 0;


	if(LedMatrix_IsBusy()==false)
	{
		LedMatrix_Clear();
		delayMs(4000);
	}


	if(LedMatrix_IsBusy()==false)
	{
		c.R = 10;
		c.G = 0;
		c.B = 0;
		LedMatrix_PlainColor(c);
			delayMs(4000);
	}

	if(LedMatrix_IsBusy()==false)
	{
		c.R = 0;
		c.G = 10;
		c.B = 0;
		LedMatrix_PlainColor(c);
			delayMs(4000);
	}

	if(LedMatrix_IsBusy()==false)
	{
		c.R = 0;
		c.G = 0;
		c.B = 10;
		LedMatrix_PlainColor(c);
			delayMs(4000);
	}

	if(LedMatrix_IsBusy()==false)
	{
		c.R = 10;
		c.G = 10;
		c.B = 0;
		LedMatrix_PrintLed(3,3,c);
		delayMs(1000);
	}

	if(LedMatrix_IsBusy()==false)
	{
		c.R = 20;
		c.G = 20;
		c.B = 0;
		LedMatrix_PrintLed(3,4,c);
		delayMs(1000);
	}

	if(LedMatrix_IsBusy()==false)
	{
		c.R = 30;
		c.G = 30;
		c.B = 0;
		LedMatrix_PrintLed(3,5,c);
		delayMs(1000);
	}

*/


/*
	c.R = 0;
	c.G = 30;
	c.B = 0;
	if(LedMatrix_IsBusy()==false)
		LedMatrix_PlainColor(c);
	delayMs(4000);

*/





	/*
	c.R = 0;
	c.G = 0;
	c.B = 255;
	LedMatrix_PlainColor(c);
	delayMs(4000);
	*/

	//LedMatrix_ColorTest();
	//while(LedMatrix_ColorTestRunning());

	//char text[] = "Hello world!";

	//Color fontColor = {.R=0,.G=0, .B=0};
	//Color backgroundColor = {.R=0,.G=0,.B=255};
	//LedMatrix_Print(text,sizeof(text)/sizeof(char),fontColor,backgroundColor);

	//while(1);


}
