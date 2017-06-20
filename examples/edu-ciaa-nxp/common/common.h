#ifndef EXAMPLES_EDU_CIAA_NXP_COMMON_COMMON_H_
#define EXAMPLES_EDU_CIAA_NXP_COMMON_COMMON_H_

#include "sapi.h"         /* <= sAPI header */

extern gpioMap_t leds[];

char* itoa( int value, char* result, int base );
void prvSetupHardware( void );
void printSlacks( char s, int32_t * slackArray, TickType_t xCur );
void periodicTaskBody( void* params );

#endif /* EXAMPLES_EDU_CIAA_NXP_COMMON_COMMON_H_ */
