#ifndef SLACK_H
#define SLACK_H

#include "task.h"

#ifdef __cplusplus
extern "C" {
#endif

#define U_CEIL( x, y )    ( ( x / y ) + ( x % y != 0 ) )
#define U_FLOOR( x, y )   ( x / y )
#define ONE_TICK 	      ( TickType_t ) 1U

#define getTaskSsTCB( x ) ( ( SsTCB_t * ) pvTaskGetThreadLocalStoragePointer( ( TaskHandle_t ) x, 0 ) )

/* Task types. */
typedef enum
{
	PERIODIC_TASK,
	APERIODIC_TASK
} SsTaskType_t;

/*
 * The system available slack. It's the minimum value of all the task's
 * slacks.
 */
extern volatile BaseType_t xSlackSD;

/*
 * This list contains references to all the ready tasks, ordered by their
 * absolute deadlines.
 */
extern List_t xDeadlineTaskList;

/*
 * This list contains references to all the tasks that account for the
 * available slack of the system. Tasks at the idle priority level are not
 * accounted.
 */
extern List_t xSsTaskList;

/*
 * This list stores tasks that have been blocked by insufficient available
 * slack. They need to be stored in a separated list because we don't know
 * in advance when there is enough available slack. The blocked (delayed)
 * list of FreeRTOS stores real-time tasks that are blocked in waiting of
 * a resource -- with a timeout or for an unspecified amount of time. That
 * list could be used to store the slack-blocked tasks, identifying the tasks
 * waiting for slack from the resource-blocked ones by means of the SsTCB.
 * Instead, a separate list was used.
 */
extern List_t xSsTaskBlockedList;

struct SsTCB
{
	SsTaskType_t xTaskType;

	/* Release count for this task. The release count is incremented each
	time the task is inserted in a ready list, after it has been blocked by
	an invocation to xTaskDelayUntil(). The flag uxDelayUntil is set to
	TRUE when xTaskDelayUntil() puts the task in the delayed list, and is set
	to FALSE when the task is unblocked and moved to the ready list. */
	UBaseType_t uxReleaseCount;
	UBaseType_t uxDelayUntil;

	TickType_t xPreviousWakeTime;
	TickType_t xTimeToWake;         /* Tick time at which the task wants to wake. */

	TickType_t xWcrt;				/* Worst case response time. */
	TickType_t xWcet;				/* Worst case execution time. */
	TickType_t xPeriod;			    /* Task period. */
	TickType_t xDeadline;		 	/* Task relative deadline. */
	TickType_t xA;					/* RTA3 */
	TickType_t xB;					/* RTA3 */

	ListItem_t xSsTaskListItem;    /* Item for the xSsTaskList list */
	ListItem_t xSsTaskBlockedListItem;
	ListItem_t xDeadlineTaskListItem;  /* Used to reference the task from the xDeadlineTaskList */

	volatile TickType_t xCur;   	/* Accumulated execution time measured in ticks. */

	BaseType_t xSlack;              /* Task slack */
	BaseType_t xSlackK;			    /* Task slack value at the critical instant. */

#if ( configUSE_SLACK_METHOD == 0 )
	TickType_t xTtma;               /* Maximally delayed completion time */
	TickType_t xDi;                 /* The absolute deadline of the next release */
#endif

	/* Stores the tick at which the task release ended. This a dirty
	way to know if we must decrement the available slack of the task
	at xTaskIncrementTick(). */
	TickType_t  xEndTick;

	BaseType_t xId;
};

typedef struct SsTCB SsTCB_t;

void vApplicationDeadlineMissedHook( char *pcTaskName, UBaseType_t uxRelease, TickType_t xTickCount );
void vApplicationNotSchedulable( void );

void vSlackSystemSetup( void );
void vSlackSchedulerSetup( void );

void vSlackSetTaskParams( TaskHandle_t xTask, const SsTaskType_t xTaskType, const TickType_t xPeriod, const TickType_t xDeadline, const TickType_t xWcet, const BaseType_t xId );
BaseType_t xSlackCalculateTasksWcrt( List_t * pxTasksList );

void vSlackUpdateAvailableSlack( volatile BaseType_t * xSlackSD, const List_t * pxTasksList );
void vSlackGainSlack( const TaskHandle_t xTask, const TickType_t xTicks, const List_t * pxTasksList );
void vSlackDecrementAllTasksSlack( const TickType_t xTicks, const TickType_t xTickCount, const List_t * pxTasksList );
void vSlackDecrementTasksSlack( TaskHandle_t pxTask, const TickType_t xTicks, const TickType_t xTickCount, const List_t * pxTasksList );

TickType_t xSlackGetWorkLoad( TaskHandle_t xTask, const TickType_t xTc, const List_t * pxTasksList );
void vTaskCalculateSlack( TaskHandle_t xTask, const TickType_t xTc, const List_t * pxTasksList );

#if ( configUSE_SLACK_METHOD == 0 )
BaseType_t prvTaskCalcSlack( const TaskHandle_t xTask, const TickType_t xTc, const TickType_t xT, const TickType_t xWc, const List_t * pxTasksList );
#endif

void vTasksGetSlacks( int32_t *taskSlackArray ) PRIVILEGED_FUNCTION;

#ifdef __cplusplus
}
#endif

#endif /* SLACK_H */
