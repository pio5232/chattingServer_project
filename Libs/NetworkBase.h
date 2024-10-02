#pragma once
#include <unordered_map>
#include "Session.h"
#include <iostream>
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
		virtual void OnConnected(const SOCKADDR_IN& clientInfo, ULONGLONG sessionId); 
		virtual void OnDisconnected(ULONGLONG sessionId);
		virtual void OnError(int errCode, WCHAR* cause);
		virtual void OnRecv(C_Utility::CSerializationBuffer& buffer, ULONGLONG sessionId);

		SharedSession GetSession(ULONGLONG sessionId) 
		{
			SRWLockGuard lockGuard(&_lock);
			
			if (_sessionMap.find(sessionId) == _sessionMap.end())
				return nullptr;

			return _sessionMap[sessionId];
		}
		const NetAddress GetNetAddr() const { return _netAddr; }
	private:
		void ProcessAccept(SharedSession& sessionRef, DWORD transferredBytes = 0);
		void ProcessConnect(SharedSession& sessionRef, DWORD transferredBytes = 0);
		bool ProcessRecv(SharedSession& sessionRef, DWORD transferredBytes = 0);
		bool ProcessSend(SharedSession& sessionRef, DWORD transferredBytes = 0);
		bool ProcessDisconnect(SharedSession& sessionRef, DWORD transferredBytes = 0);
		
		void AddSession();
		void DeleteSession();
		void Dispatch(IocpEvent* iocpEvent, DWORD transferredBytes);
		void WorkerThread();
		HANDLE _iocpHandle;
		std::vector<std::thread> _workerThreads;

		std::unordered_map<ULONGLONG, SharedSession> _sessionMap; // [ Server - User Count / Client - Dummy Count]
		uint _maxSessionCnt;
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