/*
 * rng.c
 *
 *  Created on: 19.03.2018
 *      Author: opetany
 */

#include "rng.h"
#include "nrf.h"
#include "mydefinitions.h"

uint32_t rnd8Bits()
{
	RNG->EVENTS_VALRDY = 0;

	while (RNG->EVENTS_VALRDY == 0);

	return RNG->VALUE;
}

void fillUint16TableWithRandomValues(uint16_t* table, uint16_t size)
{
	RNG->TASKS_START = 1;

	// fill table a random values
	for(uint16_t i = 0; i < size; i++)
	{
		table[i] = (rnd8Bits() << 8) | rnd8Bits();
	}

	RNG->TASKS_STOP = 1;
}

