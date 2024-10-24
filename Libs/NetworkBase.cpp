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

void C_Network::NetAddress::Init(SOCKADDR_IN sockAddr)
{
	_sockAddr = sockAddr;
}

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

C_Network::NetworkBase::NetworkBase(const NetAddress& netAddr, uint maxSessionCnt) : _netAddr(netAddr), _iocpHandle(nullptr)
{
	_sessionMgr = std::make_unique<SessionManager>(maxSessionCnt);
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

	_workerThreads.reserve(10);// (concurrentThreadCnt * 2);

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

C_Network::Session* C_Network::NetworkBase::CreateSession(SOCKET sock, SOCKADDR_IN* pSockAddr)
{
	TODO_UPDATE_EX_LIST;
	
	Session* newSession = _sessionMgr->AddSession(sock, pSockAddr);

	CreateIoCompletionPort(reinterpret_cast<HANDLE>(sock), _iocpHandle, reinterpret_cast<ULONG_PTR>(newSession), 0);

	return newSession;
}

//void C_Network::NetworkBase::AddSession()
//{
//	TODO_DEFINITION;
//	TODO_UPDATE_EX_LIST;
//}

//void C_Network::NetworkBase::DeleteSession(C_Network::Session* sessionPtr)
//{
//	TODO_DEFINITION;
//	TODO_UPDATE_EX_LIST;
//}

void C_Network::NetworkBase::Dispatch(C_Network::IocpEvent* iocpEvent, DWORD transferredBytes)
{
	Session* session = iocpEvent->_owner;

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

		if (session->CanDisconnect()) // Disconn
		{
			OnDisconnected(session->GetId());

			_sessionMgr->DeleteSession(session);
		}
		break;

	case IocpEventType::Send:
		
		ProcessSend(session, transferredBytes);
		
		if (session->CanDisconnect()) // Disconn
		{
			OnDisconnected(session->GetId());

			_sessionMgr->DeleteSession(session);
		}
		
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

void C_Network::NetworkBase::OnRecv(C_Utility::CSerializationBuffer& buffer, ULONGLONG sessionId, uint16 type)
{
}

void C_Network::NetworkBase::Send(ULONGLONG sessionId, C_Network::SharedSendBuffer buffer)
{
	Session* session = _sessionMgr->GetSession(sessionId);

	if (!session)
		return;
	
	session->Send(buffer);
}

void C_Network::NetworkBase::ProcessAccept(Session* sessionPtr, DWORD transferredBytes)
{
	// TODO : Session* newSession = CreateSession(clientSock, &clientInfo); 

	sessionPtr->ProcessAccept();
}

void C_Network::NetworkBase::ProcessConnect(Session* sessionPtr, DWORD transferredBytes)
{
	// TODO : Session* newSession = CreateSession(clientSock, &clientInfo);

	sessionPtr->ProcessConnect();
}

bool C_Network::NetworkBase::ProcessRecv(Session* sessionPtr, DWORD transferredBytes)
{
	sessionPtr->_recvEvent._owner = nullptr;

	if (transferredBytes == 0)
	{
		return false;
	}
	
	if (!sessionPtr->_recvBuffer.MoveRearRetBool(transferredBytes))
	{
		//sessionPtr->CheckDisconnect();
		TODO_LOG_ERROR;
		printf("Buffer Overflow\n");
		return false;
	}

	uint dataSize = sessionPtr->_recvBuffer.GetUseSize();

	uint processingLen = 0;

	C_Utility::CSerializationBuffer tempBuffer(3000);

	while (1)
	{
		tempBuffer.Clear();

		int bufferSize = sessionPtr->_recvBuffer.GetUseSize();

		// packetheader보다 작은 상태
		if (bufferSize < sizeof(PacketHeader))
			break;

		PacketHeader header;

		sessionPtr->_recvBuffer.PeekRetBool(reinterpret_cast<char*>(&header), sizeof(PacketHeader));

		if (bufferSize < (sizeof(PacketHeader) + header.size))
			break;
		

		sessionPtr->_recvBuffer.MoveFront(sizeof(header));

		if (!sessionPtr->_recvBuffer.DequeueRetBool(tempBuffer.GetRearPtr(), header.size))
		{
			TODO_LOG_ERROR; printf("OnRecv Dequeue Error\n");
			TODO_UPDATE_EX_LIST; // 이 경우에 session Disconnect?
			break;
		}
		
		tempBuffer.MoveRearPos(header.size);

		//if EchoServer
		OnRecv(tempBuffer, sessionPtr->GetId(), PACKET_TYPE_ECHO); //, header.type); // Echo Server를 테스트하는 상황에서는 header의 type을 따지지 않는다.
		
		// Else
		//OnRecv(tempBuffer, sessionPtr->GetId(), header.type); 
	}

	sessionPtr->PostRecv();

	return true;
}

bool C_Network::NetworkBase::ProcessSend(Session* sessionPtr, DWORD transferredBytes)
{
	return sessionPtr->ProcessSend(transferredBytes);
}

bool C_Network::NetworkBase::ProcessDisconnect(Session* sessionPtr, DWORD transferredBytes)
{
	bool bRet = sessionPtr->ProcessDisconnect();
	
	if (bRet)
		// or !bRet에 대한 로직 생성
		TODO_UPDATE_EX_LIST;

	//DeleteSession(sessionPtr);

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

	LINGER linger;
	linger.l_linger = 0;
	linger.l_onoff = 0;
	setsockopt(_listenSock, SOL_SOCKET, SO_LINGER, (char*)&linger, sizeof(linger));
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
		if (!OnConnectionRequest(clientInfo)) // Client가 서버에 접속할 수 없는 이유가 있을 때
		{
			TODO_DEFINITION // 거절한 이유와 해당 ip는 무엇인지 로그 기록
		}
		if (!CanConnect()) // Server의 상황이 Connect할만하지 않으면. 또는 어떠한 이유로 Server가 연결할 수 없으면
		{
			continue;
		}

		Session* newSession = CreateSession(clientSock, &clientInfo);
		
		//printf("client join\n");
		
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
