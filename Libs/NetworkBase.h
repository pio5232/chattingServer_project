#pragma once
#include <unordered_map>
#include "Session.h"
namespace C_Network
{
	/*-----------------------
			NetworkBase
	-----------------------*/
	class NetworkBase
	{
	public:
		NetworkBase(NetAddress netAddr, uint maxSessionCnt);
		virtual ~NetworkBase() = 0;

		virtual void Start() {};
		virtual void Create() abstract = 0;
		virtual void Connect() abstract = 0;
		virtual void Disconnect() {}
		virtual void Send() abstract = 0;

		//const NetAddress& GetNetAddr() const { return _netAddr; }
	private:
		void WorkerThread();
		void Init();
		const NetAddress _netAddr; 
		HANDLE _iocpHandle;
		std::vector<std::thread> _workerThreads;

		// NetServer -> listen EndPoint
		// NetClient -> dest EndPoint
	};



	/*-----------------------
			NetServer
	-----------------------*/
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

		void AcceptThread();

	private:
		SOCKET _listenSock;
		std::thread _acceptThread;
		std::unordered_map<ULONGLONG, SharedSession> _sessionMap;
	};



	/*-----------------------
			NetClient
	-----------------------*/
	class NetClient : public NetworkBase
	{
	public:
		NetClient(NetAddress destEndPoint, uint maxSessionCnt);
		virtual ~NetClient();
	};
}