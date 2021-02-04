#include "FreeRTOS.h"
#include "task.h"
#include "slack_tests.h"

static BaseType_t ulDelayUntilFlag = pdFALSE;

uint32_t cs_costs[TASK_COUNT][RELEASE_COUNT + 2];

void vInitArray()
{
	/* Zeroes cs_cost[][] */
	for(int i = 0; i < TASK_COUNT; i++)
	{
		#if ( configKERNEL_TEST == 1 )
		for(int j = 0; j < RELEASE_COUNT + 2; j++)
		{
			//(*cs_costs)[i][j] = 0;
			cs_costs[i][j] = 0;
		}
		#endif
		#if ( configKERNEL_TEST == 2 || configKERNEL_TEST == 3 || configKERNELTRACE == 4 )
		for(int j = 0; j < RELEASE_COUNT + 1; j++)
		{
			//(*cs_costs)[i][j] = 0;
			cs_costs[i][j] = 0;
		}
		#endif
	}
}

#if ( configKERNEL_TEST == 1 ) && ( tskKERNEL_VERSION_MAJOR >= 9 )
void vMacroTaskDelay()
{
	STOPWATCH_RESET();
	vTaskGetTraceInfo( xTaskGetCurrentTaskHandle(), CPU_CYCLES, 0 );
    ulDelayUntilFlag = pdTRUE;
}
/*-----------------------------------------------------------*/

void vMacroTaskSwitched()
{
	if ( ulDelayUntilFlag == pdTRUE ) {
		vTaskGetTraceInfo( xTaskGetCurrentTaskHandle(), CPU_CYCLES, 1 );
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

#if ( configKERNEL_TEST == 1 )
//void vTaskGetTraceInfo( TaskHandle_t xTask, xType *pxArray, uint32_t time, uint32_t r )
void vTaskGetTraceInfo( TaskHandle_t xTask, uint32_t time, uint32_t r )
{
	SsTCB_t *pxSsTCB = getTaskSsTCB( xTask );

	//if ( (*pxArray)[pxSsTCB->xId][0] < RELEASE_COUNT )
	if ( cs_costs[pxSsTCB->xId][0] < RELEASE_COUNT )
	{
		if( r == 0 )
		{
			//(*pxArray)[pxSsTCB->xId][1] = 1;
			cs_costs[pxSsTCB->xId][1] = 1;
			//(*pxArray)[pxSsTCB->xId][ (*pxArray)[ pxSsTCB->xId ][0] + 2 ] = time;
			cs_costs[pxSsTCB->xId][ cs_costs[ pxSsTCB->xId ][0] + 2 ] = time;
		}
		else
		{
			//if( (*pxArray)[pxSsTCB->xId][1] == 1 )
			if( cs_costs[pxSsTCB->xId][1] == 1 )
			{
				//(*pxArray)[pxSsTCB->xId][ (*pxArray)[ pxSsTCB->xId ][0] + 2] = time - (*pxArray)[ pxSsTCB->xId ][ (*pxArray)[ pxSsTCB->xId ][0] + 2];
				//(*pxArray)[pxSsTCB->xId][1] = 0;
				//(*pxArray)[pxSsTCB->xId][0] += 1U;
				cs_costs[pxSsTCB->xId][ cs_costs[ pxSsTCB->xId ][0] + 2] = time - cs_costs[ pxSsTCB->xId ][ cs_costs[ pxSsTCB->xId ][0] + 2];
				cs_costs[pxSsTCB->xId][1] = 0;
				cs_costs[pxSsTCB->xId][0] += 1U;
			}
		}
	}
}
#endif
/*-----------------------------------------------------------*/

#if ( configKERNEL_TEST == 2 )
/* Slack method cost measured in ceil/floor operations */
void vTaskGetTraceInfo( TaskHandle_t xTask, BaseType_t xCeilFloorCost )
{
	SsTCB_t *pxSsTCB = getTaskSsTCB( xTask );

	if ( ( *cs_costs )[ pxSsTCB->xId ][ 0 ] < RELEASE_COUNT )
	{
		( *cs_costs )[ pxSsTCB->xId ][ (*cs_costs)[ pxSsTCB->xId ][0] + 1 ] = xCeilFloorCost;
		( *cs_costs )[ pxSsTCB->xId ][ 0 ] += 1U;
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
