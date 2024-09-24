#pragma once

#include "NetworkBase.h"
namespace C_Network
{
	class NetServer : public NetworkBase
	{
	public:
		NetServer(NetAddress netAddr, uint maxSessionCnt);
		virtual ~NetServer();
		virtual bool OnConnectionRequest(IN_ADDR ip, USHORT us_port) abstract = 0;
		virtual void OnClientJoin(SOCKADDR_IN st_clientInfo, ULONG_PTR dw_session_id) = 0; // ... // accept 접속처리 완료 후 호출
		virtual void OnClientLeave(ULONG_PTR dw_session_id) = 0;

		//virtual void OnRecv(ULONG_PTR dw_session_id, CSerializationBuffer* p_packet) = 0;

		virtual void OnError(int i_error_code, WCHAR* sz_display) = 0;
		
	private:
		SOCKET _listenSock;
		std::thread _acceptThread;
	};

}

