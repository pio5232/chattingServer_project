#include "LibsPch.h"
#include "NetworkBase.h"
#include "ClientPacketHandler.h"
/*---------------------------------------
			EchoPacketHandler
---------------------------------------*/
std::unordered_map<uint16, C_Network::ClientEchoPacketHandler::PacketFunc> C_Network::ClientEchoPacketHandler::packetFuncs;
C_Network::NetworkBase* C_Network::ClientEchoPacketHandler::_owner = nullptr;

void C_Network::ClientEchoPacketHandler::Init(C_Network::NetworkBase* owner) 
{
	_owner = owner;

	packetFuncs.clear();
	packetFuncs[PACKET_TYPE_ECHO] = ClientEchoPacketHandler::ProcessEchoFunc;
}


C_Network::NetworkErrorCode C_Network::ClientEchoPacketHandler::ProcessPacket(ULONGLONG sessionId, uint16 packetType, C_Utility::CSerializationBuffer& buffer)
{
	if (packetFuncs.find(packetType) == packetFuncs.end())
		return C_Network::NetworkErrorCode::CANNOT_FIND_PACKET_FUNC;

	return packetFuncs[packetType](sessionId, buffer);
}

C_Network::NetworkErrorCode C_Network::ClientEchoPacketHandler::ProcessEchoFunc(ULONGLONG sessionId, C_Utility::CSerializationBuffer& buffer)
{
	EchoPacket packet;

	buffer >> packet.data;

	SharedSendBuffer sendBuffer = MakePacket(PACKET_TYPE_ECHO, 8, packet.data);
	
	_owner->Send(sessionId, sendBuffer);

	return C_Network::NetworkErrorCode::NONE;
}
