/*
 * ss_davis.c
 *
 *  Created on: 22 jul. 2020
 *      Author: Francisco E. Páez
 */
#include "slack.h"
#include "slackConfig.h"
#if ( configKERNEL_TEST > 0 )
#include "slack_tests.h"
#endif

#if ( configUSE_SLACK_METHOD == 1 )

/* from "Scheduling Slack Time on Fixed Priority Pre-emptive Systems" paper */
void vSlackCalculateSlack_alg( TaskHandle_t xTask, const TickType_t xTc,
        const List_t * pxTasksList )
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
            pxHigherPrioTaskTCB = ( TaskHandle_t* ) listGET_LIST_ITEM_OWNER( pxHigherPrioTaskListItem );
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
                    pxHigherPrioTaskTCB = ( TaskHandle_t* ) listGET_LIST_ITEM_OWNER( pxHigherPrioTaskListItem );
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
/*-----------------------------------------------------------*/

#endif
