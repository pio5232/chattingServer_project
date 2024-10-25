#include "LibsPch.h"
#include "ChattingServer.h"

C_Network::ChattingServer::ChattingServer(const NetAddress& netAddr, uint maxSessionCnt) : NetServer(netAddr, maxSessionCnt)
{
	const uint roomCnt = 20;
	
	uint maxRoomUserCnt = maxSessionCnt / roomCnt;
	if (maxSessionCnt % roomCnt != 0)
		++maxRoomUserCnt;
	
	_roomMgr = std::make_unique<RoomManager>(roomCnt, maxRoomUserCnt);
}

C_Network::ChattingServer::~ChattingServer()
{
}

bool C_Network::ChattingServer::OnConnectionRequest(const SOCKADDR_IN& clientInfo)
{
	return false;
}

void C_Network::ChattingServer::OnConnected(const SOCKADDR_IN& clientInfo, ULONGLONG sessionId)
{
}

void C_Network::ChattingServer::OnDisconnected(ULONGLONG sessionId)
{
}

void C_Network::ChattingServer::OnError(int errCode, WCHAR* cause)
{
}

void C_Network::ChattingServer::OnRecv(C_Utility::CSerializationBuffer& buffer, ULONGLONG sessionId, uint16 type)
{
}
