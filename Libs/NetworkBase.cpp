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

const std::wstring C_Network::NetAddress::GetIpAddress() const
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

C_Network::NetworkBase::NetworkBase(const NetAddress& netAddr, uint maxSessionCnt) : _netAddr(netAddr),_maxSessionCnt(maxSessionCnt), _iocpHandle(nullptr)
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
	
	SharedSession newSession = std::make_shared<Session>(sock, pSockAddr, this);
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
	SharedSession session = iocpEvent->_owner;

	// 상호 참조 ( 이 경우는 순환 참조가 발생할 수 있어 NetworkBase에서 Session을 가지는 형태로 사용한다. )
	switch (iocpEvent->_type)
	{
	case IocpEventType::Accept:
		ProcessAccept(session);
		break;

	case IocpEventType::Connect:
		ProcessConnect(session);
		//OnConnected(sessionPtr->GetNetAddr().GetSockAddr(), sessionPtr->GetId());
		break;

	case IocpEventType::Recv:
		ProcessRecv(session, transferredBytes);
		break;

	case IocpEventType::Send:
		ProcessSend(session, transferredBytes);
		break;

	case IocpEventType::Disconnect: 
		ProcessDisconnect(session, transferredBytes);
		//OnDisconnected(sessionPtr->GetId());
		break;
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
			PostQueuedCompletionStatus(_iocpHandle, 0, 0, nullptr);
			break;
		}

		Dispatch(iocpEvent, transferredBytes);
	}
	printf("Worker End\n");
}

bool C_Network::NetworkBase::OnConnectionRequest(const SOCKADDR_IN& clientInfo)
{
	TODO_UPDATE_EX_LIST;
	return true;
}

void C_Network::NetworkBase::OnConnected(const SOCKADDR_IN& clientInfo, ULONGLONG sessionId)
{
	TODO_UPDATE_EX_LIST;
}

void C_Network::NetworkBase::OnDisconnected(ULONGLONG sessionId)
{
	TODO_UPDATE_EX_LIST;
}

void C_Network::NetworkBase::OnError(int errCode, WCHAR* cause)
{
	TODO_UPDATE_EX_LIST;
}

void C_Network::NetworkBase::OnRecv(C_Utility::CSerializationBuffer& buffer, ULONGLONG sessionId)
{
}

void C_Network::NetworkBase::ProcessAccept(SharedSession& sessionRef, DWORD transferredBytes)
{
	AddSession();
	sessionRef->ProcessAccept();
}

void C_Network::NetworkBase::ProcessConnect(SharedSession& sessionRef, DWORD transferredBytes)
{
	AddSession();
	sessionRef->ProcessConnect();
}

bool C_Network::NetworkBase::ProcessRecv(SharedSession& sessionRef, DWORD transferredBytes)
{
	sessionRef->_recvEvent._owner = nullptr;

	if (transferredBytes == 0)
	{
		sessionRef->Disconnect();
		TODO_LOG_ERROR_WSA("Recv");
		return false;
	}
	
	if (!sessionRef->_recvBuffer.MoveRearRetBool(transferredBytes))
	{
		sessionRef->Disconnect();
		TODO_LOG_ERROR;
		printf("Buffer Overflow\n");
		return false;
	}

	uint dataSize = sessionRef->_recvBuffer.GetUseSize();

	uint processingLen = 0;

	C_Utility::CSerializationBuffer tempBuffer(1000);

	while (1)
	{
		tempBuffer.Clear();

		int bufferSize = sessionRef->_recvBuffer.GetUseSize();

		// packetheader보다 작은 상태
		if (bufferSize < sizeof(PacketHeader))
			break;

		PacketHeader header;

		sessionRef->_recvBuffer.PeekRetBool(reinterpret_cast<char*>(&header), sizeof(PacketHeader));

		if (bufferSize < header.size)
			break;
		
		sessionRef->_recvBuffer.MoveFront(sizeof(header));

		if (!sessionRef->_recvBuffer.DequeueRetBool(tempBuffer.GetRearPtr(), header.size))
		{
			TODO_LOG_ERROR; printf("OnRecv Dequeue Error\n");
			TODO_UPDATE_EX_LIST; // 이 경우에 session Disconnect?
			break;
		}
		
		tempBuffer.MoveRearPos(header.size);
		OnRecv(tempBuffer, sessionRef->GetId());
	}

	sessionRef->PostRecv();

	return true;
}

bool C_Network::NetworkBase::ProcessSend(SharedSession& sessionRef, DWORD transferredBytes)
{
	return sessionRef->ProcessSend(transferredBytes);
}

bool C_Network::NetworkBase::ProcessDisconnect(SharedSession& sessionRef, DWORD transferredBytes)
{
	bool bRet = sessionRef->ProcessDisconnect();
	
	if (bRet)
		// or !bRet에 대한 로직 생성
		TODO_UPDATE_EX_LIST;
	DeleteSession();

	return bRet;
}



	/*-----------------------
			NetServer
	-----------------------*/
C_Network::NetServer::NetServer(const NetAddress& netAddr, uint maxSessionCnt)
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
		TODO_LOG_ERROR_WSA("Create Sock");
		CCrash(L"Listen socket is Invalid");
	}

	DWORD bindRet = bind(_listenSock, (SOCKADDR*)&_netAddr.GetSockAddr(), sizeof(SOCKADDR_IN));

	if (bindRet == SOCKET_ERROR)
	{
		TODO_LOG_ERROR_WSA("Bind");
		CCrash(L"Bind Error\n");
	}

	DWORD listenRet = ::listen(_listenSock, SOMAXCONN);
	if (listenRet == SOCKET_ERROR)
	{
		TODO_LOG_ERROR_WSA("Listen");
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
		if (!OnConnectionRequest(clientInfo))
		{
			TODO_DEFINITION // 거절한 이유와 해당 ip는 무엇인지 로그 기록
		}

		SharedSession newSession = CreateSession(clientSock, &clientInfo);
		
		printf("client join\n");
		
		newSession->ProcessAccept();

		OnConnected(clientInfo, newSession->GetId());
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
