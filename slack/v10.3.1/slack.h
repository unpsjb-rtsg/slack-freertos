#ifndef SLACK_H
#define SLACK_H

#include "FreeRTOS.h"
#include "task.h"

#ifdef __cplusplus
extern "C" {
#endif

/*****************************************************************************
 * Macros and definitions
 ****************************************************************************/
#define U_CEIL( x, y )    ( ( x / y ) + ( x % y != 0 ) )
#define U_FLOOR( x, y )   ( x / y )
#define ONE_TICK 	      ( TickType_t ) 1U
#define getTaskSsTCB( x ) ( ( SsTCB_t * ) pvTaskGetThreadLocalStoragePointer( ( TaskHandle_t ) x, 0 ) )

#define SS_FIXED 0
#define SS_DAVIS 1

/*****************************************************************************
 * Public data declaration
 ****************************************************************************/
/* Task types. */
typedef enum
{
	PERIODIC_TASK,
	APERIODIC_TASK
} SsTaskType_t;

/*
 * The system available slack. It's the minimum value of all the task's slacks.
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
 * a resource -- with a timeout or for an unspecified amount of time. Although
 * that list could be used to store the slack-blocked tasks, identifying the
 * tasks waiting for slack from the resource-blocked ones by means of the SsTCB,
 * the use of a separate list was used instead.
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
	TickType_t xTimeToWake;         /* Time at which the task wants to wake. */

	TickType_t xWcrt;				/* Worst case response time. */
	TickType_t xWcet;				/* Worst case execution time. */
	TickType_t xPeriod;			    /* Task period. */
	TickType_t xDeadline;		 	/* Task relative deadline. */
	TickType_t xA;					/* RTA3 */
	TickType_t xB;					/* RTA3 */

	ListItem_t xSsTaskListItem;    /* Item for the xSsTaskList list */
	ListItem_t xSsTaskBlockedListItem;
	ListItem_t xDeadlineTaskListItem;  /* Task reference in xDeadlineTaskList */

	volatile TickType_t xCur;   	/* Accumulated execution. */

	BaseType_t xSlack;              /* Task slack */
	BaseType_t xSlackK;			    /* Slack value at the critical instant. */

#if ( configUSE_SLACK_METHOD == 0 )
	TickType_t xTtma;               /* Maximally delayed completion time */
	TickType_t xDi;                 /* Absolute deadline of the next release */
#endif

	/* Stores the tick at which the task release ended. This a dirty
	way to know if we must decrement the available slack of the task
	at xTaskIncrementTick(). */
	TickType_t  xEndTick;

	BaseType_t xId;
};

typedef struct SsTCB SsTCB_t;

/*****************************************************************************
 * Public functions declaration
 ****************************************************************************/

/**
 * Application defined hook (or callback) function called when a tasks miss its
 * deadline.
 *
 * @param pcTaskName
 * @param xSsTCB
 * @param xTickCount
 */
void vApplicationDeadlineMissedHook( char *pcTaskName, const SsTCB_t *xSsTCB,
        TickType_t xTickCount );

/**
 * Application defined hook (or callback) function called when the task group
 * is not schedulable.
 */
void vApplicationNotSchedulable( void );

/**
 * Setup the required tasks lists. This function must be called before setting
 * the tasks parameters with vSlackSetTaskParams().
 */
void vSlackSystemSetup( void );

/**
 * Perform the initialization steps required before the FreeRTOS scheduler is
 * started. Run the schedulability test and the initial slack calcualtions.
 */
void vSlackSchedulerSetup( void );

/**
 * Set the task parameters.
 *
 * @param xTask
 * @param xTaskType
 * @param xPeriod
 * @param xDeadline
 * @param xWcet
 * @param xId
 */
void vSlackSetTaskParams( TaskHandle_t xTask, const SsTaskType_t xTaskType,
        const TickType_t xPeriod, const TickType_t xDeadline,
        const TickType_t xWcet, const BaseType_t xId );

/**
 *
 * @param pxTasksList
 * @return
 */
BaseType_t xSlackCalculateTasksWcrt( List_t * pxTasksList );

/**
 *
 * @param xSlackSD
 * @param pxTasksList
 */
void vSlackUpdateAvailableSlack( volatile BaseType_t * xSlackSD,
        const List_t * pxTasksList );

/**
 *
 * @param xTask
 * @param xTicks
 * @param pxTasksList
 */
void vSlackGainSlack( const TaskHandle_t xTask, const TickType_t xTicks,
        const List_t * pxTasksList );

/**
 *
 * @param xTicks
 * @param xTickCount
 * @param pxTasksList
 */
void vSlackDecrementAllTasksSlack( const TickType_t xTicks,
        const TickType_t xTickCount, const List_t * pxTasksList );

/**
 *
 * @param pxTask
 * @param xTicks
 * @param xTickCount
 * @param pxTasksList
 */
void vSlackDecrementTasksSlack( TaskHandle_t pxTask, const TickType_t xTicks,
        const TickType_t xTickCount, const List_t * pxTasksList );

/**
 *
 * @param xTask
 * @param xTc
 * @param pxTasksList
 * @return
 */
TickType_t xSlackGetWorkLoad( TaskHandle_t xTask, const TickType_t xTc,
        const List_t * pxTasksList );

/**
 *
 * @param xTask
 * @param xTc
 * @param pxTasksList
 */
void vTaskCalculateSlack( TaskHandle_t xTask, const TickType_t xTc,
        const List_t * pxTasksList );

/**
 *
 * @param taskSlackArray
 */
void vTasksGetSlacks( int32_t *taskSlackArray );

#ifdef __cplusplus
}
#endif

#endif /* SLACK_H */
