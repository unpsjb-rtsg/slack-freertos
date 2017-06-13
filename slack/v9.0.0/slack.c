#include "FreeRTOS.h"
#include "task.h"
#include "slack.h"

#if ( configKERNEL_TEST > 0 )
#include "slack_tests.h"
#if ( configKERNEL_TEST == 2 )
static BaseType_t xCeilFloorCost = 0;
#endif
#if ( configKERNEL_TEST == 4 )
static BaseType_t xLoopCost = 0;
#endif
#endif

#if ( configUSE_SLACK_METHOD == 0 )
static inline void prvTaskCalculateSlack_fixed1( TaskHandle_t  xTask, const TickType_t xTc, const List_t * pxTasksList ) __attribute__((always_inline));
#endif
#if ( configUSE_SLACK_METHOD == 1 )
static inline void prvTaskCalculateSlack_davis1( TaskHandle_t xTask, const TickType_t xTc, const List_t * pxTasksList ) __attribute__((always_inline));
#endif

volatile BaseType_t xSlackSD;

List_t xDeadlineTaskList;

List_t xSsTaskList;

List_t xSsTaskBlockedList;

void vSlackSetTaskParams( TaskHandle_t xTask, const SsTaskType_t xTaskType, const TickType_t xPeriod, const TickType_t xDeadline, const TickType_t xWcet, const BaseType_t xId )
{
	UBaseType_t uxTaskPriority = uxTaskPriorityGet( xTask );
	SsTCB_t * pxNewSsTCB = pvPortMalloc( sizeof( SsTCB_t ) );

	if( ( uxTaskPriority == tskIDLE_PRIORITY ) || ( uxTaskPriority == configMAX_PRIORITIES - 1 ) )
	{
		// error?
	}

	pxNewSsTCB->xTaskType = xTaskType;

	pxNewSsTCB->xPeriod = xPeriod;
	pxNewSsTCB->xDeadline = xDeadline;
	pxNewSsTCB->xWcet = xWcet;
	pxNewSsTCB->xA = xWcet;
	pxNewSsTCB->xB = xPeriod;
	pxNewSsTCB->xId = xId;

	pxNewSsTCB->uxReleaseCount = 1U;
	pxNewSsTCB->xPreviousWakeTime = ( TickType_t ) 0U;
	pxNewSsTCB->xTimeToWake = ( TickType_t ) 0U;
	pxNewSsTCB->xWcrt = 0U;
	pxNewSsTCB->xEndTick = ( TickType_t ) 0U;
	pxNewSsTCB->xSlack = 0U;
#if ( configUSE_SLACK_METHOD == 0 )
	pxNewSsTCB->xTtma = 0U;
	pxNewSsTCB->xDi = 0U;
#endif
	pxNewSsTCB->xCur = ( TickType_t ) 0U;

	if( xTaskType == PERIODIC_TASK )
	{
		vListInitialiseItem( &( pxNewSsTCB->xSsTaskListItem ) );
		listSET_LIST_ITEM_VALUE( &( pxNewSsTCB->xSsTaskListItem ), ( TickType_t ) configMAX_PRIORITIES - ( TickType_t ) uxTaskPriority );
		listSET_LIST_ITEM_OWNER( &( pxNewSsTCB->xSsTaskListItem ), xTask );
		vListInsert( &xSsTaskList, &( ( pxNewSsTCB )->xSsTaskListItem ) );

		vListInitialiseItem( &( pxNewSsTCB->xDeadlineTaskListItem ) );
		listSET_LIST_ITEM_OWNER( &( pxNewSsTCB->xDeadlineTaskListItem ), xTask );
		listSET_LIST_ITEM_VALUE( &( pxNewSsTCB->xDeadlineTaskListItem ), pxNewSsTCB->xDeadline );
		/* The list item value of xDeadlineTaskListItem is updated when the
			task is moved into the ready list. */
	}

	if( xTaskType == APERIODIC_TASK )
	{
		vListInitialiseItem( &( pxNewSsTCB->xSsTaskBlockedListItem ) );
		listSET_LIST_ITEM_OWNER( &( pxNewSsTCB->xSsTaskBlockedListItem ), xTask );
		listSET_LIST_ITEM_VALUE( &( pxNewSsTCB->xSsTaskBlockedListItem ), 0 );
	}

	vTaskSetThreadLocalStoragePointer( xTask, 0, ( void * ) pxNewSsTCB );
}
/*-----------------------------------------------------------*/

void vSlackSystemSetup( void )
{
    vListInitialise( &xSsTaskList );
    vListInitialise( &xDeadlineTaskList );
    vListInitialise( &xSsTaskBlockedList );
}
/*-----------------------------------------------------------*/

void vSlackSchedulerSetup( void )
{
	xSlackSD = 0;

	/* Calculate worst case execution times of tasks. */
    BaseType_t xSchedulable = xSlackCalculateTasksWcrt( &xSsTaskList );

    if( xSchedulable == pdFALSE )
    {
        vApplicationNotSchedulable();
    }

    ListItem_t *pxTaskListItem = listGET_HEAD_ENTRY( &xSsTaskList );

    /* Calculate slacks at xTickCount = 0 */
    while( listGET_END_MARKER( &( xSsTaskList ) ) != pxTaskListItem )
    {
    	TaskHandle_t xTask = ( TaskHandle_t ) listGET_LIST_ITEM_OWNER( pxTaskListItem );
    	SsTCB_t *pxTaskSs = getTaskSsTCB( xTask );

    	vTaskCalculateSlack( xTask, (TickType_t) 0U, &xSsTaskList );
    	pxTaskSs->xSlackK = pxTaskSs->xSlack;

    	/* Deadline */
    	UBaseType_t uxTaskPriority = uxTaskPriorityGet( xTask );
    	if( uxTaskPriority != tskIDLE_PRIORITY )
    	{
    		listSET_LIST_ITEM_VALUE( &( ( pxTaskSs )->xDeadlineTaskListItem ), pxTaskSs->xDeadline );
    		vListInsert( &xDeadlineTaskList, &( ( pxTaskSs )->xDeadlineTaskListItem ) );
    	}

    	pxTaskListItem = listGET_NEXT( pxTaskListItem );
    }

    vSlackUpdateAvailableSlack( &xSlackSD, &xSsTaskList );
}
/*-----------------------------------------------------------*/

/**
 * RTA - Worst Case Response Time calculation.
 * "Improved Response-Time Analysis Calculations"
 * http://doi.ieeecomputersociety.org/10.1109/REAL.1998.739773
 */
BaseType_t xSlackCalculateTasksWcrt( List_t * pxTasksList )
{
	TickType_t xW = 0U;

	ListItem_t *pxTaskListItem = listGET_HEAD_ENTRY( pxTasksList );

	TaskHandle_t xTask = ( TaskHandle_t ) listGET_LIST_ITEM_OWNER( pxTaskListItem );
	SsTCB_t *pxTask = pvTaskGetThreadLocalStoragePointer( xTask, 0 );

	/* First task WCRT. */
	TickType_t xT = pxTask->xWcet;
	pxTask->xWcrt = xT;

	/* Check first task deadline. */
	if( pxTask->xWcrt > pxTask->xPeriod )
	{
		return pdFALSE;
	}

	// Next task
	pxTaskListItem = listGET_NEXT( pxTaskListItem );

	/* Process all the periodic tasks in xTasks. */
	while( listGET_END_MARKER( pxTasksList ) != pxTaskListItem )
	{
		xTask = ( TaskHandle_t ) listGET_LIST_ITEM_OWNER( pxTaskListItem );
		pxTask = pvTaskGetThreadLocalStoragePointer( xTask, 0 );

		xT = xT + pxTask->xWcet;

		while( xT <= pxTask->xDeadline )
		{
			xW = 0;

			/* Calculates the workload of the higher priority tasks than pxTask. */
			ListItem_t * pxHigherPrioTaskListItem = listGET_HEAD_ENTRY( pxTasksList );
			do
			{
				SsTCB_t *pxHigherPrioTask = pvTaskGetThreadLocalStoragePointer( ( TaskHandle_t ) listGET_LIST_ITEM_OWNER( pxHigherPrioTaskListItem ), 0 );

				xW = xW + ( U_CEIL( xT, pxHigherPrioTask->xPeriod ) * pxHigherPrioTask->xWcet );

				pxHigherPrioTaskListItem = listGET_NEXT( pxHigherPrioTaskListItem );
			}
			while( pxHigherPrioTaskListItem != pxTaskListItem );

			xW = xW + pxTask->xWcet;

			if( xT == xW )
			{
				break;
			}
			else
			{
				xT = xW;
			}
		}

		if( xT > pxTask->xDeadline )
		{
			return pdFALSE;
		}

		pxTask->xWcrt = xT;

		pxTaskListItem = listGET_NEXT( pxTaskListItem );
	}

	return pdTRUE;
}
/*-----------------------------------------------------------*/

inline void vSlackUpdateAvailableSlack( volatile BaseType_t * xSlackSD, const List_t * pxTasksList )
{
	ListItem_t * pxAppTasksListItem = listGET_HEAD_ENTRY( pxTasksList );

	SsTCB_t * ssTCB = getTaskSsTCB( listGET_LIST_ITEM_OWNER( pxAppTasksListItem ) );
	*xSlackSD = ssTCB->xSlack;

	while( listGET_END_MARKER( pxTasksList ) != pxAppTasksListItem )
	{
		ssTCB = getTaskSsTCB( listGET_LIST_ITEM_OWNER( pxAppTasksListItem ) );

		if( ssTCB->xSlack < *xSlackSD )
		{
			*xSlackSD = ssTCB->xSlack;
		}

		pxAppTasksListItem = listGET_NEXT( pxAppTasksListItem );
	}
}
/*-----------------------------------------------------------*/

inline void vSlackGainSlack( const TaskHandle_t xTask, const TickType_t xTicks, const List_t * pxTasksList )
{
    SsTCB_t * ssTCB = getTaskSsTCB( xTask );
    ListItem_t * pxAppTasksListItem = listGET_NEXT( &( ssTCB->xSsTaskListItem ) );

    while( listGET_END_MARKER( pxTasksList ) != pxAppTasksListItem )
    {
        ssTCB = getTaskSsTCB( listGET_LIST_ITEM_OWNER( pxAppTasksListItem ) );
        ssTCB->xSlack += ( BaseType_t ) xTicks;

        pxAppTasksListItem = listGET_NEXT( pxAppTasksListItem );
    }
}
/*-----------------------------------------------------------*/

inline void vSlackDecrementAllTasksSlack( const TickType_t xTicks, const TickType_t xTickCount, const List_t * pxTasksList )
{
	ListItem_t * pxAppTasksListItem = listGET_HEAD_ENTRY( pxTasksList );

	while( listGET_END_MARKER( pxTasksList ) != pxAppTasksListItem )
	{
		SsTCB_t * xTask = getTaskSsTCB( listGET_LIST_ITEM_OWNER( pxAppTasksListItem ) );

		if( xTask->xSlack > 0 )
		{
			if( xTask->xEndTick < ( xTickCount - ( TickType_t ) 1U ) )
			{
				xTask->xSlack = xTask->xSlack - ( BaseType_t ) xTicks;
			}
		}

		pxAppTasksListItem = listGET_NEXT( pxAppTasksListItem );
	}
}
/*-----------------------------------------------------------*/

inline void vSlackDecrementTasksSlack( TaskHandle_t pxTask, const TickType_t xTicks, const TickType_t xTickCount, const List_t * pxTasksList )
{
	const ListItem_t * pxAppTasksListEndMarker = &( getTaskSsTCB( pxTask )->xSsTaskListItem );
    ListItem_t * pxAppTasksListItem = listGET_HEAD_ENTRY( pxTasksList );

    while( pxAppTasksListEndMarker != pxAppTasksListItem )
    {
        SsTCB_t * xTask = getTaskSsTCB( listGET_LIST_ITEM_OWNER( pxAppTasksListItem ) );

        if( xTask->xSlack > 0 )
        {
            if( xTask->xEndTick < ( xTickCount - ( TickType_t ) 1U ) )
            {
               xTask->xSlack = xTask->xSlack - ( BaseType_t ) xTicks;
            }
        }

        pxAppTasksListItem = listGET_NEXT( pxAppTasksListItem );
    }
}
/*-----------------------------------------------------------*/

inline TickType_t xSlackGetWorkLoad( TaskHandle_t xTask, const TickType_t xTc, const List_t * pxTasksList )
{
	ListItem_t *pxTaskListItem = &( getTaskSsTCB( xTask ) )->xSsTaskListItem;

	TickType_t xW = ( TickType_t ) 0U;	// Workload
	TickType_t xA = ( TickType_t ) 0U;
	TickType_t xC = ( TickType_t ) 0U;

	// Until we process all the maximum priority tasks (including pxTask)
	while( listGET_END_MARKER( pxTasksList ) != pxTaskListItem )
	{
#if ( configKERNEL_TEST == 4 )
		xLoopCost = xLoopCost + 1;
#endif

		SsTCB_t *pxTask = getTaskSsTCB( listGET_LIST_ITEM_OWNER( pxTaskListItem ) );

		// The number of instances of pxHigherPrioTask in [0, xT)
		xA = U_FLOOR( xTc, pxTask->xPeriod );

#if ( configKERNEL_TEST == 2 )
		xCeilFloorCost = xCeilFloorCost + 1;
#endif

		if( xTc > ( TickType_t ) 0U )
		{
			if( pxTask->xWcet == pxTask->xCur )
			{
				if( xA >= pxTask->uxReleaseCount )
				{
					xC = ( TickType_t ) 0U;
				}
				else
				{
					xC = pxTask->xWcet;
				}
			}
			else
			{
				xC = pxTask->xWcet;
			}
		}

		// Accumulated workload
		xW = xW + ( xA * pxTask->xWcet ) + xC;

		pxTaskListItem = pxTaskListItem->pxPrevious;
	}

	return xW;
}
/*-----------------------------------------------------------*/

#if ( configUSE_SLACK_METHOD == 0 )
inline BaseType_t prvTaskCalcSlack( const TaskHandle_t xTask, const TickType_t xTc, const TickType_t xT, const TickType_t xWc, const List_t * pxTasksList )
{
    ListItem_t *pxTaskListItem = &( getTaskSsTCB( xTask )->xSsTaskListItem );
    TickType_t xW = 0;

    // process all the maximum priority tasks
    while( listGET_END_MARKER( pxTasksList ) != pxTaskListItem )
    {
        #if ( configKERNEL_TEST == 4 )
            xLoopCost = xLoopCost + 1;
        #endif

        SsTCB_t * pxTask = getTaskSsTCB( listGET_LIST_ITEM_OWNER( pxTaskListItem ) );
    	// accumulated workload of higher priority tasks in [0, xT)
    	xW = xW + ( U_CEIL( xT, pxTask->xPeriod ) * pxTask->xWcet );

        #if ( configKERNEL_TEST == 2 )
            xCeilFloorCost = xCeilFloorCost + 1;
        #endif

    	// get next higher priority task list item
        pxTaskListItem = ( ( pxTaskListItem )->pxPrevious );
    }

    return ( BaseType_t ) xT - ( BaseType_t ) xTc - ( BaseType_t ) xW + ( BaseType_t ) xWc;
}
#endif
/*-----------------------------------------------------------*/

void vTaskCalculateSlack( TaskHandle_t xTask, const TickType_t xTc, const List_t * pxTasksList )
{
#if ( configKERNEL_TEST == 2 )
	xCeilFloorCost = 0;
#endif
#if ( configKERNEL_TEST == 3 )
	STOPWATCH_RESET();
#endif
#if ( configKERNEL_TEST == 4 )
	xLoopCost = 0;
#endif

#if ( configUSE_SLACK_METHOD == 0 )
	prvTaskCalculateSlack_fixed1( xTask, xTc, pxTasksList );
#endif
#if ( configUSE_SLACK_METHOD == 1 )
	prvTaskCalculateSlack_davis1( xTask, xTc, pxTasksList );
#endif

#if ( configKERNEL_TEST == 2 )
	if (xTc > 0)
	{
		vTaskGetTraceInfo( xTask, xCeilFloorCost );
	}
#endif
#if ( configKERNEL_TEST == 3 )
	uint32_t cycles = CPU_CYCLES;
	if (xTc > 0)
	{
		vTaskGetTraceInfo( xTask, cycles );
	}
#endif
#if ( configKERNEL_TEST == 4 )
    if (xTc > 0)
    {
    	vTaskGetTraceInfo( xTask, xLoopCost );
    }
#endif

}
/*-----------------------------------------------------------*/

#if ( configUSE_SLACK_METHOD == 0 )
static inline void prvTaskCalculateSlack_fixed1( TaskHandle_t xTask, const TickType_t xTc, const List_t * pxTasksList )
{
    SsTCB_t * pxTask = getTaskSsTCB( xTask );

    ListItem_t *pxTaskListItem = &( pxTask->xSsTaskListItem );
    ListItem_t *pxHigherPrioTaskListItem = ( ( pxTaskListItem )->pxPrevious );

    TickType_t xXi = ( TickType_t ) 0U;
    TickType_t xDi = pxTask->xDeadline;
    if ( xTc > ( TickType_t ) 0U )
    {
    	// xTimeToWake has the instant in which the task should be removed
    	// from the blocked list.
    	xXi = pxTask->xTimeToWake;
    	xDi = xXi + pxTask->xDeadline;
    }

    pxTask->xDi = xDi;

    // if xTask is the highest priority task
    if( listGET_END_MARKER( pxTasksList ) == pxHigherPrioTaskListItem )
    {
        pxTask->xSlack = xDi - xTc - pxTask->xWcet;
        pxTask->xTtma = xDi;
        return;
    }

    BaseType_t xKmax = 0U;
    BaseType_t xTmax = portMAX_DELAY;

    // TCB of the higher priority task.
    TaskHandle_t * pxHigherPrioTaskTCB = ( TaskHandle_t ) listGET_LIST_ITEM_OWNER( pxHigherPrioTaskListItem );
    SsTCB_t * pxHigherPrioTask = getTaskSsTCB( pxHigherPrioTaskTCB );

    // Corollary 2 (follows theorem 5)
    if ( ( pxHigherPrioTask->xDi + pxHigherPrioTask->xWcet >= xDi ) && ( xDi >= pxHigherPrioTask->xTtma ) )
    {
        pxTask->xSlack = pxHigherPrioTask->xSlack - pxTask->xWcet;
        pxTask->xTtma = pxHigherPrioTask->xTtma;
        return;
    }

    // Theorem 3
    TickType_t xIntervalo = xXi + pxTask->xDeadline - pxTask->xWcrt + pxTask->xWcet;

    // Corollary 1 (follows theorem 4)
    if ( ( pxHigherPrioTask->xDi + pxHigherPrioTask->xWcet >= xIntervalo ) && ( pxHigherPrioTask->xDi + pxHigherPrioTask->xWcet <= xDi ) )
    {
        xIntervalo = pxHigherPrioTask->xDi + pxHigherPrioTask->xWcet;
        xKmax = pxHigherPrioTask->xSlack - pxTask->xWcet;
        xTmax = pxHigherPrioTask->xTtma;
    }

    TickType_t xWc = xSlackGetWorkLoad( xTask, xTc, pxTasksList );

    // Calculate slack at xTask deadline (xDi)
    BaseType_t xK2 = prvTaskCalcSlack( xTask, xTc, xDi, xWc, pxTasksList );

    if ( xK2 >= xKmax )
    {
        if ( xK2 == xKmax )
        {
            if ( xTmax > xDi )
            {
                xTmax = xDi;
            }
        }
        else
        {
            xTmax = xDi;
        }
        xKmax = xK2;
    }

    TickType_t xii;

    // Find the slack in [intervalo, xDi)
    do
    {
        #if ( configKERNEL_TEST == 4 )
        xLoopCost = xLoopCost + 1;
        #endif

        pxHigherPrioTaskTCB = ( TaskHandle_t ) listGET_LIST_ITEM_OWNER( pxHigherPrioTaskListItem );
        pxHigherPrioTask = getTaskSsTCB( pxHigherPrioTaskTCB );

        xii = U_CEIL( xIntervalo, pxHigherPrioTask->xPeriod ) * pxHigherPrioTask->xPeriod;

        #if ( configKERNEL_TEST == 2 )
        xCeilFloorCost = xCeilFloorCost + 1;
        #endif

        while( xii < xDi )
        {
            #if ( configKERNEL_TEST == 4 )
            xLoopCost = xLoopCost + 1;
            #endif

            xK2 = prvTaskCalcSlack( xTask, xTc, xii, xWc, pxTasksList );

            if( xK2 > xKmax )
            {
                xKmax = xK2;
                xTmax = xii;
            }
            else if( ( xK2 == xKmax ) && ( xii < xTmax ) )
            {
                xTmax = xii;
            }

            xii = xii + pxHigherPrioTask->xPeriod;
        }

        // Get the next higher priority task
        pxHigherPrioTaskListItem = pxHigherPrioTaskListItem->pxPrevious;
    }
    while ( listGET_END_MARKER( pxTasksList ) != pxHigherPrioTaskListItem );

    pxTask->xSlack = xKmax;
    pxTask->xTtma = xTmax;
}
#endif /* configUSE_SLACK_METHOD == 0 */
/*-----------------------------------------------------------*/

#if ( configUSE_SLACK_METHOD == 1 )
/* from "Scheduling Slack Time on Fixed Priority Pre-emptive Systems" paper */
static inline void prvTaskCalculateSlack_davis1( TaskHandle_t xTask, const TickType_t xTc, const List_t * pxTasksList )
{
	SsTCB_t *pxTask = getTaskSsTCB( xTask );

	TaskHandle_t *pxHigherPrioTaskTCB = NULL;
	SsTCB_t *pxHigherPrioTask = NULL;

	ListItem_t *pxTaskListItem = &( pxTask ->xSsTaskListItem );
	ListItem_t *pxHigherPrioTaskListItem = pxTaskListItem->pxPrevious;

	TickType_t xS = ( TickType_t ) 0U; // amount of slack which may be stolen
	TickType_t xW = ( TickType_t ) 0U; // busy period

	TickType_t xD = pxTask->xDeadline;
	if ( xTc > ( TickType_t ) 0U )
	{
		// xTimeToWake has the instant in which the task should be removed from
		// the blocked list. In this method all times are relative to the
		//current tick time, so xTc must be subtracted from.
		xD = pxTask->xTimeToWake - xTc + pxTask->xDeadline;
	}

	while( xW <= xD )
	{
		TickType_t xWm = xW;
		TickType_t xSum = ( TickType_t ) 0U; // summation

		// from lower to higher priority task -- this differs from the method.
		pxHigherPrioTaskListItem = pxTaskListItem;
		do
		{
			pxHigherPrioTaskTCB = ( TaskHandle_t ) listGET_LIST_ITEM_OWNER( pxHigherPrioTaskListItem );
			pxHigherPrioTask = getTaskSsTCB( pxHigherPrioTaskTCB );

			TickType_t xIj = ( TickType_t ) 0U;
			if( xTc > xIj )
			{
				// pxHigherPrioTask has finished, and xItemToWake has the
				// time when the task should be removed from the blocked
				// list, which is the earliest possible next release.
				xIj = pxHigherPrioTask->xTimeToWake - xTc;
			}

			TickType_t xWX = ( TickType_t ) 0U;
			if( xWm > xIj )
			{
				xWX = xWm - xIj;
			}

			xSum = xSum + ( U_CEIL( xWX, pxHigherPrioTask->xPeriod ) * pxHigherPrioTask->xWcet );

			pxHigherPrioTaskListItem = ( ( pxHigherPrioTaskListItem )->pxPrevious );
		}
		while( listGET_END_MARKER( pxTasksList ) != pxHigherPrioTaskListItem );

		xW = xS + xSum;

		if( xW == xWm )
		{
			// xV should be equal to the minimum value between xD - xWm and
			// the ceils.
			TickType_t xV = ( TickType_t ) 0U;
			if( xD > xWm )
			{
				xV = xD - xWm;

				TickType_t xV_t = portMAX_DELAY;

				// loop from lower to higher priority task, this differs
				// from the original paper.
				pxHigherPrioTaskListItem = pxTaskListItem;
				do
				{
					pxHigherPrioTaskTCB = ( TaskHandle_t ) listGET_LIST_ITEM_OWNER( pxHigherPrioTaskListItem );
					pxHigherPrioTask = getTaskSsTCB( pxHigherPrioTaskTCB );

					TickType_t xIj = ( TickType_t ) 0U;
					if( xTc > xIj )
					{
						// pxHigherPrioTask has finished, and xTimeToWake
						// has the time when the task should be removed
						// from the blocked list, which is the earliest
						// possible next release.
						xIj = pxHigherPrioTask->xTimeToWake - xTc;
					}

					TickType_t xWX = ( TickType_t ) 0U;
					if( xWm > xIj )
					{
						xWX = xWm - xIj;
					}
					xV_t = U_CEIL( xWX, pxHigherPrioTask->xPeriod ) * pxHigherPrioTask->xPeriod;
					xV_t = xV_t + xIj - xWm;

					if( xV_t < xV )
					{
						xV = xV_t;

						// if xV is zero, then we should break, as is the
						// minimum possible value we are seeking for.
						if( xV == ( TickType_t ) 0U )
						{
							break;
						}
					}

					pxHigherPrioTaskListItem = ( ( pxHigherPrioTaskListItem )->pxPrevious );
				}
				while( listGET_END_MARKER( pxTasksList ) != pxHigherPrioTaskListItem );
			}

			xS = xS + xV + ONE_TICK;
			xW = xW + xV + ONE_TICK;
		}
	}

	pxTask->xSlack = xS - ONE_TICK;
}
#endif /* configUSE_SLACK_METHOD == 1 */
/*-----------------------------------------------------------*/
