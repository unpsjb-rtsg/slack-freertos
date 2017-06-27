/*****************************************************************************
 * Includes
 ****************************************************************************/
#include "mbed.h"
#include "FreeRTOS.h"
#include "task.h"
#include "slack.h"
#include "utils.h"
#include "common.h"

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
static void aperiodic_task_body( void* params );

/*****************************************************************************
 * Private data
 ****************************************************************************/
static TaskHandle_t task_handles[ TASK_CNT ];

/*****************************************************************************
 * Public data
 ****************************************************************************/
Serial pc( USBTX, USBRX );
DigitalOut leds[] = { LED1, LED2, LED3, LED4 };
#ifdef TRACEALYZER_v3_1_3
traceString slack_channel;
#endif

/*****************************************************************************
 * Private functions
 ****************************************************************************/
static void aperiodic_task_body( void* params )
{
    int32_t slackArray[ 7 ];

    SsTCB_t *pxTaskSsTCB;

#if( tskKERNEL_VERSION_MAJOR == 8 )
    pxTaskSsTCB = pxTaskGetTaskSsTCB( NULL );
#endif
#if( tskKERNEL_VERSION_MAJOR == 9 )
    pxTaskSsTCB = getTaskSsTCB( NULL );
#endif

    vTaskDelay( rand() % ATASK_MAX_DELAY );

    for(;;)
    {
#ifdef TRACEALYZER_v3_1_3
        vTracePrintF( slack_channel, "%d - %d", xSlackSD, pxTaskSsTCB->xSlack );
#endif

        pxTaskSsTCB->xCur = ( TickType_t ) 0;

        printSlacks( 'S', slackArray, pxTaskSsTCB->xCur );

        vUtilsEatCpu( rand() % ATASK_WCET );

        printSlacks( 'E', slackArray, pxTaskSsTCB->xCur );

#ifdef TRACEALYZER_v3_1_3
        vTracePrintF( slack_channel, "%d - %d", xSlackSD, pxTaskSsTCB->xSlack );
#endif

        vTaskDelay( rand() % ATASK_MAX_DELAY );
    }
}

/*****************************************************************************
 * Public functions
 ****************************************************************************/
int main(void)
{
    // Initializes the trace recorder, but does not start the tracing.
#ifdef TRACEALYZER_v3_0_2
    vTraceInitTraceData();
#endif
#ifdef TRACEALYZER_v3_1_3
    vTraceEnable( TRC_INIT );
    slack_channel = xTraceRegisterString("Slack Events");
#endif

	pc.baud(9600);
    pc.printf("Example 2\n");

	// turn off all the on board LEDs.
	leds[0] = 0;
	leds[1] = 0;
	leds[2] = 0;
	leds[3] = 0;

    // Periodic tasks.
	xTaskCreate( periodicTaskBody, "T1", 256, NULL, TASK_1_PRIO, &task_handles[ 0 ] );
	xTaskCreate( periodicTaskBody, "T2", 256, NULL, TASK_2_PRIO, &task_handles[ 1 ] );
	xTaskCreate( periodicTaskBody, "T3", 256, NULL, TASK_3_PRIO, &task_handles[ 2 ] );
	xTaskCreate( periodicTaskBody, "T4", 256, NULL, TASK_4_PRIO, &task_handles[ 3 ] );

    // Aperiodic task.
    TaskHandle_t xApTaskHandle;
    xTaskCreate ( aperiodic_task_body, "TA", 256, NULL, ATASK_1_PRIO, &xApTaskHandle );

#if( configUSE_SLACK_STEALING == 1 )
    #if( tskKERNEL_VERSION_MAJOR == 9 )
    {
        vSlackSystemSetup();
    }
    #endif

    // additional parameters needed by the slack stealing framework
#if( tskKERNEL_VERSION_MAJOR == 8 )
    vTaskSetParams( task_handles[ 0 ], TASK_1_PERIOD, TASK_1_PERIOD, TASK_1_WCET, 1 );
    vTaskSetParams( task_handles[ 1 ], TASK_2_PERIOD, TASK_2_PERIOD, TASK_2_WCET, 2 );
    vTaskSetParams( task_handles[ 2 ], TASK_3_PERIOD, TASK_3_PERIOD, TASK_3_WCET, 3 );
    vTaskSetParams( task_handles[ 3 ], TASK_4_PERIOD, TASK_4_PERIOD, TASK_4_WCET, 4 );
#endif
#if( tskKERNEL_VERSION_MAJOR == 9 )
    vSlackSetTaskParams( task_handles[ 0 ], PERIODIC_TASK, TASK_1_PERIOD, TASK_1_PERIOD, TASK_1_WCET, 1 );
    vSlackSetTaskParams( task_handles[ 1 ], PERIODIC_TASK, TASK_2_PERIOD, TASK_2_PERIOD, TASK_2_WCET, 2 );
    vSlackSetTaskParams( task_handles[ 2 ], PERIODIC_TASK, TASK_3_PERIOD, TASK_3_PERIOD, TASK_3_WCET, 3 );
    vSlackSetTaskParams( task_handles[ 3 ], PERIODIC_TASK, TASK_4_PERIOD, TASK_4_PERIOD, TASK_4_WCET, 4 );

    // Aperiodic task.
    vSlackSetTaskParams( xApTaskHandle, APERIODIC_TASK, 0, 0, 0, 1 );
#endif
#endif

#if( tskKERNEL_VERSION_MAJOR == 9 )
    vSlackSchedulerSetup();
#endif

    // Start the tracing.
#ifdef TRACEALYZER_v3_0_2
    uiTraceStart();
#endif
#ifdef TRACEALYZER_v3_1_3
    vTraceEnable( TRC_START );
#endif

    // Start the scheduler.
    vTaskStartScheduler();

    // Should never arrive here.
    for(;;);
}
