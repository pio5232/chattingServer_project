#include "LibsPch.h"
#include "ClientPacketHandler.h"
#include "EchoServer.h"

C_Network::EchoServer::EchoServer(const NetAddress& netAddr, uint maxSessionCnt) : NetServer(netAddr, maxSessionCnt) 
{
}

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

// �ϳ��� ��Ŷ���� �Ǻ��� ������. ���⼭ ���� SerializationBuffer�� ��Ʈ��ũ ����� ������ �����͸� �����.
void C_Network::EchoServer::OnRecv(C_Utility::CSerializationBuffer& buffer, ULONGLONG sessionId, uint16 type)
{
	// ���� �� ��Ŷ�� ���°� ���� ������ ���¿� �´��� Ȯ���ϴ� �۾��� �ʿ��ϴ�.

	// TODO : Check PacketHeader (���ǵ� ��Ŷ ����� �´���) 

	// �˸°� �� Packet�� ���� ó��.
	C_Network::ClientEchoPacketHandler::ProcessPacket(sessionId, type, buffer);
}

void C_Network::EchoServer::InitHandler()
{
	C_Network::ClientEchoPacketHandler::Init(this);
}

