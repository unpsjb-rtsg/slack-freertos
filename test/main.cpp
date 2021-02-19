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

#define DEADLINE_MISSED     1
#define NOT_SCHEDULABLE     2
#define MALLOC_FAILED       5

/*****************************************************************************
 * Private data declaration
 ****************************************************************************/
/**
 * \brief Union used when retriving data from the serial port.
 */
union int_union {
    char c[4];
    int i;
} int_u;

/**
 * \brief Real-time task simple model.
 */
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
/* Prototypes for the standard FreeRTOS callback/hook functions implemented 
 * within this file. The extern "C" is required to avoid name mangling between 
 * C and C++ code. */
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
/**
 * \brief Perfomen a busy wait for the specified ticks.
 */
static void vBusyWait( TickType_t ticks );

/**
 * Periodic task.
 */
static void prvPeriodicTask( void *pvParameters );

/**
 * \brief Get 4 bytes from the serial port.
 */
static int getc();

/**
 * \brief Write 4 bytes into the serial port.
 */
static void putc(uint32_t i);

/* The prototype shows it is a naked function - in effect this is just an
assembly function. */
static void HardFault_Handler( void ) __attribute__( ( naked ) );

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
static int getc()
{
    int_u.c[3] = pc.getc();
    int_u.c[2] = pc.getc();
    int_u.c[1] = pc.getc();
    int_u.c[0] = pc.getc();
    return int_u.i;
}
/*-----------------------------------------------------------*/

static void putc(uint32_t i)
{
    int_u.i = i;
    pc.putc(int_u.c[3]);
    pc.putc(int_u.c[2]);
    pc.putc(int_u.c[1]);
    pc.putc(int_u.c[0]);    
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
                putc(0);
                putc(configKERNEL_TEST);
                putc(SLACK);
                putc(SLACK_METHOD);
                putc(SLACK_K);

				for(int i = 0; i < TASK_COUNT; i++)
				{
                    putc(i);
                    for(int j = 1; j < RELEASE_COUNT + 1; j++) {
                        putc(cs_costs[i][j]);
                    }
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

	/* Read the number of tasks. */
	int num_task = getc();
	int i, j;

    /* Read each task from the serial port. */
	for (j = 0; j < num_task; j++) {
		str[j].id = j + 1;
		str[j].c = getc();
		str[j].t = getc();
		str[j].d = getc();
    }
    
    /* Reserve memory for task_handle array. */
    TaskHandle_t *task_handle = ( TaskHandle_t * ) pvPortMalloc( 
                                        sizeof( TaskHandle_t ) * TASK_COUNT );

    /* Periodic tasks. */
    for (i = 0; i < num_task; i++) {
    	xTaskCreate( prvPeriodicTask, "T01", TASK_STACK_SIZE, ( void * ) i, 
                            configMAX_PRIORITIES - (2 + i),  &task_handle[i] );
    }

    for (i = 0; i < num_task; i++) {
    	vSlackSetTaskParams( task_handle[ i ], PERIODIC_TASK, str[i].t, str[i].d, 
                                                                str[i].c, i );
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

    putc(3);
    
	taskDISABLE_INTERRUPTS();
    
	for( ;; )
	{
        leds[ 3 ] = 1;
        wait_ms(1000);
        leds[ 3 ] = 0;
        wait_ms(1000);
	}
}
/*-----------------------------------------------------------*/

void vApplicationNotSchedulable( void )
{
    putc(NOT_SCHEDULABLE);

    taskDISABLE_INTERRUPTS();

	for( ;; )
	{
        leds[ 1 ] = 1;
        wait_ms(1000);
        leds[ 1 ] = 0;
        wait_ms(1000);
	}
}
/*-----------------------------------------------------------*/

void vApplicationDeadlineMissedHook( char *pcTaskName, const SsTCB_t *xSsTCB,
        TickType_t xTickCount )
{
    ( void ) xSsTCB;
    putc(DEADLINE_MISSED);

    taskDISABLE_INTERRUPTS();

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
    putc(MALLOC_FAILED);
    taskDISABLE_INTERRUPTS();

	for( ;; )
	{
        leds[ 2 ] = 1;
        wait_ms(1000);
        leds[ 2 ] = 0;
        wait_ms(1000);
	}
}
/*-----------------------------------------------------------*/