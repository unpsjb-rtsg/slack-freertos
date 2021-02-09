#ifndef SLACKCONFIG_H_
#define SLACKCONFIG_H_

/* Required for integrating the SsTCB into the task TCB. */
#define configNUM_THREAD_LOCAL_STORAGE_POINTERS 1

#if SLACK == 1
/* Add functionality to be added to FreeRTOS's tasks.c source file. */
#define configINCLUDE_FREERTOS_TASK_C_ADDITIONS_H 1

/* Call vSlackSchedulerSetup() from vTaskStartScheduler(). */
#define FREERTOS_TASKS_C_ADDITIONS_INIT() vSlackSchedulerSetup()
#endif

/* Required for identify the IDLE task in slacks methods and deadline check. */
#define INCLUDE_xTaskGetIdleTaskHandle  1

#define INCLUDE_xTaskGetCurrentTaskHandle 1

#define configUSE_PORT_OPTIMISED_TASK_SELECTION 1

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

#endif /* SLACKCONFIG_H_ */
