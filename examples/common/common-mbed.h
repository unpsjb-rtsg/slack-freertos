#ifndef EXAMPLES_COMMON_COMMON_MBED_H_
#define EXAMPLES_COMMON_COMMON_MBED_H_

#include "mbed.h"

extern Serial pc;
extern DigitalOut leds[];
extern SemaphoreHandle_t xMutex;

#ifdef TRACEALYZER_v3_1_3
extern traceString slack_channel;
#endif

/* The extern "C" is required to avoid name mangling between C and C++ code. */
extern "C"
{
// FreeRTOS callback/hook functions
void vApplicationMallocFailedHook( void );
void vApplicationStackOverflowHook( TaskHandle_t pxTask, char *pcTaskName );
}

void vCommonPeriodicTask( void* params );
void vCommonAperiodicTask( void* params );

#endif /* EXAMPLES_COMMON_COMMON_MBED_H_ */
