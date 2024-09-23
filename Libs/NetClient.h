#pragma once

#include "NetworkBase.h"

namespace C_Network
{
	class NetClient : public NetworkBase
	{
	public:
		NetClient(NetAddress destEndPoint, uint maxSessionCnt);
		virtual ~NetClient();
	};
}
