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
		NetworkBase(const NetAddress& netAddr, uint maxSessionCnt);
		virtual ~NetworkBase() = 0;

		virtual void Begin();
		virtual void End();
		
		virtual bool OnConnectionRequest(const SOCKADDR_IN& clientInfo);
		virtual void OnConnected(const SOCKADDR_IN& clientInfo, ULONGLONG sessionId); // ... // accept 접속처리 완료 후 호출
		virtual void OnDisconnected(ULONGLONG sessionId);
		virtual void OnError(int errCode, WCHAR* cause);

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
		NetServer(const NetAddress& netAddr, uint maxSessionCnt);
		virtual ~NetServer();

		virtual void Begin() override final;
		virtual void End() override final;

	private:
		void AcceptThread();

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