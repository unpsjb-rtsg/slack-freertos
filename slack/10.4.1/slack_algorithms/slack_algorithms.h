/*
 * slack_algorithms.h
 *
 *  Created on: 22 jul. 2020
 *      Author: Francisco E. Páez
 */

#ifndef SLACK_V10_4_1_SLACK_ALGORITHMS_SLACK_ALGORITHMS_H_
#define SLACK_V10_4_1_SLACK_ALGORITHMS_SLACK_ALGORITHMS_H_

#include "FreeRTOS.h"

void vTaskCalculateSlack_alg( TaskHandle_t xTask, const TickType_t xTc,
        const List_t * pxTasksList );

#endif /* SLACK_V10_4_1_SLACK_ALGORITHMS_SLACK_ALGORITHMS_H_ */
