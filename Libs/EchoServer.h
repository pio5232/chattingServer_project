#pragma once

#include "NetworkBase.h"

namespace C_Network
{
	/*--------------------------
			EchoServer
	--------------------------*/
	class EchoServer : public NetServer
	{
	public:
		EchoServer(const NetAddress& netAddr, uint maxSessionCnt);
		~EchoServer();

		virtual bool OnConnectionRequest(const SOCKADDR_IN& clientInfo);
		virtual void OnConnected(const SOCKADDR_IN& clientInfo, ULONGLONG sessionId);
		virtual void OnDisconnected(ULONGLONG sessionId);
		virtual void OnError(int errCode, WCHAR* cause);

	};
}