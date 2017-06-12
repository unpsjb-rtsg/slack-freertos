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

// Slack Stealing
#if( configUSE_SLACK_STEALING == 1 )
void vApplicationDebugAction( void *param );
void vApplicationNotSchedulable( void );
void vApplicationDeadlineMissedHook( char *pcTaskName, UBaseType_t uxRelease, TickType_t xTickCount );
#endif
}

#endif /* EXAMPLES_LPC1768_COMMON_COMMON_H_ */
