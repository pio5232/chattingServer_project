#include "LibsPch.h"
#include "Session.h"

C_Network::IocpEvent::IocpEvent(IocpEventType type) : _type(type) {}

void C_Network::IocpEvent::Init()
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

C_Network::Session::Session(SOCKET sock) : _socket(sock), _recvEvent(), _sendEvent(), _isConnected(1)
{
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

void C_Network::Session::Dispatch(IocpEvent* iocpEvent, int transferredBytes)
{
	switch (iocpEvent->GetType())
	{
		case IocpEventType::Recv:
			ProcessRecv(transferredBytes); break;
		case IocpEventType::Send:
			ProcessSend(transferredBytes); break;
		case IocpEventType::Accept:
			ProcessAccept(); break;
		case IocpEventType::Connect:
			ProcessConnect(); break;
		case IocpEventType::Disconnect:
			ProcessDisconnect(); break;
		default:break;
	}
}

void C_Network::Session::Disconnect()
{
	char isConnected = InterlockedExchange8(&_isConnected, false);
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
	return false;
}

bool C_Network::Session::ProcessConnect()
{
	return false;
}

bool C_Network::Session::ProcessAccept()
{
	return false;
}

bool C_Network::Session::ProcessDisconnect()
{
	return false;
}

// Post
void C_Network::Session::PostSend()
{
}

void C_Network::Session::PostRecv()
{
	_recvEvent.Init();
	_recvEvent._owner = shared_from_this();

	WSABUF buf;
	buf.buf = _recvBuffer.GetWritePtr();
	buf.len = _recvBuffer.FreeSize();

	DWORD flag = 0;
 	
	int recvRet = WSARecv(_socket, &buf, 1, nullptr, &flag, reinterpret_cast<LPWSAOVERLAPPED>(&_recvEvent), nullptr);

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

void C_Network::Session::PostConnect()
{
}

void C_Network::Session::PostDisconnect()
{
}

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

void C_Network::Session::OnRecvPacket(char* buffer, uint len)
{
}
