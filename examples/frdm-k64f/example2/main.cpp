/*****************************************************************************
 *
 * Example 2 for FRDM-K64F
 *
 * This program consist of 4 real-time periodic tasks and 2 aperiodic tasks,
 * the later only scheduled when there is available slack in the system. All
 * the tasks write a string with some data to to the serial port, when
 * starting and finishing each instance.
 *
 * Before writing to the serial port, the tasks try to take a shared mutex.
 * This could lead to the following problem: when an aperiodic task has taken
 * the mutex and then the available slack depletes, the periodic tasks can't
 * take the mutex, and a missed deadline will occur.
 *
 * This program requires FreeRTOS v10.0.0 or later.
 *
 * Created on: 24 Jun. 2017
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
#include "common-mbed.h"
#include "utils.h"

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
#define ATASK_MAX_DELAY 8000
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
TaskHandle_t task_handles[ TASK_CNT ];
static TaskHandle_t xApTaskHandle1, xApTaskHandle2;

/*****************************************************************************
 * Public data
 ****************************************************************************/
Serial pc( USBTX, USBRX );
DigitalOut leds[] = { LED_RED, LED_GREEN, LED_BLUE, LED_RED };
SemaphoreHandle_t xMutex = NULL;
#if defined( TRACEALYZER_v3_3_1 )
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
#if defined( TRACEALYZER_v3_3_1 )
    // Initializes the trace recorder, but does not start the tracing.
    vTraceEnable( TRC_INIT );
    slack_channel = xTraceRegisterString("Slack Events");
#endif

    pc.baud( BAUDRATE );
    pc.baud( BAUDRATE );
    pc.printf( "Example %d\n\r", EXAMPLE );
    pc.printf( "Using FreeRTOS %s\n\r", tskKERNEL_VERSION_NUMBER );

    // Turn off all the on board LEDs.
    leds[0] = 1;
    leds[1] = 1;
    leds[2] = 1;
    leds[3] = 1;

    // Create mutex.
    xMutex = xSemaphoreCreateMutex();

    // Periodic tasks.
    xTaskCreate( vCommonPeriodicTask, "T1", 256, NULL, TASK_1_PRIO, &task_handles[ 0 ] );
    xTaskCreate( vCommonPeriodicTask, "T2", 256, NULL, TASK_2_PRIO, &task_handles[ 1 ] );
    xTaskCreate( vCommonPeriodicTask, "T3", 256, NULL, TASK_3_PRIO, &task_handles[ 2 ] );
    xTaskCreate( vCommonPeriodicTask, "T4", 256, NULL, TASK_4_PRIO, &task_handles[ 3 ] );

    // Aperiodic task.
    xTaskCreate( vCommonAperiodicTask, "TA1", 256, NULL, ATASK_1_PRIO, &xApTaskHandle1 );
    xTaskCreate( vCommonAperiodicTask, "TA2", 256, NULL, ATASK_2_PRIO, &xApTaskHandle2 );

#if( configUSE_SLACK_STEALING == 1 )
    vSlackSetTaskParams( task_handles[ 0 ], PERIODIC_TASK, TASK_1_PERIOD,
            TASK_1_PERIOD, TASK_1_WCET, 1 );
    vSlackSetTaskParams( task_handles[ 1 ], PERIODIC_TASK, TASK_2_PERIOD,
            TASK_2_PERIOD, TASK_2_WCET, 2 );
    vSlackSetTaskParams( task_handles[ 2 ], PERIODIC_TASK, TASK_3_PERIOD,
            TASK_3_PERIOD, TASK_3_WCET, 3 );
    vSlackSetTaskParams( task_handles[ 3 ], PERIODIC_TASK, TASK_4_PERIOD,
            TASK_4_PERIOD, TASK_4_WCET, 4 );

    /* Aperiodic task */
    vSlackSetTaskParams( xApTaskHandle1, APERIODIC_TASK, ATASK_MAX_DELAY, 0,
            ATASK_WCET, 1 );
    vSlackSetTaskParams( xApTaskHandle2, APERIODIC_TASK, ATASK_MAX_DELAY, 0,
            ATASK_WCET, 2 );
#endif

#if defined( TRACEALYZER_v3_3_1 )
    // Start the tracing.
    vTraceEnable( TRC_START );
#endif

    // Start the scheduler.
    vTaskStartScheduler();

    // Should never arrive here.
    for(;;);
}
