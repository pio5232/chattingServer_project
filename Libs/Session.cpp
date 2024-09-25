#include "LibsPch.h"
#include "Session.h"

C_Network::IocpEvent::IocpEvent(IocpEventType type) : _type(type) {}

void C_Network::IocpEvent::Init()
{
	OVERLAPPED::hEvent = 0;
	OVERLAPPED::Internal = 0;
	OVERLAPPED::InternalHigh = 0;
	OVERLAPPED::Offset = 0;
	OVERLAPPED::OffsetHigh = 0;
}

void C_Network::Session::Dispatch(IocpEvent* iocpEvent, int numOfBytes)
{
}
