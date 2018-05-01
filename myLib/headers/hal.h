#ifndef HAL_H_
#define HAL_H_

#include <stdint.h>

#if defined ( __GNUC__ ) && !defined (__CC_ARM) /* GNU Compiler */
  #ifndef __weak
    #define __weak   __attribute__((weak))
  #endif /* __weak */
  #ifndef __packed
    #define __packed __attribute__((__packed__))
  #endif /* __packed */
#endif /* __GNUC__ */

#define PREEMPTION_PRIORITY_BITS 	2

#define NVIC_PRIORITYGROUP_3 		(uint32_t)0x00000004		//deprecated, use like is in devKit board

// ============ for 2 bits preemption ====================
#define HIGH_IRQ_PRIO 				1
#define MIDDLE_IRQ_PRIO 			2
#define LOW_IRQ_PRIO 				3

// =================================== Functions ==========================================
void boardInit(void);
void buttonInterruptInit(void);
void error(void);
void gpioGeneratePulse(uint8_t pin);
#endif
