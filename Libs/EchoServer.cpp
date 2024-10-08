#include "LibsPch.h"
#include "EchoServer.h"

C_Network::EchoServer::EchoServer(const NetAddress& netAddr, uint maxSessionCnt) : NetServer(netAddr, maxSessionCnt) {}

C_Network::EchoServer::~EchoServer() {}

bool C_Network::EchoServer::OnConnectionRequest(const SOCKADDR_IN& clientInfo)
{
	return false;
}

void C_Network::EchoServer::OnConnected(const SOCKADDR_IN& clientInfo, ULONGLONG sessionId)
{
}

void C_Network::EchoServer::OnDisconnected(ULONGLONG sessionId)
{
}

void C_Network::EchoServer::OnError(int errCode, WCHAR* cause)
{
}

void C_Network::EchoServer::OnRecv(C_Utility::CSerializationBuffer& buffer, ULONGLONG sessionId)
{
	SharedSession session = GetSession(sessionId);

	EchoPacket packet;
	
	buffer >> packet.data;

	SharedSendBuffer sendBuffer = std::make_shared<C_Utility::CSerializationBuffer>(5000);

	C_Utility::CSerializationBuffer* serialBuffer = sendBuffer.get();
	
	*serialBuffer << (uint16)8 << packet.data;

	session->Send(sendBuffer);
}

