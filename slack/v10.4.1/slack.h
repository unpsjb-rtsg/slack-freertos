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
/** Task types. */
typedef enum
{
	PERIODIC_TASK,
	APERIODIC_TASK
} SsTaskType_t;

/**
 * \brief The system available slack.
 *
 * The system available slack is the minimum value of all the task's slacks.
 */
extern volatile BaseType_t xSlackSD;

/**
 * \brief Task's deadlines list.
 *
 * This list contains references to all the ready tasks, ordered by their
 * absolute deadlines.
 */
extern List_t xDeadlineTaskList;

/**
 * \brief List containing all the RTT of the system.
 *
 * This list contains references to all the tasks that account for the
 * available slack of the system. Tasks at the idle priority level are not
 * accounted.
 */
extern List_t xSsTaskList;

/**
 * \brief List of tasks blocked by insufficient available slack.
 *
 * This list stores tasks that have been blocked by insufficient available
 * slack. They need to be stored in a separated list because we don't know
 * in advance when there is enough available slack.
 *
 * The blocked (delayed) list of FreeRTOS stores real-time tasks that are
 * blocked in waiting of a resource, with a timeout or for an unspecified
 * amount of time. Although that list could be used to store the slack-blocked
 * tasks, identifying the tasks waiting for slack from the resource-blocked ones
 * by means of the \ref SsTCB could be time consuming.
 */
extern List_t xSsTaskBlockedList;

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

	TickType_t xWcrt;				/**< Worst case response time. */
	TickType_t xWcet;				/**< Worst case execution time. */
	TickType_t xPeriod;			    /**< Task period. */
	TickType_t xDeadline;		 	/**< Task relative deadline. */
	TickType_t xA;					/**< Used by RTA3. */
	TickType_t xB;					/**< Used by RTA3. */

	ListItem_t xSsTaskListItem;         /**< Task reference in \ref xSsTaskList . */
	ListItem_t xSsTaskBlockedListItem;  /**< Task reference in \ref xSlackDelayedTaskList . */
	ListItem_t xDeadlineTaskListItem;   /**< Task reference in \ref xDeadlineTaskList . */

	volatile TickType_t xCur;   	/**< Accumulated execution time. */

	BaseType_t xSlack;              /**< Task slack. */
	BaseType_t xSlackK;			    /**< Slack value at the critical instant. */

#if ( configUSE_SLACK_METHOD == 0 )
	TickType_t xTtma;               /**< Maximally delayed completion time. */
	TickType_t xDi;                 /**< Absolute deadline of the next release. */
#endif

	TickType_t  xEndTick;
	/**< Stores the tick at which the task release ended. This a dirty way to
	 * know if we must decrement the available slack of the task at xTaskIncrementTick(). */

	BaseType_t xId;                 /**< Task id. */
};

typedef struct SsTCB SsTCB_t;

/*****************************************************************************
 * Public functions declaration
 ****************************************************************************/

/**
 * \brief Hook function called when a task miss its deadline.
 *
 * This application defined hook (callback) function is called whenever a task
 * miss its deadline.
 *
 * @param pcTaskName Name of the task.
 * @param xSsTCB Pointer to the \ref SsTCB of the task.
 * @param xTickCount Tick value at which the deadline was missed.
 */
void vApplicationDeadlineMissedHook( char *pcTaskName, const SsTCB_t *xSsTCB,
        TickType_t xTickCount );

/**
 * \brief Hook function called when the task group is not schedulable.
 *
 * This application defined hook (callback) function is called when the
 * schedulability analysis performed on the task set shows that is not
 * schedulable.
 */
void vApplicationNotSchedulable( void );

/**
 * \brief Perform the setup of the required tasks lists.
 *
 * This function must be called **before** setting the tasks parameters with
 * \ref vSlackSetTaskParams().
 */
void vSlackSystemSetup( void );

/**
 * \brief Perform the initialization steps required before the scheduler starts.
 *
 * This function perform the initialization required before the FreeRTOS scheduler
 * is started: executes the schedulability test and the initial slack calculations.
 *
 * This function must be called **before** \ref vTaskStartScheduler().
 */
void vSlackSchedulerSetup( void );

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
 * @param xId A numerical identifier.
 */
void vSlackSetTaskParams( TaskHandle_t xTask, const SsTaskType_t xTaskType,
        const TickType_t xPeriod, const TickType_t xDeadline,
        const TickType_t xWcet, const BaseType_t xId );

/**
 * \brief Calculates the worst case response time of the RTT tasks.
 *
 * It uses the algorithm described in [Improved Response-Time Analysis
 * Calculations](http://doi.ieeecomputersociety.org/10.1109/REAL.1998.739773).
 *
 * @return pdTRUE if the task set is schedulable or pdFALSE if it's not.
 */
BaseType_t xSlackCalculateTasksWcrt();

/**
 * \brief Updates the system available slack.
 *
 * The system available slack is calculated as the minimum value of all the
 * tasks available slacks.
 *
 * @param xSlackSD Pointer to \ref xSlackSD.
 * @param pxTasksList List of tasks. It should be a pointer to \ref xSsTaskList.
 */
void vSlackUpdateAvailableSlack( volatile BaseType_t * xSlackSD,
        const List_t * pxTasksList );

/**
 * \brief Add \p xTicks to all lower priority tasks than \p xTask .
 *
 * @param xTask Task which has gained slack.
 * @param xTicks Amount of ticks to add to the slack counters.
 * @param pxTasksList List of tasks. It should be a pointer to \ref xSsTaskList.
 */
void vSlackGainSlack( const TaskHandle_t xTask, const TickType_t xTicks,
        const List_t * pxTasksList );

/**
 * \brief Decrement the slack counter of all the tasks.
 *
 * Subtract the specified \p xTicks amount from the slack counters of all the tasks.
 * The available slack of a task is stored in \ref SsTCB.xSlack.
 *
 * @param xTicks Amount of ticks to subtract from the available slack of the tasks.
 * @param pxTasksList The list of tasks. It should be a pointer to \ref xSsTaskList.
 */
void vSlackDecrementAllTasksSlack( const TickType_t xTicks,
        const List_t * pxTasksList );

/**
 * \brief Reduce \p xTicks to all higher priority tasks than \p pxTask.
 *
 * Substract the specified \p xTicks amount from the slack counters of higher priority tasks than \p xTask.
 *
 * @param pxTask The task currently running.
 * @param xTicks Amount of ticks to subtract from the slack counters of higher priority tasks.
 * @param pxTasksList List of tasks. It should be a pointer to \ref xSsTaskList.
 */
void vSlackDecrementTasksSlack( TaskHandle_t xTask, const TickType_t xTicks,
        const List_t * pxTasksList );

/**
 * \brief Calculates the system workload at the instant \p xTc
 *
 * This utility function calculates the system workload at the time \p xTc, for
 * the subsystem composed of \p xTask and all the higher priority tasks. The
 * result is used in the calculation of the available slack of \p xTask.
 *
 * @param xTask The task that defines the subsystem.
 * @param xTc The time at which the workload should be calculated.
 * @param pxTasksList List of tasks. It should be a pointer to \ref xSsTaskList.
 * @return The system workload at the instant \p xTc.
 */
TickType_t xSlackGetWorkLoad( TaskHandle_t xTask, const TickType_t xTc,
        const List_t * pxTasksList );

/**
 * \brief Calculates the available slack of task \p xTask at \p xTc .
 *
 * This is a wrapper function that call the appropiate slack method.
 *
 * @param xTask The task which available slack should be calculated.
 * @param xTc The time at which the slack calculation should be done.
 * @param pxTasksList List of tasks. It should be a pointer to \ref xSsTaskList.
 */
void vTaskCalculateSlack( TaskHandle_t xTask, const TickType_t xTc,
        const List_t * pxTasksList );

/**
 * \brief Return the system available slack.
 *
 * @return The system available slack.
 */
TickType_t xSlackGetAvailableSlack( void );

/**
 *
 * @param taskSlackArray
 */
void vTasksGetSlacks( int32_t *taskSlackArray );

#ifdef __cplusplus
}
#endif

#endif /* SLACK_H */
