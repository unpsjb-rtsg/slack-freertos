/*****************************************************************************
 * Includes
 ****************************************************************************/
#include "mbed.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "slack.h"
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
/* None */

/*****************************************************************************
 * Private data
 ****************************************************************************/
static TaskHandle_t task_handles[ TASK_CNT ];
static TaskHandle_t xApTaskHandle1, xApTaskHandle2;

/*****************************************************************************
 * Public data
 ****************************************************************************/
Serial pc( USBTX, USBRX );
DigitalOut leds[] = { LED1, LED2, LED3, LED4 };
SemaphoreHandle_t xMutex = NULL;
#if defined( TRACEALYZER_v3_1_3 ) || defined( TRACEALYZER_v3_3_1 )
traceString slack_channel;
#endif

/*****************************************************************************
 * Private functions
 ****************************************************************************/
/* None */

/*****************************************************************************
 * Public functions
 ****************************************************************************/
int main(void)
{
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
	xTaskCreate( vCommonPeriodicTask, "T1", 256, (void*) 1, TASK_1_PRIO, &task_handles[ 0 ] );
	xTaskCreate( vCommonPeriodicTask, "T2", 256, (void*) 2, TASK_2_PRIO, &task_handles[ 1 ] );
	xTaskCreate( vCommonPeriodicTask, "T3", 256, (void*) 3, TASK_3_PRIO, &task_handles[ 2 ] );
	xTaskCreate( vCommonPeriodicTask, "T4", 256, (void*) 4, TASK_4_PRIO, &task_handles[ 3 ] );

    // Aperiodic tasks.
    xTaskCreate( vCommonAperiodicTask, "A1", 256, (void*) 1, ATASK_1_PRIO, &xApTaskHandle1 );
    xTaskCreate( vCommonAperiodicTask, "A2", 256, (void*) 2, ATASK_2_PRIO, &xApTaskHandle2 );

#if( configUSE_SLACK_STEALING == 1 )
    vSlackSetTaskParams( task_handles[ 0 ], PERIODIC_TASK, TASK_1_PERIOD, TASK_1_PERIOD, TASK_1_WCET );
    vSlackSetTaskParams( task_handles[ 1 ], PERIODIC_TASK, TASK_2_PERIOD, TASK_2_PERIOD, TASK_2_WCET );
    vSlackSetTaskParams( task_handles[ 2 ], PERIODIC_TASK, TASK_3_PERIOD, TASK_3_PERIOD, TASK_3_WCET );
    vSlackSetTaskParams( task_handles[ 3 ], PERIODIC_TASK, TASK_4_PERIOD, TASK_4_PERIOD, TASK_4_WCET );

    // Aperiodic task.
    vSlackSetTaskParams( xApTaskHandle1, APERIODIC_TASK, ATASK_MAX_DELAY, 0, ATASK_WCET );
    vSlackSetTaskParams( xApTaskHandle2, APERIODIC_TASK, ATASK_MAX_DELAY, 0, ATASK_WCET );
#endif

#if TZ == 1
    // Start the tracing.
    vTraceEnable( TRC_START );
#endif

    // Start the scheduler.
    vTaskStartScheduler();

    // Should never arrive here.
    for(;;);
}
