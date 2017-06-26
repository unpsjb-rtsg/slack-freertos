#ifndef EXAMPLES_LPC1768_COMMON_COMMON_H_
#define EXAMPLES_LPC1768_COMMON_COMMON_H_

#include "mbed.h"

extern Serial pc;
extern DigitalOut leds[];

/* The extern "C" is required to avoid name mangling between C and C++ code. */
extern "C"
{
// FreeRTOS callback/hook functions
void vApplicationMallocFailedHook( void );
void vApplicationStackOverflowHook( TaskHandle_t pxTask, char *pcTaskName );
}

void printSlacks( char s, int32_t * slackArray, TickType_t xCur );
void periodicTaskBody( void* params );

#endif /* EXAMPLES_LPC1768_COMMON_COMMON_H_ */
