/* [[[cog
import cog
import sys
import xml.etree.cElementTree as et

def get_rts_from_element(elem):
    """ extrae el str de elem """

    def get_int(string):
        """ convierte string a int """
        try:
            return int(string)
        except ValueError:
            return int(float(string))

    rts_id, rts = 0, []
    if elem.tag == 'S':
        rts_id = get_int(elem.get("count"))
        for t in elem.iter("i"):
            task = t.attrib
            for k, v in task.items():
                task[k] = get_int(v)
            rts.append(task)

    return rts_id, rts

def test_rts(rts):
    for task in rts:
        task_string = "{0},{1},{2}\n".format(task["C"], task["T"], task["D"])
        sys.stdout.write(task_string)

rts_to_test_id = int(RTS_TO_TEST)
rts_to_test = []

context = et.iterparse(RTS_FILE, events=('start', 'end', ))
context = iter(context)
event, root = context.next()
for event, elem in context:
    rts_id, rts_to_test = get_rts_from_element(elem)
    if rts_to_test_id == rts_id and rts_to_test and event == "end":
        break
    root.clear()
del context
]]] */
// [[[end]]]
#include "mbed.h"
#include "FreeRTOS.h"
#include "task.h"

#if ( configUSE_SLACK_STEALING == 1 )
#include "slack.h"
#endif

#define ONE_TICK ( ( configCPU_CLOCK_HZ / configTICK_RATE_HZ ) - 1UL )

#ifndef TASK_COUNT
/* [[[cog
cog.outl("#define TASK_COUNT {0}".format(len(rts_to_test)))
]]]*/
// [[[end]]]
#endif

/*[[[cog
for task in rts_to_test:
    cog.outl("#define TASK_{0}_PERIOD {1}".format(task["nro"], int(task["T"])))
]]] */

// [[[end]]]

/* [[[cog
for task in rts_to_test:
    cog.outl("#define TASK_{0}_WCET {1}".format(task["nro"], int(task["C"])))
    cog.outl("#define TASK_{0}_RUNTIME {1}".format(task["nro"], int(task["C"])))
]]] */

// [[[end]]]

BaseType_t xTasksParams[ TASK_COUNT ][ 2 ] =
{    
/* [[[cog
for task in rts_to_test:
    cog.outl("{0} TASK_{1}_PERIOD, TASK_{2}_RUNTIME {3},".format("{", int(task["nro"]), int(task["nro"]), "}"))    
]]] */
// [[[end]]]
};

#define TASK_STACK_SIZE configMINIMAL_STACK_SIZE

/* Prototypes for the standard FreeRTOS callback/hook functions implemented within this file. */
/* The extern "C" is required to avoid name mangling between C and C++ code. */
#if defined (__cplusplus)
extern "C" {
#endif

#if( configUSE_SLACK_STEALING == 1 )
void vApplicationDebugAction( void *param );
void vApplicationNotSchedulable( void );
void vApplicationDeadlineMissedHook( char *pcTaskName, UBaseType_t uxRelease, TickType_t xTickCount );
#endif
void vApplicationMallocFailedHook( void );
void vApplicationStackOverflowHook( TaskHandle_t pxTask, char *pcTaskName );

#if defined (__cplusplus)
}
#endif

void vEatCpu( BaseType_t ticks );

void task_body( void* );

TaskHandle_t * task_handle;

Serial pc( USBTX, USBRX );

static DigitalOut mbed_leds[ ] = { LED1, LED2, LED3, LED4 };

/* ========================================================================= */
xType *cs_costs;
/* ========================================================================= */

int main() 
{
    #if ( configKERNEL_TEST == 1 )
    ulDelayTime = 0;
    ulDelayTime1 = 0;
    #endif
    
	pc.baud(9600);
    
    /* Reserve memory for task_handle array */
    task_handle = ( TaskHandle_t * ) pvPortMalloc( sizeof( TaskHandle_t ) * TASK_COUNT );

    /* Periodic tasks. */
    /* xTaskCreate( task_body, "T01", TASK_STACK_SIZE, ( void * ) 0, configMAX_PRIORITIES - 2, &task_handle[0] ); */    
/* [[[cog
for idx, task in enumerate( rts_to_test ):
    cog.outl("xTaskCreate( task_body, \"T{0:02}\", TASK_STACK_SIZE, ( void * ) {1}, configMAX_PRIORITIES - {2}, &task_handle[{3}] );".format( int(task["nro"]), idx, idx + 2, idx ))
]]]*/
// [[[end]]]

#if ( configUSE_SLACK_STEALING == 1 )
    /* vTaskSetParams( task_handle[ 0 ], TASK_1_PERIOD, TASK_1_PERIOD, TASK_1_WCET, 0 ); */
/* [[[cog
for idx, task in enumerate( rts_to_test ):
    cog.outl("vTaskSetParams( task_handle[ {0} ], TASK_{1}_PERIOD, TASK_{1}_PERIOD, TASK_{1}_WCET, {0} );".format( idx, idx + 1 ))
]]]*/    
// [[[end]]]
#endif

#if ( configUSE_SLACK_STEALING == 0 ) && ( configKERNEL_TEST == 1 )
    /* vTaskSetParams( task_handle[ 0 ], 0 ); */
/* [[[cog
for idx, task in enumerate( rts_to_test ):
    cog.outl("vTaskSetParams( task_handle[ {0} ], {0} );".format(idx))
]]]*/    
// [[[end]]]
#endif

    /* Reserve memory for cs_cost[][] array */
    cs_costs = ( xType* ) pvPortMalloc( sizeof( uint32_t ) * ( TASK_COUNT * (RELEASE_COUNT + 2)));

    /* Zeroes cs_cost[][] */
    for(int i = 0; i < TASK_COUNT; i++)
    {
        #if ( configKERNEL_TEST == 1 )
        for(int j = 0; j < RELEASE_COUNT + 2; j++) 
        {            
            (*cs_costs)[i][j] = 0;
        }
        #endif
        #if ( configKERNEL_TEST == 2 || configKERNEL_TEST == 3 || configKERNELTRACE == 4 )
        for(int j = 0; j < RELEASE_COUNT + 1; j++) 
        {            
            (*cs_costs)[i][j] = 0;
        }
        #endif
    }
    
	vTaskStartScheduler();

    for(;;);
}

void task_body( void* params )
{
	TickType_t xPreviousWakeTime = ( TickType_t ) 0U;
    int id = ( int ) params;

    for(;;)
	{               
        vEatCpu( xTasksParams[ id ][ 1 ]  );
        
        if( id == ( TASK_COUNT - 1) )
        {
            if( (*cs_costs)[ ( TASK_COUNT - 1) ][ 0 ] >= RELEASE_COUNT )
            {
				vTaskSuspendAll();
				pc.printf("%d\n", 0);
                
                pc.printf("%d\n", SLACK);
                pc.printf("%d\n", SLACK_METHOD);
                pc.printf("%d\n", SLACK_K);
                
				for(int i = 0; i < TASK_COUNT; i++)
				{
					pc.printf("%d\t", i);
                    #if ( configKERNEL_TEST == 1 )
					for(int j = 2; j < RELEASE_COUNT + 2; j++) {
						pc.printf( "%d\t", (*cs_costs)[i][j]);
					}
                    #endif
                    #if ( configKERNEL_TEST == 2 || configKERNEL_TEST == 3 || configKERNEL_TEST == 4 )
                    for(int j = 1; j < RELEASE_COUNT + 1; j++) {
						pc.printf( "%d\t", (*cs_costs)[i][j]);
					}
                    #endif
					pc.printf("\n");
				}
				for(;;);
			}
        }

		vTaskDelayUntil( &xPreviousWakeTime, ( TickType_t ) xTasksParams[ id ][ 0 ] );
	}
}

void vApplicationMallocFailedHook( void )
{
	taskDISABLE_INTERRUPTS();

    pc.printf( "%d\n", 5 );

	for( ;; )
	{
        mbed_leds[ 3 ] = 1;
        wait_ms(1000);
        mbed_leds[ 3 ] = 0;
        wait_ms(1000);
	}
}

#if ( configCHECK_FOR_STACK_OVERFLOW > 0 )
void vApplicationStackOverflowHook( TaskHandle_t pxTask, char *pcTaskName )
{
	( void ) pcTaskName;
	( void ) pxTask;

	taskDISABLE_INTERRUPTS();

    pc.printf( "%d\n", 4 );

	for( ;; )
	{
        mbed_leds[ 2 ] = 1;
        wait_ms(1000);
        mbed_leds[ 2 ] = 0;
        wait_ms(1000);
	}
}
#endif

#if ( configUSE_SLACK_STEALING == 1 )
void vApplicationDebugAction( void *param )
{
    ( void ) param;
    
	taskDISABLE_INTERRUPTS();
    
    pc.printf( "%d\n", 3 );

	for( ;; )
	{
        mbed_leds[ 4 ] = 1;
        wait_ms(1000);
        mbed_leds[ 4 ] = 0;
        wait_ms(1000);
	}
}

void vApplicationNotSchedulable( void )
{
	taskDISABLE_INTERRUPTS();
    
    pc.printf( "%d\n", 2 );

	for( ;; )
	{
        mbed_leds[ 1 ] = 1;
        wait_ms(1000);
        mbed_leds[ 1 ] = 0;
        wait_ms(1000);
	}
}

void vApplicationDeadlineMissedHook( char *pcTaskName, UBaseType_t uxRelease, TickType_t xTickCount )
{
    ( void ) *pcTaskName;
    ( void ) uxRelease;
    ( void ) xTickCount;
    
    taskDISABLE_INTERRUPTS();
    
    pc.printf( "%d\n", 1 );
    
    for( ;; )
    {
        mbed_leds[ 0 ] = 1;
        wait_ms( 1000 );
        mbed_leds[ 0 ] = 0;
        wait_ms( 1000 );
    }
}
#endif

void vEatCpu( BaseType_t ticks )
{
    BaseType_t xI;
    BaseType_t xLim = ( ticks * ONE_TICK ) / 5;

    for( xI = 0; xI < xLim; xI++ )
    {
        asm("nop");
    }
}

/* ========================================================================= */

#if ( configKERNEL_TEST == 1 )
void vMacroTaskDelay()
{
	STOPWATCH_RESET();
	ulDelayTime = CPU_CYCLES;
    vTaskGetTraceInfo( cs_costs, ulDelayTime, 0 );
}

void vMacroTaskSwitched()
{
	ulDelayTime1 = CPU_CYCLES;
    vTaskGetTraceInfo( cs_costs, ulDelayTime1, 1 );    
}
#endif
