#include "mbed.h"
#include "FreeRTOS.h"
#include "task.h"
#include "slack.h"
#include "utils.h"
#include "common.h"

#define TASK_CNT 4
#define TASK_1_WCET 1000
#define TASK_2_WCET 1000
#define TASK_3_WCET 1000
#define TASK_4_WCET 1000
#define TASK_1_PERIOD 3000
#define TASK_2_PERIOD 4000
#define TASK_3_PERIOD 6000
#define TASK_4_PERIOD 12000

void aperiodic_task_body( void* params );

TaskHandle_t task_handles[ TASK_CNT ];

Serial pc( USBTX, USBRX );
DigitalOut leds[] = { LED1, LED2, LED3, LED4 };

int main()
{
	// Initializes the trace recorder, but does not start the tracing.
#ifdef TRACEALYZER_v3_0_2
	vTraceInitTraceData();
#endif
#ifdef TRACEALYZER_v3_1_3
	vTraceEnable( TRC_INIT );
#endif

	pc.baud(9600);
    pc.printf("Example 3\n");

	// Turn off all the on board LEDs.
	leds[0] = 0;
	leds[1] = 0;
	leds[2] = 0;
	leds[3] = 0;

#if( tskKERNEL_VERSION_MAJOR == 9 )
	vSlackSystemSetup();
#endif

    // Create the periodic tasks.
    xTaskCreate( periodicTaskBody, "T1", 256, NULL, configMAX_PRIORITIES - 3, &task_handles[ 0 ] );  // max priority
    xTaskCreate( periodicTaskBody, "T2", 256, NULL, configMAX_PRIORITIES - 4, &task_handles[ 1 ] );
    xTaskCreate( periodicTaskBody, "T3", 256, NULL, configMAX_PRIORITIES - 5, &task_handles[ 2 ] );
    xTaskCreate( periodicTaskBody, "T4", 256, NULL, configMAX_PRIORITIES - 6, &task_handles[ 3 ] );

#if( configUSE_SLACK_STEALING == 1 )
    // Additional parameters needed by the slack stealing framework.
#if( tskKERNEL_VERSION_MAJOR == 8 )
    vTaskSetParams( task_handles[ 0 ], TASK_1_PERIOD, TASK_1_PERIOD, TASK_1_WCET, 1 );
    vTaskSetParams( task_handles[ 1 ], TASK_2_PERIOD, TASK_2_PERIOD, TASK_2_WCET, 2 );
    vTaskSetParams( task_handles[ 2 ], TASK_3_PERIOD, TASK_3_PERIOD, TASK_3_WCET, 3 );
    vTaskSetParams( task_handles[ 3 ], TASK_4_PERIOD, TASK_4_PERIOD, TASK_4_WCET, 4 );

    // Create the aperiodic task.
    xTaskCreate( aperiodic_task_body, "TA", 256, NULL, configMAX_PRIORITIES - 1, NULL );
#endif
#if( tskKERNEL_VERSION_MAJOR == 9 )
    vSlackSetTaskParams( task_handles[ 0 ], PERIODIC_TASK, TASK_1_PERIOD, TASK_1_PERIOD, TASK_1_WCET, 1 );
    vSlackSetTaskParams( task_handles[ 1 ], PERIODIC_TASK, TASK_2_PERIOD, TASK_2_PERIOD, TASK_2_WCET, 2 );
    vSlackSetTaskParams( task_handles[ 2 ], PERIODIC_TASK, TASK_3_PERIOD, TASK_3_PERIOD, TASK_3_WCET, 3 );
    vSlackSetTaskParams( task_handles[ 3 ], PERIODIC_TASK, TASK_4_PERIOD, TASK_4_PERIOD, TASK_4_WCET, 4 );

    // Create the aperiodic tasks.
    TaskHandle_t xApTaskHandle1, xApTaskHandle2;
    xTaskCreate( aperiodic_task_body, "TA1", 256, NULL, configMAX_PRIORITIES - 1, &xApTaskHandle1 );
    xTaskCreate( aperiodic_task_body, "TA2", 256, NULL, configMAX_PRIORITIES - 2, &xApTaskHandle2 );
    vSlackSetTaskParams( xApTaskHandle1, APERIODIC_TASK, 0, 0, 0, 5 );
    vSlackSetTaskParams( xApTaskHandle2, APERIODIC_TASK, 0, 0, 0, 5 );
#endif
#endif

#if( tskKERNEL_VERSION_MAJOR == 9 )
    vSlackSchedulerSetup();
#endif

    // Starts the tracing.
#ifdef TRACEALYZER_v3_0_2
    uiTraceStart();
#endif
#ifdef TRACEALYZER_v3_1_3
    vTraceEnable( TRC_START );
#endif

    vTaskStartScheduler();

    for(;;);
}

void aperiodic_task_body( void* params )
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

	for(;;)
	{
		pxTaskSsTCB->xCur = ( TickType_t ) 0;

		printSlacks( 'S', slackArray, pxTaskSsTCB->xCur );

		vUtilsEatCpu( 100 + ( rand() % 2000 ) );

		printSlacks( 'E', slackArray, pxTaskSsTCB->xCur );

		vTaskDelay( rand() % 8000 );
	}
}
