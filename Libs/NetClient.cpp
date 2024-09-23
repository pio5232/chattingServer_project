#include "LibsPch.h"
#include "NetClient.h"

C_Network::NetClient::NetClient(NetAddress destEndPoint, uint maxSessionCnt)
	: NetworkBase(destEndPoint, maxSessionCnt)
{
	
}
