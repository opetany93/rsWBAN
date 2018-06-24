/*
 * rng.h
 *
 *  Created on: 19.03.2018
 *      Author: opetany
 */

#ifndef SOURCE_RNG_H_
#define SOURCE_RNG_H_

#include <stdint.h>

uint32_t rnd8(void);
void fillUint16TableWithRandomValues(uint16_t* table, uint16_t size);


#endif /* SOURCE_RNG_H_ */
