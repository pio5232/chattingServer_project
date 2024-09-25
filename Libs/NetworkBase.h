#pragma once
#include "Session.h"
namespace C_Network
{
	/*--------------------
		  NetAddress
	--------------------*/
	class NetAddress
	{
	public:
		NetAddress(SOCKADDR_IN sockAddr);
		NetAddress(std::wstring ip, uint16 port);
		NetAddress(const NetAddress& other);

		inline const SOCKADDR_IN& GetSockAddr() const { return _sockAddr; }
		std::wstring	GetIpAddress();
		inline const uint16 GetPort() const { return ntohs(_sockAddr.sin_port); }

		static IN_ADDR IpToAddr(const WCHAR* ip);
	private:
		SOCKADDR_IN _sockAddr = {};
	};


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

		bool isWorkerRunning;
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