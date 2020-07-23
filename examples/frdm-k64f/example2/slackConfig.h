#ifndef EXAMPLES_FRDM_K64F_EXAMPLE2_SLACKCONFIG_H_
#define EXAMPLES_FRDM_K64F_EXAMPLE2_SLACKCONFIG_H_

/* ========================================================================= */
#define configUSE_PORT_OPTIMISED_TASK_SELECTION 0
#define configNUM_THREAD_LOCAL_STORAGE_POINTERS 1

#define INCLUDE_xTaskGetIdleTaskHandle  1 /* Required for identify the IDLE task in slacks methods and deadline check */

/*
 * Slack methods available:
 * 0 = Fixed
 * 1 = Davis
 */
#define configUSE_SLACK_STEALING        1 /* 1: Use slack stealing methods, 0: No slack. */
#define configUSE_SLACK_METHOD          0 /* Slack method to use */
#define configUSE_SLACK_K               0 /* Only calculate slack at the scheduler start */
#define configMAX_SLACK_PRIO            2 /* priority levels that are used for slack. */
#define configMIN_SLACK_SD              0 /* Minimum amount of available slack. */

/*
 * How the task simulate the execution time.
 * 0 = execute asm nops
 * 1 = exact tick count
 */
#define configTASK_EXEC                 0
/* ========================================================================= */

#endif /* EXAMPLES_FRDM_K64F_EXAMPLE2_SLACKCONFIG_H_ */
