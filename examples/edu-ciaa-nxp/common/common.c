#include "freertos.h"
#include "task.h"
#include "slack.h"
#include "utils.h"
#include "common.h"

/**
 * C++ version 0.4 char* style "itoa":
 * Written by Lukás Chmela
 * Released under GPLv3.
 */
char* itoa(int value, char* result, int base)
{
   // check that the base if valid
   if (base < 2 || base > 36) { *result = '\0'; return result; }

   char* ptr = result, *ptr1 = result, tmp_char;
   int tmp_value;

   do {
      tmp_value = value;
      value /= base;
      *ptr++ = "zyxwvutsrqponmlkjihgfedcba9876543210123456789abcdefghijklmnopqrstuvwxyz" [35 + (tmp_value - value * base)];
   } while ( value );

   // Apply negative sign
   if (tmp_value < 0) *ptr++ = '-';
   *ptr-- = '\0';
   while(ptr1 < ptr) {
      tmp_char = *ptr;
      *ptr--= *ptr1;
      *ptr1++ = tmp_char;
   }
   return result;
}

/* Sets up system hardware */
void prvSetupHardware(void)
{
	/* Inicializar la placa */
	boardConfig();

	/* Inicializar GPIOs */
	gpioConfig( 0, GPIO_ENABLE );

	/* Configuración de pines de salida para Leds de la CIAA-NXP */
	gpioConfig( LEDR, GPIO_OUTPUT );
	gpioConfig( LEDG, GPIO_OUTPUT );
	gpioConfig( LEDB, GPIO_OUTPUT );
	gpioConfig( LED1, GPIO_OUTPUT );
	gpioConfig( LED2, GPIO_OUTPUT );
	gpioConfig( LED3, GPIO_OUTPUT );

   /* Inicializar UART_USB a 115200 baudios */
   uartConfig( UART_USB, 115200 );
}

void printSlacks( char s, int32_t * slackArray, TickType_t xCur )
{
	/* Buffer */
	static char uartBuff[10];

	vTaskSuspendAll();
	vTasksGetSlacks( slackArray );

	uartWriteString( UART_USB, pcTaskGetTaskName(NULL) );
    uartWriteByte( UART_USB, '\t' );

    uartWriteByte( UART_USB, s );
    uartWriteByte( UART_USB, '\t' );

    itoa( slackArray[0], uartBuff, 10 );
    uartWriteString( UART_USB, uartBuff );
    uartWriteByte( UART_USB, '\t' );

    int i;
    for( i = 2; i <= 5; i++ ) {
    	itoa( slackArray[i], uartBuff, 10 );
    	uartWriteString( UART_USB, uartBuff );
    	uartWriteByte( UART_USB, '\t' );
    }

    itoa( xCur, uartBuff, 10 );
    uartWriteString( UART_USB, uartBuff );
    uartWriteByte( UART_USB, '\t' );

    uartWriteString( UART_USB, "\n\r" );

	xTaskResumeAll();
}

void periodicTaskBody( void* params )
{
    ( void ) params;

	SsTCB_t *pxTaskSsTCB;

#if( tskKERNEL_VERSION_MAJOR == 8 )
	pxTaskSsTCB = pxTaskGetTaskSsTCB( NULL );
#endif
#if( tskKERNEL_VERSION_MAJOR == 9 )
	pxTaskSsTCB = getTaskSsTCB( NULL );
#endif

    int32_t slackArray[ 6 ];

	for(;;)
    {
#ifdef TRACEALYZER_v3_1_3
        vTracePrintF( slack_channel, "%d - %d", xSlackSD, pxTaskSsTCB->xSlack );
#endif

	    gpioWrite( leds[ pxTaskSsTCB->xId - 1], ON);

		printSlacks( 'S', slackArray, pxTaskSsTCB->xCur );

#if ( configTASK_EXEC == 0 )
		vUtilsEatCpu( pxTaskSsTCB->xWcet - 300 );
#endif
#if ( configTASK_EXEC == 1 )
		while( pxTaskSsTCB->xCur <  ( pxTaskSsTCB->xWcet - 200 ) )
		{
			asm("nop");
		}
#endif

		printSlacks( 'E', slackArray, pxTaskSsTCB->xCur );

		gpioWrite( leds[ pxTaskSsTCB->xId - 1], OFF);

#ifdef TRACEALYZER_v3_1_3
		vTracePrintF( slack_channel, "%d - %d", xSlackSD, pxTaskSsTCB->xSlack );
#endif

		vTaskDelayUntil( &( pxTaskSsTCB->xPreviousWakeTime ), pxTaskSsTCB->xPeriod );
    }
}
