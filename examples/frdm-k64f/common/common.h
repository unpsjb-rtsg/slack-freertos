#ifndef EXAMPLES_FRDM_K64F_COMMON_COMMON_H_
#define EXAMPLES_FRDM_K64F_COMMON_COMMON_H_

#include "mbed.h"
#include "FreeRTOS.h"
#include "task.h"

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

void printSlacks( char s, int32_t * slackArray, TickType_t xCur );
void periodicTaskBody( void* params );
}

#endif /* EXAMPLES_FRDM_K64F_COMMON_COMMON_H_ */
