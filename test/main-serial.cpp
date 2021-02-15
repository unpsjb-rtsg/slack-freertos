/*****************************************************************************
 * Includes
 ****************************************************************************/
#include "mbed.h"
#include "FreeRTOS.h"
#include "task.h"

#if ( configUSE_SLACK_STEALING == 1 )
#include "slack.h"
#include "slack_tests.h"
#endif

/*****************************************************************************
 * Macros and definitions
 ****************************************************************************/
#define TASK_STACK_SIZE configMINIMAL_STACK_SIZE
#define BAUDRATE 9600

/*****************************************************************************
 * Private data declaration
 ****************************************************************************/
union int_union {
    char c[4];
    int i;
} int_u;

// Tarea de TR
struct task_t {
    int id;                    // task id
    int c;                     // worst case execution time (wcet)
    int t;                     // period
    int d;                     // relative deadline
};

/*****************************************************************************
 * Public data declaration
 ****************************************************************************/
/* None */

/*****************************************************************************
 * Public functions declaration
 ****************************************************************************/
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

/*****************************************************************************
 * Private functions declaration
 ****************************************************************************/
static void vBusyWait( TickType_t ticks );
static void prvPeriodicTask( void *pvParameters );
/* The prototype shows it is a naked function - in effect this is just an
assembly function. */
static void HardFault_Handler( void ) __attribute__( ( naked ) );
static void putc(int i);
static int getc();

/*****************************************************************************
 * Private data
 ****************************************************************************/
/* None */

/*****************************************************************************
 * Public data
 ****************************************************************************/
Serial pc( USBTX, USBRX );
DigitalOut leds[ ] = { LED1, LED2, LED3, LED4 };
task_t str[TASK_COUNT];

/*****************************************************************************
 * Private functions
 ****************************************************************************/
static void putc(int i)
{
    int_u.i = i;
    pc.putc(int_u.c[3]);
    pc.putc(int_u.c[2]);
    pc.putc(int_u.c[1]);
    pc.putc(int_u.c[0]);
}
/*-----------------------------------------------------------*/

static int getc()
{
    int_u.c[3] = pc.getc();
    int_u.c[2] = pc.getc();
    int_u.c[1] = pc.getc();
    int_u.c[0] = pc.getc();
    return int_u.i;
}
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
					for(int j = 1; j < RELEASE_COUNT + 1; j++) {
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

static void prvGetRegistersFromStack( uint32_t *pulFaultStackAddress )
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

    // to avoid annoying "variable 'rX' set but not used"
    (void)(r0);
    (void)(r1);
    (void)(r2);
    (void)(r3);
    (void)(r12);
    (void)(lr);
    (void)(pc);
    (void)(psr);

    /* When the following line is hit, the variables contain the register values. */
    for( ;; );
}
/*-----------------------------------------------------------*/

/*****************************************************************************
 * Public functions
 ****************************************************************************/
int main()
{
    pc.baud(9600);
    
    while (pc.readable() == 0) {
    	;
    }

	// read the number of tasks
	int num_task = getc();
	int i, j;
    
    // read task-set from serial
	for (j = 0; j < num_task; j++) {
		str[j].id = j + 1;

		// read C
		str[j].c = getc();

		// read T
		str[j].t = getc();

		// read D
		str[j].d = getc();
        
     }
    
    /* Reserve memory for task_handle array */
    TaskHandle_t *task_handle = ( TaskHandle_t * ) pvPortMalloc( sizeof( TaskHandle_t ) * TASK_COUNT );

    /* Periodic tasks. */
    for (i = 0; i < num_task; i++) {
    	xTaskCreate( prvPeriodicTask, "T01", TASK_STACK_SIZE, ( void * ) i, configMAX_PRIORITIES - (2 + i),  &task_handle[i] );
    }

    for (i = 0; i < num_task; i++) {
    	vSlackSetTaskParams( task_handle[ i ], PERIODIC_TASK, str[i].t, str[i].d, str[i].c, i );
    }

    vInitArray();

    vTaskStartScheduler();

    for(;;);
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
        leds[ 3 ] = 1;
        wait_ms(1000);
        leds[ 3 ] = 0;
        wait_ms(1000);
	}
}

void vApplicationNotSchedulable( void )
{
	taskDISABLE_INTERRUPTS();
    
    pc.printf( "%d\n", 2 );

	for( ;; )
	{
        leds[ 1 ] = 1;
        wait_ms(1000);
        leds[ 1 ] = 0;
        wait_ms(1000);
	}
}

void vApplicationDeadlineMissedHook( char *pcTaskName, const SsTCB_t *xSsTCB,
        TickType_t xTickCount )
{
    ( void ) xSsTCB;

    taskDISABLE_INTERRUPTS();

    pc.printf( "\n\r%s missed its deadline at %d\n\r", pcTaskName, xTickCount);

    for( ;; )
    {
        leds[ 0 ] = 1;
        wait_ms( 1000 );
        leds[ 0 ] = 0;
        wait_ms( 1000 );
    }
}
#endif
/*-----------------------------------------------------------*/

void vApplicationMallocFailedHook( void )
{
	taskDISABLE_INTERRUPTS();

    pc.printf( "%d\n", 5 );

	for( ;; )
	{
        leds[ 2 ] = 1;
        wait_ms(1000);
        leds[ 2 ] = 0;
        wait_ms(1000);
	}
}
/*-----------------------------------------------------------*/