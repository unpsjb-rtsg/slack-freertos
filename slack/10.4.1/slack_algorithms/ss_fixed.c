/*
 * ss_fixed.c
 *
 *  Created on: 22 jul. 2020
 *      Author: Francisco E. Páez
 */
#include "slack.h"
#include "slackConfig.h"
#if ( configKERNEL_TEST > 0 )
#include "slack_tests.h"
#endif

#if ( configUSE_SLACK_METHOD == 0 )

/**
 *
 * @param xTask
 * @param xTc
 * @param xT
 * @param xWc
 * @param pxTasksList
 * @return
 */
static BaseType_t prvTaskCalcSlack( const TaskHandle_t xTask, const TickType_t xTc,
        const TickType_t xT, const TickType_t xWc, const List_t * pxTasksList );

/* from "Low Cost Slack Stealing Method for RM/DM" paper. */
void vSlackCalculateSlack_alg( TaskHandle_t xTask, const TickType_t xTc,
        const List_t * pxTasksList )
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
    TaskHandle_t pxHigherPrioTaskTCB = ( TaskHandle_t )
            listGET_LIST_ITEM_OWNER( pxHigherPrioTaskListItem );
    SsTCB_t * pxHigherPrioTask = getTaskSsTCB( pxHigherPrioTaskTCB );

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
        pxHigherPrioTask = getTaskSsTCB( pxHigherPrioTaskTCB );

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

    return ( BaseType_t ) xT - ( BaseType_t ) xTc - ( BaseType_t ) xW +
            ( BaseType_t ) xWc;
}
/*-----------------------------------------------------------*/

#endif
