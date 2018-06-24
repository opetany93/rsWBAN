#ifndef HAL_H_
#define HAL_H_

#include <stdint.h>

#define SYSTEM_OFF()				NRF_POWER->SYSTEMOFF = 1U

#define PREEMPTION_PRIORITY_BITS 	2

// ============ for 2 bits preemption ====================
#define HIGH_IRQ_PRIO 				1
#define MIDDLE_IRQ_PRIO 			2
#define LOW_IRQ_PRIO 				3

// =================================== Functions ==========================================
void boardInit(void);
void buttonInterruptInit(void);
void error(void);
void gpioGeneratePulse(uint8_t pin);
void sleep(void);

#endif
