/*****************************************************************************
 *
 * Example 1 for EDU-CIAA-NXP
 *
 * This program consist of 4 real-time periodic tasks. Each task write a string
 * with some data to the serial port, when starting and finishing each instance.
 * Before writing to the serial port, the tasks try to take a shared mutex.
 *
 * Author: Francisco E. Páez
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

#define TASK_CNT 3
#define TASK_1_WCET 1000
#define TASK_2_WCET 1000
#define TASK_3_WCET 1000
#define TASK_1_PERIOD 3000
#define TASK_2_PERIOD 4000
#define TASK_3_PERIOD 6000
#define TASK_1_PRIO configMAX_PRIORITIES - configSS_SLACK_PRIOS - 1
#define TASK_2_PRIO configMAX_PRIORITIES - configSS_SLACK_PRIOS - 2
#define TASK_3_PRIO configMAX_PRIORITIES - configSS_SLACK_PRIOS - 3

#define mainMAX_MSG_LEN ( 150 )

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
static char cMessage[ mainMAX_MSG_LEN ];

/*****************************************************************************
 * Public data
 ****************************************************************************/
gpioMap_t leds[] = { LED1, LED2, LED3 };

/*****************************************************************************
 * Private functions
 ****************************************************************************/
/* None */

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

    uartWriteString( UART_USB, "EDU-CIAA-NXP -- Example 1\r\n" );
    sprintf(cMessage, "> FreeRTOS %s\n\r", tskKERNEL_VERSION_NUMBER );
    uartWriteString( UART_USB, cMessage);
#if defined( TRACEALYZER )
    uartWriteString( UART_USB, "> Tracealyzer compiled.\r\n");
#else
    uartWriteString( UART_USB, "> Tracealyzer not compiled.\r\n");
#endif

    // Periodic tasks.
    xTaskCreate( vCommonPeriodicTask, "T1", 256, NULL, TASK_1_PRIO, &task_handles[ 0 ] );
    xTaskCreate( vCommonPeriodicTask, "T2", 256, NULL, TASK_2_PRIO, &task_handles[ 1 ] );
    xTaskCreate( vCommonPeriodicTask, "T3", 256, NULL, TASK_3_PRIO, &task_handles[ 2 ] );

#if ( configUSE_SLACK_STEALING == 1 )
    vSlackSetTaskParams( task_handles[ 0 ], PERIODIC_TASK, TASK_1_PERIOD,
            TASK_1_PERIOD, TASK_1_WCET );
    vSlackSetTaskParams( task_handles[ 1 ], PERIODIC_TASK, TASK_2_PERIOD,
            TASK_2_PERIOD, TASK_2_WCET );
    vSlackSetTaskParams( task_handles[ 2 ], PERIODIC_TASK, TASK_3_PERIOD,
            TASK_3_PERIOD, TASK_3_WCET );
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
