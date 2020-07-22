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
static void vCommonPrintSlacks( char s, int32_t * slackArray, TickType_t xCur );

/*****************************************************************************
 * Private data
 ****************************************************************************/
static TaskHandle_t task_handles[ TASK_CNT ];
static TaskHandle_t xApTaskHandle1, xApTaskHandle2;

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
static void vCommonPrintSlacks( char s, int32_t * slackArray, TickType_t xCur )
{
    pc.printf("%s\t%c\t%d\t%d\t%d\t%d\t%d\t%d\t%d\n",
            pcTaskGetTaskName(NULL), s,
            slackArray[0], slackArray[2], slackArray[3],
            slackArray[4], slackArray[5], slackArray[6],
            xCur);
}

static void vAperiodicTask( void* params )
{
    int32_t slackArray[ 7 ];

    SsTCB_t *pxTaskSsTCB;

    pxTaskSsTCB = getTaskSsTCB( NULL );

    vTaskDelay( rand() % pxTaskSsTCB->xPeriod );

    for(;;)
    {
        #if TZ == 1
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

        #if TZ == 1
        vTracePrintF( slack_channel, "%d - %d", xSlackSD, pxTaskSsTCB->xSlack );
        #endif

        vTaskDelay( rand() % pxTaskSsTCB->xPeriod );
    }
}

static void vPeriodicTask( void* params )
{
    ( void ) params;

    SsTCB_t *pxTaskSsTCB = getTaskSsTCB( NULL );

    int32_t slackArray[ 7 ];

    UBaseType_t xRndRun = 0;

    for(;;)
    {
        #if TZ == 1
        vTracePrintF( slack_channel, "%d - %d", xSlackSD, pxTaskSsTCB->xSlack );
        #endif

        if ( xSemaphoreTake( xMutex, portMAX_DELAY ) )
        {
            vTasksGetSlacks( slackArray );
            vCommonPrintSlacks( 'S', slackArray, pxTaskSsTCB->xCur );
            xSemaphoreGive( xMutex );
        }

        leds[ pxTaskSsTCB->xId - 1] = 1;

        xRndRun = (UBaseType_t) rand() % ( pxTaskSsTCB->xWcet - 200 );

        #if ( configTASK_EXEC == 0 )
        {
            //vUtilsEatCpu( pxTaskSsTCB->xWcet - 200 );
            vUtilsEatCpu( xRndRun );
        }
        #endif
        #if ( configTASK_EXEC == 1 )
        {
            while( pxTaskSsTCB->xCur < pxTaskSsTCB->xWcet )
            {
                asm("nop");
            }
        }
        #endif

        leds[ pxTaskSsTCB->xId - 1] = 0;

        if ( xSemaphoreTake( xMutex, portMAX_DELAY ) )
        {
            vTasksGetSlacks( slackArray );
            vCommonPrintSlacks( 'E', slackArray, pxTaskSsTCB->xCur );
            xSemaphoreGive( xMutex );
        }

        #if TZ == 1
        vTracePrintF( slack_channel, "%d - %d", xSlackSD, pxTaskSsTCB->xSlack );
        #endif

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

    // Initializes the trace recorder, but does not start the tracing.
#if TZ == 1
    vTraceEnable( TRC_INIT );
    slack_channel = xTraceRegisterString("Slack Events");
#endif

	pc.baud( BAUDRATE );
	pc.printf( "Example %d\n", EXAMPLE );
	pc.printf( "Using FreeRTOS %s\n", tskKERNEL_VERSION_NUMBER );

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
    xTaskCreate( vAperiodicTask, "TA1", 256, NULL, ATASK_1_PRIO, &xApTaskHandle1 );
    xTaskCreate( vAperiodicTask, "TA2", 256, NULL, ATASK_2_PRIO, &xApTaskHandle2 );

#if configUSE_SLACK_STEALING == 1
    vSlackSystemSetup();

    // additional parameters needed by the slack stealing framework
    vSlackSetTaskParams( task_handles[ 0 ], PERIODIC_TASK, TASK_1_PERIOD, TASK_1_PERIOD, TASK_1_WCET, 1 );
    vSlackSetTaskParams( task_handles[ 1 ], PERIODIC_TASK, TASK_2_PERIOD, TASK_2_PERIOD, TASK_2_WCET, 2 );
    vSlackSetTaskParams( task_handles[ 2 ], PERIODIC_TASK, TASK_3_PERIOD, TASK_3_PERIOD, TASK_3_WCET, 3 );
    vSlackSetTaskParams( task_handles[ 3 ], PERIODIC_TASK, TASK_4_PERIOD, TASK_4_PERIOD, TASK_4_WCET, 4 );

    vSlackSetTaskParams( xApTaskHandle1, APERIODIC_TASK, ATASK_MAX_DELAY, 0, ATASK_WCET, 1 );
    vSlackSetTaskParams( xApTaskHandle2, APERIODIC_TASK, ATASK_MAX_DELAY, 0, ATASK_WCET, 2 );

    vSlackSchedulerSetup();
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
