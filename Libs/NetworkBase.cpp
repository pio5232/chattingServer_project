#include "LibsPch.h"
#include "NetworkBase.h"
#include <WS2tcpip.h>

	/*--------------------
		  NetAddress
	--------------------*/

C_Network::NetAddress::NetAddress(SOCKADDR_IN sockAddr) :_sockAddr(sockAddr) {}


C_Network::NetAddress::NetAddress(std::wstring ip, uint16 port)
{
	memset(&_sockAddr, 0, sizeof(_sockAddr));

	_sockAddr.sin_family = AF_INET;
	_sockAddr.sin_port = htons(port);
	_sockAddr.sin_addr = IpToAddr(ip.c_str());
}

C_Network::NetAddress::NetAddress(const NetAddress& other) : _sockAddr(other._sockAddr) {}

std::wstring C_Network::NetAddress::GetIpAddress()
{
	WCHAR ipWstr[100];

	// 주소체계, &IN_ADDR
	InetNtopW(AF_INET, &_sockAddr.sin_addr, ipWstr, sizeof(ipWstr) / sizeof(WCHAR));

	return std::wstring(ipWstr);
}

IN_ADDR C_Network::NetAddress::IpToAddr(const WCHAR* ip)
{
	IN_ADDR addr;
	InetPtonW(AF_INET, ip, &addr);
	return addr;
}



	/*-----------------------
		  NetworkBase
	-----------------------*/

C_Network::NetworkBase::NetworkBase(NetAddress netAddr, uint maxSessionCnt) : _netAddr(netAddr), isWorkerRunning(true)
{
	SYSTEM_INFO sys;
	GetSystemInfo(&sys);
	
	uint concurrentThreadCnt = sys.dwNumberOfProcessors;

	concurrentThreadCnt = concurrentThreadCnt * 0.7;
	if (concurrentThreadCnt == 0)
		concurrentThreadCnt = 1;

	Init();

	_iocpHandle = CreateIoCompletionPort(INVALID_HANDLE_VALUE, 0, 0, concurrentThreadCnt);

	if (_iocpHandle == NULL)
	{
		TODO_LOG_ERROR;
		CCrash(L"Iocp Handle is Invalid");
	}

	// max operating count = Concurrent * 2
	_workerThreads.reserve(concurrentThreadCnt * 2);

	for (int i = 0; i < _workerThreads.capacity(); i++)
	{
		_workerThreads[i] = std::thread([this]() {WorkerThread(); });
	}

	// Print Success and ConcurrentCnt
	TODO_LOG_SUCCESS;
}

C_Network::NetworkBase::~NetworkBase() 
{
	WSACleanup();

	for (auto& t : _workerThreads)
	{
		if (t.joinable())
			t.join();
	}

	TODO_LOG_SUCCESS;

	CloseHandle(_iocpHandle);
}


void C_Network::NetworkBase::WorkerThread()
{
	IocpEvent *iocpEvent;
	DWORD transferredBytes;

	Session* pSession;
	while (1)
	{
		iocpEvent = nullptr;
		transferredBytes = 0;
		pSession = nullptr;

		bool gqcsRet = GetQueuedCompletionStatus(_iocpHandle, &transferredBytes, 
			reinterpret_cast<PULONG_PTR>(&pSession), reinterpret_cast<LPOVERLAPPED*>(&iocpEvent), INFINITE);

		// if ret == false => pOverlapped = nullptr;
		if(!iocpEvent)
		{
			TODO_LOG_ERROR_WSA;
			printf("GQCS return is Null\n");
			isWorkerRunning = false;
			continue;
		}

		// if Transferred == 0  => recv 0 
		SharedIocpBase iocpObjRef = iocpEvent->_owner;
		
		iocpObjRef->Dispatch(iocpEvent, transferredBytes);
	}

}

void C_Network::NetworkBase::Init()
{
	WSAData wsa;
	int iRet = WSAStartup(MAKEWORD(2, 2), &wsa);

	if (iRet)
	{
		TODO_LOG_ERROR;
		CCrash(L"WSAStart Error");
	}

}





	/*-----------------------
			NetServer
	-----------------------*/
C_Network::NetServer::NetServer(NetAddress netAddr, uint maxSessionCnt)
	: NetworkBase(netAddr, maxSessionCnt)
{
	_listenSock = socket(AF_INET, SOCK_STREAM, 0);
	if (_listenSock == INVALID_SOCKET)
	{
		TODO_LOG_ERROR_WSA;
		CCrash(L"Listen socket is Invalid");
	}

	DWORD bindRet = bind(_listenSock, (SOCKADDR*)&netAddr.GetSockAddr(), sizeof(SOCKADDR_IN));

	if (bindRet == SOCKET_ERROR)
	{
		TODO_LOG_ERROR_WSA;
		CCrash(L"Bind Error\n");
	}

	DWORD listenRet = ::listen(_listenSock, SOMAXCONN);
	if (listenRet == SOCKET_ERROR)
	{
		TODO_LOG_ERROR_WSA;
		CCrash(L"listen Error\n");
	}
}

C_Network::NetServer::~NetServer()
{
}


	/*-----------------------
			NetClient
	-----------------------*/
C_Network::NetClient::NetClient(NetAddress destEndPoint, uint maxSessionCnt)
	: NetworkBase(destEndPoint, maxSessionCnt)
{

}

C_Network::NetClient::~NetClient()
{
}
