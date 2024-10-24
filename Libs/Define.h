#pragma once

using MemoryGuard = unsigned int;
//using uint16 = unsigned short;
using uint16 = unsigned short;
using uint = unsigned int;

namespace C_Utility
{
	class CSerializationBuffer;
}
namespace C_Network
{
	//using SharedIocpBase = std::shared_ptr<class IocpObjBase>;
	//using SharedSession = std::shared_ptr<class Session>;

	using SharedSendBufChunk = std::shared_ptr<class SendBufferChunk>;
	//using SharedSendBuffer = std::shared_ptr<class SendBuffer>;
	using SharedSendBuffer = std::shared_ptr<class C_Utility::CSerializationBuffer>;
}
//using SharedIocpBase = std::shared_ptr<class C_Network::IocpObjBase>;

#define TODO_TLS_LOG_ERROR
#define TODO_TLS_LOG_SUCCESS

#define TODO_LOG_SUCCESS
#define TODO_LOG_ERROR
#define TODO_LOG

#define TODO_UPDATE_EX_LIST// ���߿� �񵿱�� �����ؾ� �ϴ� �ڵ�. (CONNECT, ACCEPT, New->Mempool ���� ��)..
#define TODO_DEFINITION // ����ϱ����� �����ؾ��ϴ� �ڵ�.


#define TODO_LOG_ERROR_WSA(x) printf("[%s WsaGetLastError - %d] \n",#x, WSAGetLastError())
