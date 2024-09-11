#pragma once

namespace C_Network
{
	class INetworkBase
	{
	public:
		INetworkBase();
		virtual ~INetworkBase();

		virtual void Init() {};
		virtual void Start() {};
		virtual void Create() abstract = 0;
		virtual void Connect() abstract = 0;
		virtual void Disconnect() abstract = 0;
		virtual void Send() abstract = 0;

	};
	class INetServer : public INetworkBase
	{
	public:
		INetServer();
		virtual ~INetServer();
		virtual bool OnConnectionRequest(IN_ADDR ip, USHORT us_port) abstract = 0;
		virtual void OnClientJoin(SOCKADDR_IN st_clientInfo, ULONG_PTR dw_session_id) = 0; // ... // accept 접속처리 완료 후 호출
		virtual void OnClientLeave(ULONG_PTR dw_session_id) = 0;

		//virtual void OnRecv(ULONG_PTR dw_session_id, CSerializationBuffer* p_packet) = 0;

		virtual void OnError(int i_error_code, WCHAR* sz_display) = 0;

	};


	class INetClient : public INetworkBase
	{
	public:
		INetClient();
		virtual ~INetClient();
	};

}