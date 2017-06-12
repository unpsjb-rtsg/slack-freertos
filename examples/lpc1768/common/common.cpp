#include "FreeRTOS.h"
#include "task.h"
#include "slack.h"
#include "common.h"
#include "utils.h"

void printSlacks( char s, int32_t * slackArray, TickType_t xCur )
{
	vTaskSuspendAll();
	vTasksGetSlacks( slackArray );
	pc.printf("%s\t%c\t%d\t%d\t%d\t%d\t%d\t%d\t%d\n",
			pcTaskGetTaskName(NULL), s,
			slackArray[0], slackArray[2], slackArray[3],
			slackArray[4], slackArray[5], slackArray[6],
			xCur);
	xTaskResumeAll();
}

void periodicTaskBody( void* params )
{
	SsTCB_t *pxTaskSsTCB = getTaskSsTCB( NULL );

    int32_t slackArray[ 7 ];

	for(;;)
    {
		printSlacks( 'S', slackArray, pxTaskSsTCB->xCur );

		leds[ pxTaskSsTCB->xId - 1] = 1;

#if ( configTASK_EXEC == 0 )
		vUtilsEatCpu( pxTaskSsTCB->xWcet - 250 );
#endif
#if ( configTASK_EXEC == 1 )
		while( pxTaskSsTCB->xCur <  pxTaskSsTCB->xWcet )
		{
			asm("nop");
		}
#endif

		leds[ pxTaskSsTCB->xId - 1] = 0;

		printSlacks( 'E', slackArray, pxTaskSsTCB->xCur );

		vTaskDelayUntil( &( pxTaskSsTCB->xPreviousWakeTime ), pxTaskSsTCB->xPeriod );
    }
}

void vApplicationMallocFailedHook( void )
{
	taskDISABLE_INTERRUPTS();

    pc.printf( "Malloc failed\r\n" );

	for( ;; )
	{
        leds[ 2 ] = 1;
        wait_ms(1000);
        leds[ 2 ] = 0;
        wait_ms(500);
	}
}

void vApplicationStackOverflowHook( TaskHandle_t pxTask, char *pcTaskName )
{
	( void ) pxTask;

	taskDISABLE_INTERRUPTS();

	pc.printf( "%s\tStack overflow\r\n", pcTaskName );

	for( ;; )
	{
        leds[ 2 ] = 1;
        wait_ms(500);
        leds[ 2 ] = 0;
        wait_ms(500);
	}
}

#if ( configUSE_SLACK_STEALING == 1 )
void vApplicationDebugAction( void *param )
{
    ( void ) param;

	taskDISABLE_INTERRUPTS();

	for( ;; )
	{
        leds[ 4 ] = 1;
        wait_ms(1000);
        leds[ 4 ] = 0;
        wait_ms(1000);
	}
}

void vApplicationNotSchedulable( void )
{
	taskDISABLE_INTERRUPTS();

	pc.printf( "RTS not schedulable.\r\n" );

	for( ;; )
	{
        leds[ 1 ] = 1;
        wait_ms(1000);
        leds[ 1 ] = 0;
        wait_ms(1000);
	}
}

void vApplicationDeadlineMissedHook( char *pcTaskName, UBaseType_t uxRelease, TickType_t xTickCount )
{
    ( void ) uxRelease;
    ( void ) xTickCount;

    taskDISABLE_INTERRUPTS();

    pc.printf( "%s\tdeadline miss at %d\r\n", pcTaskName, xTickCount );

    for( ;; )
    {
        leds[ 0 ] = 1;
        wait_ms( 1000 );
        leds[ 0 ] = 0;
        wait_ms( 1000 );
    }
}
#endif