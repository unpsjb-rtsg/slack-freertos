#ifndef SLACK_V10_4_1_SLACK_TESTS_H_
#define SLACK_V10_4_1_SLACK_TESTS_H_

#include "slack.h"

#if defined (__cplusplus)
extern "C" {
#endif

extern uint32_t cs_costs[TASK_COUNT][RELEASE_COUNT + 1];

void vInitArray();

/* === slack methods ceil/floor cost ======================================= */
#if configKERNEL_TEST == 2
void vTaskGetTraceInfo( TaskHandle_t xTask, BaseType_t xCeilFloorCost );
#endif
/* ========================================================================= */

/* === prvTaskCalculateSlack cost ========================================== */
#if configKERNEL_TEST == 3
typedef uint32_t xType[TASK_COUNT][RELEASE_COUNT + 1];
extern xType *cs_costs;
void vTaskGetTraceInfo( TaskHandle_t xTask, const uint32_t cycles );
#endif
/* ========================================================================= */

/* === prvTaskCalculateSlack cost measured in loops ======================== */
#if configKERNEL_TEST == 4
typedef uint32_t xType[TASK_COUNT][RELEASE_COUNT + 1];
//BaseType_t xLoopCost = 0;
extern xType *cs_costs;
void vTaskGetTraceInfo( TaskHandle_t xTask, BaseType_t xLoopCost );
#endif
/* ========================================================================= */

#if ( configDO_SLACK_TRACE == 1)
    extern xType *xResults;
    extern int xRecSlackIdx;
#endif

#if defined (__cplusplus)
}
#endif

#endif /* SLACK_V10_4_1_SLACK_TESTS_H_ */
