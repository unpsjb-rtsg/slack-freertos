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
#define ATASK_MAX_DELAY 4000

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

        vUtilsEatCpu( ATASK_WCET );

        printSlacks( 'E', slackArray, pxTaskSsTCB->xCur );

        vTaskDelay( rand() % ATASK_MAX_DELAY );
    }
}

/*****************************************************************************
 * Public functions
 ****************************************************************************/
int main()
{
	pc.baud( BAUDRATE );
    pc.printf( "Example 2\n" );

	// turn off all the on board LEDs.
	leds[0] = 1;
	leds[1] = 1;
	leds[2] = 1;
	leds[3] = 1;

#if( tskKERNEL_VERSION_MAJOR == 9 )
	vSlackSystemSetup();
#endif

    // create periodic tasks
    xTaskCreate( periodicTaskBody, "T1", 256, NULL, configMAX_PRIORITIES - 2, &task_handles[ 0 ] );  // max priority
    xTaskCreate( periodicTaskBody, "T2", 256, NULL, configMAX_PRIORITIES - 3, &task_handles[ 1 ] );
    xTaskCreate( periodicTaskBody, "T3", 256, NULL, configMAX_PRIORITIES - 4, &task_handles[ 2 ] );
    xTaskCreate( periodicTaskBody, "T4", 256, NULL, configMAX_PRIORITIES - 5, &task_handles[ 3 ] );

    /* Aperiodic task */
    TaskHandle_t xApTaskHandle;
    xTaskCreate ( aperiodic_task_body, "TA", 256, NULL, configMAX_PRIORITIES - 1, &xApTaskHandle );

#if( configUSE_SLACK_STEALING == 1 )
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

    /* Aperiodic task */
    vSlackSetTaskParams( xApTaskHandle, APERIODIC_TASK, 0, 0, 0, 5 );
#endif
#endif

#if( tskKERNEL_VERSION_MAJOR == 9 )
    vSlackSchedulerSetup();
#endif

    vTaskStartScheduler();

    for(;;);
}
