#ifndef SLACK_V9_0_0_SLACK_TESTS_H_
#define SLACK_V9_0_0_SLACK_TESTS_H_

#include "slack.h"

#if ( configDO_SLACK_TRACE == 1)
    extern xType *xResults;
    extern int xRecSlackIdx;
#endif

void vTaskGetTraceInfo( SsTCB_t *pxSsTCB );

#if ( configKERNEL_TEST == 2 )
BaseType_t xCeilFloorCost = 0;
#endif

#if ( configKERNEL_TEST == 4 )
BaseType_t xLoopCost = 0;
#endif

#if ( configKERNEL_TEST == 2 ) || ( configKERNEL_TEST == 3 ) || ( configKERNEL_TEST == 4 )
    extern xType *cs_costs;
#endif

#if ( configUSE_SLACK_STEALING == 0 ) && ( configKERNEL_TEST == 1 )
    /* Set the Id */
    void vTaskSetParams( TaskHandle_t xTask, const BaseType_t xId ) PRIVILEGED_FUNCTION;
#endif

#endif /* SLACK_V9_0_0_SLACK_TESTS_H_ */
