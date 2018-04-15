/*
 * protocolTests.c
 *
 *  Created on: 19.03.2018
 *      Author: opetany
 */

#include "protocolTests.h"
#include "nrf.h"
#include "mydefinitions.h"
#include "protocol.h"

//void testProtocolWithRadioPowerHandler(volatile uint8_t* packetLength, char* packet)
//{
//	*packetLength = ((sync_packet_t *)packet)->testPacketLength;
//
//	if ( 1 == ((sync_packet_t *)packet)->turnOff )
//	{
//		NRF_POWER->SYSTEMOFF = 1U;
//	}
//
//	switch(((sync_packet_t *)packet)->txPower)
//	{
//		case 1:
//		{
//			RADIO->TXPOWER = RADIO_TXPOWER_TXPOWER_Neg20dBm;
//			break;
//		}
//
//		case 2:
//		{
//			RADIO->TXPOWER = RADIO_TXPOWER_TXPOWER_Neg16dBm;
//			break;
//		}
//
//		case 3:
//		{
//			RADIO->TXPOWER = RADIO_TXPOWER_TXPOWER_Neg12dBm;
//			break;
//		}
//
//		case 4:
//		{
//			RADIO->TXPOWER = RADIO_TXPOWER_TXPOWER_Neg8dBm;
//			break;
//		}
//
//		case 5:
//		{
//			RADIO->TXPOWER = RADIO_TXPOWER_TXPOWER_Neg4dBm;
//			break;
//		}
//
//		case 6:
//		{
//			RADIO->TXPOWER = RADIO_TXPOWER_TXPOWER_0dBm;
//			break;
//		}
//
//		case 7:
//		{
//			RADIO->TXPOWER = RADIO_TXPOWER_TXPOWER_Pos3dBm;
//			break;
//		}
//
//		case 8:
//		{
//			RADIO->TXPOWER = RADIO_TXPOWER_TXPOWER_Pos4dBm;
//			break;
//		}
//	}
//}
