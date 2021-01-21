/*
 * chip.h
 *
 *  Created on: 21 ene. 2021
 *      Author: Francisco
 */

#ifndef EXAMPLES_LM3S6965EVB_CHIP_H_
#define EXAMPLES_LM3S6965EVB_CHIP_H_

/* Definitions that would come from the processor header file. */
#define __CORTEX_M                  (0x03)
#define __CM3_REV                   0x0202
#define __MPU_PRESENT               1
#define __NVIC_PRIO_BITS            8 /* Silicon has three, QEMU has 8. */

__attribute__( ( always_inline ) ) static inline uint32_t __get_PRIMASK(void)
{
  uint32_t result;

  __asm volatile ("MRS %0, primask" : "=r" (result) );
  return(result);
}

__attribute__( ( always_inline ) ) static inline void __set_PRIMASK(uint32_t priMask)
{
  __asm volatile ("MSR primask, %0" : : "r" (priMask) : "memory");
}


#endif /* EXAMPLES_LM3S6965EVB_CHIP_H_ */
