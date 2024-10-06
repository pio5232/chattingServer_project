#include "LibsPch.h"
#include "NetworkBase.h"
#include "Session.h"

using namespace C_Utility;
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

C_Network::Session::Session(SOCKET sock, SOCKADDR_IN* pSockAddr, NetworkBase* server) : _socket(sock), _recvEvent(), _sendEvent(),_connectEvent(), _disconnEvent(),
_isConnected(1), _netAddr(*pSockAddr), _sendFlag(0), _recvBuffer()
{
	InitializeSRWLock(&_sendBufferLock);

	TODO_UPDATE_EX_LIST;
	// 비동기로 바뀔 경우 sessinonId의 변경을 atomic하게 해줘야한다.
	static  ULONGLONG sessionId = 0;

	_sessionId = InterlockedIncrement(&sessionId);;
}

C_Network::Session::~Session()
{
	if (_socket != INVALID_SOCKET)
	{
		closesocket(_socket);
		_socket = INVALID_SOCKET;
	}
}

void C_Network::Session::Send(SharedSendBuffer sendBuf)
{
	{
		SRWLockGuard lockGuard(&_sendBufferLock);
		_sendBufferQ.push(sendBuf);
	}
	
	PostSend();
	//if (InterlockedExchange8(&_sendFlag, 1) == 0)
	//{
	//	int qSize;
	//	{
	//		SRWLockGuard lockGuard(&_sendBufferLock);
	//		qSize = _sendBufferQ.size();
	//	}
	//	
	//	// 이것도아님
	//	if (qSize > 0)
	//		PostSend();
	//	else
	//		InterlockedExchange8(&_sendFlag, 0);
	//}

	// --------------------------------------------------------
	//char isSending = InterlockedExchange8(&_sendFlag, 1);

	//{
	//	SRWLockGuard lockGuard(&_sendBufferLock);
	//	_sendBufferQ.push(sendBuf);
	//}
	//if (isSending == 0)
	//{
	//	PostSend();
	//}
}

void C_Network::Session::Disconnect()
{
	char isConnected = InterlockedExchange8(&_isConnected, 0);
	if (!isConnected)
		return;

	closesocket(_socket);
}

//bool C_Network::Session::ProcessRecv(DWORD transferredBytes)
//{
//	_recvEvent._owner = nullptr;
//
//	if (transferredBytes == 0)
//	{
//		Disconnect();
//		TODO_LOG_ERROR_WSA;
//		return false;
//	}
//
//	if (!_recvBuffer.MoveRearRetBool(transferredBytes))
//	{
//		Disconnect();
//		TODO_LOG_ERROR;
//		printf("Buffer Overflow\n");
//		return false;
//	}
//
//	uint dataSize = _recvBuffer.GetUseSize();
//	
//	int procLen = OnRecv();
//
//	if (procLen < 0 || procLen > dataSize || !_recvBuffer.MoveFrontRetBool(procLen))
//	{
//		Disconnect();
//		TODO_LOG_ERROR;
//		return false;
//	}
//	
//	PostRecv();
//
//	return true;
//}

bool C_Network::Session::ProcessSend(DWORD transferredBytes)
{
	_sendEvent._owner = nullptr;
	_sendEvent._pendingBuffs.clear();
	
	InterlockedExchange8(&_sendFlag, 0);

	PostSend();
	//bool isEmpty;

	//{
	//	SRWLockGuard lockGuard(&_sendBufferLock);
	//	isEmpty = _sendBufferQ.empty();
	//}

	//if (isEmpty)
	//	InterlockedExchange8(&_sendFlag, 0);
	//else
	//	PostSend();


	return true;
}


// --------------------------------------------------- ASynchronization ---------------------------------------------------
bool C_Network::Session::ProcessConnect()
{
	_connectEvent._owner = nullptr;

	PostRecv();

	return false;
}

bool C_Network::Session::ProcessAccept()
{	
	TODO_DEFINITION;
	TODO_UPDATE_EX_LIST;

	PostRecv();

	return false;
}

bool C_Network::Session::ProcessDisconnect()
{
	_disconnEvent._owner = nullptr;

	return false;
}

// -----------------------------------------------------------------------------------------------------------------------
// Post
void C_Network::Session::PostSend()
{
	if (!_isConnected)
		return;

	if (_sendBufferQ.empty() || InterlockedExchange8(&_sendFlag, 1) == 1)
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

			_sendEvent._pendingBuffs.push_back(sendData);
		}
	}
	
	for (auto& sendData : _sendEvent._pendingBuffs)
	{
		WSABUF buf;

		buf.buf = sendData->GetFrontPtr();
		buf.len = sendData->GetDataSize();

		wsabufs.push_back(buf);
	}
	
	int sendRet = WSASend(_socket, wsabufs.data(), wsabufs.size(), nullptr, 0, reinterpret_cast<LPWSAOVERLAPPED>(&_sendEvent), nullptr);
	
	if (sendRet == SOCKET_ERROR)
	{
		DWORD wsaErr = WSAGetLastError();
		if (wsaErr != WSA_IO_PENDING)
		{
			_sendEvent._owner = nullptr;
			printf("WSASend Error - [ errCode %d ]\n", wsaErr);
			Disconnect();
		}
	}
	
}

void C_Network::Session::PostRecv()
{
	_recvEvent.Reset();
	_recvEvent._owner = shared_from_this();

	WSABUF buf[2];

	int directEnqueueSize = _recvBuffer.DirectEnqueueSize();
	int remainderSize = _recvBuffer.GetFreeSize()  - directEnqueueSize;
	int wsabufSize = 1;

	buf[0].buf = _recvBuffer.GetFrontBufferPtr();
	buf[0].len = directEnqueueSize;

	if (remainderSize > 0)
	{
		++wsabufSize;
		buf[1].buf = _recvBuffer.GetStartBufferPtr();
		buf[1].len = remainderSize;
	}

	DWORD flag = 0;
 	
	int recvRet = WSARecv(_socket, buf, wsabufSize, nullptr, &flag, reinterpret_cast<LPWSAOVERLAPPED>(& _recvEvent), nullptr);

	if (recvRet == SOCKET_ERROR)
	{
		DWORD err = WSAGetLastError();

		if (err != WSA_IO_PENDING)
		{
			printf("WSARecv Error - [ errCode %d ]\n", err);
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

//uint C_Network::Session::OnRecv()
//{
//	uint processingLen = 0;
//	
//	
//	while (1)
//	{
//		int bufferSize = _recvBuffer.GetUseSize();
//		
//		// packetheader보다 작은 상태
//		if (bufferSize < sizeof(PacketHeader))
//			break;
//
//		PacketHeader header;
//		
//		if (!_recvBuffer.PeekRetBool(reinterpret_cast<char*>(&header), sizeof(PacketHeader)))
//		{
//			TODO_LOG_ERROR; printf("_recvBuffer peek Failed\n");
//			break;
//		}
//
//		// echoServer 클라이언트는 size 2 / data 8이 들어오므로 
//		// 이 경우는 그냥 무시하도록 한다.
//		// 원래대로라면 uint16 size 10 /ulonlong data 8 이라면 잘 된다.
//
//		if (bufferSize < header.size)
//			break;
//
//		_netPtr->OnRecv();
//
//		
//
//
//
//
//
//
//		uint availableSize = bufferSize - processingLen;
//		if (availableSize < sizeof(PacketHeader))
//			break;
//
//		uint16 packetLen = reinterpret_cast<PacketHeader*>(readPtr)->size;
//
//		if (packetLen < availableSize)
//			break;
//
//		_netPtr->OnRecv(readPtr, packetLen);
//
//		processingLen += packetLen;
//	}
//	
//	return processingLen;
//}


//void C_Network::Session::OnConnected() {}
//void C_Network::Session::OnDisconnected() {}
//
//void C_Network::Session::OnSend() {}
