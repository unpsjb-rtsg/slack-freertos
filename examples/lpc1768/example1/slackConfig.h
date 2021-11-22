#ifndef EXAMPLES_LPC1768_EXAMPLE1_SLACKCONFIG_H_
#define EXAMPLES_LPC1768_EXAMPLE1_SLACKCONFIG_H_

#define EXAMPLE 1

/* ========================================================================= */
/* Required for integrating the SsTCB into the task TCB. */
#define configNUM_THREAD_LOCAL_STORAGE_POINTERS     1

/* Add functionality to be added to FreeRTOS's tasks.c source file. */
#define configINCLUDE_FREERTOS_TASK_C_ADDITIONS_H   1

/* Executes vSlackSchedulerSetup() at scheduler initialization. */
#define FREERTOS_TASKS_C_ADDITIONS_INIT()           vSlackSchedulerSetup()

/* Required for identify the IDLE task in slacks methods and deadline check. */
#define INCLUDE_xTaskGetIdleTaskHandle              1

/*
 * Slack methods available:
 * 0 = Fixed
 * 1 = Davis
 */
#define configUSE_SLACK_STEALING        1 /* 1: Use slack stealing methods, 0: No slack. */
#define configUSE_SLACK_METHOD          0 /* Slack method to use */
#define configUSE_SLACK_K               0 /* Only calculate slack at the scheduler start */
#define configMAX_SLACK_PRIO            2 /* priority levels that are used for slack. */
#define configMIN_SLACK_SD              1 /* Minimum amount of available slack. */
#define configSS_STORAGE_POINTER_INDEX  0 /* Which index use on TLS. */
#define configSS_VERIFY_DEADLINE        1 /* Verify deadlines on each tick interrupt. */
#define configSS_VERIFY_SCHEDULABILITY  1 /* Verify that the task set is schedulable under RM/DM. */
/* ========================================================================= */

#endif /* EXAMPLES_LPC1768_EXAMPLE1_SLACKCONFIG_H_ */
