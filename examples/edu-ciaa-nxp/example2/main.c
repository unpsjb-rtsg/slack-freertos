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
#include "semphr.h"
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

#define mainMAX_MSG_LEN 50

#define MUTEX_BLOCK_TIME 50

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
static void vAperiodicTask( void* params );

/*****************************************************************************
 * Private data
 ****************************************************************************/
static TaskHandle_t task_handles[ TASK_CNT ];
static TaskHandle_t atask_handles[ ATASK_CNT ];
static char cMessage[ mainMAX_MSG_LEN ];
static SemaphoreHandle_t mutex;

/*****************************************************************************
 * Public data
 ****************************************************************************/
gpioMap_t leds[] = { LED1, LED2, LED3 };
gpioMap_t aleds[] = { LEDR, LEDG, LEDB };

/*****************************************************************************
 * Private functions
 ****************************************************************************/
void vPeriodicTask( void* params )
{
    int32_t taskId = (int32_t) params;

    int32_t slackArray[ 6 ];

    SsTCB_t *pxTaskSsTCB = pvSlackGetTaskSsTCB( NULL );

    for(;;)
    {
        gpioWrite( leds[ taskId - 1], ON);

        vTasksGetSlacks( slackArray );
        if ( xSemaphoreTake( mutex, MUTEX_BLOCK_TIME ) == pdTRUE ) {
            vCommonPrintSlacks( 'S', slackArray, pxTaskSsTCB );
            xSemaphoreGive( mutex );
        }

        vUtilsBusyWait( pxTaskSsTCB->xWcet - 300 );

        vTasksGetSlacks( slackArray );

        if ( xSemaphoreTake( mutex, MUTEX_BLOCK_TIME ) == pdTRUE ) {
            vCommonPrintSlacks( 'E', slackArray, pxTaskSsTCB );
            xSemaphoreGive( mutex );
        }

        gpioWrite( leds[ taskId - 1], OFF);

        xTaskDelayUntil( &( pxTaskSsTCB->xPreviousWakeTime ), pxTaskSsTCB->xPeriod );
    }
}
/*-----------------------------------------------------------*/

/**
 * Aperiodic task.
 * @param params Not used.
 */
static void vAperiodicTask( void* params )
{
    int32_t taskId = (int32_t) params;

    int32_t slackArray[ 6 ];

    SsTCB_t *pxTaskSsTCB = pvSlackGetTaskSsTCB( NULL );

    vTaskDelay( rand() % ATASK_MAX_DELAY );

    for(;;)
    {
        pxTaskSsTCB->xCur = ( TickType_t ) 0;

        gpioWrite( aleds[ taskId - 1], ON );

        vTasksGetSlacks( slackArray );
        if ( xSemaphoreTake( mutex, MUTEX_BLOCK_TIME ) == pdTRUE ) {
            vCommonPrintSlacks( 'S', slackArray, pxTaskSsTCB );
            xSemaphoreGive( mutex );
        }

        vUtilsBusyWait( rand() % ATASK_WCET );

        vTasksGetSlacks( slackArray );
        if ( xSemaphoreTake( mutex, MUTEX_BLOCK_TIME ) == pdTRUE ) {
            vCommonPrintSlacks( 'E', slackArray, pxTaskSsTCB );
            xSemaphoreGive( mutex );
        }

        gpioWrite( aleds[ taskId - 1], OFF );

        vTaskDelay( rand() % ATASK_MAX_DELAY );

        // uxReleaseCount is not incremented when vTaskDelay() is used.
        pxTaskSsTCB->uxReleaseCount += 1;
    }
}

/*****************************************************************************
 * Public functions
 ****************************************************************************/
void vApplicationTickHook( void )
{
	vSlackDeadlineCheck();
}

/**
 * @brief	main routine for FreeRTOS blinky example
 * @return	Nothing, function should not exit
 */
int main(void)
{
	vCommonSetupHardware();

#if defined( TRACEALYZER )
    // Initializes the trace recorder, but does not start the tracing.
    vTraceEnable( TRC_INIT );
#endif

#if defined( TRACEALYZER )
    // Initializes the trace recorder, but does not start the tracing.
    vTraceEnable( TRC_INIT );
#endif

    uartWriteString( UART_USB, "EDU-CIAA-NXP -- Example 2\r\n" );
    sprintf( cMessage, "> FreeRTOS %s\n\r", tskKERNEL_VERSION_NUMBER );
    uartWriteString( UART_USB, cMessage);
#if defined( TRACEALYZER )
    uartWriteString( UART_USB, "> Tracealyzer compiled.\r\n");
#else
    uartWriteString( UART_USB, "> Tracealyzer not compiled.\r\n");
#endif

    // Initialize the mutex
    mutex = xSemaphoreCreateMutex();

    // create periodic tasks
    xTaskCreate( vPeriodicTask, "T1", 256, (void*) 1, TASK_1_PRIO, &task_handles[ 0 ] );
    xTaskCreate( vPeriodicTask, "T2", 256, (void*) 2, TASK_2_PRIO, &task_handles[ 1 ] );
    xTaskCreate( vPeriodicTask, "T3", 256, (void*) 3, TASK_3_PRIO, &task_handles[ 2 ] );

    /* Aperiodic task */
    xTaskCreate ( vAperiodicTask, "TA1", 256, (void*) 1, ATASK_1_PRIO, &atask_handles[ 0 ] );
    xTaskCreate ( vAperiodicTask, "TA2", 256, (void*) 2, ATASK_2_PRIO, &atask_handles[ 1 ] );
    xTaskCreate ( vAperiodicTask, "TA3", 256, (void*) 3, ATASK_3_PRIO, &atask_handles[ 2 ] );

#if( configUSE_SLACK_STEALING == 1 )
    vSlackSetTaskParams( task_handles[ 0 ], PERIODIC_TASK, TASK_1_PERIOD,
            TASK_1_PERIOD, TASK_1_WCET );
    vSlackSetTaskParams( task_handles[ 1 ], PERIODIC_TASK, TASK_2_PERIOD,
            TASK_2_PERIOD, TASK_2_WCET );
    vSlackSetTaskParams( task_handles[ 2 ], PERIODIC_TASK, TASK_3_PERIOD,
            TASK_3_PERIOD, TASK_3_WCET );

    /* Aperiodic task */
    vSlackSetTaskParams( atask_handles[ 0 ], APERIODIC_TASK, ATASK_MAX_DELAY,
            0, ATASK_WCET );
    vSlackSetTaskParams( atask_handles[ 1 ], APERIODIC_TASK, ATASK_MAX_DELAY,
            0, ATASK_WCET );
    vSlackSetTaskParams( atask_handles[ 2 ], APERIODIC_TASK, ATASK_MAX_DELAY,
            0, ATASK_WCET );
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
