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
		// uin16 Messageũ��� �����Ѵ�. ������ PacketHandler�� ó���ϰ� �Ǵ� ���� �ϳ��� ������ �����ͷ� ���� �� ó���ϱ� �����̴�.
		//, uint16) > ; // sessionId, Message ���� ����, Message ũ��.
		using PacketFunc = bool(*)(ULONGLONG, C_Utility::CSerializationBuffer&);//std::function<bool(ULONGLONG, char*)>;


		static void Init(class NetworkBase* owner);

		// ����ȭ ���ۿ� �����͸� ä����! ���� ���ø��� Ȱ��.
		template <typename... Types>
		static SharedSendBuffer MakePacket(uint16 packetType, uint16 packetSize, Types... args )
		{
			// TODO : CSerializationBuffer ���� Pool���� ������ ����ϵ��� �����.

			SharedSendBuffer sendBuffer = std::make_shared<C_Utility::CSerializationBuffer>(PACKET_SIZE_MAX);
			
			PacketHeader packetHeader;
			//packetHeader.type = packetType;
			packetHeader.size = packetSize;

			sendBuffer->PutData(reinterpret_cast<char*>(&packetHeader), sizeof(PacketHeader));

			SetPacket(sendBuffer, args...);

			return sendBuffer;
		}

		// ������ �� : CSerializationBuffer�� ���� Operator�� ���ǵǾ� ���� ���� ��쿡�� ����������Ѵ�.!! 
		// ����� ���� �ڷ����� ��쿡�� ���� Contents���� �����ε��� �� �� �ְ� �����س���.
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

		// processPacket -> packet ó��
		static bool ProcessPacket(ULONGLONG sessionId, uint16 packetType, C_Utility::CSerializationBuffer& buffer);

		static bool ProcessEchoFunc(ULONGLONG sessionId, C_Utility::CSerializationBuffer& buffer);

	private:
		static std::unordered_map<uint16, PacketFunc> packetFuncs;
		static class NetworkBase* _owner;
	};
}
