#include "LibsPch.h"
#include "NetServer.h"

C_Network::NetServer::NetServer(NetAddress netAddr, uint maxSessionCnt) 
	: NetworkBase(netAddr, maxSessionCnt)
{
}

C_Network::NetServer::~NetServer()
{
}
