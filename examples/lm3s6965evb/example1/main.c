/*****************************************************************************
 *
 * Example 1 - lm3s6965evb
 *
 * This program consist of 4 real-time periodic tasks and 2 aperiodic tasks,
 * the later only scheduled when there is available slack in the system. All
 * the tasks write a string with some data to to the serial port, when
 * starting and finishing each instance.
 *
 * Before writing to the serial port, the tasks try to take a shared mutex.
 * This could lead to the following problem: when an aperiodic task has taken
 * the mutex and then the available slack depletes, the periodic tasks can't
 * take the mutex, and a missed deadline will occur.
 *
 * This program requires FreeRTOS v10.0.0 or later.
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
 *
 * Based on the FreeRTOS demo app for QEMU.
 * See: https://www.freertos.org/cortex-m3-qemu-lm3S6965-demo.html
 *      http://www.freertos.org/portlm3sx965.html
 *
 * Created on: 12 dec. 2020
 *     Author: Francisco E. Páez
 *
 *****************************************************************************/
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
#include "slack.h"

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

/*-----------------------------------------------------------*/

/* Task stack sizes. */
#define taskDEFAULT_STACK   ( configMINIMAL_STACK_SIZE + 40 )

/* Task priorities. */
#define TASK_1_PRIO     ( configMAX_PRIORITIES - configMAX_SLACK_PRIO - 1 )
#define TASK_2_PRIO     ( configMAX_PRIORITIES - configMAX_SLACK_PRIO - 2 )
#define TASK_3_PRIO     ( configMAX_PRIORITIES - configMAX_SLACK_PRIO - 3 )
#define TASK_4_PRIO     ( configMAX_PRIORITIES - configMAX_SLACK_PRIO - 4 )
#define ATASK_1_PRIO    ( configMAX_PRIORITIES - 1 )
#define ATASK_2_PRIO    ( configMAX_PRIORITIES - 2 )
#define ATASK_WCET 2000
#define ATASK_MAX_DELAY 4000

#define mainMAX_MSG_LEN ( 150 )

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

/*-----------------------------------------------------------*/

/**
 *
 * @param pvParameters
 */
static void prvPeriodicTask( void *pvParameters );

/**
 *
 * @param params
 */
static void prvAperiodicTask( void* params );

/**
 * Configure the hardware.
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

/*
 * Basic polling UART write function.
 */
static void prvPrintString( const char * pcString );

/*-----------------------------------------------------------*/

/* The welcome text. */
const char * const pcWelcomeMessage = "   www.FreeRTOS.org";

static SemaphoreHandle_t xMutex = NULL;

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

    // Create the mutex.
    xMutex = xSemaphoreCreateMutex();

    TaskHandle_t task1;
    TaskHandle_t task2;
    TaskHandle_t task3;
    TaskHandle_t task4;
    TaskHandle_t atask1;
    TaskHandle_t atask2;

    /* Start the tasks defined within this file/specific to this demo. */
    xTaskCreate( prvPeriodicTask, "T1", taskDEFAULT_STACK, (void*) 1, TASK_1_PRIO, &task1 );
    xTaskCreate( prvPeriodicTask, "T2", taskDEFAULT_STACK, (void*) 2, TASK_2_PRIO, &task2 );
    xTaskCreate( prvPeriodicTask, "T3", taskDEFAULT_STACK, (void*) 3, TASK_3_PRIO, &task3 );
    xTaskCreate( prvPeriodicTask, "T4", taskDEFAULT_STACK, (void*) 4, TASK_4_PRIO, &task4 );

    xTaskCreate( prvAperiodicTask, "TA1", 256, NULL, ATASK_1_PRIO, &atask1 );
    xTaskCreate( prvAperiodicTask, "TA2", 256, NULL, ATASK_2_PRIO, &atask2 );

    /* Uncomment the following line to configure the high frequency interrupt
    used to measure the interrupt jitter time. */
    //vSetupHighFrequencyTimer();

    // Configure additional parameters needed by the slack stealing framework.
    vSlackSetTaskParams( task1, PERIODIC_TASK, 3000,  3000,  1000, 1 );
    vSlackSetTaskParams( task2, PERIODIC_TASK, 4000,  4000,  1000, 2 );
    vSlackSetTaskParams( task3, PERIODIC_TASK, 6000,  6000,  1000, 3 );
    vSlackSetTaskParams( task4, PERIODIC_TASK, 12000, 12000, 1000, 4 );

    vSlackSetTaskParams( atask1, APERIODIC_TASK, ATASK_MAX_DELAY, 0, ATASK_WCET, 1 );
    vSlackSetTaskParams( atask2, APERIODIC_TASK, ATASK_MAX_DELAY, 0, ATASK_WCET, 2 );

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

static void prvPrintString( const char * pcString )
{
    while( *pcString != 0x00 )
    {
        UARTCharPut( UART0_BASE, *pcString );
        pcString++;
    }
}
/*-----------------------------------------------------------*/

static void vPrintSlacks( char *buf, char s, int32_t * slackArray, TickType_t xCur )
{
    sprintf(buf, "%s\t%c\t%d\t%d\t%d\t%d\t%d\t%d\t%d\n\r",
            pcTaskGetTaskName(NULL), s,
            slackArray[0], slackArray[2], slackArray[3],
            slackArray[4], slackArray[5], slackArray[6],
            xCur);
    prvPrintString( buf );
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
    static char cMessage[ mainMAX_MSG_LEN ];

    int id = ( int ) pvParameters;

    int32_t slackArray[ 7 ];

    sprintf(cMessage, "FreeRTOS %s + SS", tskKERNEL_VERSION_NUMBER);
    vOLEDStringDraw( cMessage, 0, 0, mainFULL_SCALE );

    SsTCB_t *pxTaskSsTCB = getTaskSsTCB( NULL );

    for( ;; )
    {
        vTasksGetSlacks( slackArray );
        if ( xSemaphoreTake( xMutex, portMAX_DELAY ) )
        {
            vPrintSlacks( cMessage, 'S', slackArray, pxTaskSsTCB->xCur );
            xSemaphoreGive( xMutex );
        }

        sprintf( cMessage, "%s - %u", pcTaskGetTaskName( NULL ), pxTaskSsTCB->uxReleaseCount );
        vOLEDStringDraw( cMessage, 0, (mainCHARACTER_HEIGHT+1)*id, mainFULL_SCALE );

        vBusyWait( pxTaskSsTCB->xWcet - 200 );

        vTasksGetSlacks( slackArray );
        if ( xSemaphoreTake( xMutex, portMAX_DELAY ) )
        {
            vPrintSlacks( cMessage, 'E', slackArray, pxTaskSsTCB->xCur );
            xSemaphoreGive( xMutex );
        }

        vTaskDelayUntil( &( pxTaskSsTCB->xPreviousWakeTime ), pxTaskSsTCB->xPeriod );
    }
}
/*-----------------------------------------------------------*/

static void prvAperiodicTask( void *pvParameters )
{
    static char cMessage[ mainMAX_MSG_LEN ];

    int32_t slackArray[ 7 ];

    SsTCB_t *pxTaskSsTCB;

    pxTaskSsTCB = getTaskSsTCB( NULL );

    vTaskDelay( rand() % pxTaskSsTCB->xPeriod );

    for(;;)
    {
        pxTaskSsTCB->xCur = ( TickType_t ) 0;

        vTasksGetSlacks( slackArray );
        if ( xSemaphoreTake( xMutex, portMAX_DELAY ) )
        {
            vPrintSlacks( cMessage, 'S', slackArray, pxTaskSsTCB->xCur );
            xSemaphoreGive( xMutex );
        }

        vBusyWait( rand() % pxTaskSsTCB->xWcet );

        vTasksGetSlacks( slackArray );
        if ( xSemaphoreTake( xMutex, portMAX_DELAY ) )
        {
            vPrintSlacks( cMessage, 'E', slackArray, pxTaskSsTCB->xCur );
            xSemaphoreGive( xMutex );
        }

        vTaskDelay( rand() % pxTaskSsTCB->xPeriod );
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
