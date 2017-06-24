#include "FreeRTOS.h"

#define U_CEIL( x, y )    ( ( x / y ) + ( x % y != 0 ) )
#define U_FLOOR( x, y )   ( x / y )

void vUtilsEatCpu( UBaseType_t ticks );
