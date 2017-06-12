#include "mbed.h"
#include "FreeRTOS.h"
#include "task.h"
#include "slack.h"
#include "utils.h"
#include "common.h"

#define TASK_CNT 4
#define TASK_1_PERIOD 3000
#define TASK_2_PERIOD 4000
#define TASK_3_PERIOD 6000
#define TASK_4_PERIOD 12000
#define TASK_1_WCET 1000
#define TASK_2_WCET 1000
#define TASK_3_WCET 1000
#define TASK_4_WCET 1000

/* The extern "C" is required to avoid name mangling between C and C++ code. */
extern "C"
{
void vApplicationMallocFailedHook( void );
void vApplicationStackOverflowHook( TaskHandle_t pxTask, char *pcTaskName );

#if( configUSE_SLACK_STEALING == 1 )
void vApplicationDebugAction( void *param );
void vApplicationNotSchedulable( void );
void vApplicationDeadlineMissedHook( char *pcTaskName, UBaseType_t uxRelease, TickType_t xTickCount );
#endif
}

void task_body( void* params );
void aperiodic_task_body( void* params );

TaskHandle_t task_handles[ TASK_CNT ];

Serial pc( USBTX, USBRX );
DigitalOut leds[] = { LED1, LED2, LED3, LED4 };

int main()
{
	pc.baud(9600);
    pc.printf("Example 2\n");

	// turn off all the on board LEDs.
	leds[0] = 0;
	leds[1] = 0;
	leds[2] = 0;
	leds[3] = 0;

	vSlackSystemSetup();

    // create periodic tasks
    xTaskCreate( task_body, "T1", 256, NULL, configMAX_PRIORITIES - 2, &task_handles[ 0 ] );  // max priority
    xTaskCreate( task_body, "T2", 256, NULL, configMAX_PRIORITIES - 3, &task_handles[ 1 ] );
    xTaskCreate( task_body, "T3", 256, NULL, configMAX_PRIORITIES - 4, &task_handles[ 2 ] );
    xTaskCreate( task_body, "T4", 256, NULL, configMAX_PRIORITIES - 5, &task_handles[ 3 ] );

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
#endif
    /* Aperiodic task */
    TaskHandle_t xApTaskHandle;
    xTaskCreate ( aperiodic_task_body, "TA", 256, NULL, configMAX_PRIORITIES - 1, &xApTaskHandle );
    vSlackSetTaskParams( xApTaskHandle, APERIODIC_TASK, 0, 0, 0, 5 );
#endif

    vSlackSchedulerSetup();

    vTaskStartScheduler();

    for(;;);
}

void aperiodic_task_body( void* params )
{
	int32_t slackArray[ 7 ];

	SsTCB_t *pxTaskSsTCB = getTaskSsTCB( NULL );

	for(;;)
	{
		vTaskSuspendAll();
		vTasksGetSlacks( slackArray );
		pc.printf("%s\tS\t%d\t%d\t%d\t%d\t%d\t%d\n",
				pcTaskGetTaskName(NULL), slackArray[0], slackArray[2],
				slackArray[3], slackArray[4], slackArray[5], slackArray[6]);
		xTaskResumeAll();

		vUtilsEatCpu( 2000 );

		vTaskSuspendAll();
		vTasksGetSlacks( slackArray );
		pc.printf("%s\tE\t%d\t%d\t%d\t%d\t%d\t%d\t%d\n",
				pcTaskGetTaskName(NULL), slackArray[0], slackArray[2],
				slackArray[3], slackArray[4], slackArray[5], slackArray[6],
				pxTaskSsTCB->xCur);
		xTaskResumeAll();

		vTaskDelay( rand() % 4000 );
	}
}

void task_body( void* params )
{
	SsTCB_t *pxTaskSsTCB = getTaskSsTCB( NULL );

    int32_t slackArray[ 7 ];

	for(;;)
    {
		vTaskSuspendAll();
		vTasksGetSlacks( slackArray );
		pc.printf("%s\tS\t%d\t%d\t%d\t%d\t%d\t%d\n",
				pcTaskGetTaskName(NULL), slackArray[0], slackArray[2],
				slackArray[3], slackArray[4], slackArray[5], slackArray[6]);
		xTaskResumeAll();

		leds[ pxTaskSsTCB->xId - 1] = 1;

#if ( configTASK_EXEC == 0 )
		vUtilsEatCpu( pxTaskSsTCB->xWcet - 250 );
#endif
#if ( configTASK_EXEC == 1 )
		while( pxTaskSsTCB->xCur <  pxTaskSsTCB->xWcet )
		{
			asm("nop");
		}
#endif

		leds[ pxTaskSsTCB->xId - 1] = 0;

		vTaskSuspendAll();
		vTasksGetSlacks( slackArray );
		pc.printf("%s\tE\t%d\t%d\t%d\t%d\t%d\t%d\t%d\n",
				pcTaskGetTaskName(NULL), slackArray[0], slackArray[2],
				slackArray[3], slackArray[4], slackArray[5], slackArray[6],
				pxTaskSsTCB->xCur);
		xTaskResumeAll();

		vTaskDelayUntil( &( pxTaskSsTCB->xPreviousWakeTime ), pxTaskSsTCB->xPeriod );
    }
}
