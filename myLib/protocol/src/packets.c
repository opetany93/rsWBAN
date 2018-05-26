/*
 * packets.c
 *
 *  Created on: May 25, 2018
 *      Author: arkri
 */
#include "packets.h"

//=======================================================================================
inline bool isPacketSync(sync_packet_t* packetPtr)
{
	return PACKET_sync == packetPtr->packetType;
}

//=======================================================================================
inline bool checkApprovals(sync_packet_t* packetPtr, uint8_t channel)
{
	return packetPtr->approvals & (1 << channel);
}

//=======================================================================================
inline bool isPacketInit(init_packet_t* packetPtr)
{
	return PACKET_init == packetPtr->packetType;
}

//=======================================================================================
inline bool isPacketData(data_packet_t* packetPtr)
{
	return PACKET_data == packetPtr->packetType;
}

