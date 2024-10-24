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
	// size = payload�� size
	// ��Ʈ��ũ �ڵ忡�� PacketHeader�� �и��ϰ� �Ǹ� PacketHeader��ŭ�� ������� ������� �ʴ´�.
	struct PacketHeader
	{
		uint16 size;
		//uint16 type; // EchoServer�� �׽�Ʈ�� ���� type�� ����.
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