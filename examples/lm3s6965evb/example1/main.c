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
 * take the mutex, and a missed deadline will occur. To avoid this situation,
 * a timeout is used when the periodic tasks is waiting to obtain the mutex.
 *
 * This program requires FreeRTOS v10.0.0 or later.
 *
 * Use the following command to start running the application in QEMU, pausing
 * to wait for a debugger connection:
 * "qemu-system-arm -machine lm3s6965evb -s -S -kernel build\lm3s6965evb-example1.elf"
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

/*****************************************************************************
 * Includes
 ****************************************************************************/
/* Standard includes. */
#include <stdio.h>
#include <stdlib.h>
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

/*****************************************************************************
 * Macros and definitions
 ****************************************************************************/
/* Task stack sizes. */
#define taskDEFAULT_STACK   ( configMINIMAL_STACK_SIZE + 40 )

/* Task priorities. */
#define TASK_1_PRIO     ( configMAX_PRIORITIES - configMAX_SLACK_PRIO - 1 )
#define TASK_2_PRIO     ( configMAX_PRIORITIES - configMAX_SLACK_PRIO - 2 )
#define TASK_3_PRIO     ( configMAX_PRIORITIES - configMAX_SLACK_PRIO - 3 )
#define TASK_4_PRIO     ( configMAX_PRIORITIES - configMAX_SLACK_PRIO - 4 )
#define ATASK_1_PRIO    ( configMAX_PRIORITIES - 1 )
#define ATASK_2_PRIO    ( configMAX_PRIORITIES - 2 )
#define ATASK_WCET      ( 200 )
#define ATASK_MAX_DELAY ( 400 )

#define MUTEX_TIMEOUT 10
#define mainMAX_MSG_LEN ( 150 )

/* Constants used when writing strings to the display. */
#define mainCHARACTER_HEIGHT                ( 9 )
#define mainMAX_ROWS_128                    ( mainCHARACTER_HEIGHT * 14 )
#define mainMAX_ROWS_96                     ( mainCHARACTER_HEIGHT * 10 )
#define mainMAX_ROWS_64                     ( mainCHARACTER_HEIGHT * 7 )
#define mainFULL_SCALE                      ( 15 )
#define ulSSI_FREQUENCY                     ( 3500000UL )

/*****************************************************************************
 * Private data declaration
 ****************************************************************************/
/* None */

/*****************************************************************************
 * Public data declaration
 ****************************************************************************/
/* None */

/*****************************************************************************
 * Private functions declaration
 ****************************************************************************/
/**
 * Entry function for the periodic tasks.
 * @param pvParameters Should be a numerical id.
 */
static void prvPeriodicTask( void *pvParameters );

/**
 * Entry function for the aperiodic tasks.
 * @param pvParameters Should be a numerical id.
 */
static void prvAperiodicTask( void* pvParameters );

/**
 * Configure the hardware.
 */
static void prvSetupHardware( void );

/**
 * Basic polling UART write function.
 */
static void prvPrintString( const char * pcString );

/* Functions to access the OLED.  The one used depends on the dev kit being used. */
static void ( *vOLEDInit )( uint32_t ) = NULL;
static void ( *vOLEDStringDraw )( const char *, uint32_t, uint32_t, unsigned char ) = NULL;
static void ( *vOLEDImageDraw )( const unsigned char *, uint32_t, uint32_t, uint32_t, uint32_t ) = NULL;
static void ( *vOLEDClear )( void ) = NULL;

/*****************************************************************************
 * Private data
 ****************************************************************************/
static SemaphoreHandle_t xMutex = NULL;

static char cMessage[ mainMAX_MSG_LEN ];

// tick, task id, system available slack, 4 periodic tasks slacks
static int32_t slackArray[ 7 ];

/*****************************************************************************
 * Public data
 ****************************************************************************/
/* None */

/*****************************************************************************
 * Private functions
 ****************************************************************************/
static void prvSetupHardware( void )
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
            (int) slackArray[0], (int) slackArray[2], (int) slackArray[3],
            (int) slackArray[4], (int) slackArray[5], (int) slackArray[6],
            (int) xCur);
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
    int id = ( int ) pvParameters;

    SsTCB_t *pxTaskSsTCB = pvSlackGetTaskSsTCB( NULL );

    for( ;; )
    {
        if ( xSemaphoreTake( xMutex, MUTEX_TIMEOUT ) )
        {
            vTasksGetSlacks( slackArray );
            vPrintSlacks( cMessage, 'S', slackArray, pxTaskSsTCB->xCur );
            xSemaphoreGive( xMutex );
        }
        else
        {
            prvPrintString("!");
        }

        sprintf( cMessage, "%s - %d", pcTaskGetTaskName( NULL ), (int) pxTaskSsTCB->uxReleaseCount );
        vOLEDStringDraw( cMessage, 0, (mainCHARACTER_HEIGHT+1)*(id-1), mainFULL_SCALE );

        vBusyWait( pxTaskSsTCB->xWcet - 10 );

        if ( xSemaphoreTake( xMutex, MUTEX_TIMEOUT ) )
        {
            vTasksGetSlacks( slackArray );
            vPrintSlacks( cMessage, 'E', slackArray, pxTaskSsTCB->xCur );
            xSemaphoreGive( xMutex );
        }
        else
        {
            prvPrintString("!");
        }

        vTaskDelayUntil( &( pxTaskSsTCB->xPreviousWakeTime ), pxTaskSsTCB->xPeriod );
    }
}
/*-----------------------------------------------------------*/

static void prvAperiodicTask( void *pvParameters )
{
    static char cMessage[ mainMAX_MSG_LEN ];

    SsTCB_t *pxTaskSsTCB;

    pxTaskSsTCB = pvSlackGetTaskSsTCB( NULL );

    vTaskDelay( rand() % pxTaskSsTCB->xPeriod );

    for(;;)
    {
        pxTaskSsTCB->xCur = ( TickType_t ) 0;

        if ( xSemaphoreTake( xMutex, portMAX_DELAY ) )
        {
            vTasksGetSlacks( slackArray );
            vPrintSlacks( cMessage, 'S', slackArray, pxTaskSsTCB->xCur );
            xSemaphoreGive( xMutex );
        }

        vBusyWait( rand() % pxTaskSsTCB->xWcet );

        if ( xSemaphoreTake( xMutex, portMAX_DELAY ) )
        {
            vTasksGetSlacks( slackArray );
            vPrintSlacks( cMessage, 'E', slackArray, pxTaskSsTCB->xCur );
            xSemaphoreGive( xMutex );
        }

        vTaskDelay( rand() % pxTaskSsTCB->xPeriod );
    }
}
/*-----------------------------------------------------------*/

/*****************************************************************************
 * Public functions
 ****************************************************************************/
int main( void )
{
    // Verify that configUSE_SLACK_STEALING is enabled
    configSS_ASSERT_EQUAL( configUSE_SLACK_STEALING, 1 );
    // Verify that tskKERNEL_VERSION_MAJOR is >= 10
    configSS_ASSERT_GREATHER_OR_EQUAL( tskKERNEL_VERSION_MAJOR, 10);

#if TZ == 1
    /* Initialise the trace recorder.  Use of the trace recorder is optional.
    See http://www.FreeRTOS.org/trace for more information and the comments at
    the top of this file regarding enabling trace in this demo. */
    vTraceEnable( TRC_INIT );
#endif

    prvSetupHardware();

    // Create the mutex.
    xMutex = xSemaphoreCreateMutex();

    TaskHandle_t task1;
    TaskHandle_t task2;
    TaskHandle_t task3;
    TaskHandle_t task4;
    TaskHandle_t atask1;
    TaskHandle_t atask2;

    // Create the tasks
    xTaskCreate( prvPeriodicTask, "T1", taskDEFAULT_STACK, (void*) 1, TASK_1_PRIO, &task1 );
    xTaskCreate( prvPeriodicTask, "T2", taskDEFAULT_STACK, (void*) 2, TASK_2_PRIO, &task2 );
    xTaskCreate( prvPeriodicTask, "T3", taskDEFAULT_STACK, (void*) 3, TASK_3_PRIO, &task3 );
    xTaskCreate( prvPeriodicTask, "T4", taskDEFAULT_STACK, (void*) 4, TASK_4_PRIO, &task4 );
    xTaskCreate( prvAperiodicTask, "TA1", 256, NULL, ATASK_1_PRIO, &atask1 );
    xTaskCreate( prvAperiodicTask, "TA2", 256, NULL, ATASK_2_PRIO, &atask2 );

    // Configure additional parameters needed by the slack stealing framework.
    vSlackSetTaskParams( task1, PERIODIC_TASK, 300,  300,  100, 1 );
    vSlackSetTaskParams( task2, PERIODIC_TASK, 400,  400,  100, 2 );
    vSlackSetTaskParams( task3, PERIODIC_TASK, 600,  600,  100, 3 );
    vSlackSetTaskParams( task4, PERIODIC_TASK, 1200, 1200, 100, 4 );
    vSlackSetTaskParams( atask1, APERIODIC_TASK, ATASK_MAX_DELAY, 0, ATASK_WCET, 1 );
    vSlackSetTaskParams( atask2, APERIODIC_TASK, ATASK_MAX_DELAY, 0, ATASK_WCET, 2 );

    /* Map the OLED access functions to the driver functions that are appropriate
    for the evaluation kit being used. */
    configASSERT( ( HWREG( SYSCTL_DID1 ) & SYSCTL_DID1_PRTNO_MASK ) == SYSCTL_DID1_PRTNO_6965 );
    vOLEDInit = OSRAM128x64x4Init;
    vOLEDStringDraw = OSRAM128x64x4StringDraw;
    vOLEDImageDraw = OSRAM128x64x4ImageDraw;
    vOLEDClear = OSRAM128x64x4Clear;

    /* Initialise the OLED and display a startup message. */
    vOLEDInit( ulSSI_FREQUENCY );

    /* Initialize random number generator with seed zero to have a reproducible
     * trace. */
    srand((unsigned) 0);

#if TZ == 1
    vTraceEnable( TRC_START );
#endif

    /* Start the scheduler. */
    vTaskStartScheduler();

    /* Will only get here if there was insufficient memory to create the idle
    task. */
    for( ;; );
}
/*-----------------------------------------------------------*/

void vApplicationDeadlineMissedHook( char *pcTaskName, const SsTCB_t *xSsTCB,
        TickType_t xTickCount )
{
    taskDISABLE_INTERRUPTS();
    sprintf(cMessage, "\n\r%s missed its deadline at %d\n\r", pcTaskName, (int) xTickCount);
    prvPrintString( cMessage );
    for (;; ) {}
}
/*-----------------------------------------------------------*/

void vApplicationNotSchedulable( void )
{
    taskDISABLE_INTERRUPTS();
    for (;; ) {}
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
/*-----------------------------------------------------------*/

int __error__(char *pcFilename, unsigned long ulLine) {
    return 0;
}
/*-----------------------------------------------------------*/
