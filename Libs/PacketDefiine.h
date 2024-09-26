#pragma once

#include "Define.h"

namespace C_Network
{

	enum : uint16
	{
		S_C_MOVE = 0,
		C_S_MOVE,

		S_C_LOGIN,
		C_S_LOGIN,
	};

	// size = header  + payload
	struct PacketHeader
	{
		uint16 size;
		uint16 type;
	};


}