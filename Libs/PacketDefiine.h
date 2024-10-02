#pragma once

#include "Define.h"

namespace C_Network
{
	const int ServerPort = 6000;

	const int MAX_PACKET_SIZE = 1000;
	TODO_DEFINITION;
	enum : uint16 // packet order
	{
		S_C_MOVE = 0,
		C_S_MOVE,

		S_C_LOGIN,
		C_S_LOGIN,
	};

	// size = header  + payload
	// header must not have padding
	struct PacketHeader
	{
		uint16 size;
		//uint16 type;
	};

	struct EchoPacketHeader
	{
		uint16 size;
	};

	struct EchoPacket
	{
		__int64 data;
	};

}