#ifndef EXAMPLES_LPC1768_EXAMPLE1_SLACKCONFIG_H_
#define EXAMPLES_LPC1768_EXAMPLE1_SLACKCONFIG_H_

#define EXAMPLE 1

#define configMAX_PRIORITIES			( 15 )

/* ========================================================================= */
/* Required for integrating the SsTCB into the task TCB. */
#define configNUM_THREAD_LOCAL_STORAGE_POINTERS 1

/* Add functionality to be added to FreeRTOS's tasks.c source file. */
#define configINCLUDE_FREERTOS_TASK_C_ADDITIONS_H 1

#define FREERTOS_TASKS_C_ADDITIONS_INIT() vSlackSchedulerSetup()

/* Required for identify the IDLE task in slacks methods and deadline check. */
#define INCLUDE_xTaskGetIdleTaskHandle  1

#define configUSE_PORT_OPTIMISED_TASK_SELECTION 0

/*
 * Slack methods available:
 * 0 = Fixed
 * 1 = Davis
 */
#define configUSE_SLACK_STEALING        1 /* 1: Use slack stealing methods, 0: No slack. */
#define configUSE_SLACK_METHOD          0 /* Slack method to use */
#define configUSE_SLACK_K               0 /* Only calculate slack at the scheduler start */
#define configMAX_SLACK_PRIO            1 /* priority levels that are used for slack. */
#define configMIN_SLACK_SD              0 /* Minimum amount of available slack. */

/*
 * How the task simulate the execution time.
 * 0 = execute asm nops
 * 1 = exact tick count
 */
#define configTASK_EXEC                 0

/* ========================================================================= */

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

#define TASK_COUNT 10
#define RELEASE_COUNT 10

/* trace */
#define configDO_SLACK_TRACE 0

#define SLACK			configUSE_SLACK_STEALING
#define SLACK_METHOD 	configUSE_SLACK_METHOD
#define SLACK_K			configUSE_SLACK_K


/* Test to perform.:
1. Cost in CPU cycles of the vTaskDelayUntil() kernel function.
2. Amount of ceil and floor operations performed by the Slack Stealing method.
3. Execution cost of the Slack Stealing method in CPU cycles.
4. Amount of for and while loops required by the Slack Stealing method.
*/
#define configKERNEL_TEST 1

/* ========================================================================= */

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

#endif /* EXAMPLES_LPC1768_EXAMPLE1_SLACKCONFIG_H_ */
