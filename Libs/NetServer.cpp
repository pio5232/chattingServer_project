#include "LibsPch.h"
#include "NetServer.h"

C_Network::NetServer::NetServer(NetAddress netAddr, uint maxSessionCnt) 
	: NetworkBase(netAddr, maxSessionCnt)
{
	_listenSock = socket(AF_INET, SOCK_STREAM, 0);
	if (_listenSock == INVALID_SOCKET)
	{
		TODO_LOG_ERROR_WSA;
		CCrash(L"Listen socket is Invalid");
	}

	DWORD bindRet = bind(_listenSock, (SOCKADDR*)&netAddr.GetSockAddr(), sizeof(SOCKADDR_IN));

	if (bindRet == SOCKET_ERROR)
	{
		TODO_LOG_ERROR_WSA;
		CCrash(L"Bind Error\n");
	}

	DWORD listenRet = ::listen(_listenSock, SOMAXCONN);
	if (listenRet == SOCKET_ERROR)
	{
		TODO_LOG_ERROR_WSA;
		CCrash(L"listen Error\n");
	}
}

C_Network::NetServer::~NetServer()
{
}
