#pragma once

#include "Define.h"

namespace C_Network
{
	const int ServerPort = 6000;

	const int MAX_PACKET_SIZE = 1000;
	TODO_DEFINITION;
	enum PacketType: uint16 // packet order
	{
		S_C_MOVE = 0,
		S_C_LOGIN = 1,

		C_S_MOVE = 100,
		C_S_LOGIN = 101,
	
		PACKET_TYPE_ECHO = 65535,
	};

	enum PacketSize
	{
		PACKET_SIZE_ECHO = 10,

		PACKET_SIZE_MAX = 500,
	};
	// size = payload의 size
	// 네트워크 코드에서 PacketHeader를 분리하게 되면 PacketHeader만큼의 사이즈는 사용하지 않는다.
	struct PacketHeader
	{
		uint16 size;
		//uint16 type; // EchoServer를 테스트할 때는 type이 없다.
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