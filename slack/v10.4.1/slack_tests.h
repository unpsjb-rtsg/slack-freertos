#ifndef SLACK_V10_4_1_SLACK_TESTS_H_
#define SLACK_V10_4_1_SLACK_TESTS_H_

#include "slack.h"

#if defined (__cplusplus)
extern "C" {
#endif

/* === delay_until() cost ================================================== */
#if configKERNEL_TEST == 1
uint32_t ulDelayTime;
uint32_t ulDelayTime1;

typedef uint32_t xType[TASK_COUNT][RELEASE_COUNT + 2];
void vTaskGetTraceInfo( TaskHandle_t xTask, xType *pxArray, uint32_t time, uint32_t r );
#endif
/* ========================================================================= */

/* === slack methods ceil/floor cost ======================================= */
#if configKERNEL_TEST == 2
typedef uint32_t xType[TASK_COUNT][RELEASE_COUNT + 1];
//BaseType_t xCeilFloorCost = 0;
extern xType *cs_costs;
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

#if ( configUSE_SLACK_STEALING == 0 ) && ( configKERNEL_TEST == 1 )
    /* Set the Id */
    void vTaskSetParams( TaskHandle_t xTask, const BaseType_t xId ) PRIVILEGED_FUNCTION;
#endif

#if defined (__cplusplus)
}
#endif

#endif /* SLACK_V10_4_1_SLACK_TESTS_H_ */
