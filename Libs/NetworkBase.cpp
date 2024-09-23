#include "LibsPch.h"
#include "NetworkBase.h"
#include <WS2tcpip.h>

/*--------------------
	  NetAddress
--------------------*/

C_Network::NetAddress::NetAddress(SOCKADDR_IN sockAddr) :_sockAddr(sockAddr) {}


C_Network::NetAddress::NetAddress(std::wstring ip, uint16 port)
{
	memset(&_sockAddr, 0, sizeof(_sockAddr));

	_sockAddr.sin_family = AF_INET;
	_sockAddr.sin_port = htons(port);
	_sockAddr.sin_addr = IpToAddr(ip.c_str());
}

C_Network::NetAddress::NetAddress(const NetAddress& other) : _sockAddr(other._sockAddr) {}

std::wstring C_Network::NetAddress::GetIpAddress()
{
	WCHAR ipWstr[100];

	// 주소체계, &IN_ADDR
	InetNtopW(AF_INET, &_sockAddr.sin_addr, ipWstr, sizeof(ipWstr) / sizeof(WCHAR));

	return std::wstring(ipWstr);
}

IN_ADDR C_Network::NetAddress::IpToAddr(const WCHAR* ip)
{
	IN_ADDR addr;
	InetPtonW(AF_INET, ip, &addr);
	return addr;
}



/*-----------------------
	  NetworkBase
-----------------------*/

C_Network::NetworkBase::NetworkBase(NetAddress netAddr, uint maxSessionCnt) : _netAddr(netAddr)
{
	SYSTEM_INFO sys;
	GetSystemInfo(&sys);
	
	uint concurrentThreadCnt = sys.dwNumberOfProcessors;

	concurrentThreadCnt = concurrentThreadCnt * 0.7;
	if (concurrentThreadCnt == 0)
		concurrentThreadCnt = 1;

	Init();

	_iocpHandle = CreateIoCompletionPort(INVALID_HANDLE_VALUE, 0, 0, concurrentThreadCnt);

	if (_iocpHandle == INVALID_HANDLE_VALUE)
	{
		TODO_LOG_ERROR;
		CCrash(L"Iocp Handle is Invalid");
	}

	// Print Success and ConcurrentCnt
	TODO_LOG_SUCCESS;
}

C_Network::NetworkBase::~NetworkBase() 
{
	WSACleanup();
}



void C_Network::NetworkBase::Init()
{
	WSAData wsa;
	int iRet = WSAStartup(MAKEWORD(2, 2), &wsa);

	if (iRet)
	{
		TODO_LOG_ERROR;
		CCrash(L"WSAStart Error");
	}

}
