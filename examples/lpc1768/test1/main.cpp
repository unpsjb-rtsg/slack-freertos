#include "mbed.h"
#include "FreeRTOS.h"
#include "task.h"

#if ( configUSE_SLACK_STEALING == 1 )
#include "slack.h"
#endif

#if ( tskKERNEL_VERSION_MAJOR >= 9 )
#include "slack_tests.h"
#endif

#ifndef TASK_COUNT
#define TASK_COUNT 10
#endif

#define TASK_1_PERIOD 64
#define TASK_2_PERIOD 321
#define TASK_3_PERIOD 354
#define TASK_4_PERIOD 392
#define TASK_5_PERIOD 429
#define TASK_6_PERIOD 468
#define TASK_7_PERIOD 498
#define TASK_8_PERIOD 515
#define TASK_9_PERIOD 711
#define TASK_10_PERIOD 882

#define TASK_1_WCET 1
#define TASK_1_RUNTIME 1
#define TASK_2_WCET 3
#define TASK_2_RUNTIME 3
#define TASK_3_WCET 1
#define TASK_3_RUNTIME 1
#define TASK_4_WCET 8
#define TASK_4_RUNTIME 8
#define TASK_5_WCET 2
#define TASK_5_RUNTIME 2
#define TASK_6_WCET 2
#define TASK_6_RUNTIME 2
#define TASK_7_WCET 2
#define TASK_7_RUNTIME 2
#define TASK_8_WCET 8
#define TASK_8_RUNTIME 8
#define TASK_9_WCET 14
#define TASK_9_RUNTIME 14
#define TASK_10_WCET 3
#define TASK_10_RUNTIME 3

#define TASK_STACK_SIZE configMINIMAL_STACK_SIZE

/* Prototypes for the standard FreeRTOS callback/hook functions implemented within this file. */
/* The extern "C" is required to avoid name mangling between C and C++ code. */
#if defined (__cplusplus)
extern "C" {
#endif

#if( configUSE_SLACK_STEALING == 1 )
void vApplicationDebugAction( void *param );
#endif
void vApplicationMallocFailedHook( void );
void vApplicationStackOverflowHook( TaskHandle_t pxTask, char *pcTaskName );

#if defined (__cplusplus)
}
#endif

Serial pc( USBTX, USBRX );

DigitalOut leds[ ] = { LED1, LED2, LED3, LED4 };

static void vBusyWait( TickType_t ticks );

static void prvPeriodicTask( void *pvParameters );

int main() 
{
    pc.baud(9600);
    
    /* Reserve memory for task_handle array */
    TaskHandle_t *task_handle = ( TaskHandle_t * ) pvPortMalloc( sizeof( TaskHandle_t ) * TASK_COUNT );

    /* Periodic tasks. */
    /* xTaskCreate( task_body, "T01", TASK_STACK_SIZE, ( void * ) 0, configMAX_PRIORITIES - 2, &task_handle[0] ); */    
xTaskCreate( prvPeriodicTask, "T01", TASK_STACK_SIZE, ( void * ) 0, configMAX_PRIORITIES - 2,  &task_handle[0] );
xTaskCreate( prvPeriodicTask, "T02", TASK_STACK_SIZE, ( void * ) 1, configMAX_PRIORITIES - 3,  &task_handle[1] );
xTaskCreate( prvPeriodicTask, "T03", TASK_STACK_SIZE, ( void * ) 2, configMAX_PRIORITIES - 4,  &task_handle[2] );
xTaskCreate( prvPeriodicTask, "T04", TASK_STACK_SIZE, ( void * ) 3, configMAX_PRIORITIES - 5,  &task_handle[3] );
xTaskCreate( prvPeriodicTask, "T05", TASK_STACK_SIZE, ( void * ) 4, configMAX_PRIORITIES - 6,  &task_handle[4] );
xTaskCreate( prvPeriodicTask, "T06", TASK_STACK_SIZE, ( void * ) 5, configMAX_PRIORITIES - 7,  &task_handle[5] );
xTaskCreate( prvPeriodicTask, "T07", TASK_STACK_SIZE, ( void * ) 6, configMAX_PRIORITIES - 8,  &task_handle[6] );
xTaskCreate( prvPeriodicTask, "T08", TASK_STACK_SIZE, ( void * ) 7, configMAX_PRIORITIES - 9,  &task_handle[7] );
xTaskCreate( prvPeriodicTask, "T09", TASK_STACK_SIZE, ( void * ) 8, configMAX_PRIORITIES - 10, &task_handle[8] );
xTaskCreate( prvPeriodicTask, "T10", TASK_STACK_SIZE, ( void * ) 9, configMAX_PRIORITIES - 11, &task_handle[9] );

#if ( configUSE_SLACK_STEALING == 1 ) && ( tskKERNEL_VERSION_MAJOR == 8 )
    /* vTaskSetParams( task_handle[ 0 ], TASK_1_PERIOD, TASK_1_PERIOD, TASK_1_WCET, 0 ); */
vTaskSetParams( task_handle[ 0 ], TASK_1_PERIOD, TASK_1_PERIOD, TASK_1_WCET, 0 );
vTaskSetParams( task_handle[ 1 ], TASK_2_PERIOD, TASK_2_PERIOD, TASK_2_WCET, 1 );
vTaskSetParams( task_handle[ 2 ], TASK_3_PERIOD, TASK_3_PERIOD, TASK_3_WCET, 2 );
vTaskSetParams( task_handle[ 3 ], TASK_4_PERIOD, TASK_4_PERIOD, TASK_4_WCET, 3 );
vTaskSetParams( task_handle[ 4 ], TASK_5_PERIOD, TASK_5_PERIOD, TASK_5_WCET, 4 );
vTaskSetParams( task_handle[ 5 ], TASK_6_PERIOD, TASK_6_PERIOD, TASK_6_WCET, 5 );
vTaskSetParams( task_handle[ 6 ], TASK_7_PERIOD, TASK_7_PERIOD, TASK_7_WCET, 6 );
vTaskSetParams( task_handle[ 7 ], TASK_8_PERIOD, TASK_8_PERIOD, TASK_8_WCET, 7 );
vTaskSetParams( task_handle[ 8 ], TASK_9_PERIOD, TASK_9_PERIOD, TASK_9_WCET, 8 );
vTaskSetParams( task_handle[ 9 ], TASK_10_PERIOD, TASK_10_PERIOD, TASK_10_WCET, 9 );
#endif

#if ( configUSE_SLACK_STEALING == 1 ) && ( tskKERNEL_VERSION_MAJOR >= 9 )
    /* vSlackSetTaskParams( task_handle[ 0 ], TASK_1_PERIOD, TASK_1_PERIOD, TASK_1_WCET, 0 ); */
vSlackSetTaskParams( task_handle[ 0 ], PERIODIC_TASK, TASK_1_PERIOD, TASK_1_PERIOD, TASK_1_WCET, 0 );
vSlackSetTaskParams( task_handle[ 1 ], PERIODIC_TASK, TASK_2_PERIOD, TASK_2_PERIOD, TASK_2_WCET, 1 );
vSlackSetTaskParams( task_handle[ 2 ], PERIODIC_TASK, TASK_3_PERIOD, TASK_3_PERIOD, TASK_3_WCET, 2 );
vSlackSetTaskParams( task_handle[ 3 ], PERIODIC_TASK, TASK_4_PERIOD, TASK_4_PERIOD, TASK_4_WCET, 3 );
vSlackSetTaskParams( task_handle[ 4 ], PERIODIC_TASK, TASK_5_PERIOD, TASK_5_PERIOD, TASK_5_WCET, 4 );
vSlackSetTaskParams( task_handle[ 5 ], PERIODIC_TASK, TASK_6_PERIOD, TASK_6_PERIOD, TASK_6_WCET, 5 );
vSlackSetTaskParams( task_handle[ 6 ], PERIODIC_TASK, TASK_7_PERIOD, TASK_7_PERIOD, TASK_7_WCET, 6 );
vSlackSetTaskParams( task_handle[ 7 ], PERIODIC_TASK, TASK_8_PERIOD, TASK_8_PERIOD, TASK_8_WCET, 7 );
vSlackSetTaskParams( task_handle[ 8 ], PERIODIC_TASK, TASK_9_PERIOD, TASK_9_PERIOD, TASK_9_WCET, 8 );
vSlackSetTaskParams( task_handle[ 9 ], PERIODIC_TASK, TASK_10_PERIOD, TASK_10_PERIOD, TASK_10_WCET, 9 );
#endif

#if ( configUSE_SLACK_STEALING == 0 ) && ( configKERNEL_TEST == 1 )
    /* vTaskSetParams( task_handle[ 0 ], 0 ); */
vTaskSetParams( task_handle[ 0 ], 0 );
vTaskSetParams( task_handle[ 1 ], 1 );
vTaskSetParams( task_handle[ 2 ], 2 );
vTaskSetParams( task_handle[ 3 ], 3 );
vTaskSetParams( task_handle[ 4 ], 4 );
vTaskSetParams( task_handle[ 5 ], 5 );
vTaskSetParams( task_handle[ 6 ], 6 );
vTaskSetParams( task_handle[ 7 ], 7 );
vTaskSetParams( task_handle[ 8 ], 8 );
vTaskSetParams( task_handle[ 9 ], 9 );
#endif

	vInitArray();

    vTaskStartScheduler();

    for(;;);
}
/*-----------------------------------------------------------*/

static void prvPeriodicTask( void *pvParameters )
{
	int id = ( int ) pvParameters;

    SsTCB_t *pxTaskSsTCB = getTaskSsTCB( NULL );

    for(;;)
	{               
    	vBusyWait( pxTaskSsTCB->xWcet );
        
        if( id == ( TASK_COUNT - 1) )
        {
            if( cs_costs[ ( TASK_COUNT - 1) ][ 0 ] >= RELEASE_COUNT )
            {
				vTaskSuspendAll();
				pc.printf("%d\n", 0);
				pc.printf("%d\n", configKERNEL_TEST );
                pc.printf("%d\n", SLACK);
                pc.printf("%d\n", SLACK_METHOD);
                pc.printf("%d\n", SLACK_K);
                
				for(int i = 0; i < TASK_COUNT; i++)
				{
					pc.printf("%d\t", i);
                    #if ( configKERNEL_TEST == 1 )
					for(int j = 2; j < RELEASE_COUNT + 2; j++) {
						pc.printf( "%d\t", cs_costs[i][j]);
					}
                    #endif
                    #if ( configKERNEL_TEST == 2 || configKERNEL_TEST == 3 || configKERNEL_TEST == 4 )
                    for(int j = 1; j < RELEASE_COUNT + 1; j++) {
						pc.printf( "%d\t", cs_costs[i][j]);
					}
                    #endif
					pc.printf("\n");
				}
				for(;;);
			}
        }

		vTaskDelayUntil( &( pxTaskSsTCB->xPreviousWakeTime ), pxTaskSsTCB->xPeriod );
	}
}
/*-----------------------------------------------------------*/

#if ( configUSE_SLACK_STEALING == 1 )
void vApplicationDebugAction( void *param )
{
    ( void ) param;
    
	taskDISABLE_INTERRUPTS();
    
    pc.printf( "%d\n", 3 );

	for( ;; )
	{
        leds[ 4 ] = 1;
        wait_ms(1000);
        leds[ 4 ] = 0;
        wait_ms(1000);
	}
}
#endif
/*-----------------------------------------------------------*/

static void vBusyWait( TickType_t ticks )
{
    TickType_t elapsedTicks = 0;
    TickType_t currentTick = 0;
    while ( elapsedTicks < ticks ) {
        currentTick = xTaskGetTickCount();
        while ( currentTick == xTaskGetTickCount() ) {
            asm("nop");
        }
        elapsedTicks++;
    }
}
/*-----------------------------------------------------------*/

/* The prototype shows it is a naked function - in effect this is just an
assembly function. */
static void HardFault_Handler( void ) __attribute__( ( naked ) );

/* The fault handler implementation calls a function called
prvGetRegistersFromStack(). */
static void HardFault_Handler(void)
{
    __asm volatile
    (
        " tst lr, #4                                                \n"
        " ite eq                                                    \n"
        " mrseq r0, msp                                             \n"
        " mrsne r0, psp                                             \n"
        " ldr r1, [r0, #24]                                         \n"
        " ldr r2, handler2_address_const                            \n"
        " bx r2                                                     \n"
        " handler2_address_const: .word prvGetRegistersFromStack    \n"
    );
}
/*-----------------------------------------------------------*/

void prvGetRegistersFromStack( uint32_t *pulFaultStackAddress )
{
	/* These are volatile to try and prevent the compiler/linker optimising them
	away as the variables never actually get used.  If the debugger won't show the
	values of the variables, make them global my moving their declaration outside
	of this function. */
	volatile uint32_t r0;
	volatile uint32_t r1;
	volatile uint32_t r2;
	volatile uint32_t r3;
	volatile uint32_t r12;
	volatile uint32_t lr; /* Link register. */
	volatile uint32_t pc; /* Program counter. */
	volatile uint32_t psr;/* Program status register. */

    r0 = pulFaultStackAddress[ 0 ];
    r1 = pulFaultStackAddress[ 1 ];
    r2 = pulFaultStackAddress[ 2 ];
    r3 = pulFaultStackAddress[ 3 ];

    r12 = pulFaultStackAddress[ 4 ];
    lr = pulFaultStackAddress[ 5 ];
    pc = pulFaultStackAddress[ 6 ];
    psr = pulFaultStackAddress[ 7 ];

    /* When the following line is hit, the variables contain the register values. */
    for( ;; );
}
/*-----------------------------------------------------------*/
