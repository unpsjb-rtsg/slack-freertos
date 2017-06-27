/*****************************************************************************
 * Includes
 ****************************************************************************/
#include "mbed.h"
#include "FreeRTOS.h"
#include "task.h"
#include "slack.h"
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

/*****************************************************************************
 * Public data
 ****************************************************************************/
Serial pc( USBTX, USBRX );
DigitalOut leds[] = { LED_RED, LED_GREEN, LED_BLUE, LED_RED };
#ifdef TRACEALYZER_v3_1_3
traceString slack_channel;
#endif

/*****************************************************************************
 * Private functions
 ****************************************************************************/
/* None */

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
    vTraceEnable( TRC_INIT );
    slack_channel = xTraceRegisterString("Slack Events");
#endif

	pc.baud( BAUDRATE );
    pc.printf( "Example 1\n" );

	// turn off all the on board LEDs.
	leds[0] = 1;
	leds[1] = 1;
	leds[2] = 1;
	leds[3] = 1;

    // Periodic tasks.
    xTaskCreate( periodicTaskBody, "T1", 256, NULL, TASK_1_PRIO, &task_handles[ 0 ] );
    xTaskCreate( periodicTaskBody, "T2", 256, NULL, TASK_2_PRIO, &task_handles[ 1 ] );
    xTaskCreate( periodicTaskBody, "T3", 256, NULL, TASK_3_PRIO, &task_handles[ 2 ] );
    xTaskCreate( periodicTaskBody, "T4", 256, NULL, TASK_4_PRIO, &task_handles[ 3 ] );

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
#endif

    #if( tskKERNEL_VERSION_MAJOR == 9 )
    {
    	vSlackSchedulerSetup();
    }
    #endif
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
