#pragma once

#include <queue>
#include <stack>
#include <functional>
#include "PacketDefiine.h"
#include <unordered_map>
#include <unordered_set>


namespace C_Network
{
	/*----------------------
			IocpEvent
	----------------------*/
	enum class IocpEventType : unsigned char
	{
		Accept,
		Connect,
		Recv,
		Send,
		Disconnect,
		EventMax,
		// PreRecv, 0 byte Recv
	};
	
	struct IocpEvent : private OVERLAPPED
	{
	public:
		IocpEvent(IocpEventType type);
		void Reset();

		class Session* _owner;

		const IocpEventType _type;
	};


	struct RecvEvent : public IocpEvent
	{
	public:
		RecvEvent() : IocpEvent(IocpEventType::Recv) {}
	};

	struct SendEvent : public IocpEvent
	{
	public:
		SendEvent() : IocpEvent(IocpEventType::Send) { _pendingBuffs.reserve(10); }
		std::vector<SharedSendBuffer> _pendingBuffs;
	};

	struct ConnectEvent : public IocpEvent
	{
	public:
		ConnectEvent() : IocpEvent(IocpEventType::Connect) {}
	};

	struct DisconnectEvent : public IocpEvent
	{
	public:
		DisconnectEvent() : IocpEvent(IocpEventType::Disconnect) {}
	};


	/*------------------------------
				Session 
	------------------------------*/
	struct Session 
	{
	public:
		Session();
		Session(SOCKET sock, SOCKADDR_IN* pSockAddr);
		~Session();

		void Init(SOCKET sock, SOCKADDR_IN* pSockAddr);
		void Send(SharedSendBuffer sendBuf);
		bool CanDisconnect(); // true - Disconn, false - connecting.

		//bool ProcessRecv(DWORD transferredBytes);
		C_Network::NetworkErrorCode ProcessSend(DWORD transferredBytes);
		bool ProcessConnect();
		bool ProcessAccept();
		bool ProcessDisconnect();

		const NetAddress& GetNetAddr() const { return _netAddr; }
		SOCKET GetSock() const { return _socket; }
		ULONGLONG GetId () const { return _sessionId; }

		// Regist.
		void PostSend();
		void PostRecv();
		void PostConnect();
		void PostDisconnect();
		void PostAccept();
	private:

		SRWLOCK _sendBufferLock;
		SOCKET _socket;
		NetAddress _netAddr;
		ULONGLONG _sessionId;

		std::queue<SharedSendBuffer> _sendBufferQ;

		//volatile char _isConnected;
		volatile char _sendFlag; // Use - 1, unUse - 0
	    volatile ULONG _ioCount;
	public:
		C_Utility::CRingBuffer _recvBuffer;
		RecvEvent _recvEvent;
		SendEvent _sendEvent;
		ConnectEvent _connectEvent;
		DisconnectEvent _disconnEvent;


	};


	/*--------------------------------------
				Session Manager
	--------------------------------------*/

	class SessionManager
	{
	public:
		SessionManager(uint maxSessionCnt);
		Session* AddSession(SOCKET sock, SOCKADDR_IN* pSockAddr);
		void DeleteSession(Session* sessionPtr);

		Session* GetSession(ULONGLONG sessionId); // sessionId로 session을 찾는다.

		bool IsFull() 
		{
			SRWLockGuard lockGuard(&_indexListLock); 
			return _availableSessionidxList.empty(); 
		}


	private:

		// Accept에서는 차지 않았다가 차는 경우가 존재하지 않는다.
		uint GetAvailableIndex();
		// [ Server - User Count / Client - Dummy Count], <id, index>
		std::unordered_map<ULONGLONG, uint> _idToIndexMap; 
		std::unordered_map<uint, ULONGLONG> _indexToIdMap;

		std::vector<Session*> _sessionArr;
		std::stack<uint> _availableSessionidxList;

		SRWLOCK _mapLock;
		SRWLOCK _indexListLock;
		uint _maxSessionCnt;

		volatile ULONG _curSessionCnt;
		
	};
}
