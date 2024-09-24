#pragma once

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
		HANDLE _iocpHandle;
		std::vector<std::thread> _workerThreads;

		// NetServer -> listen EndPoint
		// NetClient -> dest EndPoint
		const NetAddress _netAddr; 
	};


}