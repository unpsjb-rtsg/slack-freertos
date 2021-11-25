#ifndef EXAMPLES_COMMON_COMMON_MBED_H_
#define EXAMPLES_COMMON_COMMON_MBED_H_

#include "mbed.h"

typedef int32_t xType[250][7];
typedef int32_t xType2[50][7];

extern xType *sdArray;
extern xType2 *sdArray2;
extern Serial pc;
extern DigitalOut leds[];
extern SemaphoreHandle_t xMutex;

/* The extern "C" is required to avoid name mangling between C and C++ code. */
extern "C"
{
// FreeRTOS callback/hook functions
void vApplicationTickHook(void);
void vApplicationMallocFailedHook( void );
void vApplicationStackOverflowHook( TaskHandle_t pxTask, char *pcTaskName );
}

/**
 * Periodic task.
 *
 * @param params
 */
void vCommonPeriodicTask( void* params );

/**
 * Aperiodic task.
 *
 * @param params
 */
void vCommonAperiodicTask( void* params );

#endif /* EXAMPLES_COMMON_COMMON_MBED_H_ */
