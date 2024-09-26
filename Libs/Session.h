#pragma once

#include "PacketDefiine.h"
namespace C_Network
{
	/*----------------------
			IocpEvent
	----------------------*/
	enum class IocpEventType : unsigned char
	{
		Recv,
		Send,
		Accept,
		Connect,
		Disconnect,
		// PreRecv, 0 byte Recv
	};

	class IocpEvent : private OVERLAPPED
	{
	public:
		IocpEvent(IocpEventType type);
		void Init();
		inline const IocpEventType GetType() const { return _type; }

		SharedIocpBase _owner;

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


	/*----------------------
			IocpObjBase
	----------------------*/
	class IocpObjBase : public std::enable_shared_from_this<IocpObjBase>
	{
	public:
		//virtual HANDLE GetHandle() abstract = 0;
		virtual void Dispatch(class IocpEvent* iocpEvent, int numOfBytes = 0) abstract = 0;
	};


	/*------------------------------
				Session
	------------------------------*/
	class Session : public IocpObjBase
	{
	public:
		Session(SOCKET sock);
		~Session();

		void Dispatch(class IocpEvent* iocpEvent, int numOfBytes = 0);

		void Disconnect();

		bool ProcessRecv(DWORD transferredBytes);
		bool ProcessSend(DWORD transferredBytes);
		bool ProcessConnect();
		bool ProcessAccept();
		bool ProcessDisconnect();


		// Regist.
		void PostSend();
		void PostRecv();
		void PostConnect();
		void PostDisconnect();

		uint OnRecv();
		
		virtual void OnRecvPacket(char* buffer, uint len);
		virtual void OnConnected();
		virtual void OnDisconnected();

	private:
		SOCKET _socket;
		NetAddress _netAddr;
		ULONGLONG _sessionId;

		RecvBuffer _recvBuffer;

		RecvEvent _recvEvent;
		SendEvent _sendEvent;

		volatile char _isConnected;
	};
}
