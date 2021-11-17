#include "utils.h"
#include "task.h"

#define ONE_TICK ( ( configCPU_CLOCK_HZ / configTICK_RATE_HZ ) - 1UL )

/* The prototype shows it is a naked function - in effect this is just an
assembly function. */
void HardFault_Handler( void ) __attribute__( ( naked, aligned(8) ) );

/* The fault handler implementation calls a function called
prvGetRegistersFromStack(). */
void HardFault_Handler(void)
{
    __asm volatile
    (
        " tst lr, #4                                                \n"
        " ite eq                                                    \n"
        " mrseq r0, msp                                             \n"
        " mrsne r0, psp                                             \n"
        " ldr r1, [r0, #24]                                         \n"
        " ldr r2, handler2_address_const                            \n"
        " bx r2                                                     \n"
        " handler2_address_const: .word prvGetRegistersFromStack    \n"
    );
}

void prvGetRegistersFromStack( uint32_t *pulFaultStackAddress )
{
    /* These are volatile to try and prevent the compiler/linker optimising them
    away as the variables never actually get used.  If the debugger won't show the
    values of the variables, make them global my moving their declaration outside
    of this function. */
    __attribute__((unused)) volatile uint32_t r0;
    __attribute__((unused)) volatile uint32_t r1;
    __attribute__((unused)) volatile uint32_t r2;
    __attribute__((unused)) volatile uint32_t r3;
    __attribute__((unused)) volatile uint32_t r12;
    __attribute__((unused)) volatile uint32_t lr; /* Link register. */
    __attribute__((unused)) volatile uint32_t pc; /* Program counter. */
    __attribute__((unused)) volatile uint32_t psr;/* Program status register. */

    r0 = pulFaultStackAddress[ 0 ];
    r1 = pulFaultStackAddress[ 1 ];
    r2 = pulFaultStackAddress[ 2 ];
    r3 = pulFaultStackAddress[ 3 ];
    r12 = pulFaultStackAddress[ 4 ];
    lr = pulFaultStackAddress[ 5 ];
    pc = pulFaultStackAddress[ 6 ];
    psr = pulFaultStackAddress[ 7 ];

    // to avoid annoying "variable 'rX' set but not used"
    (void)(r0);
    (void)(r1);
    (void)(r2);
    (void)(r3);
    (void)(r12);
    (void)(lr);
    (void)(pc);
    (void)(psr);

    /* When the following line is hit, the variables contain the register values. */
    for( ;; );
}
/*-----------------------------------------------------------*/

void vUtilsBusyWait( TickType_t ticks )
{
    TickType_t elapsedTicks = 0;
    TickType_t currentTick = 0;
    while ( elapsedTicks < ticks ) {
        currentTick = xTaskGetTickCount();
        while ( currentTick == xTaskGetTickCount() ) {
            asm("nop");
        }
        elapsedTicks++;
    }
}
/*-----------------------------------------------------------*/
