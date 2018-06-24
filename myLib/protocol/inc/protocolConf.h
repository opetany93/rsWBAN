/*
 * conf.h
 *
 *  Created on: Jun 24, 2018
 *      Author: Arek Bochynski
 */

#ifndef PROTOCOL_CONF_H_
#define PROTOCOL_CONF_H_


// sensor defines
#define ATTEMPTS_OF_CONNECT		20		// number of attempts of connect sensor node to coordinator


//host defines
#define TICKS_RTC0_CHANNEL0		65 		// number of RTC ticks after which starts determining the time slots are by RTC1
#define TICKS_RTC0_CHANNEL1		1638	// number of RTC ticks defining the duration of the SUF
#define TICKS_OF_TIME_SLOT		392		// number of RTC ticks defining the duration of the time slot


// for both
#define BROADCAST_CHANNEL		30UL
#define FIRST_CHANNEL			0UL
#define SYNC_CHANNEL			29UL


#endif /* PROTOCOL_CONF_H_ */
