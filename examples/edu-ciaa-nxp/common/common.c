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
	boardInit();

	// Initialize UART @ 115200 bauds.
	uartConfig( UART_USB, 115200 );
}
/*-----------------------------------------------------------*/

void vCommonPrintSlacks( char s, int32_t * slackArray, SsTCB_t *pxTaskSsTCB )
{
    /* Buffer */
    static char uartBuff[50];
    vTaskSuspendAll();
    sprintf(uartBuff, "%s\t[%4d] %c\t%d\t%d\t%d\t%d\t%d\t%d\n\r",
            pcTaskGetTaskName(NULL), pxTaskSsTCB->uxReleaseCount, s,
            slackArray[0], slackArray[2], slackArray[3],
            slackArray[4], slackArray[5], pxTaskSsTCB->xCur);
    uartWriteString( UART_USB, uartBuff );
    xTaskResumeAll();
}
/*-----------------------------------------------------------*/

void vCommonPeriodicTask( void* params )
{
    ( void ) params;

    SsTCB_t *pxTaskSsTCB = pvSlackGetTaskSsTCB( NULL );

    int32_t slackArray[ 6 ];

    for(;;)
    {
        gpioWrite( leds[ pxTaskSsTCB->xId - 1], ON);

        vTasksGetSlacks( slackArray );

        vCommonPrintSlacks( 'S', slackArray, pxTaskSsTCB );

        vUtilsBusyWait( pxTaskSsTCB->xWcet - 300 );

        vTasksGetSlacks( slackArray );

        vCommonPrintSlacks( 'E', slackArray, pxTaskSsTCB );

        gpioWrite( leds[ pxTaskSsTCB->xId - 1], OFF);

        xTaskDelayUntil( &( pxTaskSsTCB->xPreviousWakeTime ), pxTaskSsTCB->xPeriod );
    }
}
/*-----------------------------------------------------------*/

