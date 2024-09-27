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

		virtual void Begin();
		virtual void End();
		
		//virtual void Create() abstract = 0;
		//virtual void Connect() abstract = 0;
		//virtual void Disconnect() {}
		//virtual void Send() abstract = 0;

	private:
		void AddSession();
		void DeleteSession();
		void Dispatch(IocpEvent* iocpEvent, DWORD transferredBytes);
		void WorkerThread();
		HANDLE _iocpHandle;
		std::vector<std::thread> _workerThreads;

		std::unordered_map<ULONGLONG, SharedSession> _sessionMap; // [ Server - User Count / Client - Dummy Count]

	protected:
		SharedSession CreateSession(SOCKET sock, SOCKADDR_IN* pSockAddr);
		SRWLOCK _lock;
		const NetAddress _netAddr; 
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

		virtual void Begin() override;
		virtual void End() override;
		virtual bool OnConnectionRequest(SOCKADDR_IN* pClientInfo);
		virtual void OnClientJoin(SOCKADDR_IN st_clientInfo, ULONG_PTR dw_session_id); // ... // accept 접속처리 완료 후 호출
		virtual void OnClientLeave(ULONG_PTR dw_session_id);

		//virtual void OnRecv(ULONG_PTR dw_session_id, CSerializationBuffer* p_packet) = 0;
		virtual void OnError(int i_error_code, WCHAR* sz_display);

	private:
		void AcceptThread();
		//virtual void Start() override;
		//virtual void Create() override;
		//virtual void Connect() override;
		//virtual void Disconnect() override;
		//virtual void Send() override;

	private:
		SOCKET _listenSock;
		std::thread _acceptThread;
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