/*****************************************************************************
 *
 * Example 1 for FRDM-K64F
 *
 * This program consist of 4 real-time periodic tasks. Each task write a string
 * with some data to the serial port, when starting and finishing each instance.
 * Before writing to the serial port, the tasks try to take a shared mutex.
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
SemaphoreHandle_t xMutex = NULL;

/*****************************************************************************
 * Private functions
 ****************************************************************************/
/* None */

/*****************************************************************************
 * Public functions
 ****************************************************************************/
int main()
{
#if defined( TRACEALYZER )
    // Initializes the trace recorder, but does not start the tracing.
    vTraceEnable( TRC_INIT );
#endif

    pc.baud( BAUDRATE );
    pc.printf( "FRMD-K64F -- Example %d\n\r", EXAMPLE );
    pc.printf( "> FreeRTOS %s\n\r", tskKERNEL_VERSION_NUMBER );
#if defined( TRACEALYZER )
    pc.printf( "> Tracealyzer v3.3.1\n\r");
#endif

    // turn off all the on board LEDs.
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

#if( configUSE_SLACK_STEALING == 1 )
    vSlackSetTaskParams( task_handles[ 0 ], PERIODIC_TASK, TASK_1_PERIOD,
            TASK_1_PERIOD, TASK_1_WCET, 1 );
    vSlackSetTaskParams( task_handles[ 1 ], PERIODIC_TASK, TASK_2_PERIOD,
            TASK_2_PERIOD, TASK_2_WCET, 2 );
    vSlackSetTaskParams( task_handles[ 2 ], PERIODIC_TASK, TASK_3_PERIOD,
            TASK_3_PERIOD, TASK_3_WCET, 3 );
    vSlackSetTaskParams( task_handles[ 3 ], PERIODIC_TASK, TASK_4_PERIOD,
            TASK_4_PERIOD, TASK_4_WCET, 4 );
#endif

#if defined( TRACEALYZER )
    // Start the tracing.
    vTraceEnable( TRC_START );
#endif

    // Start the scheduler.
    vTaskStartScheduler();

    // Should never arrive here.
    for(;;);
}
