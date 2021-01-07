/*
 * FreeRTOS Kernel V10.4.1
 * Copyright (C) 2020 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * http://www.FreeRTOS.org
 * http://aws.amazon.com/freertos
 *
 * 1 tab == 4 spaces!
 */


/*
 * Creates all the demo application tasks, then starts the scheduler.  The WEB
 * documentation provides more details of the standard demo application tasks.
 * In addition to the standard demo tasks, the following tasks and tests are
 * defined and/or created within this file:
 *
 * "OLED" task - the OLED task is a 'gatekeeper' task.  It is the only task that
 * is permitted to access the display directly.  Other tasks wishing to write a
 * message to the OLED send the message on a queue to the OLED task instead of
 * accessing the OLED themselves.  The OLED task just blocks on the queue waiting
 * for messages - waking and displaying the messages as they arrive.
 *
 * "Check" hook -  This only executes every five seconds from the tick hook.
 * Its main function is to check that all the standard demo tasks are still
 * operational.  Should any unexpected behaviour within a demo task be discovered
 * the tick hook will write an error to the OLED (via the OLED task).  If all the
 * demo tasks are executing with their expected behaviour then the check task
 * writes PASS to the OLED (again via the OLED task), as described above.
 *
 * Use the following command to start running the application in QEMU, pausing
 * to wait for a debugger connection:
 * "qemu-system-arm -machine lm3s6965evb -s -S -kernel [pat_to]\RTOSDemo.elf"
 *
 * To enable FreeRTOS+Trace:
 *  1) Add #include "trcRecorder.h" to the bottom of FreeRTOSConfig.h.
 *  2) Call vTraceEnable( TRC_START ); at the top of main.
 *  3) Ensure the "FreeRTOS+Trace Recorder" folder in the Project Explorer
 *     window is not excluded from the build.
 *
 * To retrieve the trace files:
 *  1) Use the Memory windows in the Debug perspective to dump RAM from the
 *     RecorderData variable.
 */

/*************************************************************************
 * Please ensure to read http://www.freertos.org/portlm3sx965.html
 * which provides information on configuring and running this demo for the
 * various Luminary Micro EKs.
 *************************************************************************/

/* Standard includes. */
#include <stdio.h>
#include <string.h>

/* Scheduler includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

/* Hardware library includes. */
#include "hw_memmap.h"
#include "hw_types.h"
#include "hw_sysctl.h"
#include "hw_uart.h"
#include "sysctl.h"
#include "gpio.h"
#include "grlib.h"
#include "osram128x64x4.h"
#include "uart.h"

#include "bitmap.h"
/* Demo app includes.
#include "death.h"
#include "blocktim.h"
#include "semtest.h"
#include "bitmap.h"
#include "QPeek.h"
#include "recmutex.h"
#include "QueueSet.h"
#include "EventGroupsDemo.h"
#include "MessageBufferDemo.h"
#include "StreamBufferDemo.h"
*/

#include "slack.h"

/*-----------------------------------------------------------*/

/* The time between cycles of the 'check' functionality (defined within the
tick hook. */
#define mainCHECK_DELAY                     ( ( TickType_t ) 5000 / portTICK_PERIOD_MS )

/* Task stack sizes. */
#define mainOLED_TASK_STACK_SIZE            ( configMINIMAL_STACK_SIZE + 40 )
#define mainMESSAGE_BUFFER_TASKS_STACK_SIZE ( 100 )

/* Task priorities. */
#define mainCHECK_TASK_PRIORITY             ( tskIDLE_PRIORITY + 3 )
#define mainSEM_TEST_PRIORITY               ( tskIDLE_PRIORITY + 1 )
#define mainCREATOR_TASK_PRIORITY           ( tskIDLE_PRIORITY + 3 )
#define mainGEN_QUEUE_TASK_PRIORITY         ( tskIDLE_PRIORITY )

/* The maximum number of message that can be waiting for display at any one
time. */
#define mainOLED_QUEUE_SIZE                 ( 3 )

/* Dimensions the buffer into which the jitter time is written. */
#define mainMAX_MSG_LEN                     25

/* The period of the system clock in nano seconds.  This is used to calculate
the jitter time in nano seconds. */
#define mainNS_PER_CLOCK                    ( ( uint32_t ) ( ( 1.0 / ( double ) configCPU_CLOCK_HZ ) * 1000000000.0 ) )

/* Constants used when writing strings to the display. */
#define mainCHARACTER_HEIGHT                ( 9 )
#define mainMAX_ROWS_128                    ( mainCHARACTER_HEIGHT * 14 )
#define mainMAX_ROWS_96                     ( mainCHARACTER_HEIGHT * 10 )
#define mainMAX_ROWS_64                     ( mainCHARACTER_HEIGHT * 7 )
#define mainFULL_SCALE                      ( 15 )
#define ulSSI_FREQUENCY                     ( 3500000UL )

#define TASK_1_PRIO configMAX_PRIORITIES - configMAX_SLACK_PRIO - 1

/*-----------------------------------------------------------*/

/*
 * The display is written two by more than one task so is controlled by a
 * 'gatekeeper' task.  This is the only task that is actually permitted to
 * access the display directly.  Other tasks wanting to display a message send
 * the message to the gatekeeper.
 */
static void prvTask( void *pvParameters );

/*
 * Configure the hardware for the demo.
 */
static void prvSetupHardware( void );

/*
 * Configures the high frequency timers - those used to measure the timing
 * jitter while the real time kernel is executing.
 */
//extern void vSetupHighFrequencyTimer( void );

/*
 * Hook functions that can get called by the kernel.
 */
void vApplicationStackOverflowHook( TaskHandle_t pxTask, char *pcTaskName );
void vApplicationTickHook( void );

/*
 * Basic polling UART write function.
 */
static void prvPrintString( const char * pcString );

/*-----------------------------------------------------------*/

/* The welcome text. */
const char * const pcWelcomeMessage = "   www.FreeRTOS.org";

/*-----------------------------------------------------------*/

/* Functions to access the OLED.  The one used depends on the dev kit
being used. */
void ( *vOLEDInit )( uint32_t ) = NULL;
void ( *vOLEDStringDraw )( const char *, uint32_t, uint32_t, unsigned char ) = NULL;
void ( *vOLEDImageDraw )( const unsigned char *, uint32_t, uint32_t, uint32_t, uint32_t ) = NULL;
void ( *vOLEDClear )( void ) = NULL;

/*************************************************************************
 * Please ensure to read http://www.freertos.org/portlm3sx965.html
 * which provides information on configuring and running this demo for the
 * various Luminary Micro EKs.
 *************************************************************************/
int main( void )
{
    /* Initialise the trace recorder.  Use of the trace recorder is optional.
    See http://www.FreeRTOS.org/trace for more information and the comments at
    the top of this file regarding enabling trace in this demo.
    vTraceEnable( TRC_START ); */

    prvSetupHardware();

    TaskHandle_t task1;
    TaskHandle_t task2;
    TaskHandle_t task3;

    /* Start the tasks defined within this file/specific to this demo. */
    xTaskCreate( prvTask, "T1", mainOLED_TASK_STACK_SIZE, (void*) 1, configMAX_PRIORITIES - configMAX_SLACK_PRIO - 1, &task1 );
    xTaskCreate( prvTask, "T2", mainOLED_TASK_STACK_SIZE, (void*) 2, configMAX_PRIORITIES - configMAX_SLACK_PRIO - 2, &task2 );
    xTaskCreate( prvTask, "T3", mainOLED_TASK_STACK_SIZE, (void*) 3, configMAX_PRIORITIES - configMAX_SLACK_PRIO - 3, &task3 );

    /* Uncomment the following line to configure the high frequency interrupt
    used to measure the interrupt jitter time. */
    //vSetupHighFrequencyTimer();

    vSlackSystemSetup();

    // Configure additional parameters needed by the slack stealing framework.
    vSlackSetTaskParams( task1, PERIODIC_TASK, 3000, 3000, 100, 1 );
    vSlackSetTaskParams( task2, PERIODIC_TASK, 4000, 4000, 100, 2 );
    vSlackSetTaskParams( task3, PERIODIC_TASK, 6000, 6000, 100, 3 );

    vSlackSchedulerSetup();

    /* Map the OLED access functions to the driver functions that are appropriate
    for the evaluation kit being used. */
    configASSERT( ( HWREG( SYSCTL_DID1 ) & SYSCTL_DID1_PRTNO_MASK ) == SYSCTL_DID1_PRTNO_6965 );
    vOLEDInit = OSRAM128x64x4Init;
    vOLEDStringDraw = OSRAM128x64x4StringDraw;
    vOLEDImageDraw = OSRAM128x64x4ImageDraw;
    vOLEDClear = OSRAM128x64x4Clear;
    //ulMaxY = mainMAX_ROWS_64;
    //pucImage = pucBasicBitmap;
    //ulY = ulMaxY;

    /* Initialise the OLED and display a startup message. */
    vOLEDInit( ulSSI_FREQUENCY );

    /* Start the scheduler. */
    vTaskStartScheduler();

    /* Will only get here if there was insufficient memory to create the idle
    task. */
    for( ;; );
}
/*-----------------------------------------------------------*/

void prvSetupHardware( void )
{
    /* If running on Rev A2 silicon, turn the LDO voltage up to 2.75V.  This is
    a workaround to allow the PLL to operate reliably. */
    if( DEVICE_IS_REVA2 )
    {
        SysCtlLDOSet( SYSCTL_LDO_2_75V );
    }

    /* Set the clocking to run from the PLL at 50 MHz */
    SysCtlClockSet( SYSCTL_SYSDIV_4 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN | SYSCTL_XTAL_8MHZ );
}
/*-----------------------------------------------------------*/

void vApplicationTickHook( void )
{
    return;
}
/*-----------------------------------------------------------*/

static void prvPrintString( const char * pcString )
{
    while( *pcString != 0x00 )
    {
        UARTCharPut( UART0_BASE, *pcString );
        pcString++;
    }
}
/*-----------------------------------------------------------*/

void prvTask( void *pvParameters )
{
    static char cMessage[ mainMAX_MSG_LEN ];

    int id = ( int ) pvParameters;

    sprintf(cMessage, "FreeRTOS %s + SS", tskKERNEL_VERSION_NUMBER);
    vOLEDStringDraw( cMessage, 0, 0, mainFULL_SCALE );

    SsTCB_t *pxTaskSsTCB = getTaskSsTCB( NULL );

    for( ;; )
    {
        sprintf( cMessage, "%s - %u", pcTaskGetTaskName( NULL ), pxTaskSsTCB->uxReleaseCount );
        vOLEDStringDraw( cMessage, 0, (mainCHARACTER_HEIGHT+1)*id, mainFULL_SCALE );
        prvPrintString( cMessage );
        prvPrintString( "\r\n" );
        vTaskDelayUntil( &( pxTaskSsTCB->xPreviousWakeTime ), pxTaskSsTCB->xPeriod );
    }
}
/*-----------------------------------------------------------*/

volatile char *pcOverflowedTask = NULL; /* Prevent task name being optimised away. */
void vApplicationStackOverflowHook( TaskHandle_t pxTask, char *pcTaskName )
{
    ( void ) pxTask;
    pcOverflowedTask = pcTaskName;
    vAssertCalled( __FILE__, __LINE__ );
    for( ;; );
}
/*-----------------------------------------------------------*/

void vAssertCalled( const char *pcFile, uint32_t ulLine )
{
volatile uint32_t ulSetTo1InDebuggerToExit = 0;

    taskENTER_CRITICAL();
    {
        while( ulSetTo1InDebuggerToExit == 0 )
        {
            /* Nothing to do here.  Set the loop variable to a non zero value in
            the debugger to step out of this function to the point that caused
            the assertion. */
            ( void ) pcFile;
            ( void ) ulLine;
        }
    }
    taskEXIT_CRITICAL();
}

/* configUSE_STATIC_ALLOCATION is set to 1, so the application must provide an
implementation of vApplicationGetIdleTaskMemory() to provide the memory that is
used by the Idle task. */
void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize )
{
/* If the buffers to be provided to the Idle task are declared inside this
function then they must be declared static - otherwise they will be allocated on
the stack and so not exists after this function exits. */
static StaticTask_t xIdleTaskTCB;
static StackType_t uxIdleTaskStack[ configMINIMAL_STACK_SIZE ];

    /* Pass out a pointer to the StaticTask_t structure in which the Idle task's
    state will be stored. */
    *ppxIdleTaskTCBBuffer = &xIdleTaskTCB;

    /* Pass out the array that will be used as the Idle task's stack. */
    *ppxIdleTaskStackBuffer = uxIdleTaskStack;

    /* Pass out the size of the array pointed to by *ppxIdleTaskStackBuffer.
    Note that, as the array is necessarily of type StackType_t,
    configMINIMAL_STACK_SIZE is specified in words, not bytes. */
    *pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
}
/*-----------------------------------------------------------*/

char * _sbrk_r (struct _reent *r, int incr)
{
    /* Just to keep the linker quiet. */
    ( void ) r;
    ( void ) incr;

    /* Check this function is never called by forcing an assert() if it is. */
    configASSERT( incr == -1 );

    return NULL;
}

int __error__(char *pcFilename, unsigned long ulLine) {
    return 0;
}

void vApplicationDeadlineMissedHook( char *pcTaskName, const SsTCB_t *xSsTCB,
        TickType_t xTickCount )
{
    taskDISABLE_INTERRUPTS();
    for (;; ) {}
}

void vApplicationNotSchedulable( void )
{
    taskDISABLE_INTERRUPTS();
    for (;; ) {}
}
