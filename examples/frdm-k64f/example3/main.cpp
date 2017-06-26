/*****************************************************************************
 * Includes
 ****************************************************************************/
#include "common.h"
#include "utils.h"
#include "mbed.h"
#include "FreeRTOS.h"
#include "task.h"
#include "slack.h"

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

#define ATASK_WCET 2000
#define ATASK_MAX_DELAY 8000

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
static void aperiodic_task_body( void* params );

/*****************************************************************************
 * Private data
 ****************************************************************************/
TaskHandle_t task_handles[ TASK_CNT ];

/*****************************************************************************
 * Public data
 ****************************************************************************/
Serial pc( USBTX, USBRX );

// mbed original LED naming
// LED1 = LED_RED
// LED2 = LED_GREEN
// LED3 = LED_BLUE
// LED4 = LED_RED
DigitalOut leds[] = { LED_RED, LED_GREEN, LED_BLUE, LED_RED };

/*****************************************************************************
 * Private functions
 ****************************************************************************/
#if( configUSE_SLACK_STEALING == 1 )
static void aperiodic_task_body( void* params )
{
    ( void ) params;

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
        pxTaskSsTCB->xCur = ( TickType_t ) 0;

        printSlacks( 'S', slackArray, pxTaskSsTCB->xCur );

        vUtilsEatCpu( 100 + ( rand() % ATASK_WCET ) );

        printSlacks( 'E', slackArray, pxTaskSsTCB->xCur );

        vTaskDelay( rand() % ATASK_MAX_DELAY );
    }
}
#endif

/*****************************************************************************
 * Public functions
 ****************************************************************************/
int main()
{
	// Initializes the trace recorder, but does not start the tracing.
#ifdef TRACEALYZER_v3_0_2
	vTraceInitTraceData();
#endif
#ifdef TRACEALYZER_v3_1_3
#if TRC_CFG_RECORDER_MODE == TRC_RECORDER_MODE_SNAPSHOT
	vTraceEnable( TRC_INIT );
#endif
#if TRC_CFG_RECORDER_MODE == TRC_RECORDER_MODE_STREAMING
	vTraceEnable( TRC_START );
#endif
#endif

	pc.baud( BAUDRATE );
    pc.printf( "Example 3\n" );

	// Turn off all the on board LEDs.
	leds[0] = 1;
	leds[1] = 1;
	leds[2] = 1;
	leds[3] = 1;

#if( configUSE_SLACK_STEALING == 1 )
#if( tskKERNEL_VERSION_MAJOR == 9 )
	vSlackSystemSetup();
#endif
#endif

    // Create the periodic tasks.
    xTaskCreate( periodicTaskBody, "T1", 256, (void *) TASK_1_PERIOD, configMAX_PRIORITIES - 3, &task_handles[ 0 ] );  // max priority
    xTaskCreate( periodicTaskBody, "T2", 256, (void *) TASK_2_PERIOD, configMAX_PRIORITIES - 4, &task_handles[ 1 ] );
    xTaskCreate( periodicTaskBody, "T3", 256, (void *) TASK_3_PERIOD, configMAX_PRIORITIES - 5, &task_handles[ 2 ] );
    xTaskCreate( periodicTaskBody, "T4", 256, (void *) TASK_4_PERIOD, configMAX_PRIORITIES - 6, &task_handles[ 3 ] );

#if( configUSE_SLACK_STEALING == 1 )
    // Additional parameters needed by the slack stealing framework.
#if( tskKERNEL_VERSION_MAJOR == 8 )
    vTaskSetParams( task_handles[ 0 ], TASK_1_PERIOD, TASK_1_PERIOD, TASK_1_WCET, 1 );
    vTaskSetParams( task_handles[ 1 ], TASK_2_PERIOD, TASK_2_PERIOD, TASK_2_WCET, 2 );
    vTaskSetParams( task_handles[ 2 ], TASK_3_PERIOD, TASK_3_PERIOD, TASK_3_WCET, 3 );
    vTaskSetParams( task_handles[ 3 ], TASK_4_PERIOD, TASK_4_PERIOD, TASK_4_WCET, 4 );

    // Create the aperiodic task.
    xTaskCreate( aperiodic_task_body, "TA", 256, NULL, configMAX_PRIORITIES - 1, NULL );
#endif
#if( tskKERNEL_VERSION_MAJOR == 9 )
    vSlackSetTaskParams( task_handles[ 0 ], PERIODIC_TASK, TASK_1_PERIOD, TASK_1_PERIOD, TASK_1_WCET, 1 );
    vSlackSetTaskParams( task_handles[ 1 ], PERIODIC_TASK, TASK_2_PERIOD, TASK_2_PERIOD, TASK_2_WCET, 2 );
    vSlackSetTaskParams( task_handles[ 2 ], PERIODIC_TASK, TASK_3_PERIOD, TASK_3_PERIOD, TASK_3_WCET, 3 );
    vSlackSetTaskParams( task_handles[ 3 ], PERIODIC_TASK, TASK_4_PERIOD, TASK_4_PERIOD, TASK_4_WCET, 4 );

    // Create the aperiodic tasks.
    TaskHandle_t xApTaskHandle1, xApTaskHandle2;
    xTaskCreate( aperiodic_task_body, "TA1", 256, NULL, configMAX_PRIORITIES - 1, &xApTaskHandle1 );
    xTaskCreate( aperiodic_task_body, "TA2", 256, NULL, configMAX_PRIORITIES - 2, &xApTaskHandle2 );
    vSlackSetTaskParams( xApTaskHandle1, APERIODIC_TASK, 0, 0, 0, 5 );
    vSlackSetTaskParams( xApTaskHandle2, APERIODIC_TASK, 0, 0, 0, 5 );
#endif
#endif

#if( configUSE_SLACK_STEALING == 1 )
#if( tskKERNEL_VERSION_MAJOR == 9 )
    vSlackSchedulerSetup();
#endif
#endif

    // Starts the tracing.
#ifdef TRACEALYZER_v3_0_2
    uiTraceStart();
#endif
#ifdef TRACEALYZER_v3_1_3
#if TRC_CFG_RECORDER_MODE == TRC_RECORDER_MODE_SNAPSHOT
    vTraceEnable( TRC_START );
#endif
#endif

    vTaskStartScheduler();

    for(;;);
}