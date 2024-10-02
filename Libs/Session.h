#pragma once

#include <queue>
#include <functional>
#include "PacketDefiine.h"

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
		//inline const IocpEventType GetType() const { return _type; }

		SharedSession _owner;

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
				Session (only Use Server)
	------------------------------*/
	struct Session : std::enable_shared_from_this<Session>
	{
	public:
		//using OnRecvFunc = std::function<void(char*, uint)>;

		Session(SOCKET sock, SOCKADDR_IN* pSockAddr, class NetworkBase* netPtr);
		~Session();

		void Send(SharedSendBuffer sendBuf);
		void Disconnect();

		bool ProcessRecv(DWORD transferredBytes);
		bool ProcessSend(DWORD transferredBytes);
		bool ProcessConnect();
		bool ProcessAccept();
		bool ProcessDisconnect();

		//virtual void OnConnected();
		//virtual void OnDisconnected();
		//virtual void OnSend();

		const NetAddress& GetNetAddr() const { return _netAddr; }
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

		volatile char _isConnected;
		volatile char _sendFlag; // Use - 1, unUse - 0
	public:
		C_Utility::CRingBuffer _recvBuffer;
		RecvEvent _recvEvent;
		SendEvent _sendEvent;
		ConnectEvent _connectEvent;
		DisconnectEvent _disconnEvent;

	};
}
