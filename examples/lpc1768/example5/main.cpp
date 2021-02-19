/*****************************************************************************
 *
 * Example 5
 *
 * This program consist of 4 real-time periodic tasks and 2 aperiodic tasks,
 * the later only scheduled when there is available slack in the system. All
 * the tasks write a string with some data to to the serial port, when
 * starting and finishing each instance.
 *
 * Before writing to the serial port, the tasks try to take a shared mutex.
 * This could lead to the following problem: when an aperiodic task has taken
 * the mutex and then the available slack depletes, the periodic tasks can't
 * take the mutex, and a missed deadline will occur. To avoid this situation,
 * a timeout is used when the periodic tasks is waiting to obtain the mutex.
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
#define TASK_1_PRIO configMAX_PRIORITIES - configMAX_SLACK_PRIO - 1
#define TASK_2_PRIO configMAX_PRIORITIES - configMAX_SLACK_PRIO - 2
#define TASK_3_PRIO configMAX_PRIORITIES - configMAX_SLACK_PRIO - 3
#define TASK_4_PRIO configMAX_PRIORITIES - configMAX_SLACK_PRIO - 4
#define ATASK_WCET 2000
#define ATASK_MAX_DELAY 4000
#define ATASK_1_PRIO configMAX_PRIORITIES - 1
#define ATASK_2_PRIO configMAX_PRIORITIES - 2
#define MUTEX_TIMEOUT 10

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
static void vAperiodicTask( void* params );
static void vCommonPrintSlacks( char s, int32_t * slackArray, SsTCB_t *pxTaskSsTCB );

/*****************************************************************************
 * Private data
 ****************************************************************************/
static TaskHandle_t task_handles[ TASK_CNT ];
static TaskHandle_t xApTaskHandle1, xApTaskHandle2;
static int32_t slackArray[ 7 ];

/*****************************************************************************
 * Public data
 ****************************************************************************/
SemaphoreHandle_t xMutex = NULL;
Serial pc( USBTX, USBRX );
DigitalOut leds[] = { LED1, LED2, LED3, LED4 };
#if TZ == 1
traceString slack_channel;
#endif

/*****************************************************************************
 * Private functions
 ****************************************************************************/
static void vCommonPrintSlacks( char s, int32_t * slackArray, SsTCB_t *pxTaskSsTCB )
{
    pc.printf("%s [%3d] %c\t%d\t%d\t%d\t%d\t%d\t%d\t%d\n\r",
            pcTaskGetTaskName(NULL), pxTaskSsTCB->uxReleaseCount, s,
            slackArray[0], slackArray[2], slackArray[3],
            slackArray[4], slackArray[5], slackArray[6],
            pxTaskSsTCB->xCur);
}
/*-----------------------------------------------------------*/

static void vAperiodicTask( void* params )
{
    SsTCB_t *pxTaskSsTCB;

    pxTaskSsTCB = getTaskSsTCB( NULL );

    vTaskDelay( rand() % pxTaskSsTCB->xPeriod );

    for(;;)
    {
        #if TZ == 1
        vTracePrintF( slack_channel, "%d - %d", xSlackGetAvailableSlack(), pxTaskSsTCB->xSlack );
        #endif

        pxTaskSsTCB->xCur = ( TickType_t ) 0;

        if ( xSemaphoreTake( xMutex, portMAX_DELAY ) )
        {
            vTasksGetSlacks( slackArray );
            vCommonPrintSlacks( 'S', slackArray, pxTaskSsTCB );
            xSemaphoreGive( xMutex );
        }

        vUtilsBusyWait( rand() % pxTaskSsTCB->xWcet );

        if ( xSemaphoreTake( xMutex, portMAX_DELAY ) )
        {
            vTasksGetSlacks( slackArray );
            vCommonPrintSlacks( 'E', slackArray, pxTaskSsTCB );
            xSemaphoreGive( xMutex );
        }

        #if TZ == 1
        vTracePrintF( slack_channel, "%d - %d", xSlackGetAvailableSlack(), pxTaskSsTCB->xSlack );
        #endif

        vTaskDelay( rand() % pxTaskSsTCB->xPeriod );

        pxTaskSsTCB->uxReleaseCount = pxTaskSsTCB->uxReleaseCount + 1;
    }
}
/*-----------------------------------------------------------*/

static void vPeriodicTask( void* params )
{
    ( void ) params;

    SsTCB_t *pxTaskSsTCB = getTaskSsTCB( NULL );

    UBaseType_t xRndRun = 0;

    for(;;)
    {
        #if TZ == 1
        vTracePrintF( slack_channel, "%d - %d", xSlackGetAvailableSlack(), pxTaskSsTCB->xSlack );
        #endif

        if ( xSemaphoreTake( xMutex, MUTEX_TIMEOUT ) )
        {
            vTasksGetSlacks( slackArray );
            vCommonPrintSlacks( 'S', slackArray, pxTaskSsTCB );
            xSemaphoreGive( xMutex );
        }

        leds[ pxTaskSsTCB->xId - 1] = 1;

        xRndRun = (UBaseType_t) rand() % ( pxTaskSsTCB->xWcet - 300 );
        vUtilsBusyWait( xRndRun );

        leds[ pxTaskSsTCB->xId - 1] = 0;

        if ( xSemaphoreTake( xMutex, MUTEX_TIMEOUT ) )
        {
            vTasksGetSlacks( slackArray );
            vCommonPrintSlacks( 'E', slackArray, pxTaskSsTCB );
            xSemaphoreGive( xMutex );
        }

        #if TZ == 1
        vTracePrintF( slack_channel, "%d - %d", xSlackGetAvailableSlack(), pxTaskSsTCB->xSlack );
        #endif

        vTaskDelayUntil( &( pxTaskSsTCB->xPreviousWakeTime ), pxTaskSsTCB->xPeriod );
    }
}
/*-----------------------------------------------------------*/

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
    // Verify that tskKERNEL_VERSION_MAJOR is >= 10
    configSS_ASSERT_GREATHER_OR_EQUAL( tskKERNEL_VERSION_MAJOR, 10);

#if TZ == 1
    // Initializes the trace recorder, but does not start the tracing.
    vTraceEnable( TRC_INIT );
    slack_channel = xTraceRegisterString("Slack Events");
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

    // Aperiodic tasks.
    xTaskCreate( vAperiodicTask, "A1", 256, NULL, ATASK_1_PRIO, &xApTaskHandle1 );
    xTaskCreate( vAperiodicTask, "A2", 256, NULL, ATASK_2_PRIO, &xApTaskHandle2 );

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

    vSlackSetTaskParams( xApTaskHandle1, APERIODIC_TASK, ATASK_MAX_DELAY, 0,
            ATASK_WCET, 1 );
    vSlackSetTaskParams( xApTaskHandle2, APERIODIC_TASK, ATASK_MAX_DELAY, 0,
            ATASK_WCET, 2 );
#endif

#if TZ == 1
    // Start the tracing.
    vTraceEnable( TRC_START );
#endif

    /* Initialize random number generator with seed zero to have a reproducible
     * trace. */
    srand((unsigned) 0);

    // Start the scheduler.
    vTaskStartScheduler();

    // Should never arrive here.
    for(;;);
}
