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
#define U_CEIL( x, y )    			( ( x / y ) + ( x % y != 0 ) )
#define U_FLOOR( x, y )   			( x / y )
#define ONE_TICK 	      			( TickType_t ) 1U
/**
 * \brief Return a pointer to the `SsTCB` associated to the task.
 */
#define pvSlackGetTaskSsTCB( x ) 	( ( SsTCB_t * ) pvTaskGetThreadLocalStoragePointer( ( TaskHandle_t ) x, configSS_STORAGE_POINTER_INDEX ) )

#define SS_FIXED 0
#define SS_DAVIS 1

/*****************************************************************************
 * Public data declaration
 ****************************************************************************/
/**
 * Task types.
 */
typedef enum
{
	PERIODIC_TASK,
	APERIODIC_TASK
} SsTaskType_t;

/**
 * Extended Task Control Block
 */
struct SsTCB
{
	SsTaskType_t xTaskType;         /**< Task type (RTT o NRTT). */

	UBaseType_t uxReleaseCount;
	/**< Release counter of the task. The counter is incremented each time the
	 * task is inserted in a ready list, after it has been blocked by an
	 * invocation to xTaskDelayUntil(). */

	UBaseType_t uxDelayUntil;
	/**< Set to TRUE when xTaskDelayUntil() puts the task in the delayed list,
	 * set to FALSE when the task is unblocked and moved to the ready list. */

	TickType_t xPreviousWakeTime;
	/**< Holds the time at which the task was last unblocked. */

	TickType_t xTimeToWake;         /**< Time at which the task wants to wake. */

	TickType_t xWcet;               /**< Worst case execution time. */
	TickType_t xWcrt;				/**< Worst case response time. */
	TickType_t xPeriod;			    /**< Task period. */
	TickType_t xDeadline;		 	/**< Task relative deadline. */
	TickType_t xA;					/**< Used by RTA3. */
	TickType_t xB;					/**< Used by RTA3. */

	ListItem_t xSsTaskListItem;         /**< Task reference in \ref xSsTaskList . */
	ListItem_t xSsTaskBlockedListItem;  /**< Task reference in \ref xSlackDelayedTaskList . */
#if ( configSS_VERIFY_DEADLINE == 1 )
	ListItem_t xDeadlineTaskListItem;   /**< Task reference in \ref xDeadlineTaskList . */
#endif

	volatile TickType_t xCur;   	/**< Accumulated execution time. */

	BaseType_t xSlack;              /**< Task slack. */
	BaseType_t xSlackK;			    /**< Slack value at the critical instant. */

	TickType_t xTtma;               /**< Maximally delayed completion time. */
	TickType_t xDi;                 /**< Absolute deadline of the next release. */
};

typedef struct SsTCB SsTCB_t;

/*****************************************************************************
 * Public functions declaration
 ****************************************************************************/

/**
 * \brief Perform the deadline check of the RTTs.
 *
 * If a deadline miss is detected \ref vApplicationDeadlineMissedHook() is called.
 *
 * For a precise deadline verification this function should be called from the
 * tick hook.
 */
#if ( configSS_VERIFY_DEADLINE == 1 )
void vSlackDeadlineCheck( void );
#endif

/**
 * \brief Hook function called when a task miss its deadline.
 *
 * This application defined hook (callback) function is called when a task
 * deadline miss is detected.
 *
 * @param pcTaskName Name of the task.
 * @param xSsTCB Pointer to the \ref SsTCB of the task.
 * @param xTickCount Tick value at which the deadline was missed.
 */
#if ( configSS_VERIFY_DEADLINE == 1 )
void vApplicationDeadlineMissedHook( char *pcTaskName, const SsTCB_t *xSsTCB,
        TickType_t xTickCount );
#endif

/**
 * \brief Hook function called when the task group is not schedulable.
 *
 * This application defined hook (callback) function is called when the
 * schedulability analysis performed on the task set shows that is not
 * schedulable.
 */
#if ( configSS_VERIFY_SCHEDULABILITY == 1 )
void vApplicationNotSchedulable( void );
#endif

/**
 * \brief Set additional task parameters.
 *
 * This function set the task parameters required to calculate the slack.
 *
 * @param xTask Task handle provided by \ref xTaskCreate().
 * @param xTaskType Task type.
 * @param xPeriod Period of the task.
 * @param xDeadline Relative deadline of the task.
 * @param xWcet Worst case execution time of the task.
 */
void vSlackSetTaskParams( TaskHandle_t xTask, const SsTaskType_t xTaskType,
        const TickType_t xPeriod, const TickType_t xDeadline,
        const TickType_t xWcet );


/**
 * \brief Return the current system available slack.
 *
 * @return The systema available slack.
 */
TickType_t xSlackGetAvailableSlack();

/**
 * \brief Record the available slack of each task in \p pxArray.
 *
 * @param pxArray a pointer to an array where the counters are copied.
 */
void vTasksGetSlacks( int32_t *pxArray );

#ifdef __cplusplus
}
#endif

#endif /* SLACK_H */
