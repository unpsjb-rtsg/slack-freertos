#include "slack.h"
#include "slack_utils.h"

BaseType_t __wrap_xTaskDelayUntil( TickType_t * const pxPreviousWakeTime, const TickType_t xTimeIncrement )
{
TickType_t xTimeToWake;
BaseType_t xAlreadyYielded, xShouldDelay = pdFALSE;

    #if ( configUSE_SLACK_STEALING == 1 )
        /* SS: current extended TCB. */
        SsTCB_t *pxCurrentSsTCB = getSsTCB( pxCurrentTCB );
    #endif

    configASSERT( pxPreviousWakeTime );
    configASSERT( ( xTimeIncrement > 0U ) );
    configASSERT( uxSchedulerSuspended == 0 );

    vTaskSuspendAll();
    {
        /* Minor optimisation.  The tick count cannot change in this
        block. */
        const TickType_t xConstTickCount = xTickCount;

        /* Generate the tick time at which the task wants to wake. */
        xTimeToWake = *pxPreviousWakeTime + xTimeIncrement;

        if( xConstTickCount < *pxPreviousWakeTime )
        {
            /* The tick count has overflowed since this function was
            lasted called.  In this case the only time we should ever
            actually delay is if the wake time has also overflowed,
            and the wake time is greater than the tick time.  When this
            is the case it is as if neither time had overflowed. */
            if( ( xTimeToWake < *pxPreviousWakeTime ) && ( xTimeToWake > xConstTickCount ) )
            {
                xShouldDelay = pdTRUE;
            }
            else
            {
                mtCOVERAGE_TEST_MARKER();
            }
        }
        else
        {
            /* The tick time has not overflowed.  In this case we will
            delay if either the wake time has overflowed, and/or the
            tick time is less than the wake time. */
            if( ( xTimeToWake < *pxPreviousWakeTime ) || ( xTimeToWake > xConstTickCount ) )
            {
                xShouldDelay = pdTRUE;
            }
            else
            {
                mtCOVERAGE_TEST_MARKER();
            }
        }

        /* Update the wake time ready for the next call. */
        *pxPreviousWakeTime = xTimeToWake;

        #if ( ( configUSE_SLACK_STEALING == 1 ) && ( configSS_VERIFY_DEADLINE == 1 ) )
        {
            vSlackUpdateDeadline( pxCurrentSsTCB, xTimeToWake );
        }
        #endif

        if( xShouldDelay != pdFALSE )
        {
            traceTASK_DELAY_UNTIL( xTimeToWake );

            /* prvAddCurrentTaskToDelayedList() needs the block time, not
            the time to wake, so subtract the current tick count. */
            prvAddCurrentTaskToDelayedList( xTimeToWake - xConstTickCount, pdFALSE );
        }
        else
        {
            mtCOVERAGE_TEST_MARKER();
        }

        #if ( configUSE_SLACK_STEALING == 1 )
        {
            /* SS: The current tick is considered as consumed. */
            #if ( configSS_SLACK_K == 0 )
            {
                vSlackCalculateSlack( pxCurrentTCB, xConstTickCount );
            }
            #else
            {
                pxCurrentSsTCB->xSlack = pxCurrentSsTCB->xSlackK;
            }
            #endif

            if( pxCurrentSsTCB->xWcet > pxCurrentSsTCB->xCur )
            {
                vSlackGainSlack( pxCurrentTCB, pxCurrentSsTCB->xWcet - pxCurrentSsTCB->xCur );
            }

            if( xShouldDelay != pdFALSE )
            {
                /* Update the flag to indicate that the task was blocked by
                an invocation to vTaskDelayUntil(). The release count will
                be incremented when the task is unblocked at
                xTaskIncrementTick(). */
                pxCurrentSsTCB->uxDelayUntil = ( UBaseType_t ) pdTRUE;
            }
            else
            {
                /* The task was not blocked, because the wake time is in
                the past. Update the flag and increment the release count.
                The function will return to the task. */
                pxCurrentSsTCB->uxDelayUntil = ( UBaseType_t ) pdFALSE;
                pxCurrentSsTCB->uxReleaseCount = pxCurrentSsTCB->uxReleaseCount + 1UL;
            }
        }
        #endif
    }

    #if( configUSE_SLACK_STEALING == 1 )
    {
        if( xTaskGetAvailableSlack() > configSS_MIN_SLACK_SD )
        {
            /* Resume slack-delayed tasks if there is enough
            available slack. */
            if( listLIST_IS_EMPTY( &xSsTaskBlockedList ) == pdFALSE )
            {
                xTaskSlackResume();
            }
        }

        #if ( configDO_SLACK_TRACE == 1 )
        {
            prvTaskRecSlack();
        }
        #endif

        /* Reset the accumulated execution time. */
        pxCurrentSsTCB->xCur = ( TickType_t ) 0U;
    }
    #endif /* configUSE_SLACK_STEALING */

    xAlreadyYielded = xTaskResumeAll();

    /* Force a reschedule if xTaskResumeAll has not already done so, we may
    have put ourselves to sleep. */
    if( xAlreadyYielded == pdFALSE )
    {
        portYIELD_WITHIN_API();
    }
    else
    {
        mtCOVERAGE_TEST_MARKER();
    }

    return xShouldDelay;
}
/*-----------------------------------------------------------*/

BaseType_t __wrap_xTaskIncrementTick( void )
{
TCB_t * pxTCB;
TickType_t xItemValue;
BaseType_t xSwitchRequired = pdFALSE;

    /* Called by the portable layer each time a tick interrupt occurs.
    Increments the tick then checks to see if the new tick value will cause any
    tasks to be unblocked. */
    traceTASK_INCREMENT_TICK( xTickCount );
    if( uxSchedulerSuspended == ( UBaseType_t ) pdFALSE )
    {
        /* Minor optimisation.  The tick count cannot change in this
        block. */
        const TickType_t xConstTickCount = xTickCount + ( TickType_t ) 1;

        /* Increment the RTOS tick, switching the delayed and overflowed
        delayed lists if it wraps to 0. */
        xTickCount = xConstTickCount;

        if( xConstTickCount == ( TickType_t ) 0U ) /*lint !e774 'if' does not always evaluate to false as it is looking for an overflow. */
        {
            taskSWITCH_DELAYED_LISTS();
        }
        else
        {
            mtCOVERAGE_TEST_MARKER();
        }

        /* See if this tick has made a timeout expire.  Tasks are stored in
        the queue in the order of their wake time - meaning once one task
        has been found whose block time has not expired there is no need to
        look any further down the list. */
        if( xConstTickCount >= xNextTaskUnblockTime )
        {
            for( ;; )
            {
                if( listLIST_IS_EMPTY( pxDelayedTaskList ) != pdFALSE )
                {
                    /* The delayed list is empty.  Set xNextTaskUnblockTime
                    to the maximum possible value so it is extremely
                    unlikely that the
                    if( xTickCount >= xNextTaskUnblockTime ) test will pass
                    next time through. */
                    xNextTaskUnblockTime = portMAX_DELAY; /*lint !e961 MISRA exception as the casts are only redundant for some ports. */
                    break;
                }
                else
                {
                    /* The delayed list is not empty, get the value of the
                    item at the head of the delayed list.  This is the time
                    at which the task at the head of the delayed list must
                    be removed from the Blocked state. */
                    pxTCB = listGET_OWNER_OF_HEAD_ENTRY( pxDelayedTaskList ); /*lint !e9079 void * is used as this macro is used with timers and co-routines too.  Alignment is known to be fine as the type of the pointer stored and retrieved is the same. */
                    xItemValue = listGET_LIST_ITEM_VALUE( &( pxTCB->xStateListItem ) );

                    if( xConstTickCount < xItemValue )
                    {
                        /* It is not time to unblock this item yet, but the
                        item value is the time at which the task at the head
                        of the blocked list must be removed from the Blocked
                        state - so record the item value in
                        xNextTaskUnblockTime. */
                        xNextTaskUnblockTime = xItemValue;
                        break; /*lint !e9011 Code structure here is deedmed easier to understand with multiple breaks. */
                    }
                    else
                    {
                        mtCOVERAGE_TEST_MARKER();
                    }

                    /* It is time to remove the item from the Blocked state. */
                    listREMOVE_ITEM( &( pxTCB->xStateListItem ) );

                    /* Is the task waiting on an event also?  If so remove
                    it from the event list. */
                    if( listLIST_ITEM_CONTAINER( &( pxTCB->xEventListItem ) ) != NULL )
                    {
                        listREMOVE_ITEM( &( pxTCB->xEventListItem ) );
                    }
                    else
                    {
                        mtCOVERAGE_TEST_MARKER();
                    }

                    /* Place the unblocked task into the appropriate ready
                    list. */
                    prvAddTaskToReadyList( pxTCB );

                    #if ( configUSE_SLACK_STEALING == 1 )
                    {
                        SsTCB_t *pxSsTCB = getSsTCB( pxTCB );
                        if( pxSsTCB != NULL )
                        {
                            /* The unblocked task is in the appropriate ready
                            list. Increment the release counter if the task was
                            blocked by vTaskDelayUntil(). */
                            if( pxSsTCB->uxDelayUntil == ( UBaseType_t ) pdTRUE )
                            {
                                pxSsTCB->uxReleaseCount = pxSsTCB->uxReleaseCount + 1UL;
                                pxSsTCB->uxDelayUntil = ( UBaseType_t ) pdFALSE;
                            }
                        }
                    }
                    #endif

                    /* A task being unblocked cannot cause an immediate
                    context switch if preemption is turned off. */
                    #if (  configUSE_PREEMPTION == 1 )
                    {
                        /* Preemption is on, but a context switch should
                        only be performed if the unblocked task has a
                        priority that is equal to or higher than the
                        currently executing task. */
                        if( pxTCB->uxPriority >= pxCurrentTCB->uxPriority )
                        {
                            xSwitchRequired = pdTRUE;
                        }
                        else
                        {
                            mtCOVERAGE_TEST_MARKER();
                        }
                    }
                    #endif /* configUSE_PREEMPTION */
                }
            }
        }

        /* Tasks of equal priority to the currently running task will share
        processing time (time slice) if preemption is on, and the application
        writer has not explicitly turned time slicing off. */
        #if ( ( configUSE_PREEMPTION == 1 ) && ( configUSE_TIME_SLICING == 1 ) )
        {
            if( listCURRENT_LIST_LENGTH( &( pxReadyTasksLists[ pxCurrentTCB->uxPriority ] ) ) > ( UBaseType_t ) 1 )
            {
                xSwitchRequired = pdTRUE;
            }
            else
            {
                mtCOVERAGE_TEST_MARKER();
            }
        }
        #endif /* ( ( configUSE_PREEMPTION == 1 ) && ( configUSE_TIME_SLICING == 1 ) ) */

        #if ( configUSE_TICK_HOOK == 1 )
        {
            /* Guard against the tick hook being called when the pended tick
            count is being unwound (when the scheduler is being unlocked). */
            if( xPendedTicks == ( TickType_t ) 0 )
            {
                vApplicationTickHook();
            }
            else
            {
                mtCOVERAGE_TEST_MARKER();
            }
        }
        #endif /* configUSE_TICK_HOOK */

        #if ( configUSE_PREEMPTION == 1 )
        {
            if( xYieldPending != pdFALSE )
            {
                xSwitchRequired = pdTRUE;
            }
            else
            {
                mtCOVERAGE_TEST_MARKER();
            }
        }
        #endif /* configUSE_PREEMPTION */
    }
    else
    {
        ++xPendedTicks;

        /* The tick hook gets called at regular intervals, even if the
        scheduler is locked. */
        #if ( configUSE_TICK_HOOK == 1 )
        {
            vApplicationTickHook();
        }
        #endif
    }

    #if ( configUSE_SLACK_STEALING == 1 )
    {
        if( uxSchedulerSuspended == ( UBaseType_t ) pdFALSE )
        {
            SsTCB_t *pxCurrentSsTCB = getSsTCB( pxCurrentTCB );

            // Increment task release tick count
            if ( pxCurrentSsTCB != NULL )
            {
                pxCurrentSsTCB->xCur = pxCurrentSsTCB->xCur + ONE_TICK;
            }

            // Decrement real-time tasks slack counter by one tick
            if( ( pxCurrentTCB->uxPriority == tskIDLE_PRIORITY ) || ( pxCurrentTCB->uxPriority >= ( ( UBaseType_t ) ( configMAX_PRIORITIES - configSS_SLACK_PRIOS ) ) ) )
            {
                vSlackDecrementAllTasksSlack( ONE_TICK );
            }
            else
            {
                vSlackDecrementTasksSlack( pxCurrentTCB, ONE_TICK );
            }

            if( xTaskGetAvailableSlack() <= configSS_MIN_SLACK_SD )
            {
                UBaseType_t x;

                for( x = 1; x <= configSS_SLACK_PRIOS; x++ )
                {
                    /* Block tasks if there is not enough available slack. */
                    if( listLIST_IS_EMPTY( &pxReadyTasksLists[ configMAX_PRIORITIES - x ] ) == pdFALSE )
                    {
                        xSwitchRequired = xTaskSlackSuspend();
                        break;
                    }
                }
            }
            else
            {
                /* Resume slack-delayed tasks if there is enough available slack. */
                if( listLIST_IS_EMPTY( &xSsTaskBlockedList ) == pdFALSE )
                {
                    xTaskSlackResume();
                }
            }
        }
    }
    #endif /* configUSE_SLACK_STEALING */

    return xSwitchRequired;
}
/*-----------------------------------------------------------*/
