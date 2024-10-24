#include "LibsPch.h"
#include "NetworkUtils.h"

C_Network::RecvBuffer::RecvBuffer() : _readPos(0), _writePos(0), _buffer{} {}

thread_local C_Network::SharedSendBufChunk sendBufChunks;

void C_Network::RecvBuffer::Reset()
{
	uint useSize = UseSize();

	if (useSize == 0)
	{
		_readPos = _writePos = 0;
		return;
	}

	// _readPos < _writePos
	uint freeSize = FreeSize();
	if (freeSize < RECV_BUF_CLEAR_SIZE)
	{
		printf("RecvBuffer Memcpy 비용 발생!! 버퍼 크기가 작아요.\n");
		memcpy(&_buffer[0], &_buffer[_readPos], useSize);
		_readPos = 0;
		_writePos = useSize;
	}
}

bool C_Network::RecvBuffer::MoveReadPos(uint transferredBytes)
{
	if(UseSize()< transferredBytes)
		return false;

	_readPos += transferredBytes;
	return true;
}

bool C_Network::RecvBuffer::MoveWritePos(uint len)
{
	if (FreeSize() < len)
		return false;

	_writePos += len;
	return true;
}
