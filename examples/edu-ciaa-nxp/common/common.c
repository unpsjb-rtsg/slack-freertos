/*****************************************************************************
 * Includes
 ****************************************************************************/
#include "FreeRTOS.h"
#include "task.h"
#include "slack.h"
#include "utils.h"
#include "common.h"

/*****************************************************************************
 * Macros and definitions
 ****************************************************************************/
/* None */

/*****************************************************************************
 * Private data declaration
 ****************************************************************************/
/* None */

/*****************************************************************************
 * Public data declaration
 ****************************************************************************/
/* None */

/*****************************************************************************
 * Private functions declaration
 ****************************************************************************/
/* None */

/*****************************************************************************
 * Private data
 ****************************************************************************/
/* None */

/*****************************************************************************
 * Public data
 ****************************************************************************/
/* None */

/*****************************************************************************
 * Private functions
 ****************************************************************************/
/* None */

/*****************************************************************************
 * Public functions
 ****************************************************************************/
/**
 * C++ version 0.4 char* style "itoa":
 * Written by Lukás Chmela
 * Released under GPLv3.
 */
char* itoa(int value, char* result, int base)
{
   // check that the base if valid
   if (base < 2 || base > 36) { *result = '\0'; return result; }

   char* ptr = result, *ptr1 = result, tmp_char;
   int tmp_value;

   do {
      tmp_value = value;
      value /= base;
      *ptr++ = "zyxwvutsrqponmlkjihgfedcba9876543210123456789abcdefghijklmnopqrstuvwxyz" [35 + (tmp_value - value * base)];
   } while ( value );

   // Apply negative sign
   if (tmp_value < 0) *ptr++ = '-';
   *ptr-- = '\0';
   while(ptr1 < ptr) {
      tmp_char = *ptr;
      *ptr--= *ptr1;
      *ptr1++ = tmp_char;
   }
   return result;
}
/*-----------------------------------------------------------*/

void vCommonSetupHardware(void)
{
	// Configure board.
	boardConfig();

	// Initialize GPIO.
	gpioConfig( 0, GPIO_ENABLE );

	// Output LEDs.
	gpioConfig( LEDR, GPIO_OUTPUT );
	gpioConfig( LEDG, GPIO_OUTPUT );
	gpioConfig( LEDB, GPIO_OUTPUT );
	gpioConfig( LED1, GPIO_OUTPUT );
	gpioConfig( LED2, GPIO_OUTPUT );
	gpioConfig( LED3, GPIO_OUTPUT );

   // Initialize UART @ 115200 bauds.
   uartConfig( UART_USB, 115200 );
}
/*-----------------------------------------------------------*/

void vCommonPrintSlacks( char s, int32_t * slackArray, TickType_t xCur )
{
	/* Buffer */
	static char uartBuff[10];

	vTaskSuspendAll();
	vTasksGetSlacks( slackArray );

	uartWriteString( UART_USB, pcTaskGetTaskName(NULL) );
    uartWriteByte( UART_USB, '\t' );

    uartWriteByte( UART_USB, s );
    uartWriteByte( UART_USB, '\t' );

    itoa( slackArray[0], uartBuff, 10 );
    uartWriteString( UART_USB, uartBuff );
    uartWriteByte( UART_USB, '\t' );

    int i;
    for( i = 2; i <= 5; i++ ) {
    	itoa( slackArray[i], uartBuff, 10 );
    	uartWriteString( UART_USB, uartBuff );
    	uartWriteByte( UART_USB, '\t' );
    }

    itoa( xCur, uartBuff, 10 );
    uartWriteString( UART_USB, uartBuff );
    uartWriteByte( UART_USB, '\t' );

    uartWriteString( UART_USB, "\n\r" );

	xTaskResumeAll();
}
/*-----------------------------------------------------------*/

void vCommonPeriodicTask( void* params )
{
    ( void ) params;

	SsTCB_t *pxTaskSsTCB;

#if( tskKERNEL_VERSION_MAJOR >= 10 )
	pxTaskSsTCB = getTaskSsTCB( NULL );
#endif

    int32_t slackArray[ 6 ];

	for(;;)
    {
#if defined( TRACEALYZER_v3_3_1 )
        vTracePrintF( slack_channel, "%d - %d", xSlackSD, pxTaskSsTCB->xSlack );
#endif

	    gpioWrite( leds[ pxTaskSsTCB->xId - 1], ON);

	    vCommonPrintSlacks( 'S', slackArray, pxTaskSsTCB->xCur );

#if ( configTASK_EXEC == 0 )
		vUtilsEatCpu( pxTaskSsTCB->xWcet - 300 );
#endif
#if ( configTASK_EXEC == 1 )
		while( pxTaskSsTCB->xCur <  ( pxTaskSsTCB->xWcet - 200 ) )
		{
			asm("nop");
		}
#endif

		vCommonPrintSlacks( 'E', slackArray, pxTaskSsTCB->xCur );

		gpioWrite( leds[ pxTaskSsTCB->xId - 1], OFF);

#if defined( TRACEALYZER_v3_3_1 )
		vTracePrintF( slack_channel, "%d - %d", xSlackSD, pxTaskSsTCB->xSlack );
#endif

		vTaskDelayUntil( &( pxTaskSsTCB->xPreviousWakeTime ), pxTaskSsTCB->xPeriod );
    }
}
/*-----------------------------------------------------------*/

