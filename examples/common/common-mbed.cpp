#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "slack.h"
#include "utils.h"
#include "common-mbed.h"

/*****************************************************************************
 * Private functions
 ****************************************************************************/
static void vCommonPrintSlacks( char s, int32_t * slackArray, TickType_t xCur )
{
	pc.printf("%s\t%c\t%d\t%d\t%d\t%d\t%d\t%d\t%d\n",
			pcTaskGetTaskName(NULL), s,
			slackArray[0], slackArray[2], slackArray[3],
			slackArray[4], slackArray[5], slackArray[6],
			xCur);
}

/*****************************************************************************
 * Public functions
 ****************************************************************************/
void vCommonPeriodicTask( void* params )
{
    ( void ) params;

	SsTCB_t *pxTaskSsTCB;

#if( tskKERNEL_VERSION_MAJOR == 8 )
	pxTaskSsTCB = pxTaskGetTaskSsTCB( NULL );
#endif
#if( tskKERNEL_VERSION_MAJOR == 9 )
	pxTaskSsTCB = getTaskSsTCB( NULL );
#endif

#if EXAMPLE == 1
    int32_t slackArray[ 7 ];
#endif

	for(;;)
    {
#if EXAMPLE == 3
	    if (pxTaskSsTCB->xId == 1) {
	        if (xTaskGetTickCount() > 24900) {
	            vTaskSuspendAll();
	            int i;
	            for(i=0; i<250; i++) {
	                pc.printf("%d\t%d\t%d\t%d\t%d\t%d\t%d\n",
	                        (*sdArray)[i][0],
	                        (*sdArray)[i][1],
	                        (*sdArray)[i][2],
	                        (*sdArray)[i][3],
	                        (*sdArray)[i][4],
	                        (*sdArray)[i][5],
	                        (*sdArray)[i][6]);
	            }
	            for(i=0; i<50; i++) {
	               pc.printf("%d\t%d\t%d\t%d\t%d\t%d\t%d\n",
                            (*sdArray2)[i][0],
                            (*sdArray2)[i][1],
                            (*sdArray2)[i][2],
                            (*sdArray2)[i][3],
                            (*sdArray2)[i][4],
                            (*sdArray2)[i][5],
                            (*sdArray2)[i][6]);
	            }
	        }
	    }
#endif

#ifdef TRACEALYZER_v3_1_3
        vTracePrintF( slack_channel, "%d - %d", xSlackSD, pxTaskSsTCB->xSlack );
#endif

#if EXAMPLE == 1
        if ( xSemaphoreTake( xMutex, portMAX_DELAY ) )
        {
            vTasksGetSlacks( slackArray );
            vCommonPrintSlacks( 'S', slackArray, pxTaskSsTCB->xCur );
            xSemaphoreGive( xMutex );
        }
#endif

		leds[ pxTaskSsTCB->xId - 1] = 1;

#if ( configTASK_EXEC == 0 )
		vUtilsEatCpu( pxTaskSsTCB->xWcet - 200 );
#endif
#if ( configTASK_EXEC == 1 )
		while( pxTaskSsTCB->xCur <  pxTaskSsTCB->xWcet )
		{
			asm("nop");
		}
#endif

		leds[ pxTaskSsTCB->xId - 1] = 0;

#if EXAMPLE == 1
	    if ( xSemaphoreTake( xMutex, portMAX_DELAY ) )
		{
	        vTasksGetSlacks( slackArray );
		    vCommonPrintSlacks( 'E', slackArray, pxTaskSsTCB->xCur );
		    xSemaphoreGive( xMutex );
		}
#endif

#ifdef TRACEALYZER_v3_1_3
        vTracePrintF( slack_channel, "%d - %d", xSlackSD, pxTaskSsTCB->xSlack );
#endif

		vTaskDelayUntil( &( pxTaskSsTCB->xPreviousWakeTime ), pxTaskSsTCB->xPeriod );
    }
}

void vCommonAperiodicTask( void* params )
{
    int32_t slackArray[ 7 ];

    SsTCB_t *pxTaskSsTCB;

#if( tskKERNEL_VERSION_MAJOR == 8 )
    pxTaskSsTCB = pxTaskGetTaskSsTCB( NULL );
#endif
#if( tskKERNEL_VERSION_MAJOR == 9 )
    pxTaskSsTCB = getTaskSsTCB( NULL );
#endif

    vTaskDelay( rand() % pxTaskSsTCB->xPeriod );

    for(;;)
    {
#ifdef TRACEALYZER_v3_1_3
        vTracePrintF( slack_channel, "%d - %d", xSlackSD, pxTaskSsTCB->xSlack );
#endif

        pxTaskSsTCB->xCur = ( TickType_t ) 0;

        vTasksGetSlacks( slackArray );
        if ( xSemaphoreTake( xMutex, portMAX_DELAY ) )
        {
            vCommonPrintSlacks( 'S', slackArray, pxTaskSsTCB->xCur );
            xSemaphoreGive( xMutex );
        }

        vUtilsEatCpu( rand() % pxTaskSsTCB->xWcet );

        vTasksGetSlacks( slackArray );
        if ( xSemaphoreTake( xMutex, portMAX_DELAY ) )
        {
            vCommonPrintSlacks( 'E', slackArray, pxTaskSsTCB->xCur );
            xSemaphoreGive( xMutex );
        }

#ifdef TRACEALYZER_v3_1_3
        vTracePrintF( slack_channel, "%d - %d", xSlackSD, pxTaskSsTCB->xSlack );
#endif

        vTaskDelay( rand() % pxTaskSsTCB->xPeriod );
    }
}

#if ( configUSE_MALLOC_FAILED_HOOK == 1 )
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
#endif

#if ( configCHECK_FOR_STACK_OVERFLOW > 0 )
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
#endif

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

void vApplicationDeadlineMissedHook( char *pcTaskName, const SsTCB_t *xSsTCB, TickType_t xTickCount )
{
    ( void ) xSsTCB;

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
