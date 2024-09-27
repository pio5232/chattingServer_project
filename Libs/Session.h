#pragma once

#include <queue>
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

	class IocpEvent : private OVERLAPPED
	{
	public:
		IocpEvent(IocpEventType type);
		void Reset();
		inline const IocpEventType GetType() const { return _type; }

		SharedSession _owner;

	private:
		const IocpEventType _type;
	};


	class RecvEvent : public IocpEvent
	{
	public:
		RecvEvent() : IocpEvent(IocpEventType::Recv) {}
	};

	class SendEvent : public IocpEvent
	{
	public:
		SendEvent() : IocpEvent(IocpEventType::Send) {}
	};

	class ConnectEvent : public IocpEvent
	{
	public:
		ConnectEvent() : IocpEvent(IocpEventType::Connect) {}
	};

	class DisconnectEvent : public IocpEvent
	{
	public:
		DisconnectEvent() : IocpEvent(IocpEventType::Disconnect) {}
	};


	/*------------------------------
				Session
	------------------------------*/
	class Session :public std::enable_shared_from_this<Session>
	{
	public:
		Session(SOCKET sock, SOCKADDR_IN* pSockAddr);
		~Session();

		void Send(SharedSendBuffer sendBuf);
		void Disconnect();

		bool ProcessRecv(DWORD transferredBytes);
		bool ProcessSend(DWORD transferredBytes);
		bool ProcessConnect();
		bool ProcessAccept();
		bool ProcessDisconnect();

		virtual void OnRecvPacket(char* buffer, uint len);
		virtual void OnConnected();
		virtual void OnDisconnected();
		virtual void OnSend();

		ULONGLONG GetId () const { return _sessionId; }

	private:
		uint OnRecv();

		// Regist.
		void PostSend();
		void PostRecv();
		void PostConnect();
		void PostDisconnect();
		void PostAccept();

		SRWLOCK _sendBufferLock;
		SOCKET _socket;
		NetAddress _netAddr;
		ULONGLONG _sessionId;

		RecvBuffer _recvBuffer;
		std::queue<SharedSendBuffer> _sendBufferQ;

		RecvEvent _recvEvent;
		SendEvent _sendEvent;
		ConnectEvent _connectEvent;
		DisconnectEvent _disconnEvent;

		volatile char _isConnected;
		volatile char _sendFlag; // Use - 1, unUse - 0
	};
}
