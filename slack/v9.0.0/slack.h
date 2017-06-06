#ifndef SLACK_H
#define SLACK_H

#include "task.h"

#ifdef __cplusplus
extern "C" {
#endif

#if ( configUSE_SLACK_STEALING == 1 )

void vTaskSetParams( TaskHandle_t xTask, const TickType_t xPeriod, const TickType_t xDeadline, const TickType_t xWcet, const BaseType_t xId ) PRIVILEGED_FUNCTION;
void vTasksGetSlacks( int32_t *taskSlackArray ) PRIVILEGED_FUNCTION;

#if ( configDO_SLACK_TRACE == 1)
    extern xType *xResults;
    extern int xRecSlackIdx;
#endif

#if ( configKERNEL_TEST == 2 ) || ( configKERNEL_TEST == 3 ) || ( configKERNEL_TEST == 4 )
    extern xType *cs_costs;
#endif

#endif /* configUSE_SLACK_STEALING */

#if ( configUSE_SLACK_STEALING == 0 ) && ( configKERNEL_TEST == 1 )
    /* Set the Id */
    void vTaskSetParams( TaskHandle_t xTask, const BaseType_t xId ) PRIVILEGED_FUNCTION;
#endif

#ifdef __cplusplus
}
#endif

#endif /* SLACK_H */
