#include "LibsPch.h"
#include "Room.h"

C_Network::Room::Room(int maxUserCnt) : _maxUserCnt(maxUserCnt), _curUserCnt(0)
{
	_userList.reserve(maxUserCnt);
}

void C_Network::Room::EnterRoom(ULONGLONG userId)
{
}

void C_Network::Room::LeaveRoom(ULONGLONG userId)
{
}
