#include "mbed.h"
#include "FreeRTOS.h"
#include "task.h"
#include "slack.h"
#include "utils.h"

#define TASK_CNT 4
#define TASK_1_PERIOD 3000
#define TASK_2_PERIOD 4000
#define TASK_3_PERIOD 6000
#define TASK_4_PERIOD 12000
#define TASK_1_WCET 1000
#define TASK_2_WCET 1000
#define TASK_3_WCET 1000
#define TASK_4_WCET 1000

/* Prototypes for the standard FreeRTOS callback/hook functions implemented
 * within this file. The extern "C" is required to avoid name mangling between
 * C and C++ code. */
#if defined (__cplusplus)
extern "C" {
#endif

void vApplicationMallocFailedHook( void );
void vApplicationStackOverflowHook( TaskHandle_t pxTask, char *pcTaskName );

#if( configUSE_SLACK_STEALING == 1 )
void vApplicationDebugAction( void *param );
void vApplicationNotSchedulable( void );
void vApplicationDeadlineMissedHook( char *pcTaskName, UBaseType_t uxRelease, TickType_t xTickCount );
#endif

#if defined (__cplusplus)
}
#endif

void task_body( void* params );

TaskHandle_t task_handles[ TASK_CNT ];

Serial pc( USBTX, USBRX );

// mbed LPC 1768 on-board LEDs.
DigitalOut leds[] = { LED1, LED2, LED3, LED4 };

TickType_t xTaskPeriods[] = { TASK_1_PERIOD, TASK_2_PERIOD, TASK_3_PERIOD, TASK_4_PERIOD };

int main()
{
	pc.baud(9600);
    pc.printf("Example 1\n");

	// turn off all the on board LEDs.
	leds[0] = 0;
	leds[1] = 0;
	leds[2] = 0;
	leds[3] = 0;

    // create periodic tasks
    xTaskCreate( task_body, "Task1", 256, ( TickType_t * ) 0, configMAX_PRIORITIES - 2, &task_handles[ 0 ] );  // max priority
    xTaskCreate( task_body, "Task2", 256, ( TickType_t * ) 1, configMAX_PRIORITIES - 3, &task_handles[ 1 ] );
    xTaskCreate( task_body, "Task3", 256, ( TickType_t * ) 2, configMAX_PRIORITIES - 4, &task_handles[ 2 ] );
    xTaskCreate( task_body, "Task4", 256, ( TickType_t * ) 3, configMAX_PRIORITIES - 5, &task_handles[ 3 ] );

#if( configUSE_SLACK_STEALING == 1 )
    // additional parameters needed by the slack stealing framework
    vTaskSetParams( task_handles[ 0 ], TASK_1_PERIOD, TASK_1_PERIOD, TASK_1_WCET, 1 );
    vTaskSetParams( task_handles[ 1 ], TASK_2_PERIOD, TASK_2_PERIOD, TASK_2_WCET, 2 );
    vTaskSetParams( task_handles[ 2 ], TASK_3_PERIOD, TASK_3_PERIOD, TASK_3_WCET, 3 );
    vTaskSetParams( task_handles[ 3 ], TASK_4_PERIOD, TASK_4_PERIOD, TASK_4_WCET, 4 );
#endif

    vTaskStartScheduler();

    for(;;);
}

void task_body( void* params )
{
	TickType_t xPreviousWakeTime = xTaskGetTickCount();

	uint32_t xTaskId = ( uint32_t ) params;

    int32_t slackArray[ 7 ];

	for(;;)
    {
		leds[ xTaskId ] = 1;
		vUtilsEatCpu( 1000 );
		leds[ xTaskId ] = 0;

        vTaskSuspendAll();
		vTasksGetSlacks( slackArray );
		pc.printf("%s - Tick: %d - AS: %d - [ %d, %d, %d, %d ]\n",
				pcTaskGetTaskName(NULL), slackArray[0], slackArray[2],
				slackArray[3], slackArray[4], slackArray[5], slackArray[6]);
		xTaskResumeAll();

		vTaskDelayUntil( &xPreviousWakeTime, xTaskPeriods[ xTaskId ] );
    }
}

void vApplicationMallocFailedHook( void )
{
	taskDISABLE_INTERRUPTS();

	for( ;; )
	{
        leds[ 2 ] = 1;
        wait_ms(1000);
        leds[ 2 ] = 0;
        wait_ms(500);
	}
}

void vApplicationStackOverflowHook( TaskHandle_t pxTask, char *pcTaskName )
{
	( void ) pcTaskName;
	( void ) pxTask;

	taskDISABLE_INTERRUPTS();

	for( ;; )
	{
        leds[ 2 ] = 1;
        wait_ms(500);
        leds[ 2 ] = 0;
        wait_ms(500);
	}
}

#if ( configUSE_SLACK_STEALING == 1 )
void vApplicationDebugAction( void *param )
{
    ( void ) param;

	taskDISABLE_INTERRUPTS();

	for( ;; )
	{
        leds[ 4 ] = 1;
        wait_ms(1000);
        leds[ 4 ] = 0;
        wait_ms(1000);
	}
}

void vApplicationNotSchedulable( void )
{
	taskDISABLE_INTERRUPTS();

	for( ;; )
	{
        leds[ 1 ] = 1;
        wait_ms(1000);
        leds[ 1 ] = 0;
        wait_ms(1000);
	}
}

void vApplicationDeadlineMissedHook( char *pcTaskName, UBaseType_t uxRelease, TickType_t xTickCount )
{
    ( void ) uxRelease;
    ( void ) xTickCount;

    taskDISABLE_INTERRUPTS();

    pc.printf( "%s\r\n", pcTaskName );

    for( ;; )
    {
        leds[ 0 ] = 1;
        wait_ms( 1000 );
        leds[ 0 ] = 0;
        wait_ms( 1000 );
    }
}
#endif
