#ifndef EXAMPLES_LPC1768_EXAMPLE4_SLACKCONFIG_H_
#define EXAMPLES_LPC1768_EXAMPLE4_SLACKCONFIG_H_

#define EXAMPLE 4

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
 */
#define SLACK_METHOD_URRIZA_2010        0
#define SLACK_METHOD_DAVIS_1993         1

#define configUSE_SLACK_STEALING        1 /* 1: Use slack stealing methods, 0: No slack. */
#define configSS_SLACK_METHOD           SLACK_METHOD_URRIZA_2010 /* Slack method to use */
#define configSS_SLACK_K                0 /* Only calculate slack at the scheduler start */
#define configSS_SLACK_PRIOS            2 /* priority levels that are used for slack. */
#define configSS_MIN_SLACK_SD           1 /* Minimum amount of available slack. */
#define configSS_STORAGE_POINTER_INDEX  0 /* Which index use on TLS. */
#define configSS_VERIFY_DEADLINE        1 /* Verify deadlines on each tick interrupt. */
#define configSS_VERIFY_SCHEDULABILITY  1 /* Verify that the task set is schedulable under RM/DM. */

#define configUSE_PORT_OPTIMISED_TASK_SELECTION 0

/* ========================================================================= */

#endif /* EXAMPLES_LPC1768_EXAMPLE4_SLACKCONFIG_H_ */
