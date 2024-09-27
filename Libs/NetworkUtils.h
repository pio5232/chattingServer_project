#pragma once

namespace C_Network
{
	enum : uint
	{
		MAX_RECV_BUF_SIZE = 3000,
		MAX_SEND_BUF_SIZE = 4000,
	};


	/*--------------------
		  NetAddress
	--------------------*/
	class NetAddress
	{
	public:
		NetAddress(SOCKADDR_IN sockAddr);
		NetAddress(std::wstring ip, uint16 port);
		NetAddress(const NetAddress& other);

		inline const SOCKADDR_IN& GetSockAddr() const { return _sockAddr; }
		std::wstring	GetIpAddress();
		inline const uint16 GetPort() const { return ntohs(_sockAddr.sin_port); }

		static IN_ADDR IpToAddr(const WCHAR* ip);
	private:
		SOCKADDR_IN _sockAddr = {};
	};


	/*--------------------
		  RecvBuffer
	--------------------*/
	class RecvBuffer
	{
	public:
		RecvBuffer();
		void Reset();

		bool MoveReadPos(uint transferredBytes);
		bool MoveWritePos(uint len);

		uint UseSize() const { return _writePos - _readPos; }
		uint FreeSize() const { return MAX_RECV_BUF_SIZE - _writePos - 1; }
		char* GetReadPtr() { return &_buffer[_readPos]; }
		char* GetWritePtr() { return &_buffer[_writePos]; }
	private:
		char _buffer[MAX_RECV_BUF_SIZE];
		uint _readPos;
		uint _writePos;
	};


	class SendBuffer
	{
	public:
		SendBuffer(int size) : _size(size) { _buffer = static_cast<char*>(malloc(size)); printf("SendBuffer 持失切\n"); }
		~SendBuffer() { delete _buffer; printf("SendBuffer 社瑚切\n"); }

		char* GetBuffer() { return _buffer; }
		int GetSize() { return _size; }

	private:
		char* _buffer;
		int _size;

	};

	class SendBufferChunk
	{

	};

	class SendManager
	{
	public:

	};
}
