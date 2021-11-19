#include "FreeRTOS.h"
#include "task.h"
#include "slack_tests.h"

#if ( configKERNEL_TEST == 1 )
static BaseType_t ulDelayUntilFlag = pdFALSE;
#endif

uint32_t cs_costs[TASK_COUNT][RELEASE_COUNT + 1];

void vInitArray()
{
	/* Zeroes cs_cost[][] */
	for(int i = 0; i < TASK_COUNT; i++)
	{
        for(int j = 0; j < RELEASE_COUNT + 1; j++)
		{
			cs_costs[i][j] = 0;
		}
	}
}
/*-----------------------------------------------------------*/

#if ( configUSE_SLACK_STEALING == 0 )
/*
 * Just completes the SsTCB with the task parameters. Do not initialize any
 * additional task list. Used when no slack stealing support is compiled, and
 * the context switch cost of FreeRTOS is evaluated.
 */
void vSlackSetTaskParams( TaskHandle_t xTask, const SsTaskType_t xTaskType,
        const TickType_t xPeriod, const TickType_t xDeadline,
        const TickType_t xWcet, const BaseType_t xId )
{
    SsTCB_t * pxNewSsTCB = pvPortMalloc( sizeof( SsTCB_t ) );

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
    pxNewSsTCB->xSlack = 0U;
    pxNewSsTCB->xTtma = 0U;
    pxNewSsTCB->xDi = 0U;
    pxNewSsTCB->xCur = ( TickType_t ) 0U;

    vTaskSetThreadLocalStoragePointer( xTask, 0, ( void * ) pxNewSsTCB );
}
#endif
/*-----------------------------------------------------------*/

#if ( configKERNEL_TEST == 1 )
void vMacroTaskDelay()
{
	STOPWATCH_RESET();
    uint32_t cycles = CPU_CYCLES;
    BaseType_t xId = (( SsTCB_t* ) pvSlackGetTaskSsTCB( xTaskGetCurrentTaskHandle() ))->xId;
    if ( cs_costs[xId][0] < RELEASE_COUNT )
	{
        cs_costs[xId][ cs_costs[ xId ][0] + 1 ] = cycles;
    }
    ulDelayUntilFlag = pdTRUE;
}
/*-----------------------------------------------------------*/

void vMacroTaskSwitched()
{
    uint32_t cycles = CPU_CYCLES;
	if ( ulDelayUntilFlag == pdTRUE ) {
        BaseType_t xId = (( SsTCB_t* ) pvSlackGetTaskSsTCB( xTaskGetCurrentTaskHandle() ))->xId;
        if ( cs_costs[xId][0] < RELEASE_COUNT )
        {
            cs_costs[xId][ cs_costs[ xId ][0] + 1] = cycles - cs_costs[xId][ cs_costs[xId][0] + 1];
            cs_costs[xId][0] += 1U;
        }
		ulDelayUntilFlag = pdFALSE;
	}
}
/*-----------------------------------------------------------*/
#endif

#if ( configDO_SLACK_TRACE == 1 )
void prvTaskRecSlack()
{
	(*xResults)[xRecSlackIdx][ 0 ] = xTickCount;
	(*xResults)[xRecSlackIdx][ 1 ] = getSsTCB( pxCurrentTCB )->xId;
	(*xResults)[xRecSlackIdx][ 2 ] = getSsTCB( pxCurrentTCB )->xCur;

	ListItem_t *pxTaskListItem = listGET_HEAD_ENTRY( &xSsTaskList );

	BaseType_t xI = 3U;

	while( listGET_END_MARKER( &( xSsTaskList ) ) != pxTaskListItem )
	{
		(*xResults)[ xRecSlackIdx ][ xI ] = getSsTCB( listGET_LIST_ITEM_OWNER( pxTaskListItem ) )->xSlack;
		xI = xI + 1;
		pxTaskListItem = listGET_NEXT( pxTaskListItem );
	}

	xRecSlackIdx = xRecSlackIdx + 1;
}
#endif
/*-----------------------------------------------------------*/

#if ( configKERNEL_TEST == 2 )
/* Slack method cost measured in ceil/floor operations */
void vTaskGetTraceInfo( TaskHandle_t xTask, BaseType_t xCeilFloorCost )
{
	SsTCB_t *pxSsTCB = getTaskSsTCB( xTask );

	if ( cs_costs[ pxSsTCB->xId ][ 0 ] < RELEASE_COUNT )
	{
		cs_costs[ pxSsTCB->xId ][cs_costs[ pxSsTCB->xId ][0] + 1 ] = xCeilFloorCost;
		cs_costs[ pxSsTCB->xId ][ 0 ] += 1U;
	}
}
#endif
/*-----------------------------------------------------------*/

#if ( configKERNEL_TEST == 3 )
/* Measures the cost of prvTaskCalculateSlack() in cpu cycles */
void vTaskGetTraceInfo( TaskHandle_t xTask, const uint32_t cycles )
{
	SsTCB_t *pxSsTCB = getTaskSsTCB( xTask );

	if ( ( *cs_costs )[ pxSsTCB->xId ][ 0 ] < RELEASE_COUNT )
	{
		( *cs_costs )[ pxSsTCB->xId ][ (*cs_costs)[ pxSsTCB->xId ][0] + 1 ] = cycles;
		( *cs_costs )[ pxSsTCB->xId ][ 0 ] += 1U;
	}
}
#endif
/*-----------------------------------------------------------*/

#if ( configKERNEL_TEST == 4 )
/* Slack method cost measured in ceil/floor operations */
void vTaskGetTraceInfo( TaskHandle_t xTask, BaseType_t xLoopCost )
{
	SsTCB_t *pxSsTCB = getTaskSsTCB( xTask );

	if ( ( *cs_costs )[ pxSsTCB->xId ][ 0 ] < RELEASE_COUNT )
	{
		( *cs_costs )[ pxSsTCB->xId ][ (*cs_costs)[ pxSsTCB->xId ][0] + 1 ] = xLoopCost;
		( *cs_costs )[ pxSsTCB->xId ][ 0 ] += 1U;
	}
}
#endif
/*-----------------------------------------------------------*/
