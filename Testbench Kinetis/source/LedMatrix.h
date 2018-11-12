#ifndef LED_MATRIX_H_
#define LED_MATRIX_H_


/////////////////////////////////////////////////////////////////////////////////
//                             Included header files                           //
/////////////////////////////////////////////////////////////////////////////////

#include "stdint.h"
#include "stdbool.h"

/////////////////////////////////////////////////////////////////////////////////
//                       Constants and macro definitions                       //
/////////////////////////////////////////////////////////////////////////////////

#define MATRIX_WIDTH 8
#define MATRIX_HEIGHT 8

/////////////////////////////////////////////////////////////////////////////////
//                    Enumerations, structures and typedefs                    //
/////////////////////////////////////////////////////////////////////////////////

typedef struct
{
	uint8_t R,G,B;
}Color;

typedef enum{SCROLL_ONCE,SCROLL_CONTINUOUS}ScrollMode;

/////////////////////////////////////////////////////////////////////////////////
//                       		Global variables	                       	   //
/////////////////////////////////////////////////////////////////////////////////
/*
static const RED={10,0,0};
static const GREEN={};
static const BLUE={};
static const WHITE={};
static const BLACK={0,0,0};
static const RED={};
*/
/////////////////////////////////////////////////////////////////////////////////
//                           Global Function Declarations                      //
/////////////////////////////////////////////////////////////////////////////////

/**
 *
 */
void LedMatrix_Init();

/**
 *
 */
void LedMatrix_Clear();

/**
 *
 */
bool LedMatrix_IsBusy();

/**
 *
 */
void LedMatrix_Mirror(bool b);

/**
 *
 */
void LedMatrix_PrintLed(uint8_t row, uint8_t col, Color c);

/**
 *
 */
void LedMatrix_PrintLedInvalid(uint8_t row, uint8_t col, Color c);

/**
 *
 */
void LedMatrix_PrintScreen(Color * screen);

/**
 *
 */
void LedMatrix_PlainColor(Color c);

/**
 *
 */
void LedMatrix_ColorTest();

/**
 *
 */
void LedMatrix_Print(char * c, uint8_t len,Color fontColor, Color backgroundColor, ScrollMode mode);

/**
 *
 */
void LedMatrix_StopScrolling();

/**
 *
 */
void LedMatrix_StartScrolling();

#endif /* LED_MATRIX_H_ */
