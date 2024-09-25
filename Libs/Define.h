#pragma once

using MemoryGuard = unsigned int;
//using uint16 = unsigned short;
using uint16 = unsigned short;
using uint = unsigned int;

namespace C_Network
{
	using SharedIocpBase = std::shared_ptr<class IocpObjBase>;
}
//using SharedIocpBase = std::shared_ptr<class C_Network::IocpObjBase>;

#define TODO_TLS_LOG_ERROR
#define TODO_TLS_LOG_SUCCESS

#define TODO_LOG_SUCCESS
#define TODO_LOG_ERROR

#define TODO_LOG_ERROR_WSA printf("[WsaGetLastError - %d] \n",WSAGetLastError());
