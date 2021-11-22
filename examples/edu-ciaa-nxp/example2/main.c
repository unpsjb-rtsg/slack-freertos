/*****************************************************************************
 *
 * Example 2 for EDU-CIAA-NXP
 *
 * This program consist of 3 real-time periodic tasks and 3 aperiodic tasks,
 * the later only scheduled when there is available slack in the system. All
 * the tasks write a string with some data to to the serial port, when
 * starting and finishing each instance.
 *
 * Before writing to the serial port, the tasks try to take a shared mutex.
 * This could lead to the following problem: when an aperiodic task has taken
 * the mutex and then the available slack depletes, the periodic tasks can't
 * take the mutex, and a missed deadline will occur.
 *
 * Created on: 20 Jun 2017
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
#include "stdlib.h" // for rand()

/*****************************************************************************
 * Macros and definitions
 ****************************************************************************/
/* The linker does not include this code in liblpc.a because nothing in it
 * references it... */
#define CRP_NO_CRP          0xFFFFFFFF
__attribute__ ((used,section(".crp"))) const unsigned int CRP_WORD = CRP_NO_CRP ;

#define TASK_CNT 3
#define ATASK_CNT 3

#define TASK_1_WCET 1000
#define TASK_2_WCET 1000
#define TASK_3_WCET 1000
#define TASK_1_PERIOD 3000
#define TASK_2_PERIOD 4000
#define TASK_3_PERIOD 6000
#define TASK_1_PRIO configMAX_PRIORITIES - configSS_SLACK_PRIOS - 1
#define TASK_2_PRIO configMAX_PRIORITIES - configSS_SLACK_PRIOS - 2
#define TASK_3_PRIO configMAX_PRIORITIES - configSS_SLACK_PRIOS - 3

#define ATASK_WCET 2000
#define ATASK_MAX_DELAY 4000
#define ATASK_1_PRIO configMAX_PRIORITIES - 1
#define ATASK_2_PRIO configMAX_PRIORITIES - 2
#define ATASK_3_PRIO configMAX_PRIORITIES - 3

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
static void vAperiodicTask( void* params );

/*****************************************************************************
 * Private data
 ****************************************************************************/
static TaskHandle_t task_handles[ TASK_CNT ];
static TaskHandle_t atask_handles[ ATASK_CNT ];

/*****************************************************************************
 * Public data
 ****************************************************************************/
gpioMap_t leds[] = { LED1, LED2, LED3 };
gpioMap_t aleds[] = { LEDR, LEDG, LEDB };
#if defined( TRACEALYZER_v3_3_1 )
traceString slack_channel;
#endif

/*****************************************************************************
 * Private functions
 ****************************************************************************/
/**
 * Aperiodic task.
 * @param params Not used.
 */
static void vAperiodicTask( void* params )
{
    int32_t slackArray[ 6 ];

    SsTCB_t *pxTaskSsTCB;

#if( tskKERNEL_VERSION_MAJOR >= 10 )
    pxTaskSsTCB = pvSlackGetTaskSsTCB( NULL );
#endif

    vTaskDelay( rand() % ATASK_MAX_DELAY );

    for(;;)
    {
        pxTaskSsTCB->xCur = ( TickType_t ) 0;

        gpioWrite( aleds[ pxTaskSsTCB->xId - 1], ON );

        vCommonPrintSlacks( 'S', slackArray, pxTaskSsTCB->xCur );

        vUtilsBusyWait( rand() % ATASK_WCET );

        vCommonPrintSlacks( 'E', slackArray, pxTaskSsTCB->xCur );

        gpioWrite( aleds[ pxTaskSsTCB->xId - 1], OFF );

        vTaskDelay( rand() % ATASK_MAX_DELAY );
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
	vCommonSetupHardware();

#if defined( TRACEALYZER_v3_3_1 )
    // Initializes the trace recorder, but does not start the tracing.
    vTraceEnable( TRC_INIT );
    slack_channel = xTraceRegisterString("Slack Events");
#endif

    uartWriteString( UART_USB, "Example 2\r\n" );

    // create periodic tasks
    xTaskCreate( vCommonPeriodicTask, "T1", 256, NULL, TASK_1_PRIO, &task_handles[ 0 ] );
    xTaskCreate( vCommonPeriodicTask, "T2", 256, NULL, TASK_2_PRIO, &task_handles[ 1 ] );
    xTaskCreate( vCommonPeriodicTask, "T3", 256, NULL, TASK_3_PRIO, &task_handles[ 2 ] );

    /* Aperiodic task */
    xTaskCreate ( vAperiodicTask, "TA1", 256, NULL, ATASK_1_PRIO, &atask_handles[ 0 ] );
    xTaskCreate ( vAperiodicTask, "TA2", 256, NULL, ATASK_2_PRIO, &atask_handles[ 1 ] );
    xTaskCreate ( vAperiodicTask, "TA3", 256, NULL, ATASK_3_PRIO, &atask_handles[ 2 ] );

#if( configUSE_SLACK_STEALING == 1 )
    vSlackSetTaskParams( task_handles[ 0 ], PERIODIC_TASK, TASK_1_PERIOD,
            TASK_1_PERIOD, TASK_1_WCET, 1 );
    vSlackSetTaskParams( task_handles[ 1 ], PERIODIC_TASK, TASK_2_PERIOD,
            TASK_2_PERIOD, TASK_2_WCET, 2 );
    vSlackSetTaskParams( task_handles[ 2 ], PERIODIC_TASK, TASK_3_PERIOD,
            TASK_3_PERIOD, TASK_3_WCET, 3 );

    /* Aperiodic task */
    vSlackSetTaskParams( atask_handles[ 0 ], APERIODIC_TASK, ATASK_MAX_DELAY,
            0, ATASK_WCET, 1 );
    vSlackSetTaskParams( atask_handles[ 1 ], APERIODIC_TASK, ATASK_MAX_DELAY,
            0, ATASK_WCET, 2 );
    vSlackSetTaskParams( atask_handles[ 2 ], APERIODIC_TASK, ATASK_MAX_DELAY,
            0, ATASK_WCET, 3 );
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
