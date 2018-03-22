/*
 * protocolTests.h
 *
 *  Created on: 19.03.2018
 *      Author: opetany
 */

#ifndef SOURCE_PROTOCOLTESTS_H_
#define SOURCE_PROTOCOLTESTS_H_

#include <stdint.h>

void testProtocolWithRadioPowerHandler(volatile uint8_t* packetLength, char* packet);	// <-- put in radio RX handler



#endif /* SOURCE_PROTOCOLTESTS_H_ */
