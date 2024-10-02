#pragma once

#include "PacketDefiine.h"
#include <functional>
#include <unordered_map>
namespace C_Network
{
	class PacketHandler
	{
	public:
		// void (session, buffer, len) 
		using PacketFunc = std::function<bool(SharedSession, char*,uint16)>;
		
		//using MakePacketFunc = std::function<
		//PacketHandler() {}
		//~PacketHandler() {}
		
		// make all statitc
		// MakePakcet
		// ProcessPacket
		// std::unordered_map[enum_Packet_order, std::function]
		TODO_DEFINITION;
		
		// [packetNum, packetFunc]
		static std::unordered_map<uint16, PacketFunc> packetFuncs;


		template <typename T>
		SharedSendBuffer MakePacket(uint16 packetNum, uint16 packetSize)
		{
			T* packet = new T;

		}

		bool ProcessPacket(SharedSession session, uint16 packetNum, char* buffer, uint16 packetSize)
		{
			if (packetFuncs.find(packetNum) != packetFuncs.end())
				return false;

			bool bRet = packetFuncs[packetNum](session, buffer, packetSize);
			
			return bRet;
		}

	};
}
