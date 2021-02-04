#include "slack.h"
#include "slack_algorithms/slack_algorithms.h"

#if ( configKERNEL_TEST > 0 )
//#include "slack_tests.h"
#if ( configKERNEL_TEST == 2 )
static BaseType_t xCeilFloorCost = 0;
#endif
#if ( configKERNEL_TEST == 4 )
static BaseType_t xLoopCost = 0;
#endif
#endif

/*****************************************************************************
 * Private data declaration
 ****************************************************************************/
List_t xSsTaskBlockedList; // defined here for initialization

static UBaseType_t uxListInitializeFlag = pdTRUE;

/*****************************************************************************
 * Private data
 ****************************************************************************/
/**
 * \brief The system available slack.
 *
 * The system available slack is the minimum value of all the task's slacks.
 */
volatile static BaseType_t xSlackSD;

/**
 * \brief Task's deadlines list.
 *
 * This list contains references to all the ready tasks, ordered by their
 * absolute deadlines.
 */
static List_t xDeadlineTaskList;

/**
 * \brief List containing all the RTT of the system.
 *
 * This list contains references to all the tasks that account for the
 * available slack of the system. Tasks at the idle priority level are not
 * accounted.
 */
static List_t xSsTaskList;

/*****************************************************************************
 * Private functions declarations
 ****************************************************************************/
/**
 * \brief Updates the system available slack.
 *
 * The system available slack is calculated as the minimum value of all the
 * tasks available slacks.
 */
static void vSlackUpdateAvailableSlack();

/**
 * \brief Perform the setup of the required tasks lists.
 *
 * This function must be called **before** setting the tasks parameters with
 * \ref vSlackSetTaskParams().
 */
static void vSlackSystemSetup( void );

/**
 * \brief Calculates the worst case response time of the RTT tasks.
 *
 * It uses the algorithm described in [Improved Response-Time Analysis
 * Calculations](http://doi.ieeecomputersociety.org/10.1109/REAL.1998.739773).
 *
 * @return pdTRUE if the task set is schedulable or pdFALSE if it's not.
 */
static BaseType_t xSlackCalculateTasksWcrt();

/*****************************************************************************
 * Private functions implementation
 ****************************************************************************/
static inline void vSlackUpdateAvailableSlack()
{
    ListItem_t * pxAppTasksListItem = listGET_HEAD_ENTRY( &xSsTaskList );

    SsTCB_t * ssTCB = getTaskSsTCB( listGET_LIST_ITEM_OWNER( pxAppTasksListItem ) );
    xSlackSD = ssTCB->xSlack;

    while( listGET_END_MARKER( &xSsTaskList ) != pxAppTasksListItem )
    {
        ssTCB = getTaskSsTCB( listGET_LIST_ITEM_OWNER( pxAppTasksListItem ) );

        if( ssTCB->xSlack < xSlackSD )
        {
            xSlackSD = ssTCB->xSlack;
        }

        pxAppTasksListItem = listGET_NEXT( pxAppTasksListItem );
    }
}
/*-----------------------------------------------------------*/

static void vSlackSystemSetup( void )
{
    vListInitialise( &xSsTaskList );
    vListInitialise( &xDeadlineTaskList );
    vListInitialise( &xSsTaskBlockedList );
}
/*-----------------------------------------------------------*/

static BaseType_t xSlackCalculateTasksWcrt()
{
    TickType_t xW = 0U;

    ListItem_t *pxTaskListItem = listGET_HEAD_ENTRY( &xSsTaskList );

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
    while( listGET_END_MARKER( &xSsTaskList ) != pxTaskListItem )
    {
        xTask = ( TaskHandle_t ) listGET_LIST_ITEM_OWNER( pxTaskListItem );
        pxTask = pvTaskGetThreadLocalStoragePointer( xTask, 0 );

        xT = xT + pxTask->xWcet;

        while( xT <= pxTask->xDeadline )
        {
            xW = 0;

            /* Calculates the workload of the higher priority tasks than pxTask. */
            ListItem_t * pxHigherPrioTaskListItem = listGET_HEAD_ENTRY( &xSsTaskList );
            do
            {
                SsTCB_t *pxHigherPrioTask = pvTaskGetThreadLocalStoragePointer(
                        ( TaskHandle_t ) listGET_LIST_ITEM_OWNER( pxHigherPrioTaskListItem ), 0 );

                xW = xW + ( U_CEIL( xT, pxHigherPrioTask->xPeriod ) *
                        pxHigherPrioTask->xWcet );

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

/*****************************************************************************
 * Public functions implementation
 ****************************************************************************/
void vSlackSetTaskParams( TaskHandle_t xTask, const SsTaskType_t xTaskType,
        const TickType_t xPeriod, const TickType_t xDeadline,
        const TickType_t xWcet, const BaseType_t xId )
{
	UBaseType_t uxTaskPriority = uxTaskPriorityGet( xTask );
	SsTCB_t * pxNewSsTCB = pvPortMalloc( sizeof( SsTCB_t ) );

	if( ( uxTaskPriority == tskIDLE_PRIORITY ) ||
	        ( uxTaskPriority == configMAX_PRIORITIES - 1 ) )
	{
		// error?
	}

	// Initialize the list of tasks if required.
	if ( uxListInitializeFlag == pdTRUE ) {
		vSlackSystemSetup();
		uxListInitializeFlag = pdFALSE;
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
	pxNewSsTCB->xTtma = 0U;
	pxNewSsTCB->xDi = 0U;
	pxNewSsTCB->xCur = ( TickType_t ) 0U;

	if( xTaskType == PERIODIC_TASK )
	{
		vListInitialiseItem( &( pxNewSsTCB->xSsTaskListItem ) );
		listSET_LIST_ITEM_VALUE( &( pxNewSsTCB->xSsTaskListItem ),
		        ( TickType_t ) configMAX_PRIORITIES - ( TickType_t ) uxTaskPriority );
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

    	vTaskCalculateSlack( xTask, (TickType_t) 0U );
    	pxTaskSs->xSlackK = pxTaskSs->xSlack;

    	/* Deadline */
    	UBaseType_t uxTaskPriority = uxTaskPriorityGet( xTask );
    	if( uxTaskPriority != tskIDLE_PRIORITY )
    	{
    		listSET_LIST_ITEM_VALUE( &( ( pxTaskSs )->xDeadlineTaskListItem ),
    		        pxTaskSs->xDeadline );
    		vListInsert( &xDeadlineTaskList, &( ( pxTaskSs )->xDeadlineTaskListItem ) );
    	}

    	pxTaskListItem = listGET_NEXT( pxTaskListItem );
    }

    vSlackUpdateAvailableSlack();
}
/*-----------------------------------------------------------*/

void vSlackDeadlineCheck()
{
    TickType_t xTickCount = xTaskGetTickCountFromISR();

    if( listLIST_IS_EMPTY( &xDeadlineTaskList ) == pdFALSE )
    {
        ListItem_t *pxDeadlineListItem = listGET_HEAD_ENTRY( &xDeadlineTaskList );

        while( listGET_END_MARKER( &( xDeadlineTaskList ) ) != pxDeadlineListItem )
        {
            TickType_t xTaskReleaseDeadline = listGET_LIST_ITEM_VALUE( pxDeadlineListItem );
            if( xTickCount >= xTaskReleaseDeadline )
            {
                TaskHandle_t pxTask = listGET_LIST_ITEM_OWNER( pxDeadlineListItem );
                SsTCB_t *pxTaskSs = getTaskSsTCB( pxTask );
                /* The current release of task pxTCB has missed its deadline.
                 * Invoke the application missed-deadline hook function. */
                vApplicationDeadlineMissedHook( pcTaskGetName(pxTask), pxTaskSs, xTickCount );
            }
            else
            {
                /* As xDeadlineTaskList is deadline-ordered if the current
                 * xTaskReleaseDeadline is greater than the tick count, then the
                 * remaining tasks has not missed their deadlines. */
                break;
            }
            /* Get the next task list item. */
            pxDeadlineListItem = listGET_NEXT( pxDeadlineListItem );
        }
    }
}
/*-----------------------------------------------------------*/

void vSlackUpdateDeadline( SsTCB_t *pxTask, TickType_t xTimeToWake )
{
    /* Remove the current release deadline and insert the deadline for the next
     * release of pxCurrentTCB. */
    ListItem_t *pxDeadlineTaskListItem = &( pxTask->xDeadlineTaskListItem );
    uxListRemove( pxDeadlineTaskListItem );
    listSET_LIST_ITEM_VALUE( pxDeadlineTaskListItem, xTimeToWake + pxTask->xDeadline );
    vListInsert( &xDeadlineTaskList, pxDeadlineTaskListItem );
    pxTask->xTimeToWake = xTimeToWake;
}
/*-----------------------------------------------------------*/

TickType_t xSlackGetAvailableSlack( void )
{
    return xSlackSD;
}
/*-----------------------------------------------------------*/

inline void vSlackGainSlack( const TaskHandle_t xTask, const TickType_t xTicks )
{
    SsTCB_t * ssTCB = getTaskSsTCB( xTask );
    ListItem_t * pxAppTasksListItem = listGET_NEXT( &( ssTCB->xSsTaskListItem ) );

    while( listGET_END_MARKER( &xSsTaskList ) != pxAppTasksListItem )
    {
        ssTCB = getTaskSsTCB( listGET_LIST_ITEM_OWNER( pxAppTasksListItem ) );
        ssTCB->xSlack += ( BaseType_t ) xTicks;

        pxAppTasksListItem = listGET_NEXT( pxAppTasksListItem );
    }

    vSlackUpdateAvailableSlack();
}
/*-----------------------------------------------------------*/

inline void vSlackDecrementAllTasksSlack( const TickType_t xTicks )
{
	ListItem_t * pxAppTasksListItem = listGET_HEAD_ENTRY( &xSsTaskList );

	while( listGET_END_MARKER( &xSsTaskList ) != pxAppTasksListItem )
	{
		SsTCB_t * xTask = getTaskSsTCB( listGET_LIST_ITEM_OWNER( pxAppTasksListItem ) );

		if( xTask->xSlack > 0 )
		{
		    xTask->xSlack = xTask->xSlack - ( BaseType_t ) xTicks;
		}

		pxAppTasksListItem = listGET_NEXT( pxAppTasksListItem );
	}

	vSlackUpdateAvailableSlack();
}
/*-----------------------------------------------------------*/

inline void vSlackDecrementTasksSlack( TaskHandle_t xTask, const TickType_t xTicks )
{
	const ListItem_t * pxAppTasksListEndMarker = &( getTaskSsTCB( xTask )->xSsTaskListItem );
    ListItem_t * pxAppTasksListItem = listGET_HEAD_ENTRY( &xSsTaskList );

    while( pxAppTasksListEndMarker != pxAppTasksListItem )
    {
        SsTCB_t * xTask = getTaskSsTCB( listGET_LIST_ITEM_OWNER( pxAppTasksListItem ) );

        if( xTask->xSlack > 0 )
        {
            xTask->xSlack = xTask->xSlack - ( BaseType_t ) xTicks;
        }

        pxAppTasksListItem = listGET_NEXT( pxAppTasksListItem );
    }

    vSlackUpdateAvailableSlack();
}
/*-----------------------------------------------------------*/

inline TickType_t xSlackGetWorkLoad( TaskHandle_t xTask, const TickType_t xTc,
        const List_t * pxTasksList )
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

void vTaskCalculateSlack( TaskHandle_t xTask, const TickType_t xTc )
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

	vTaskCalculateSlack_alg( xTask, xTc, &xSsTaskList );

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

/**
 * \brief Record the available slack of each task in \p pxArray.
 *
 * @param pxArray a pointer to an array where the counters are copied.
 */
void vTasksGetSlacks( int32_t *pxArray )
{
    pxArray[ 0 ] = xTaskGetTickCount();
    pxArray[ 1 ] = getTaskSsTCB( xTaskGetCurrentTaskHandle() )->xId;
    pxArray[ 2 ] = xSlackGetAvailableSlack();

    ListItem_t *pxTaskListItem = listGET_HEAD_ENTRY( &xSsTaskList );

    BaseType_t xI = 3U;

    while( listGET_END_MARKER( &( xSsTaskList ) ) != pxTaskListItem )
    {
        pxArray[ xI ] = getTaskSsTCB( listGET_LIST_ITEM_OWNER( pxTaskListItem ) )->xSlack;
        xI = xI + 1;
        pxTaskListItem = listGET_NEXT( pxTaskListItem );
    }
}
/*-----------------------------------------------------------*/
