/*****************************************************************************
 *
 * Example 5 for EDU-CIAA-NXP
 *
 * This program consist of 4 real-time periodic tasks. Each task write a string
 * with some data to the serial port, when starting and finishing each instance.
 * Before writing to the serial port, the tasks try to take a shared mutex.
 *
 * This program requires FreeRTOS v10.0.0 or later.
 *
 * Created on: 24 jul. 2020
 *     Author: Francisco E. Páez
 *
 *****************************************************************************/

/*****************************************************************************
 * Includes
 ****************************************************************************/
#include "FreeRTOS.h"
#include "task.h"
#include "slack.h"
#include "utils.h"
#include "common.h"

/*****************************************************************************
 * Macros and definitions
 ****************************************************************************/
/* The linker does not include this code in liblpc.a because nothing in it
 * references it... */
#define CRP_NO_CRP          0xFFFFFFFF
__attribute__ ((used,section(".crp"))) const unsigned int CRP_WORD = CRP_NO_CRP;

#define TASK_CNT 4
#define TASK_1_WCET 1000
#define TASK_2_WCET 1000
#define TASK_3_WCET 1000
#define TASK_4_WCET 4000
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
static void vPeriodicTask( void* params );

/*****************************************************************************
 * Private data
 ****************************************************************************/
static TaskHandle_t task_handles[ TASK_CNT ];
//static TaskHandle_t xApTaskHandle1, xApTaskHandle2;

/*****************************************************************************
 * Public data
 ****************************************************************************/
gpioMap_t leds[] = { LED1, LED2, LED3 };
#ifdef TZ
traceString slack_channel;
#endif

/*****************************************************************************
 * Private functions
 ****************************************************************************/
void vPeriodicTask( void* params )
{
    ( void ) params;

    SsTCB_t *pxTaskSsTCB = pvSlackGetTaskSsTCB( NULL );

    int32_t slackArray[ 6 ];

    for(;;)
    {
#ifdef TZ
        vTracePrintF( slack_channel, "%d - %d", xSlackSD, pxTaskSsTCB->xSlack );
#endif

        gpioWrite( leds[ pxTaskSsTCB->xId - 1], ON);

        vCommonPrintSlacks( 'S', slackArray, pxTaskSsTCB->xCur );

        vUtilsBusyWait( pxTaskSsTCB->xWcet - 300 );

        vCommonPrintSlacks( 'E', slackArray, pxTaskSsTCB->xCur );

        gpioWrite( leds[ pxTaskSsTCB->xId - 1], OFF);

#ifdef TZ
        vTracePrintF( slack_channel, "%d - %d", xSlackSD, pxTaskSsTCB->xSlack );
#endif

        vTaskDelayUntil( &( pxTaskSsTCB->xPreviousWakeTime ), pxTaskSsTCB->xPeriod );
    }
}

/*****************************************************************************
 * Public functions
 ****************************************************************************/
/**
 * @brief	main routine for FreeRTOS blinky example
 * @return	Nothing, function should not exit
 */
int main(void)
{
    // Verify that configUSE_SLACK_STEALING is enabled
    configSS_ASSERT_EQUAL( configUSE_SLACK_STEALING, 1 );
    // Verify that tskKERNEL_VERSION_MAJOR is >= 9
    configSS_ASSERT_GREATHER_OR_EQUAL( tskKERNEL_VERSION_MAJOR, 9 );

	vCommonSetupHardware();

    // Initializes the trace recorder, but does not start the tracing.
#ifdef TZ
    vTraceEnable( TRC_INIT );
    slack_channel = xTraceRegisterString( "Slack Events" );
#endif

    uartWriteString( UART_USB, "Example 1\r\n" );

    // Periodic tasks.
    xTaskCreate( vPeriodicTask, "T1", 256, NULL, TASK_1_PRIO, &task_handles[ 0 ] );
    xTaskCreate( vPeriodicTask, "T2", 256, NULL, TASK_2_PRIO, &task_handles[ 1 ] );
    xTaskCreate( vPeriodicTask, "T3", 256, NULL, TASK_3_PRIO, &task_handles[ 2 ] );
    xTaskCreate( vPeriodicTask, "T4", 256, NULL, TASK_4_PRIO, &task_handles[ 3 ] );

#if( configUSE_SLACK_STEALING == 1 )
	// Configure additional parameters needed by the slack stealing framework.
    vSlackSetTaskParams( task_handles[ 0 ], PERIODIC_TASK, TASK_1_PERIOD,
    		TASK_1_PERIOD, TASK_1_WCET, 1 );
    vSlackSetTaskParams( task_handles[ 1 ], PERIODIC_TASK, TASK_2_PERIOD,
    		TASK_2_PERIOD, TASK_2_WCET, 2 );
    vSlackSetTaskParams( task_handles[ 2 ], PERIODIC_TASK, TASK_3_PERIOD,
    		TASK_3_PERIOD, TASK_3_WCET, 3 );
    vSlackSetTaskParams( task_handles[ 3 ], PERIODIC_TASK, TASK_4_PERIOD,
            TASK_4_PERIOD, TASK_4_WCET, 4 );
#endif

    // Start the tracing.
#ifdef TZ
    vTraceEnable( TRC_START );
#endif

	// Start the scheduler.
	vTaskStartScheduler();

	// Should never arrive here.
	for(;;);
}
