#include "FreeRTOS.h"
#include "task.h"
#include "slack.h"

void vTaskSetParams( TaskHandle_t xTask, const TickType_t xPeriod, const TickType_t xDeadline, const TickType_t xWcet, const BaseType_t xId )
{
	SsTCB_t *pxSsTCB = pvTaskGetThreadLocalStoragePointer( xTask, 0 );

	pxSsTCB->xPeriod = xPeriod;
	pxSsTCB->xDeadline = xDeadline;
	pxSsTCB->xWcet = xWcet;
	pxSsTCB->xA = xWcet;
	pxSsTCB->xB = xPeriod;
	pxSsTCB->xId = xId;
}
