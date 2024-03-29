/*****************************************************************************
 *
 * Example 4
 *
 * This program consist of 4 real-time periodic tasks and 2 aperiodic tasks.
 * Each task write a string with some data to to the serial port, when starting
 * and finishing each instance.
 *
 * This program requires FreeRTOS v10.0.0 or later.
 *
 * Created on: 19 jul. 2020
 *     Author: Francisco E. Páez
 *
 *****************************************************************************/

/*****************************************************************************
 * Includes
 ****************************************************************************/
#include "mbed.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "slack.h"
#include "utils.h"
#include "common-mbed.h"

/*****************************************************************************
 * Macros and definitions
 ****************************************************************************/
#define TASK_CNT 4
#define TASK_1_WCET 1000
#define TASK_2_WCET 1000
#define TASK_3_WCET 1000
#define TASK_4_WCET 1000
#define TASK_1_PERIOD 3000
#define TASK_2_PERIOD 4000
#define TASK_3_PERIOD 6000
#define TASK_4_PERIOD 12000
#define TASK_1_PRIO configMAX_PRIORITIES - configSS_SLACK_PRIOS - 1
#define TASK_2_PRIO configMAX_PRIORITIES - configSS_SLACK_PRIOS - 2
#define TASK_3_PRIO configMAX_PRIORITIES - configSS_SLACK_PRIOS - 3
#define TASK_4_PRIO configMAX_PRIORITIES - configSS_SLACK_PRIOS - 4

#define BAUDRATE 9600

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
static void vPeriodicTask( void* params );
static void vCommonPrintSlacks( char s, int32_t * slackArray, TickType_t xCur );

/*****************************************************************************
 * Private data
 ****************************************************************************/
static TaskHandle_t task_handles[ TASK_CNT ];

/*****************************************************************************
 * Public data
 ****************************************************************************/
SemaphoreHandle_t xMutex = NULL;
Serial pc( USBTX, USBRX );
DigitalOut leds[] = { LED1, LED2, LED3, LED4 };
#ifdef TRACEALYZER_v3_1_3
traceString slack_channel;
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

static void vPeriodicTask( void* params )
{
    ( void ) params;

    SsTCB_t *pxTaskSsTCB = pvSlackGetTaskSsTCB( NULL );

    int32_t slackArray[ 7 ];

    UBaseType_t xRndRun = 0;

    for(;;)
    {
        if ( xSemaphoreTake( xMutex, portMAX_DELAY ) )
        {
            vTasksGetSlacks( slackArray );
            vCommonPrintSlacks( 'S', slackArray, pxTaskSsTCB->xCur );
            xSemaphoreGive( xMutex );
        }

        leds[ pxTaskSsTCB->xId - 1] = 1;



        xRndRun = (UBaseType_t) rand() % ( pxTaskSsTCB->xWcet - 200 );
        vUtilsBusyWait( xRndRun );

        leds[ pxTaskSsTCB->xId - 1] = 0;

        if ( xSemaphoreTake( xMutex, portMAX_DELAY ) )
        {
            vTasksGetSlacks( slackArray );
            vCommonPrintSlacks( 'E', slackArray, pxTaskSsTCB->xCur );
            xSemaphoreGive( xMutex );
        }

        vTaskDelayUntil( &( pxTaskSsTCB->xPreviousWakeTime ), pxTaskSsTCB->xPeriod );
    }
}

/*****************************************************************************
 * Public functions
 ****************************************************************************/
/**
 *
 * @return Nothing, function should not exit.
 */
int main(void)
{
    // Verify that configUSE_SLACK_STEALING is enabled
    configSS_ASSERT_EQUAL( configUSE_SLACK_STEALING, 1 );
    // Verify that tskKERNEL_VERSION_MAJOR is >= 9
    configSS_ASSERT_GREATHER_OR_EQUAL( tskKERNEL_VERSION_MAJOR, 9);

#if TZ == 1
    // Initializes the trace recorder, but does not start the tracing.
    vTraceEnable( TRC_INIT );
#endif

	pc.baud( BAUDRATE );
	pc.printf( "Example %d\n\r", EXAMPLE );
	pc.printf( "Using FreeRTOS %s\n\r", tskKERNEL_VERSION_NUMBER );

	// turn off all the on board LEDs.
    leds[0] = 0;
    leds[1] = 0;
    leds[2] = 0;
    leds[3] = 0;

    // Create mutex.
    xMutex = xSemaphoreCreateMutex();

    // Periodic tasks.
    xTaskCreate( vPeriodicTask, "T1", 256, NULL, TASK_1_PRIO, &task_handles[ 0 ] );
    xTaskCreate( vPeriodicTask, "T2", 256, NULL, TASK_2_PRIO, &task_handles[ 1 ] );
    xTaskCreate( vPeriodicTask, "T3", 256, NULL, TASK_3_PRIO, &task_handles[ 2 ] );
    xTaskCreate( vPeriodicTask, "T4", 256, NULL, TASK_4_PRIO, &task_handles[ 3 ] );

#if configUSE_SLACK_STEALING == 1
    // additional parameters needed by the slack stealing framework
    vSlackSetTaskParams( task_handles[ 0 ], PERIODIC_TASK, TASK_1_PERIOD,
            TASK_1_PERIOD, TASK_1_WCET, 1 );
    vSlackSetTaskParams( task_handles[ 1 ], PERIODIC_TASK, TASK_2_PERIOD,
            TASK_2_PERIOD, TASK_2_WCET, 2 );
    vSlackSetTaskParams( task_handles[ 2 ], PERIODIC_TASK, TASK_3_PERIOD,
            TASK_3_PERIOD, TASK_3_WCET, 3 );
    vSlackSetTaskParams( task_handles[ 3 ], PERIODIC_TASK, TASK_4_PERIOD,
            TASK_4_PERIOD, TASK_4_WCET, 4 );
#endif

    // Start the tracing.
#if TZ == 1
    vTraceEnable( TRC_START );
#endif

    // Start the scheduler.
    vTaskStartScheduler();

    // Should never arrive here.
    for(;;);
}
