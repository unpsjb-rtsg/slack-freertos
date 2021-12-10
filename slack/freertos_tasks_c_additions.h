#include "slack.h"
#include "slack_utils.h"
#include "wrap.h"
#if ( configKERNEL_TEST > 0 )
#include "slack_tests.h"
#endif

/*****************************************************************************
 * Private data declaration
 ****************************************************************************/
#if ( configKERNEL_TEST > 0 )
#if ( configKERNEL_TEST == 2 )
static BaseType_t xCeilFloorCost = 0;
#endif
#if ( configKERNEL_TEST == 4 )
static BaseType_t xLoopCost = 0;
#endif
#endif

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
 * Private functions declaration
 ****************************************************************************/
#if ( configDO_SLACK_TRACE == 1 )
static void prvTaskRecSlack() PRIVILEGED_FUNCTION;
#endif

/**
 * \brief Perform the initialization steps required before the scheduler starts.
 *
 * This function perform the initialization required before the FreeRTOS scheduler
 * is started: executes the schedulability test and the initial slack calculations.
 *
 * This function is called with the FREERTOS_TASKS_C_ADDITIONS_INIT() macro.
 */
static void vSlackSchedulerSetup( void );

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
static inline TickType_t xSlackGetWorkLoad( TaskHandle_t xTask, const TickType_t xTc,
        const List_t * pxTasksList );

#if ( configSS_SLACK_METHOD == SLACK_METHOD_URRIZA_2010 )
static inline void vSlackCalculateSlack_fixed( TaskHandle_t xTask, const TickType_t xTc,
        const List_t * pxTasksList );
#endif
#if ( configSS_SLACK_METHOD == SLACK_METHOD_DAVIS_1993 )
static inline void vSlackCalculateSlack_davis( TaskHandle_t xTask, const TickType_t xTc,
        const List_t * pxTasksList );
#endif

/**
 * \brief Updates the system available slack.
 *
 * The system available slack is calculated as the minimum value of all the
 * tasks available slacks.
 */
static inline void vSlackUpdateAvailableSlack() __attribute__((always_inline));

/**
 * \brief Calculates the worst case response time of the RTT tasks.
 *
 * It uses the algorithm described in [Improved Response-Time Analysis
 * Calculations](http://doi.ieeecomputersociety.org/10.1109/REAL.1998.739773).
 *
 * @return pdTRUE if the task set is schedulable or pdFALSE if it's not.
 */
static BaseType_t xSlackCalculateTasksWcrt();

/**
 * \brief Perform the setup of the required tasks lists.
 *
 * This function must be called **before** setting the tasks parameters with
 * \ref vSlackSetTaskParams().
 */
static void vSlackSystemSetup( void );

/*****************************************************************************
 * Private functions
 ****************************************************************************/
static BaseType_t xTaskSlackResume( void )
{
    BaseType_t xSwitchRequired = pdFALSE;

    if( listLIST_IS_EMPTY( &xSsTaskBlockedList ) == pdFALSE )
    {
        ListItem_t *pxTaskListItem = listGET_HEAD_ENTRY( &xSsTaskBlockedList );

        while( listGET_END_MARKER( &xSsTaskBlockedList ) != pxTaskListItem )
        {
            TCB_t * pxTCB = ( TCB_t * ) listGET_LIST_ITEM_OWNER( pxTaskListItem );

            /* As we are in a critical section we can access the ready
            lists even if the scheduler is suspended. */
            ( void ) uxListRemove(  &( getSsTCB( pxTCB )->xSsTaskBlockedListItem ) );
            prvAddTaskToReadyList( pxTCB );

            /* Get next ready task. */
            pxTaskListItem = listGET_NEXT( pxTaskListItem );
        }

        xSwitchRequired = pdTRUE;
    }

    return xSwitchRequired;
}
/*-----------------------------------------------------------*/

static BaseType_t xTaskSlackSuspend( void )
{
    BaseType_t xSwitchRequired = pdFALSE;

    UBaseType_t x;

    for( x = 1; x <= configSS_SLACK_PRIOS; x++ )
    {
        if( listLIST_IS_EMPTY( &pxReadyTasksLists[ configMAX_PRIORITIES - x ] ) == pdFALSE )
        {
            ListItem_t const *pxTaskListEnd = listGET_END_MARKER( &pxReadyTasksLists[ configMAX_PRIORITIES - x ] );
            ListItem_t *pxTaskListItem = listGET_HEAD_ENTRY( &pxReadyTasksLists[ configMAX_PRIORITIES - x ] );

            while( pxTaskListEnd != pxTaskListItem )
            {
                TCB_t * pxTCB = ( TCB_t * ) listGET_LIST_ITEM_OWNER( pxTaskListItem );

                /* Remove task from the ready/delayed list and place in the
                suspended list. */
                if( uxListRemove( &( pxTCB->xStateListItem ) ) == ( UBaseType_t ) 0 )
                {
                    taskRESET_READY_PRIORITY( pxTCB->uxPriority );
                }
                else
                {
                    mtCOVERAGE_TEST_MARKER();
                }

                /* Is the task waiting on an event also? */
                if( listLIST_ITEM_CONTAINER( &( pxTCB->xEventListItem ) ) != NULL )
                {
                    ( void ) uxListRemove( &( pxTCB->xEventListItem ) );
                }
                else
                {
                    mtCOVERAGE_TEST_MARKER();
                }

                vListInsertEnd( &xSsTaskBlockedList, &( getSsTCB( pxTCB )->xSsTaskBlockedListItem ) );

                /* Get next ready task. */
                pxTaskListItem = listGET_NEXT( pxTaskListItem );
            }

            xSwitchRequired = pdTRUE;
        }
    }

    return xSwitchRequired;
}
/*-----------------------------------------------------------*/

static inline void vSlackUpdateAvailableSlack()
{
    ListItem_t * pxAppTasksListItem = listGET_HEAD_ENTRY( &xSsTaskList );

    SsTCB_t * ssTCB = getSsTCB( listGET_LIST_ITEM_OWNER( pxAppTasksListItem ) );
    xSlackSD = ssTCB->xSlack;

    while( listGET_END_MARKER( &xSsTaskList ) != pxAppTasksListItem )
    {
        ssTCB = getSsTCB( listGET_LIST_ITEM_OWNER( pxAppTasksListItem ) );

        if( ssTCB->xSlack < xSlackSD )
        {
            xSlackSD = ssTCB->xSlack;
        }

        pxAppTasksListItem = listGET_NEXT( pxAppTasksListItem );
    }
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

    #if ( configSS_VERIFY_SCHEDULABILITY == 1 )
    /* Calculate worst case execution times of tasks. */
    BaseType_t xSchedulable = xSlackCalculateTasksWcrt( &xSsTaskList );
    if( xSchedulable == pdFALSE )
    {
        vApplicationNotSchedulable();
    }
    #else
    xSlackCalculateTasksWcrt( &xSsTaskList );
    #endif

    ListItem_t *pxTaskListItem = listGET_HEAD_ENTRY( &xSsTaskList );

    /* Calculate slacks at xTickCount = 0 */
    while( listGET_END_MARKER( &( xSsTaskList ) ) != pxTaskListItem )
    {
        TaskHandle_t xTask = ( TaskHandle_t ) listGET_LIST_ITEM_OWNER( pxTaskListItem );
        SsTCB_t *pxTaskSs = getSsTCB( xTask );

        vSlackCalculateSlack( xTask, (TickType_t) 0U );
        pxTaskSs->xSlackK = pxTaskSs->xSlack;

        /* Deadline */
        #if ( configSS_VERIFY_DEADLINE == 1 )
        UBaseType_t uxTaskPriority = uxTaskPriorityGet( xTask );
        if( uxTaskPriority != tskIDLE_PRIORITY )
        {
            listSET_LIST_ITEM_VALUE( &( ( pxTaskSs )->xDeadlineTaskListItem ),
                    pxTaskSs->xDeadline );
            vListInsert( &xDeadlineTaskList, &( ( pxTaskSs )->xDeadlineTaskListItem ) );
        }
        #endif

        pxTaskListItem = listGET_NEXT( pxTaskListItem );
    }

    vSlackUpdateAvailableSlack();
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
/*-----------------------------------------------------------*/

#if ( configSS_VERIFY_DEADLINE == 1 )
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
                SsTCB_t *pxTaskSs = getSsTCB( pxTask );
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
#endif
/*-----------------------------------------------------------*/

#if ( configSS_VERIFY_DEADLINE == 1 )
inline void vSlackUpdateDeadline( SsTCB_t *pxTask, TickType_t xTimeToWake )
{
    /* Remove the current release deadline and insert the deadline for the next
     * release of pxCurrentTCB. */
    ListItem_t *pxDeadlineTaskListItem = &( pxTask->xDeadlineTaskListItem );
    uxListRemove( pxDeadlineTaskListItem );
    listSET_LIST_ITEM_VALUE( pxDeadlineTaskListItem, xTimeToWake + pxTask->xDeadline );
    vListInsert( &xDeadlineTaskList, pxDeadlineTaskListItem );
    pxTask->xTimeToWake = xTimeToWake;
}
#endif
/*-----------------------------------------------------------*/

inline void vSlackGainSlack( const TaskHandle_t xTask, const TickType_t xTicks )
{
    SsTCB_t * ssTCB = getSsTCB( xTask );
    ListItem_t * pxAppTasksListItem = listGET_NEXT( &( ssTCB->xSsTaskListItem ) );

    while( listGET_END_MARKER( &xSsTaskList ) != pxAppTasksListItem )
    {
        ssTCB = getSsTCB( listGET_LIST_ITEM_OWNER( pxAppTasksListItem ) );
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
        SsTCB_t * xTask = getSsTCB( listGET_LIST_ITEM_OWNER( pxAppTasksListItem ) );

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
    const ListItem_t * pxAppTasksListEndMarker = &( getSsTCB( xTask )->xSsTaskListItem );
    ListItem_t * pxAppTasksListItem = listGET_HEAD_ENTRY( &xSsTaskList );

    while( pxAppTasksListEndMarker != pxAppTasksListItem )
    {
        SsTCB_t * xTask = getSsTCB( listGET_LIST_ITEM_OWNER( pxAppTasksListItem ) );

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
    ListItem_t *pxTaskListItem = &( getSsTCB( xTask ) )->xSsTaskListItem;

    TickType_t xW = ( TickType_t ) 0U;  // Workload
    TickType_t xA = ( TickType_t ) 0U;
    TickType_t xC = ( TickType_t ) 0U;

    // Until we process all the maximum priority tasks (including pxTask)
    while( listGET_END_MARKER( pxTasksList ) != pxTaskListItem )
    {
#if ( configKERNEL_TEST == 4 )
        xLoopCost = xLoopCost + 1;
#endif

        SsTCB_t *pxTask = getSsTCB( listGET_LIST_ITEM_OWNER( pxTaskListItem ) );

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

void vSlackCalculateSlack( TaskHandle_t xTask, const TickType_t xTc )
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

#if ( configSS_SLACK_METHOD == SLACK_METHOD_URRIZA_2010 )
    vSlackCalculateSlack_fixed( xTask, xTc, &xSsTaskList );
#endif
#if ( configSS_SLACK_METHOD == SLACK_METHOD_DAVIS_1993 )
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

/**
 * \brief Record the available slack of each task in \p pxArray.
 *
 * @param pxArray a pointer to an array where the counters are copied.
 */
void vTasksGetSlacks( int32_t *pxArray )
{
    pxArray[ 0 ] = xTaskGetTickCount();
    pxArray[ 1 ] = xTaskGetAvailableSlack();

    ListItem_t *pxTaskListItem = listGET_HEAD_ENTRY( &xSsTaskList );

    BaseType_t xI = 2U;

    while( listGET_END_MARKER( &( xSsTaskList ) ) != pxTaskListItem )
    {
        pxArray[ xI ] = getSsTCB( listGET_LIST_ITEM_OWNER( pxTaskListItem ) )->xSlack;
        xI = xI + 1;
        pxTaskListItem = listGET_NEXT( pxTaskListItem );
    }
}
/*-----------------------------------------------------------*/

void vSlackSetTaskParams( TaskHandle_t xTask, const SsTaskType_t xTaskType,
        const TickType_t xPeriod, const TickType_t xDeadline,
        const TickType_t xWcet )
{
    UBaseType_t uxTaskPriority = uxTaskPriorityGet( xTask );
    SsTCB_t * pxNewSsTCB = pvPortMalloc( sizeof( SsTCB_t ) );

    if( ( uxTaskPriority == tskIDLE_PRIORITY ) ||
            ( uxTaskPriority == configMAX_PRIORITIES - 1 ) )
    {
        // error?
    }

    // Initialize the list of tasks if required.
    if ( !listLIST_IS_INITIALISED( &xSsTaskList ) ) {
        vSlackSystemSetup();
    }

    pxNewSsTCB->xTaskType = xTaskType;

    pxNewSsTCB->xPeriod = xPeriod;
    pxNewSsTCB->xDeadline = xDeadline;
    pxNewSsTCB->xWcet = xWcet;
    pxNewSsTCB->xA = xWcet;
    pxNewSsTCB->xB = xPeriod;

    pxNewSsTCB->uxReleaseCount = 1U;
    pxNewSsTCB->xPreviousWakeTime = ( TickType_t ) 0U;
    pxNewSsTCB->xTimeToWake = ( TickType_t ) 0U;
    pxNewSsTCB->xWcrt = 0U;
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

        #if ( configSS_VERIFY_DEADLINE == 1 )
        vListInitialiseItem( &( pxNewSsTCB->xDeadlineTaskListItem ) );
        listSET_LIST_ITEM_OWNER( &( pxNewSsTCB->xDeadlineTaskListItem ), xTask );
        listSET_LIST_ITEM_VALUE( &( pxNewSsTCB->xDeadlineTaskListItem ), pxNewSsTCB->xDeadline );
        /* The list item value of xDeadlineTaskListItem is updated when the
            task is moved into the ready list. */
        #endif
    }

    if( xTaskType == APERIODIC_TASK )
    {
        vListInitialiseItem( &( pxNewSsTCB->xSsTaskBlockedListItem ) );
        listSET_LIST_ITEM_OWNER( &( pxNewSsTCB->xSsTaskBlockedListItem ), xTask );
        listSET_LIST_ITEM_VALUE( &( pxNewSsTCB->xSsTaskBlockedListItem ), 0 );
    }

    vTaskSetThreadLocalStoragePointer( xTask, configSS_STORAGE_POINTER_INDEX, ( void * ) pxNewSsTCB );
}
/*-----------------------------------------------------------*/

TickType_t xSlackGetAvailableSlack() {
    return xTaskGetAvailableSlack();
}
/*-----------------------------------------------------------*/

#if ( configSS_SLACK_METHOD == SLACK_METHOD_URRIZA_2010 )
/**
 *
 * @param xTask
 * @param xTc
 * @param xT
 * @param xWc
 * @param pxTasksList
 * @return
 */
static inline BaseType_t prvTaskCalcSlack( const TaskHandle_t xTask, const TickType_t xTc,
        const TickType_t xT, const TickType_t xWc, const List_t * pxTasksList ) __attribute__((always_inline));

/* from "Low Cost Slack Stealing Method for RM/DM" paper. */
static inline void vSlackCalculateSlack_fixed( TaskHandle_t xTask, const TickType_t xTc,
        const List_t * pxTasksList )
{
    SsTCB_t * pxTask = getSsTCB( xTask );

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
    TaskHandle_t pxHigherPrioTaskTCB = ( TaskHandle_t )
            listGET_LIST_ITEM_OWNER( pxHigherPrioTaskListItem );
    SsTCB_t * pxHigherPrioTask = getSsTCB( pxHigherPrioTaskTCB );

    // Corollary 2 (follows theorem 5)
    if ( ( pxHigherPrioTask->xDi + pxHigherPrioTask->xWcet >= xDi ) &&
            ( xDi >= pxHigherPrioTask->xTtma ) )
    {
        pxTask->xSlack = pxHigherPrioTask->xSlack - pxTask->xWcet;
        pxTask->xTtma = pxHigherPrioTask->xTtma;
        return;
    }

    // Theorem 3
    TickType_t xIntervalo = xXi + pxTask->xDeadline - pxTask->xWcrt +
            pxTask->xWcet;

    // Corollary 1 (follows theorem 4)
    if ( ( pxHigherPrioTask->xDi + pxHigherPrioTask->xWcet >= xIntervalo ) &&
            ( pxHigherPrioTask->xDi + pxHigherPrioTask->xWcet <= xDi ) )
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

        pxHigherPrioTaskTCB = ( TaskHandle_t )
                listGET_LIST_ITEM_OWNER( pxHigherPrioTaskListItem );
        pxHigherPrioTask = getSsTCB( pxHigherPrioTaskTCB );

        xii = U_CEIL( xIntervalo, pxHigherPrioTask->xPeriod ) *
                pxHigherPrioTask->xPeriod;

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
/*-----------------------------------------------------------*/

static inline BaseType_t prvTaskCalcSlack( const TaskHandle_t xTask,
        const TickType_t xTc, const TickType_t xT, const TickType_t xWc,
        const List_t * pxTasksList )
{
    ListItem_t *pxTaskListItem = &( getSsTCB( xTask )->xSsTaskListItem );
    TickType_t xW = 0;

    // process all the maximum priority tasks
    while( listGET_END_MARKER( pxTasksList ) != pxTaskListItem )
    {
        #if ( configKERNEL_TEST == 4 )
            xLoopCost = xLoopCost + 1;
        #endif

        SsTCB_t * pxTask = getSsTCB( listGET_LIST_ITEM_OWNER( pxTaskListItem ) );
        // accumulated workload of higher priority tasks in [0, xT)
        xW = xW + ( U_CEIL( xT, pxTask->xPeriod ) * pxTask->xWcet );

        #if ( configKERNEL_TEST == 2 )
            xCeilFloorCost = xCeilFloorCost + 1;
        #endif

        // get next higher priority task list item
        pxTaskListItem = ( ( pxTaskListItem )->pxPrevious );
    }

    return ( BaseType_t ) xT - ( BaseType_t ) xTc - ( BaseType_t ) xW +
            ( BaseType_t ) xWc;
}
/*-----------------------------------------------------------*/
#endif


#if ( configSS_SLACK_METHOD == SLACK_METHOD_DAVIS_1993 )
/* from "Scheduling Slack Time on Fixed Priority Pre-emptive Systems" paper */
static inline void vSlackCalculateSlack_davis( TaskHandle_t xTask, const TickType_t xTc,
        const List_t * pxTasksList )
{
    SsTCB_t *pxTask = getSsTCB( xTask );

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
            pxHigherPrioTaskTCB = ( TaskHandle_t* ) listGET_LIST_ITEM_OWNER( pxHigherPrioTaskListItem );
            pxHigherPrioTask = getSsTCB( pxHigherPrioTaskTCB );

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
                    pxHigherPrioTaskTCB = ( TaskHandle_t* ) listGET_LIST_ITEM_OWNER( pxHigherPrioTaskListItem );
                    pxHigherPrioTask = getSsTCB( pxHigherPrioTaskTCB );

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
/*-----------------------------------------------------------*/
#endif
