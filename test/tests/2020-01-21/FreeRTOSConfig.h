/*
    FreeRTOS V8.1.0 - Copyright (C) 2014 Real Time Engineers Ltd. 
    All rights reserved

    VISIT http://www.FreeRTOS.org TO ENSURE YOU ARE USING THE LATEST VERSION.

    ***************************************************************************
     *                                                                       *
     *    FreeRTOS provides completely free yet professionally developed,    *
     *    robust, strictly quality controlled, supported, and cross          *
     *    platform software that has become a de facto standard.             *
     *                                                                       *
     *    Help yourself get started quickly and support the FreeRTOS         *
     *    project by purchasing a FreeRTOS tutorial book, reference          *
     *    manual, or both from: http://www.FreeRTOS.org/Documentation        *
     *                                                                       *
     *    Thank you!                                                         *
     *                                                                       *
    ***************************************************************************

    This file is part of the FreeRTOS distribution.

    FreeRTOS is free software; you can redistribute it and/or modify it under
    the terms of the GNU General Public License (version 2) as published by the
    Free Software Foundation >>!AND MODIFIED BY!<< the FreeRTOS exception.

    >>!   NOTE: The modification to the GPL is included to allow you to     !<<
    >>!   distribute a combined work that includes FreeRTOS without being   !<<
    >>!   obliged to provide the source code for proprietary components     !<<
    >>!   outside of the FreeRTOS kernel.                                   !<<

    FreeRTOS is distributed in the hope that it will be useful, but WITHOUT ANY
    WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
    FOR A PARTICULAR PURPOSE.  Full license text is available from the following
    link: http://www.freertos.org/a00114.html

    1 tab == 4 spaces!

    ***************************************************************************
     *                                                                       *
     *    Having a problem?  Start by reading the FAQ "My application does   *
     *    not run, what could be wrong?"                                     *
     *                                                                       *
     *    http://www.FreeRTOS.org/FAQHelp.html                               *
     *                                                                       *
    ***************************************************************************

    http://www.FreeRTOS.org - Documentation, books, training, latest versions,
    license and Real Time Engineers Ltd. contact details.

    http://www.FreeRTOS.org/plus - A selection of FreeRTOS ecosystem products,
    including FreeRTOS+Trace - an indispensable productivity tool, a DOS
    compatible FAT file system, and our tiny thread aware UDP/IP stack.

    http://www.OpenRTOS.com - Real Time Engineers ltd license FreeRTOS to High
    Integrity Systems to sell under the OpenRTOS brand.  Low cost OpenRTOS
    licenses offer ticketed support, indemnification and middleware.

    http://www.SafeRTOS.com - High Integrity Systems also provide a safety
    engineered and independently SIL3 certified version for use in safety and
    mission critical applications that require provable dependability.

    1 tab == 4 spaces!
*/


#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H

/*-----------------------------------------------------------
 * Application specific definitions.
 *
 * These definitions should be adjusted for your particular hardware and
 * application requirements.
 *
 * THESE PARAMETERS ARE DESCRIBED WITHIN THE 'CONFIGURATION' SECTION OF THE
 * FreeRTOS API DOCUMENTATION AVAILABLE ON THE FreeRTOS.org WEB SITE.
 *
 * See http://www.freertos.org/a00110.html.
 *----------------------------------------------------------*/

#include <stdint.h>
extern uint32_t SystemCoreClock;

#define configUSE_PREEMPTION			1
#define configUSE_IDLE_HOOK				0
#define configUSE_TICK_HOOK				0
#define configCPU_CLOCK_HZ				( SystemCoreClock )
#define configTICK_RATE_HZ				( ( TickType_t ) 1000 )
#define configMAX_PRIORITIES			TASK_COUNT_PARAM + 2
#define configMINIMAL_STACK_SIZE		( ( unsigned short ) 140 )
#define configTOTAL_HEAP_SIZE			( ( size_t ) ( 23 * 1024 ) )
#define configMAX_TASK_NAME_LEN			( 10 )
#define configUSE_TRACE_FACILITY		0
#define configUSE_16_BIT_TICKS			0
#define configIDLE_SHOULD_YIELD			1
#define configUSE_MUTEXES				0
#define configQUEUE_REGISTRY_SIZE		0
#define configCHECK_FOR_STACK_OVERFLOW	0
#define configUSE_RECURSIVE_MUTEXES		0
#define configUSE_MALLOC_FAILED_HOOK	1
#define configUSE_APPLICATION_TASK_TAG	0
#define configUSE_COUNTING_SEMAPHORES	0
#define configGENERATE_RUN_TIME_STATS	0

/* Co-routine definitions. */
#define configUSE_CO_ROUTINES 			0
#define configMAX_CO_ROUTINE_PRIORITIES ( 2 )

/* Software timer definitions. */
#define configUSE_TIMERS				0
#if ( configUSE_TIMERS == 1 )
#define configTIMER_TASK_PRIORITY		( 2 )
#define configTIMER_QUEUE_LENGTH		5
#define configTIMER_TASK_STACK_DEPTH	( configMINIMAL_STACK_SIZE * 2 )
#endif

/* ========================================================================= */
#define configUSE_PORT_OPTIMISED_TASK_SELECTION 0

/* Required for integrating the SsTCB into the task TCB. */
#define configNUM_THREAD_LOCAL_STORAGE_POINTERS 1

/* Add functionality to be added to FreeRTOS's tasks.c source file. */
#define configINCLUDE_FREERTOS_TASK_C_ADDITIONS_H 1

#define FREERTOS_TASKS_C_ADDITIONS_INIT() vSlackSchedulerSetup()

/* Required for identify the IDLE task in slacks methods and deadline check. */
#define INCLUDE_xTaskGetIdleTaskHandle  1

#define INCLUDE_xTaskGetCurrentTaskHandle 1

/*
 * Slack methods available:
 * 0 = Fixed
 * 1 = Davis
 */
#define configUSE_SLACK_STEALING 		SLACK /* 1: Use slack stealing methods, 0: No slack. */
#define configUSE_SLACK_METHOD          SLACK_METHOD /* Slack method to use */
#define configUSE_SLACK_K               SLACK_K /* Only calculate slack at the scheduler start */
#define configMAX_SLACK_PRIO            MAX_PRIO /* priority levels that are used for slack. */
#define configMIN_SLACK_SD              0 /* Minimum amount of available slack. */
/* ========================================================================= */

/* Set the following definitions to 1 to include the API function, or zero
to exclude the API function. */
#define INCLUDE_vTaskPrioritySet		1
#define INCLUDE_uxTaskPriorityGet		1
#define INCLUDE_vTaskDelete				0
#define INCLUDE_vTaskCleanUpResources	1
#define INCLUDE_vTaskSuspend			1
#define INCLUDE_vTaskDelayUntil			1
#define INCLUDE_vTaskDelay				1
#define INCLUDE_pcTaskGetTaskName       1

/* Cortex-M specific definitions. */
#ifdef __NVIC_PRIO_BITS
	/* __BVIC_PRIO_BITS will be specified when CMSIS is being used. */
	#define configPRIO_BITS       		__NVIC_PRIO_BITS
#else
	#define configPRIO_BITS       		5        /* 32 priority levels */
#endif

/* The lowest interrupt priority that can be used in a call to a "set priority"
function. */
#define configLIBRARY_LOWEST_INTERRUPT_PRIORITY			0x0f

/* The highest interrupt priority that can be used by any interrupt service
routine that makes calls to interrupt safe FreeRTOS API functions.  DO NOT CALL
INTERRUPT SAFE FREERTOS API FUNCTIONS FROM ANY INTERRUPT THAT HAS A HIGHER
PRIORITY THAN THIS! (higher priorities are lower numeric values. */
#define configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY	10

/* Interrupt priorities used by the kernel port layer itself.  These are generic
to all Cortex-M ports, and do not rely on any particular library functions. */
#define configKERNEL_INTERRUPT_PRIORITY 		( configLIBRARY_LOWEST_INTERRUPT_PRIORITY << (8 - configPRIO_BITS) )
/* !!!! configMAX_SYSCALL_INTERRUPT_PRIORITY must not be set to zero !!!!
See http://www.FreeRTOS.org/RTOS-Cortex-M3-M4.html. */
#define configMAX_SYSCALL_INTERRUPT_PRIORITY 	( configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY << (8 - configPRIO_BITS) )

/* Normal assert() semantics without relying on the provision of an assert.h
header file. */
#define configASSERT( x ) if( ( x ) == 0 ) { taskDISABLE_INTERRUPTS(); for( ;; ); }

/* Definitions that map the FreeRTOS port interrupt handlers to their CMSIS standard names. */
#define vPortSVCHandler SVC_Handler
#define xPortPendSVHandler PendSV_Handler
#define xPortSysTickHandler SysTick_Handler

/* Timing: 
 * http://stackoverflow.com/questions/13379220/generating-nano-second-delay-in-c
 * http://stackoverflow.com/questions/11530593/cycle-counter-on-arm-cortex-m4-or-m3
 * http://www.microbuilder.eu/Projects/LPC1343ReferenceDesign/DWTBenchmarking.aspx
 * http://infocenter.arm.com/help/index.jsp?topic=/com.arm.doc.ddi0337h/BIIFBHIF.html
 */

/* Addresses of registers in the Cortex-M debug hardware. */
#define DWT_CYCCNT 			( *( ( unsigned long * ) 0xE0001004 ) )
#define DWT_CONTROL 		( *( ( unsigned long * ) 0xE0001000 ) )
#define SCB_DEMCR 			( *( ( unsigned long * ) 0xE000EDFC ) )
#define TRCENA_BIT			( 0x01000000UL )
#define COUNTER_ENABLE_BIT	( 0x01UL )
#define CPU_CYCLES          DWT_CYCCNT

#define STOPWATCH_RESET()								\
{														\
	/* Enable Trace System (TRCENA) */					\
	SCB_DEMCR = SCB_DEMCR | TRCENA_BIT;					\
	/* Reset counter. */								\
	DWT_CYCCNT = 0;										\
	/* Enable counter. */								\
	DWT_CONTROL = DWT_CONTROL | COUNTER_ENABLE_BIT;		\
}

#define TASK_COUNT TASK_COUNT_PARAM
#define RELEASE_COUNT RELEASE_COUNT_PARAM

/* trace */
#define configDO_SLACK_TRACE 0

/* Test to perform.:
1. Cost in CPU cycles of the vTaskDelayUntil() kernel function.
2. Amount of ceil and floor operations performed by the Slack Stealing method.
3. Execution cost of the Slack Stealing method in CPU cycles.
4. Amount of for and while loops required by the Slack Stealing method.
*/
#define configKERNEL_TEST KERNEL_TEST

#if ( tskKERNEL_VERSION_MAJOR == 10 )

/* === delay_until() cost ================================================== */
/* The trace macro definitions must be in this header file.                  */
#if configKERNEL_TEST == 1
void vMacroTaskDelay( void );
void vMacroTaskSwitched( void );
#define traceTASK_DELAY_UNTIL(xTimeToWake) vMacroTaskDelay();
#define traceTASK_SWITCHED_OUT()           vMacroTaskSwitched();
#endif
/* ========================================================================= */

#endif


#if ( tskKERNEL_VERSION_MAJOR == 9 )

/* === delay_until() cost ================================================== */
/* The trace macro definitions must be in this header file.                  */
#if configKERNEL_TEST == 1
void vMacroTaskDelay( void );
void vMacroTaskSwitched( void );
#define traceTASK_DELAY_UNTIL(xTimeToWake) vMacroTaskDelay();
#define traceTASK_SWITCHED_OUT()           vMacroTaskSwitched();
#endif
/* ========================================================================= */

#endif

#if ( tskKERNEL_VERSION_MAJOR == 8 )

/* === delay_until() cost ================================================== */
#if configKERNEL_TEST == 1
void vMacroTaskDelay( void );
void vMacroTaskSwitched( void );
#define traceTASK_DELAY_UNTIL() vMacroTaskDelay();
#define traceTASK_SWITCHED_OUT() vMacroTaskSwitched();

uint32_t ulDelayTime;
uint32_t ulDelayTime1;

typedef uint32_t xType[TASK_COUNT][RELEASE_COUNT + 2];
void vTaskGetTraceInfo( xType *pxArray, uint32_t time, uint32_t r );
#endif
/* ========================================================================= */

/* === slack methods ceil/floor cost ======================================= */
#if configKERNEL_TEST == 2
typedef uint32_t xType[TASK_COUNT][RELEASE_COUNT + 1];
void vTaskGetTraceInfo( void );
#endif
/* ========================================================================= */

/* === prvTaskCalculateSlack cost ========================================== */
#if configKERNEL_TEST == 3
typedef uint32_t xType[TASK_COUNT][RELEASE_COUNT + 1];
void vTaskGetTraceInfo( uint32_t cycles );
#endif
/* ========================================================================= */

/* === prvTaskCalculateSlack cost measured in loops ======================== */
#if configKERNEL_TEST == 4
typedef uint32_t xType[TASK_COUNT][RELEASE_COUNT + 1];
void vTaskGetTraceInfo( void );
#endif
/* ========================================================================= */

#endif

#endif /* FREERTOS_CONFIG_H */
