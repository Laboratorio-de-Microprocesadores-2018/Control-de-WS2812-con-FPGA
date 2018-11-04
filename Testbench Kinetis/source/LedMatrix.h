#ifndef LED_MATRIX_H_
#define LED_MATRIX_H_
#include "stdint.h"
#include "stdbool.h"

#define MATRIX_WIDTH 8
#define MATRIX_HEIGHT 8

typedef struct
{
	uint8_t R,G,B;
}Color;

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
void LedMatrix_PlainColor(Color c);

/**
 *
 */
void LedMatrix_ColorTest();

/**
 *
 */
bool LedMatrix_ColorTestRunning();

/**
 *
 */
void LedMatrix_Print(char * c, uint8_t len,Color fontColor, Color backgroundColor);

#endif /* LED_MATRIX_H_ */
