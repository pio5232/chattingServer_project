#pragma once
#include "CRingBuffer.h"
#include "CSerializationBuffer.h"
#include "CCrashDump.h"
#include "CLog.h"

/*-------RAII--------
	  LockGuard
-------------------*/

class SRWLockGuard
{
public:
	SRWLockGuard(SRWLOCK* pLock) : _pLock(pLock) { AcquireSRWLockExclusive(_pLock); }
	~SRWLockGuard() { ReleaseSRWLockExclusive(_pLock); }

	SRWLOCK* _pLock;
};
