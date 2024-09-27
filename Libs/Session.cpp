#include "LibsPch.h"
#include "Session.h"

C_Network::IocpEvent::IocpEvent(IocpEventType type) : _type(type) {}

void C_Network::IocpEvent::Reset()
{
	OVERLAPPED::hEvent = 0;
	OVERLAPPED::Internal = 0;
	OVERLAPPED::InternalHigh = 0;
	OVERLAPPED::Offset = 0;
	OVERLAPPED::OffsetHigh = 0;
}

/*------------------------------
			Session
------------------------------*/

C_Network::Session::Session(SOCKET sock, SOCKADDR_IN* pSockAddr) : _socket(sock), _recvEvent(), _sendEvent(),_connectEvent(), _disconnEvent(),
_isConnected(1), _netAddr(*pSockAddr), _sendFlag(0), _recvBuffer()
{
	InitializeSRWLock(&_sendBufferLock);

	static  ULONGLONG sessionId = 0;

	_sessionId = ++sessionId;
}

C_Network::Session::~Session()
{
	if (_socket != INVALID_SOCKET)
	{
		closesocket(_socket);
		_socket = INVALID_SOCKET;
	}
}

//void C_Network::Session::Dispatch(IocpEvent* iocpEvent, int transferredBytes)
//{
//	switch (iocpEvent->GetType())
//	{
//	case IocpEventType::Recv:
//		ProcessRecv(transferredBytes); break;
//	case IocpEventType::Send:
//		ProcessSend(transferredBytes); break;
//	case IocpEventType::Accept:
//		ProcessAccept(); break;
//	case IocpEventType::Connect:
//		ProcessConnect(); break;
//	case IocpEventType::Disconnect:
//		ProcessDisconnect(); break;
//	default:break;
//	}
//}

void C_Network::Session::Send(SharedSendBuffer sendBuf)
{
	{
		SRWLockGuard lockGuard(&_sendBufferLock);
		_sendBufferQ.push(sendBuf);
	}
	char isSending = InterlockedExchange8(&_sendFlag, 1);

	if (isSending == 0)
		PostSend();
}

void C_Network::Session::Disconnect()
{
	char isConnected = InterlockedExchange8(&_isConnected, 0);
	if (!isConnected)
		return;
}

bool C_Network::Session::ProcessRecv(DWORD transferredBytes)
{
	_recvEvent._owner = nullptr;

	if (transferredBytes == 0)
	{
		Disconnect();
		TODO_LOG_ERROR_WSA;
		return false;
	}

	if (!_recvBuffer.MoveWritePos(transferredBytes))
	{
		Disconnect();
		TODO_LOG_ERROR;
		printf("Buffer Overflow\n");
		return false;
	}

	uint dataSize = _recvBuffer.UseSize();
	
	int procLen = OnRecv();

	if (procLen < 0 || procLen > dataSize || !_recvBuffer.MoveReadPos(procLen))
	{
		Disconnect();
		TODO_LOG_ERROR;
		return false;
	}
	
	PostRecv();

	return true;
}

bool C_Network::Session::ProcessSend(DWORD transferredBytes)
{
	_sendEvent._owner = nullptr;

	OnSend();

	// lock 걸고 2가지 중 하나 실행하도록 만들기 / _sendEvent는 sendBuffer를 물고 있어야함.
	// lock
	// if (_sendQ.size() > 0 ) -> PostSend
	
	bool isEmpty;

	{
		SRWLockGuard lockGuard(&_sendBufferLock);
		isEmpty = _sendBufferQ.empty();
	}

	if (isEmpty)
		PostSend();
	else
		InterlockedExchange8(&_sendFlag, 0);
	
	return true;
}


// --------------------------------------------------- ASynchronization ---------------------------------------------------
bool C_Network::Session::ProcessConnect()
{
	_connectEvent._owner = nullptr;

	// Contents Overriding
	OnConnected();

	PostRecv();

	return false;
}

bool C_Network::Session::ProcessAccept()
{	
	TODO_DEFINITION;
	TODO_UPDATE_EX_LIST;

	OnConnected();

	PostRecv();

	return false;
}

bool C_Network::Session::ProcessDisconnect()
{
	_disconnEvent._owner = nullptr;

	OnDisconnected();

	return false;
}

// -----------------------------------------------------------------------------------------------------------------------
// Post
void C_Network::Session::PostSend()
{
	if (!_isConnected)
		return;

	_sendEvent.Reset();
	_sendEvent._owner = shared_from_this();

	std::vector<WSABUF> wsabufs;
	
	{
		SRWLockGuard lockGuard(&_sendBufferLock);
		int sendBufferCount = _sendBufferQ.size();
		wsabufs.reserve(sendBufferCount);

		while (_sendBufferQ.size())
		{
			SharedSendBuffer sendData = _sendBufferQ.front();

			_sendBufferQ.pop();
			WSABUF buf;
			buf.buf = sendData->GetBuffer();
			buf.len = sendData->GetSize();

			wsabufs.push_back(buf);
		}
	}
	
	int sendRet = WSASend(_socket, wsabufs.data(), wsabufs.size(), nullptr, 0, reinterpret_cast<LPWSAOVERLAPPED>(&_sendEvent), nullptr);
	
	if (sendRet == SOCKET_ERROR)
	{
		DWORD wsaErr = WSAGetLastError();
		if (wsaErr != WSA_IO_PENDING)
		{
			_sendEvent._owner = nullptr;
			TODO_LOG_ERROR_WSA;
			Disconnect();
		}
	}
	
}

void C_Network::Session::PostRecv()
{
	_recvEvent.Reset();
	_recvEvent._owner = shared_from_this();

	WSABUF buf;
	buf.buf = _recvBuffer.GetWritePtr();
	buf.len = _recvBuffer.FreeSize();

	DWORD flag = 0;
 	
	int recvRet = WSARecv(_socket, &buf, 1, nullptr, &flag, reinterpret_cast<LPWSAOVERLAPPED>(& _recvEvent), nullptr);

	if (recvRet == SOCKET_ERROR)
	{
		DWORD err = WSAGetLastError();

		if (err != WSA_IO_PENDING)
		{
			TODO_LOG_ERROR;
			_recvEvent._owner = nullptr;
			Disconnect();
		}
	}
}
// --------------------------------------------------- ASynchronization ---------------------------------------------------
void C_Network::Session::PostAccept()
{
	TODO_DEFINITION;
	TODO_UPDATE_EX_LIST;
}

void C_Network::Session::PostConnect()
{
	TODO_DEFINITION;
	TODO_UPDATE_EX_LIST;
}

void C_Network::Session::PostDisconnect()
{	TODO_DEFINITION;
	TODO_UPDATE_EX_LIST;
}
// -----------------------------------------------------------------------------------------------------------------------

uint C_Network::Session::OnRecv()
{
	uint processingLen = 0;
	
	uint bufferSize = _recvBuffer.UseSize();
	
	while (1)
	{
		uint availableSize = bufferSize - processingLen;
		if (availableSize < sizeof(PacketHeader))
			break;

		char* readPtr = _recvBuffer.GetReadPtr();

		uint16 packetLen = reinterpret_cast<PacketHeader*>(readPtr)->size;

		if (packetLen < availableSize)
			break;

		OnRecvPacket(readPtr, packetLen);

		processingLen += packetLen;
	}
	
	return processingLen;
}

// Client Overriding
void C_Network::Session::OnRecvPacket(char* buffer, uint len) {}
void C_Network::Session::OnConnected() {}
void C_Network::Session::OnDisconnected() {}

void C_Network::Session::OnSend() {}
