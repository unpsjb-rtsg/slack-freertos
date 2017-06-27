/*
 * @brief FreeRTOS Blinky example
 *
 * @note
 * Copyright(C) NXP Semiconductors, 2013
 * All rights reserved.
 *
 * @par
 * Software that is described herein is for illustrative purposes only
 * which provides customers with programming information regarding the
 * LPC products.  This software is supplied "AS IS" without any warranties of
 * any kind, and NXP Semiconductors and its licensor disclaim any and
 * all warranties, express or implied, including all implied warranties of
 * merchantability, fitness for a particular purpose and non-infringement of
 * intellectual property rights.  NXP Semiconductors assumes no responsibility
 * or liability for the use of the software, conveys no license or rights under any
 * patent, copyright, mask work right, or any other intellectual property rights in
 * or to any products. NXP Semiconductors reserves the right to make changes
 * in the software without notification. NXP Semiconductors also makes no
 * representation or warranty that such application will be suitable for the
 * specified use without further testing or modification.
 *
 * @par
 * Permission to use, copy, modify, and distribute this software and its
 * documentation is hereby granted, under NXP Semiconductors' and its
 * licensor's relevant copyrights in the software, without fee, provided that it
 * is used in conjunction with NXP Semiconductors microcontrollers.  This
 * copyright, permission, and disclaimer notice must appear in all copies of
 * this code.
 */
/*****************************************************************************
 * Includes
 ****************************************************************************/
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "slack.h"
#include "utils.h"
#include "common.h"
#include "stdlib.h" // for rand()

/*****************************************************************************
 * Macros and definitions
 ****************************************************************************/
/* The linker does not include this code in liblpc.a because nothing in it
 * references it... */
#define CRP_NO_CRP          0xFFFFFFFF
__attribute__ ((used,section(".crp"))) const unsigned int CRP_WORD = CRP_NO_CRP ;

#define TASK_CNT 3
#define TASK_1_WCET 1000
#define TASK_2_WCET 1000
#define TASK_3_WCET 1000
#define TASK_1_PERIOD 3000
#define TASK_2_PERIOD 4000
#define TASK_3_PERIOD 6000
#define TASK_1_PRIO configMAX_PRIORITIES - configMAX_SLACK_PRIO - 1
#define TASK_2_PRIO configMAX_PRIORITIES - configMAX_SLACK_PRIO - 2
#define TASK_3_PRIO configMAX_PRIORITIES - configMAX_SLACK_PRIO - 3

#define ATASK_WCET 2000
#define ATASK_MAX_DELAY 4000
#define APTASK_1_PRIO configMAX_PRIORITIES - 1
#define APTASK_2_PRIO configMAX_PRIORITIES - 2
#define APTASK_3_PRIO configMAX_PRIORITIES - 3

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
static void aperiodic_task_body( void* params );
static void vTimerCallback( TimerHandle_t xTimer );

/*****************************************************************************
 * Private data
 ****************************************************************************/
static TaskHandle_t task_handles[ TASK_CNT ];
static TaskHandle_t ap_task_handles[ 3 ];
static TimerHandle_t timer_handle;
static gpioMap_t ap_leds[] = { LEDR, LEDG, LEDB };

/*****************************************************************************
 * Public data
 ****************************************************************************/
gpioMap_t leds[] = { LED1, LED2, LED3 };
#ifdef TRACEALYZER_v3_1_3
traceString slack_channel;
#endif

/*****************************************************************************
 * Private functions
 ****************************************************************************/
/**
 * Aperiodic task.
 * @param params
 */
static void aperiodic_task_body( void* params )
{
    ( void ) params;

    int32_t slackArray[ 6 ];

    SsTCB_t *pxTaskSsTCB;

#if( tskKERNEL_VERSION_MAJOR == 8 )
    pxTaskSsTCB = pxTaskGetTaskSsTCB( NULL );
#endif
#if( tskKERNEL_VERSION_MAJOR == 9 )
    pxTaskSsTCB = getTaskSsTCB( NULL );
#endif

    vTaskDelay( rand() % ATASK_MAX_DELAY );

    for(;;)
    {
#ifdef TRACEALYZER_v3_1_3
        vTracePrintF( slack_channel, "%d - %d", xSlackSD, pxTaskSsTCB->xSlack );
#endif

        gpioWrite( ap_leds[ pxTaskSsTCB->xId - 1], ON);

        pxTaskSsTCB->xCur = ( TickType_t ) 0;

        printSlacks( 'S', slackArray, pxTaskSsTCB->xCur );

        vUtilsEatCpu( rand() % ATASK_WCET );

        printSlacks( 'E', slackArray, pxTaskSsTCB->xCur );

        gpioWrite( ap_leds[ pxTaskSsTCB->xId - 1], ON);

#ifdef TRACEALYZER_v3_1_3
        vTracePrintF( slack_channel, "%d - %d", xSlackSD, pxTaskSsTCB->xSlack );
#endif

        vTaskDelay( rand() % ATASK_MAX_DELAY );
    }
}

static void vTimerCallback( TimerHandle_t xTimer )
{
    ( void ) xTimer;

    uartWriteString( UART_USB, "End!\n\r" );

    for( ;; );
}

/*****************************************************************************
 * Public functions
 ****************************************************************************/
/**
 * @brief	main routine for FreeRTOS blinky example
 * @return	Nothing, function should not exit
 */
int main(void)
{
	prvSetupHardware();

	// Initializes the trace recorder, but does not start the tracing.
#ifdef TRACEALYZER_v3_0_2
	vTraceInitTraceData();
#endif
#ifdef TRACEALYZER_v3_1_3
	vTraceEnable( TRC_INIT );
    slack_channel = xTraceRegisterString("Slack Events");
#endif

    uartWriteString( UART_USB, "Example 3\r\n" );

    // Periodic tasks.
    xTaskCreate( periodicTaskBody, "T1", 256, NULL, TASK_1_PRIO, &task_handles[ 0 ] );
    xTaskCreate( periodicTaskBody, "T2", 256, NULL, TASK_2_PRIO, &task_handles[ 1 ] );
    xTaskCreate( periodicTaskBody, "T3", 256, NULL, TASK_3_PRIO, &task_handles[ 2 ] );

    // Aperiodic tasks.
    xTaskCreate ( aperiodic_task_body, "TA1", 256, NULL, APTASK_1_PRIO, &ap_task_handles[ 0 ] );
    xTaskCreate ( aperiodic_task_body, "TA2", 256, NULL, APTASK_2_PRIO, &ap_task_handles[ 1 ] );
    xTaskCreate ( aperiodic_task_body, "TA3", 256, NULL, APTASK_3_PRIO, &ap_task_handles[ 2 ] );

    // Timer task.
    timer_handle = xTimerCreate( "Timer", pdMS_TO_TICKS( 5000 ), pdFALSE, ( void * ) 0, vTimerCallback );
    xTimerStart( timer_handle, 0 );

#if( configUSE_SLACK_STEALING == 1 )

    #if( tskKERNEL_VERSION_MAJOR == 9 )
    {
        vSlackSystemSetup();
    }
    #endif

    // Configure additional parameters needed by the slack stealing framework
#if( tskKERNEL_VERSION_MAJOR == 8 )
    vTaskSetParams( task_handles[ 0 ], TASK_1_PERIOD, TASK_1_PERIOD, TASK_1_WCET, 1 );
    vTaskSetParams( task_handles[ 1 ], TASK_2_PERIOD, TASK_2_PERIOD, TASK_2_WCET, 2 );
    vTaskSetParams( task_handles[ 2 ], TASK_3_PERIOD, TASK_3_PERIOD, TASK_3_WCET, 3 );
#endif
#if( tskKERNEL_VERSION_MAJOR == 9 )
    // Set periodic tasks parameters.
    vSlackSetTaskParams( task_handles[ 0 ], PERIODIC_TASK, TASK_1_PERIOD, TASK_1_PERIOD, TASK_1_WCET, 1 );
    vSlackSetTaskParams( task_handles[ 1 ], PERIODIC_TASK, TASK_2_PERIOD, TASK_2_PERIOD, TASK_2_WCET, 2 );
    vSlackSetTaskParams( task_handles[ 2 ], PERIODIC_TASK, TASK_3_PERIOD, TASK_3_PERIOD, TASK_3_WCET, 3 );

    // Set aperiodic tasks parameters.
    vSlackSetTaskParams( ap_task_handles[ 0 ], APERIODIC_TASK, 0, 0, 0, 1 );
    vSlackSetTaskParams( ap_task_handles[ 1 ], APERIODIC_TASK, 0, 0, 0, 2 );
    vSlackSetTaskParams( ap_task_handles[ 2 ], APERIODIC_TASK, 0, 0, 0, 3 );
#endif

    #if( tskKERNEL_VERSION_MAJOR == 9 )
    {
    	vSlackSchedulerSetup();
    }
    #endif
#endif

    // Start the tracing.
#ifdef TRACEALYZER_v3_0_2
    uiTraceStart();
#endif
#ifdef TRACEALYZER_v3_1_3
    vTraceEnable( TRC_START );
#endif

	// Start the scheduler.
	vTaskStartScheduler();

	// Should never arrive here.
	for(;;);
}
