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
#define ATASK_CNT 3

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
#define ATASK_1_PRIO configMAX_PRIORITIES - 1
#define ATASK_2_PRIO configMAX_PRIORITIES - 2
#define ATASK_3_PRIO configMAX_PRIORITIES - 3

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
static void vAperiodicTask( void* params );

/*****************************************************************************
 * Private data
 ****************************************************************************/
static TaskHandle_t task_handles[ TASK_CNT ];
static TaskHandle_t atask_handles[ ATASK_CNT ];

/*****************************************************************************
 * Public data
 ****************************************************************************/
gpioMap_t leds[] = { LED1, LED2, LED3 };
gpioMap_t aleds[] = { LEDR, LEDG, LEDB };
#ifdef TRACEALYZER_v3_1_3
traceString slack_channel;
#endif

/*****************************************************************************
 * Private functions
 ****************************************************************************/
/**
 * Aperiodic task.
 * @param params Not used.
 */
static void vAperiodicTask( void* params )
{
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

        pxTaskSsTCB->xCur = ( TickType_t ) 0;

        gpioWrite( aleds[ pxTaskSsTCB->xId - 1], ON );

        vCommonPrintSlacks( 'S', slackArray, pxTaskSsTCB->xCur );

        vUtilsEatCpu( rand() % ATASK_WCET );

        vCommonPrintSlacks( 'E', slackArray, pxTaskSsTCB->xCur );

        gpioWrite( aleds[ pxTaskSsTCB->xId - 1], OFF );

#ifdef TRACEALYZER_v3_1_3
        vTracePrintF( slack_channel, "%d - %d", xSlackSD, pxTaskSsTCB->xSlack );
#endif

        vTaskDelay( rand() % ATASK_MAX_DELAY );
    }
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
	vCommonSetupHardware();

    // Initializes the trace recorder, but does not start the tracing.
#ifdef TRACEALYZER_v3_0_2
    vTraceInitTraceData();
#endif
#ifdef TRACEALYZER_v3_1_3
    vTraceEnable( TRC_INIT );
    slack_channel = xTraceRegisterString("Slack Events");
#endif

    uartWriteString( UART_USB, "Example 2\r\n" );

    // create periodic tasks
    xTaskCreate( vCommonPeriodicTask, "T1", 256, NULL, TASK_1_PRIO, &task_handles[ 0 ] );
    xTaskCreate( vCommonPeriodicTask, "T2", 256, NULL, TASK_2_PRIO, &task_handles[ 1 ] );
    xTaskCreate( vCommonPeriodicTask, "T3", 256, NULL, TASK_3_PRIO, &task_handles[ 2 ] );

    /* Aperiodic task */
    xTaskCreate ( vAperiodicTask, "TA1", 256, NULL, ATASK_1_PRIO, &atask_handles[ 0 ] );
    xTaskCreate ( vAperiodicTask, "TA2", 256, NULL, ATASK_2_PRIO, &atask_handles[ 1 ] );
    xTaskCreate ( vAperiodicTask, "TA3", 256, NULL, ATASK_3_PRIO, &atask_handles[ 2 ] );

#if( configUSE_SLACK_STEALING == 1 )
    #if( tskKERNEL_VERSION_MAJOR == 9 )
    {
        vSlackSystemSetup();
    }
    #endif

    // additional parameters needed by the slack stealing framework
#if( tskKERNEL_VERSION_MAJOR == 8 )
    vTaskSetParams( task_handles[ 0 ], TASK_1_PERIOD, TASK_1_PERIOD, TASK_1_WCET, 1 );
    vTaskSetParams( task_handles[ 1 ], TASK_2_PERIOD, TASK_2_PERIOD, TASK_2_WCET, 2 );
    vTaskSetParams( task_handles[ 2 ], TASK_3_PERIOD, TASK_3_PERIOD, TASK_3_WCET, 3 );
#endif
#if( tskKERNEL_VERSION_MAJOR == 9 )
    vSlackSetTaskParams( task_handles[ 0 ], PERIODIC_TASK, TASK_1_PERIOD,
            TASK_1_PERIOD, TASK_1_WCET, 1 );
    vSlackSetTaskParams( task_handles[ 1 ], PERIODIC_TASK, TASK_2_PERIOD,
            TASK_2_PERIOD, TASK_2_WCET, 2 );
    vSlackSetTaskParams( task_handles[ 2 ], PERIODIC_TASK, TASK_3_PERIOD,
            TASK_3_PERIOD, TASK_3_WCET, 3 );

    /* Aperiodic task */
    vSlackSetTaskParams( atask_handles[ 0 ], APERIODIC_TASK, ATASK_MAX_DELAY,
            0, ATASK_WCET, 1 );
    vSlackSetTaskParams( atask_handles[ 1 ], APERIODIC_TASK, ATASK_MAX_DELAY,
            0, ATASK_WCET, 2 );
    vSlackSetTaskParams( atask_handles[ 2 ], APERIODIC_TASK, ATASK_MAX_DELAY,
            0, ATASK_WCET, 3 );
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
