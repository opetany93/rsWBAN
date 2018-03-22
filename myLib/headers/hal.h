#ifndef HAL_H_
#define HAL_H_

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
#endif
