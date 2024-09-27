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

C_Network::NetworkBase::NetworkBase(NetAddress netAddr, uint maxSessionCnt) : _netAddr(netAddr), _iocpHandle(nullptr)
{
	InitializeSRWLock(&_lock);
	WSAData wsa;
	
	int iRet = WSAStartup(MAKEWORD(2, 2), &wsa);

	if (iRet)
	{
		TODO_LOG_ERROR;
		CCrash(L"WSAStart Error");
	}

	// Print Success and ConcurrentCnt
	TODO_LOG_SUCCESS;
}

C_Network::NetworkBase::~NetworkBase() 
{
	WSACleanup();
}

void C_Network::NetworkBase::Begin()
{
	SYSTEM_INFO sys;
	GetSystemInfo(&sys);

	uint concurrentThreadCnt = sys.dwNumberOfProcessors;

	concurrentThreadCnt = concurrentThreadCnt * 0.7;
	if (concurrentThreadCnt == 0)
		concurrentThreadCnt = 1;


	_iocpHandle = CreateIoCompletionPort(INVALID_HANDLE_VALUE, 0, 0, concurrentThreadCnt);

	if (_iocpHandle == NULL)
	{
		TODO_LOG_ERROR;
		CCrash(L"Iocp Handle is Invalid");
	}

	_workerThreads.reserve(concurrentThreadCnt * 2);

	for (int i = 0; i < _workerThreads.capacity(); i++)
	{
		_workerThreads.push_back(std::thread([this]() {this->WorkerThread(); }));
	}
}

void C_Network::NetworkBase::End()
{
	CloseHandle(_iocpHandle);

	for (auto& t : _workerThreads)
	{
		if (t.joinable())
			t.join();
	}

	TODO_LOG_SUCCESS;
}

C_Network::SharedSession C_Network::NetworkBase::CreateSession(SOCKET sock, SOCKADDR_IN* pSockAddr)
{
	TODO_UPDATE_EX_LIST;

	SharedSession newSession = std::make_shared<Session>(sock, pSockAddr);
	bool bRet1;
	{
		SRWLockGuard lock(&_lock);
		bRet1 = _sessionMap.insert(std::pair<ULONGLONG, SharedSession>(newSession->GetId(), newSession)).second;
	}

	if (!bRet1)
	{
		TODO_LOG_ERROR;
		printf("Session Insert is Failed\n");
	}

	CreateIoCompletionPort(reinterpret_cast<HANDLE>(sock), _iocpHandle, reinterpret_cast<ULONG_PTR>(newSession.get()), 0);

	return newSession;
}

void C_Network::NetworkBase::AddSession()
{
	TODO_DEFINITION;
	TODO_UPDATE_EX_LIST;
}

void C_Network::NetworkBase::DeleteSession()
{
	TODO_DEFINITION;
	TODO_UPDATE_EX_LIST;
}

void C_Network::NetworkBase::Dispatch(C_Network::IocpEvent* iocpEvent, DWORD transferredBytes)
{
	SharedSession sessionPtr = iocpEvent->_owner;

	// 상호 참조 ( 이 경우는 순환 참조가 발생할 수 있어 NetworkBase에서 Session을 가지는 형태로 사용한다. )
	switch (iocpEvent->GetType())
	{
	case IocpEventType::Accept: AddSession(); sessionPtr->ProcessAccept(); break;
	case IocpEventType::Connect:AddSession(); sessionPtr->ProcessConnect(); break;
	case IocpEventType::Recv:sessionPtr->ProcessRecv(transferredBytes); break;
	case IocpEventType::Send:sessionPtr->ProcessSend(transferredBytes); break;
	case IocpEventType::Disconnect: sessionPtr->ProcessDisconnect(); DeleteSession(); break;
	default:break;
	}
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
			TODO_LOG_ERROR;
			printf("GQCS return is Null\n");
			PostQueuedCompletionStatus(_iocpHandle, 0, 0, nullptr);
			break;
		}

		Dispatch(iocpEvent, transferredBytes);
	}
	printf("Worker End\n");
}





	/*-----------------------
			NetServer
	-----------------------*/
C_Network::NetServer::NetServer(NetAddress netAddr, uint maxSessionCnt)
	: NetworkBase(netAddr, maxSessionCnt), _listenSock(INVALID_SOCKET)
{
}

C_Network::NetServer::~NetServer()
{
}

void C_Network::NetServer::Begin()
{
	NetworkBase::Begin();

	_listenSock = socket(AF_INET, SOCK_STREAM, 0);
	if (_listenSock == INVALID_SOCKET)
	{
		TODO_LOG_ERROR_WSA;
		CCrash(L"Listen socket is Invalid");
	}

	DWORD bindRet = bind(_listenSock, (SOCKADDR*)&_netAddr.GetSockAddr(), sizeof(SOCKADDR_IN));

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

	// TODO_
	_acceptThread = std::thread([this]() { this->AcceptThread(); });
}

void C_Network::NetServer::End()
{
	closesocket(_listenSock);
	_listenSock = INVALID_SOCKET;
	
	if (_acceptThread.joinable())
		_acceptThread.join();	
	
	NetworkBase::End();
}

bool C_Network::NetServer::OnConnectionRequest(SOCKADDR_IN* pClientInfo)
{
	TODO_UPDATE_EX_LIST;
	return true;
}

void C_Network::NetServer::OnClientJoin(SOCKADDR_IN st_clientInfo, ULONG_PTR dw_session_id)
{
	TODO_UPDATE_EX_LIST;
}

void C_Network::NetServer::OnClientLeave(ULONG_PTR dw_session_id)
{
	TODO_UPDATE_EX_LIST;
}

void C_Network::NetServer::OnError(int i_error_code, WCHAR* sz_display)
{
	TODO_UPDATE_EX_LIST;
}

void C_Network::NetServer::AcceptThread()
{
	while (1)
	{
		SOCKADDR_IN clientInfo;
		int infoSize = sizeof(clientInfo);
		SOCKET clientSock = accept(_listenSock, (SOCKADDR*)&clientInfo, &infoSize);

		if (clientSock == INVALID_SOCKET)
		{
			TODO_LOG; // EXIT LOG
			printf("Accept thread return\n");
			break;
  		}
		if (!OnConnectionRequest(&clientInfo))
		{
			TODO_DEFINITION // 거절한 이유와 해당 ip는 무엇인지 로그 기록
		}
		SharedSession newSession = CreateSession(clientSock, &clientInfo);

		newSession->ProcessAccept();
	}
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
