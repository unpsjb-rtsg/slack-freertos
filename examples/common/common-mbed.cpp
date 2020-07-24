#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "slack.h"
#include "utils.h"
#include "common-mbed.h"

#ifdef __cplusplus
extern "C" {
#endif

/* The prototype shows it is a naked function - in effect this is just an
assembly function. */
void HardFault_Handler( void ) __attribute__( ( naked ) );

/* The fault handler implementation calls a function called
prvGetRegistersFromStack(). */
void HardFault_Handler(void)
{
    __asm volatile
    (
        " tst lr, #4                                                \n"
        " ite eq                                                    \n"
        " mrseq r0, msp                                             \n"
        " mrsne r0, psp                                             \n"
        " ldr r1, [r0, #24]                                         \n"
        " ldr r2, handler2_address_const                            \n"
        " bx r2                                                     \n"
        " handler2_address_const: .word prvGetRegistersFromStack    \n"
    );
}

void prvGetRegistersFromStack( uint32_t *pulFaultStackAddress )
{
    /* These are volatile to try and prevent the compiler/linker optimising them
    away as the variables never actually get used.  If the debugger won't show the
    values of the variables, make them global my moving their declaration outside
    of this function. */
    __attribute__((unused)) volatile uint32_t r0;
    __attribute__((unused)) volatile uint32_t r1;
    __attribute__((unused)) volatile uint32_t r2;
    __attribute__((unused)) volatile uint32_t r3;
    __attribute__((unused)) volatile uint32_t r12;
    __attribute__((unused)) volatile uint32_t lr; /* Link register. */
    __attribute__((unused)) volatile uint32_t pc; /* Program counter. */
    __attribute__((unused)) volatile uint32_t psr;/* Program status register. */

    r0 = pulFaultStackAddress[ 0 ];
    r1 = pulFaultStackAddress[ 1 ];
    r2 = pulFaultStackAddress[ 2 ];
    r3 = pulFaultStackAddress[ 3 ];

    r12 = pulFaultStackAddress[ 4 ];
    lr = pulFaultStackAddress[ 5 ];
    pc = pulFaultStackAddress[ 6 ];
    psr = pulFaultStackAddress[ 7 ];

    /* When the following line is hit, the variables contain the register values. */
    for( ;; );
}

#ifdef __cplusplus
}
#endif

/*****************************************************************************
 * Private functions
 ****************************************************************************/
static void vCommonPrintSlacks( char s, int32_t * slackArray, TickType_t xCur )
{
    pc.printf("%s\t%c\t%d\t%d\t%d\t%d\t%d\t%d\t%d\n\r",
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

#if(configUSE_SLACK_STEALING == 1)
#if( tskKERNEL_VERSION_MAJOR == 8 )
	pxTaskSsTCB = pxTaskGetTaskSsTCB( NULL );
#endif
#if( tskKERNEL_VERSION_MAJOR >= 9 )
	pxTaskSsTCB = getTaskSsTCB( NULL );
#endif

#if EXAMPLE == 1
    int32_t slackArray[ 7 ];
#endif
#endif

	for(;;)
    {
#if (configUSE_SLACK_STEALING == 1)
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

#if defined( TRACEALYZER_v3_1_3 ) || defined( TRACEALYZER_v3_3_1 )
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

#if (configUSE_SLACK_STEALING == 1)
#if EXAMPLE == 1
	    if ( xSemaphoreTake( xMutex, portMAX_DELAY ) )
		{
	        vTasksGetSlacks( slackArray );
		    vCommonPrintSlacks( 'E', slackArray, pxTaskSsTCB->xCur );
		    xSemaphoreGive( xMutex );
		}
#endif

#if defined( TRACEALYZER_v3_1_3 ) || defined( TRACEALYZER_v3_3_1 )
        vTracePrintF( slack_channel, "%d - %d", xSlackSD, pxTaskSsTCB->xSlack );
#endif
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
#if( tskKERNEL_VERSION_MAJOR >= 9 )
    pxTaskSsTCB = getTaskSsTCB( NULL );
#endif

    vTaskDelay( rand() % pxTaskSsTCB->xPeriod );

    for(;;)
    {
#if defined( TRACEALYZER_v3_1_3 ) || defined( TRACEALYZER_v3_3_1 )
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

#if defined( TRACEALYZER_v3_1_3 ) || defined( TRACEALYZER_v3_3_1 )
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

void vApplicationDeadlineMissedHook( char *pcTaskName, const SsTCB_t *xSsTCB,
        TickType_t xTickCount )
{
    ( void ) xSsTCB;

    taskDISABLE_INTERRUPTS();

    pc.printf( "\n%s [%d]\tdeadline miss at %d, c=%d\r\n", pcTaskName, xSsTCB->uxReleaseCount, xTickCount, xSsTCB->xCur );

    for( ;; )
    {
        /*leds[ 0 ] = 1;
        wait_ms( 1000 );
        leds[ 0 ] = 0;
        wait_ms( 1000 );*/
    }
}
#endif
