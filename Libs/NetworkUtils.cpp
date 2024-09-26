#include "LibsPch.h"
#include "NetworkUtils.h"

C_Network::RecvBuffer::RecvBuffer() : _readPos(0), _writePos(0), _buffer{} {}

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
	if(freeSize < useSize)
	memcpy(_buffer, &_buffer[_readPos], freeSize);
	_readPos = 0;
	_writePos = freeSize;
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
