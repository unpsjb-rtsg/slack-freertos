#include "common.h"
#include "mbed.h"
#include "FreeRTOS.h"
#include "task.h"
#include "slack.h"

#define TASK_CNT 4
#define TASK_1_WCET 1000
#define TASK_2_WCET 1000
#define TASK_3_WCET 1000
#define TASK_4_WCET 1000
#define TASK_1_PERIOD 3000
#define TASK_2_PERIOD 4000
#define TASK_3_PERIOD 6000
#define TASK_4_PERIOD 12000

TaskHandle_t task_handles[ TASK_CNT ];

Serial pc( USBTX, USBRX );

// mbed original LED naming
// LED1 = LED_RED
// LED2 = LED_GREEN
// LED3 = LED_BLUE
// LED4 = LED_RED
DigitalOut leds[] = { LED_RED, LED_GREEN, LED_BLUE, LED_RED };

int main()
{
	pc.baud(9600);
    pc.printf("Example 1\n");

	// turn off all the on board LEDs.
	leds[0] = 1;
	leds[1] = 1;
	leds[2] = 1;
	leds[3] = 1;

    #if( tskKERNEL_VERSION_MAJOR == 9 )
	{
		vSlackSystemSetup();
	}
    #endif

    // create periodic tasks
    xTaskCreate( periodicTaskBody, "T1", 256, NULL, configMAX_PRIORITIES - 2, &task_handles[ 0 ] );  // max priority
    xTaskCreate( periodicTaskBody, "T2", 256, NULL, configMAX_PRIORITIES - 3, &task_handles[ 1 ] );
    xTaskCreate( periodicTaskBody, "T3", 256, NULL, configMAX_PRIORITIES - 4, &task_handles[ 2 ] );
    xTaskCreate( periodicTaskBody, "T4", 256, NULL, configMAX_PRIORITIES - 5, &task_handles[ 3 ] );

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
#endif

    #if( tskKERNEL_VERSION_MAJOR == 9 )
    {
    	vSlackSchedulerSetup();
    }
    #endif

    vTaskStartScheduler();

    for(;;);
}
