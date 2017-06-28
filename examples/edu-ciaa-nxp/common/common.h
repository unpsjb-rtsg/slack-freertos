#ifndef EXAMPLES_EDU_CIAA_NXP_COMMON_COMMON_H_
#define EXAMPLES_EDU_CIAA_NXP_COMMON_COMMON_H_

#include "sapi.h"         /* <= sAPI header */

extern gpioMap_t leds[];

#ifdef TRACEALYZER_v3_1_3
extern traceString slack_channel;
#endif

char* itoa( int value, char* result, int base );
void vCommonSetupHardware( void );
void vCommonPrintSlacks( char s, int32_t * slackArray, TickType_t xCur );
void vCommonPeriodicTask( void* params );

#endif /* EXAMPLES_EDU_CIAA_NXP_COMMON_COMMON_H_ */
