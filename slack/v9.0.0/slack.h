#ifndef SLACK_H
#define SLACK_H

#include "task.h"

#ifdef __cplusplus
extern "C" {
#endif

#if ( configUSE_SLACK_STEALING == 1 )

struct SsTCB
{
	/* Release count for this task. The release count is incremented each
	time the task is inserted in a ready list, after it has been blocked by
	an invocation to xTaskDelayUntil(). The flag uxDelayUntil is set to
	TRUE when xTaskDelayUntil() puts the task in the delayed list, and is set
	to FALSE when the task is unblocked and moved to the ready list. */
	UBaseType_t uxReleaseCount;
	UBaseType_t uxDelayUntil;

	TickType_t  xWcrt;				/* Worst case response time. */
	TickType_t  xWcet;				/* Worst case execution time. */
	TickType_t  xPeriod;			/* Task period. */
	TickType_t  xDeadline;			/* Task relative deadline. */
	TickType_t 	xA;					/* RTA3 */
	TickType_t 	xB;					/* RTA3 */

	ListItem_t  xSsTaskListItem;    /* Item for the xSsTaskList list */
	ListItem_t  xSsTaskBlockedListItem;
	ListItem_t 	xDeadlineTaskListItem;  /* Used to reference the task from the xDeadlineTaskList */

	TickType_t	xCur; 				/* Accumulated execution time measured in ticks. */

	BaseType_t  xSlack;             /* Task slack */
	BaseType_t	xSlackK;			/* Task slack value at the critical instant. */

#if ( configUSE_SLACK_METHOD == 0 )
	/* Used by Fast Slack Stealing method. */
	TickType_t  xTtma;              /* Maximally delayed completion time */
	TickType_t  xDi;                /* The absolute deadline of the next release */
#endif

	/* Stores the tick at which the task release ended. This a dirty
	way to know if we must decrement the available slack of the task
	at xTaskIncrementTick(). */
	TickType_t  xEndTick;

	BaseType_t xId;
};

typedef struct SsTCB SsTCB_t;

void vTaskSetParams( TaskHandle_t xTask, const TickType_t xPeriod, const TickType_t xDeadline, const TickType_t xWcet, const BaseType_t xId ) PRIVILEGED_FUNCTION;
void vTasksGetSlacks( int32_t *taskSlackArray ) PRIVILEGED_FUNCTION;

#if ( configDO_SLACK_TRACE == 1)
    extern xType *xResults;
    extern int xRecSlackIdx;
#endif

#if ( configKERNEL_TEST == 2 ) || ( configKERNEL_TEST == 3 ) || ( configKERNEL_TEST == 4 )
    extern xType *cs_costs;
#endif

#endif /* configUSE_SLACK_STEALING */

#if ( configUSE_SLACK_STEALING == 0 ) && ( configKERNEL_TEST == 1 )
    /* Set the Id */
    void vTaskSetParams( TaskHandle_t xTask, const BaseType_t xId ) PRIVILEGED_FUNCTION;
#endif

#ifdef __cplusplus
}
#endif

#endif /* SLACK_H */
