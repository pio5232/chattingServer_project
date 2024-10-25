#pragma once

#include "Room.h"

namespace C_Network
{
	class RoomManager
	{
	public:
		RoomManager(uint maxRoomCount, uint maxRoomUserCnt) : _maxRoomUserCnt(maxRoomUserCnt)
		{
			_roomList.reserve(maxRoomCount);

			for (int i = 0; i < maxRoomCount; i++)
			{
				// TODO : ROOM POOL 사용
				Room* room = new Room(_maxRoomUserCnt);

				_roomList.push_back(room);
			}
		}

		~RoomManager()
		{
			// TODO : USEI TO ROOM POOL 
			for (auto roomPtr : _roomList)
			{
				delete roomPtr;
			}
		}
	private:
		const uint _maxRoomUserCnt; // room당 최대 user 수
		std::vector<Room*> _roomList;
	};
}