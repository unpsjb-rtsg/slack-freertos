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
#define TASK_1_PRIO configMAX_PRIORITIES - configMAX_SLACK_PRIO - 1
#define TASK_2_PRIO configMAX_PRIORITIES - configMAX_SLACK_PRIO - 2
#define TASK_3_PRIO configMAX_PRIORITIES - configMAX_SLACK_PRIO - 3
#define TASK_4_PRIO configMAX_PRIORITIES - configMAX_SLACK_PRIO - 4
#define ATASK_WCET 3500
#define ATASK_MAX_DELAY 7000
#define ATASK_1_PRIO configMAX_PRIORITIES - 1

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
static TaskHandle_t xApTaskHandle1;

/*****************************************************************************
 * Public data
 ****************************************************************************/
Serial pc( USBTX, USBRX );
DigitalOut leds[] = { LED1, LED2, LED3, LED4 };
SemaphoreHandle_t xMutex = NULL;
xType *sdArray;
xType2 *sdArray2;
volatile int count = 0;
volatile int count2 = 0;
volatile int count3 = 0;
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
    // Initializes the trace recorder, but does not start the tracing.
#ifdef TRACEALYZER_v3_0_2
    vTraceInitTraceData();
#endif
#if defined( TRACEALYZER_v3_1_3 ) || defined( TRACEALYZER_v3_3_1 )
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

    /* Reserve memory for cs_cost[][] array */
    sdArray = ( xType* ) pvPortMalloc( sizeof( int32_t ) * ( 250 * 7));
    sdArray2 = ( xType2* ) pvPortMalloc( sizeof( int32_t ) * ( 50 * 7));

    // Initialize sd array
    int i = 0; int j = 0;
    for(i = 0; i < 250; i++) {
        for(j = 0; j < 7; j++) {
            (*sdArray)[i][j] = 0;
        }
    }
    for(i = 0; i < 50; i++) {
        for(j = 0; j < 7; j++) {
            (*sdArray2)[i][j] = 0;
        }
    }

    // Periodic tasks.
	xTaskCreate( vCommonPeriodicTask, "T1", 256, NULL, TASK_1_PRIO, &task_handles[ 0 ] );
	xTaskCreate( vCommonPeriodicTask, "T2", 256, NULL, TASK_2_PRIO, &task_handles[ 1 ] );
	xTaskCreate( vCommonPeriodicTask, "T3", 256, NULL, TASK_3_PRIO, &task_handles[ 2 ] );
	xTaskCreate( vCommonPeriodicTask, "T4", 256, NULL, TASK_4_PRIO, &task_handles[ 3 ] );

    // Aperiodic tasks.
    xTaskCreate( vCommonAperiodicTask, "TA1", 256, NULL, ATASK_1_PRIO, &xApTaskHandle1 );

#if( configUSE_SLACK_STEALING == 1 )
    #if( tskKERNEL_VERSION_MAJOR >= 9 )
    {
        vSlackSystemSetup();
    }
    #endif

    // additional parameters needed by the slack stealing framework
#if( tskKERNEL_VERSION_MAJOR == 8 )
    vTaskSetParams( task_handles[ 0 ], TASK_1_PERIOD, TASK_1_PERIOD, TASK_1_WCET, 1 );
    vTaskSetParams( task_handles[ 1 ], TASK_2_PERIOD, TASK_2_PERIOD, TASK_2_WCET, 2 );
    vTaskSetParams( task_handles[ 2 ], TASK_3_PERIOD, TASK_3_PERIOD, TASK_3_WCET, 3 );
#endif
#if( tskKERNEL_VERSION_MAJOR >= 9 )
    vSlackSetTaskParams( task_handles[ 0 ], PERIODIC_TASK, TASK_1_PERIOD, TASK_1_PERIOD, TASK_1_WCET, 1 );
    vSlackSetTaskParams( task_handles[ 1 ], PERIODIC_TASK, TASK_2_PERIOD, TASK_2_PERIOD, TASK_2_WCET, 2 );
    vSlackSetTaskParams( task_handles[ 2 ], PERIODIC_TASK, TASK_3_PERIOD, TASK_3_PERIOD, TASK_3_WCET, 3 );
    vSlackSetTaskParams( task_handles[ 3 ], PERIODIC_TASK, TASK_4_PERIOD, TASK_4_PERIOD, TASK_4_WCET, 4 );
    vSlackSetTaskParams( xApTaskHandle1, APERIODIC_TASK, ATASK_MAX_DELAY, 0, ATASK_WCET, 1 );
    vSlackSchedulerSetup();
#endif
#endif

    // Start the tracing.
#ifdef TRACEALYZER_v3_0_2
    uiTraceStart();
#endif
#if defined( TRACEALYZER_v3_1_3 ) || defined( TRACEALYZER_v3_3_1 )
    vTraceEnable( TRC_START );
#endif

    // Start the scheduler.
    vTaskStartScheduler();

    // Should never arrive here.
    for(;;);
}

void vApplicationTickHook (void) {
    if (count % 100 == 0) {
        if (count2 < 250) {
            vTasksGetSlacks( (*sdArray)[count2] );
            count2 = count2 + 1;
        }
    }
    count = count + 1;
}

void traceTaskSwitchedIn() {
    vTasksGetSlacks( (*sdArray2)[count3] );
    count3 = count3 + 1;
}
