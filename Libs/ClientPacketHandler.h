#pragma once

#include "PacketDefiine.h"
#include <functional>
#include <unordered_map>
namespace C_Network
{
	/*---------------------------------------
					EchoPacketHandler
	---------------------------------------*/
	class ClientEchoPacketHandler
	{
	public:
		// uin16 Message크기는 제외한다. 어차피 PacketHandler가 처리하게 되는 것은 하나의 완전한 데이터로 왔을 때 처리하기 때문이다.
		//, uint16) > ; // sessionId, Message 내용 버퍼, Message 크기.
		using PacketFunc = bool(*)(ULONGLONG, C_Utility::CSerializationBuffer&);//std::function<bool(ULONGLONG, char*)>;


		static void Init(class NetworkBase* owner);

		// 직렬화 버퍼에 데이터를 채우자! 가변 템플릿을 활용.
		template <typename... Types>
		static SharedSendBuffer MakePacket(uint16 packetType, uint16 packetSize, Types... args )
		{
			// TODO : CSerializationBuffer 또한 Pool에서 꺼내서 사용하도록 만든다.

			SharedSendBuffer sendBuffer = std::make_shared<C_Utility::CSerializationBuffer>(PACKET_SIZE_MAX);
			
			PacketHeader packetHeader;
			//packetHeader.type = packetType;
			packetHeader.size = packetSize;

			sendBuffer->PutData(reinterpret_cast<char*>(&packetHeader), sizeof(PacketHeader));

			SetPacket(sendBuffer, args...);

			return sendBuffer;
		}

		// 주의할 점 : CSerializationBuffer에 대한 Operator가 정의되어 있지 않은 경우에는 정의해줘야한다.!! 
		// 사용자 정의 자료형의 경우에는 따로 Contents에서 오버로딩을 할 수 있게 설정해놓자.
		template <typename T, typename... Types>
		static void SetPacket(SharedSendBuffer& bufferRef, T t, Types... args)
		{
			*bufferRef << t;
			SetPacket(bufferRef, args...);
		}

		template <typename T>
		static void SetPacket(SharedSendBuffer& bufferRef, T t)
		{
			*bufferRef << t;
		}

		// processPacket -> packet 처리
		static bool ProcessPacket(ULONGLONG sessionId, uint16 packetType, C_Utility::CSerializationBuffer& buffer);

		static bool ProcessEchoFunc(ULONGLONG sessionId, C_Utility::CSerializationBuffer& buffer);

	private:
		static std::unordered_map<uint16, PacketFunc> packetFuncs;
		static class NetworkBase* _owner;
	};
}
