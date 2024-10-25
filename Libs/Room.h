#pragma once

#include "User.h"
namespace C_Network
{
	class Room
	{
	public :
		Room(int maxUserCnt);

		void EnterRoom(ULONGLONG userId);
		void LeaveRoom(ULONGLONG userId);
	private:
		int _curUserCnt;
		int _maxUserCnt;
		int _roomOrder; // �� ��°���� ������ �ǹ�.

		std::vector<User*> _userList;

	};
}
