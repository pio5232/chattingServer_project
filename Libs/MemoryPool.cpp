#include "LibsPch.h"
#include "MemoryPool.h"

using namespace C_Memory;

thread_local PoolManager poolMgr;



/*--------------------------
	   MemoryProtector
--------------------------*/

C_Memory::MemoryProtector::MemoryProtector(uint size) : _size(size), _frontGuard(GUARD_VALUE) {}


/*---------------------------------------------------------------------------------------
  Mempool::Alloc : [header + Guard(4bytes) + data + Guard(4bytes) + NextPtr] => [data]
					 ��
					ptr
---------------------------------------------------------------------------------------*/
void* C_Memory::MemoryProtector::Attach(void* ptr, uint size)
{
	new (static_cast<MemoryProtector*>(ptr)) MemoryProtector(size);

	MemoryGuard* pRightGuard = reinterpret_cast<MemoryGuard*>(reinterpret_cast<char*>(ptr) + size - MemoryProtector::_rightGuardSpace);

	*pRightGuard = GUARD_VALUE;

	if (size > MAX_ALLOC_SIZE)
	{
		// set NextPtr nullptr
		PUINT_PTR nextPtr = reinterpret_cast<PUINT_PTR>(++pRightGuard);
		*nextPtr = 0;
	}
	return static_cast<MemoryProtector*>(ptr) + 1;
}


/*-------------------------------------------------------------------------------
										   [ptr]
											��
  Mempool::Free : [header + Guard(4bytes) + data + Guard(4bytes) + NextPtr]
-------------------------------------------------------------------------------*/
C_Memory::MemoryProtector* C_Memory::MemoryProtector::Detach(void* ptr)
{
	MemoryProtector* base = static_cast<MemoryProtector*>(ptr) - 1;

	if (base->_frontGuard != GUARD_VALUE) // left guard
	{
		std::cout << "Left Guard is Not 0xaabbccdd\n";
		TODO_TLS_LOG_ERROR
	}

	if (*reinterpret_cast<MemoryGuard*>(reinterpret_cast<char*>(base) + base->_size - MemoryProtector::_rightGuardSpace) != GUARD_VALUE)
	{
		std::cout << "Right Guard is Not 0xaabbccdd\n";
		TODO_TLS_LOG_ERROR
	}

	return base;
}












/*-----------------------------------
	   IntegrationChunkManager
-----------------------------------*/

void C_Memory::IntegrationChunkManager::RegistChunk(uint poolSize, void* nodePtr)
{
	uint index = PoolInfo::GetPoolIndex(poolSize);
	SRWLockGuard lockGuard(&_lock);
	_chunkKeepingQ[index].push(nodePtr);
}

void* C_Memory::IntegrationChunkManager::GetNodeChunk(uint poolSize)
{
	uint index = PoolInfo::GetPoolIndex(poolSize);
	void* retPtr = nullptr;
	{
		SRWLockGuard lockGuard(&_lock);

		if (_chunkKeepingQ[index].size() > 0)
		{
			retPtr = _chunkKeepingQ[index].front();
			_chunkKeepingQ[index].pop();
		}
	}
	return retPtr;
}

C_Memory::IntegrationChunkManager::~IntegrationChunkManager()
{
	SRWLockGuard lockGuard(&_lock);
	for (auto& q : _chunkKeepingQ)
	{
		while (q.size())
		{
			void* ptr = q.front();
			q.pop();

			DeleteChunk(static_cast<NodeChunkBase*>(ptr));
		}
	}
}





C_Memory::NodeChunkBase::~NodeChunkBase() {}

C_Memory::MemoryPoolBase::~MemoryPoolBase() {}



/*---------------------------------------
		 Pool Manager ( 1 per Thread )
---------------------------------------*/

void* C_Memory::PoolManager::Alloc(uint size)
{
	const uint allocSize = size + sizeof(MemoryProtector) + MemoryProtector::_rightGuardSpace;

	void* ptr;
	if (allocSize > MAX_ALLOC_SIZE)
	{
		ptr = malloc(allocSize);

	}
	else
	{
		ptr = _pools[PoolInfo::GetPoolIndex(allocSize)]->Alloc();
	}

	if (!ptr)
	{
		TODO_TLS_LOG_ERROR;
		std::cout << "Ptr is Null!!!!!!!\n";
	}
	return MemoryProtector::Attach(ptr, allocSize);
}

void C_Memory::PoolManager::Free(void* ptr)
{
	MemoryProtector* base = MemoryProtector::Detach(ptr);
	uint allocSize = base->GetSize();

	if (allocSize > MAX_ALLOC_SIZE)
	{
		free(base);
		return;
	}

	_pools[PoolInfo::GetPoolIndex(allocSize)]->Free(base);
}

C_Memory::PoolManager::~PoolManager()
{
	int i = 0;
	for (MemoryPoolBase* ptr : _pools)
	{
		ptr->~MemoryPoolBase();
		_aligned_free(ptr);
	}
}

C_Memory::PoolManager::PoolManager()
{
	//POOL_COUNT = (1024 / 32) + (1024 / 128) + (2048 / 256),
	std::cout << "������ ���� ==============================\n";
			// template MemoryPool Ŭ������ ����� �����Ϳ� uint�� ��� �ֱ� ������ ��� MemoryPool Ŭ������ ������� �����ϴ�.
	int poolSize = sizeof(MemoryPool<1>);


	int i = 0;

	//_pools[0] = static_cast<MemoryPoolBase*>(_aligned_malloc(sizeof(poolSize), POOL_ALIGN_SIZE));
	//new (_pools[0]) MemoryPool<32>(3);
	for (; i < 1024 / POOL_SIZE_LEVEL_1; i++)
	{
		_pools[i] = static_cast<MemoryPoolBase*>(_aligned_malloc(poolSize, POOL_ALIGN_SIZE));
	}

	for (; i < 32 + 1024 / POOL_SIZE_LEVEL_2; i++)
	{
		_pools[i] = static_cast<MemoryPoolBase*>(_aligned_malloc(poolSize, POOL_ALIGN_SIZE));
	}

	for (; i < 48; i++)
	{
		_pools[i] = static_cast<MemoryPoolBase*>(_aligned_malloc(poolSize, POOL_ALIGN_SIZE));
	}

	for (i = 0; i < POOL_COUNT_TO_LEVEL_3; i++)
	{
		switch (PoolInfo::consArray[i])
		{
		case 32 * 1: new (_pools[i]) MemoryPool<32>(); break;
		case 32 * 2: new (_pools[i]) MemoryPool< 32 * 2>(); break;
		case 32 * 3: new (_pools[i]) MemoryPool< 32 * 3>(); break;
		case 32 * 4: new (_pools[i]) MemoryPool< 32 * 4>(); break;
		case 32 * 5: new (_pools[i]) MemoryPool< 32 * 5>(); break;
		case 32 * 6: new (_pools[i]) MemoryPool< 32 * 6>(); break;
		case 32 * 7: new (_pools[i]) MemoryPool< 32 * 7>(); break;
		case 32 * 8: new (_pools[i]) MemoryPool< 32 * 8>(); break;
		case 32 * 9: new (_pools[i]) MemoryPool< 32 * 9>(); break;
		case 32 * 10: new (_pools[i]) MemoryPool< 32 * 10>(); break;
		case 32 * 11: new (_pools[i]) MemoryPool< 32 * 11>(); break;
		case 32 * 12: new (_pools[i]) MemoryPool< 32 * 12>(); break;
		case 32 * 13: new (_pools[i]) MemoryPool< 32 * 13>(); break;
		case 32 * 14: new (_pools[i]) MemoryPool< 32 * 14>(); break;
		case 32 * 15: new (_pools[i]) MemoryPool< 32 * 15>(); break;
		case 32 * 16: new (_pools[i]) MemoryPool< 32 * 16>(); break;
		case 32 * 17: new (_pools[i]) MemoryPool< 32 * 17>(); break;
		case 32 * 18: new (_pools[i]) MemoryPool< 32 * 18>(); break;
		case 32 * 19: new (_pools[i]) MemoryPool< 32 * 19>(); break;
		case 32 * 20: new (_pools[i]) MemoryPool< 32 * 20>(); break;
		case 32 * 21: new (_pools[i]) MemoryPool< 32 * 21>(); break;
		case 32 * 22: new (_pools[i]) MemoryPool< 32 * 22>(); break;
		case 32 * 23: new (_pools[i]) MemoryPool< 32 * 23>(); break;
		case 32 * 24: new (_pools[i]) MemoryPool< 32 * 24>(); break;
		case 32 * 25: new (_pools[i]) MemoryPool< 32 * 25>(); break;
		case 32 * 26: new (_pools[i]) MemoryPool< 32 * 26>(); break;
		case 32 * 27: new (_pools[i]) MemoryPool< 32 * 27>(); break;
		case 32 * 28: new (_pools[i]) MemoryPool< 32 * 28>(); break;
		case 32 * 29: new (_pools[i]) MemoryPool< 32 * 29>(); break;
		case 32 * 30: new (_pools[i]) MemoryPool< 32 * 30>(); break;
		case 32 * 31: new (_pools[i]) MemoryPool< 32 * 31>(); break;
		case 32 * 32: new (_pools[i]) MemoryPool< 32 * 32>(); break;
		case 1024 + 128 * 1: new (_pools[i]) MemoryPool<1024 + 128 * 1>(); break;
		case 1024 + 128 * 2: new (_pools[i]) MemoryPool<1024 + 128 * 2>(); break;
		case 1024 + 128 * 3: new (_pools[i]) MemoryPool<1024 + 128 * 3>(); break;
		case 1024 + 128 * 4: new (_pools[i]) MemoryPool<1024 + 128 * 4>(); break;
		case 1024 + 128 * 5: new (_pools[i]) MemoryPool<1024 + 128 * 5>(); break;
		case 1024 + 128 * 6: new (_pools[i]) MemoryPool<1024 + 128 * 6>(); break;
		case 1024 + 128 * 7: new (_pools[i]) MemoryPool<1024 + 128 * 7>(); break;
		case 1024 + 128 * 8: new (_pools[i]) MemoryPool<1024 + 128 * 8>(); break;
		case 2048 + 256 * 1: new (_pools[i]) MemoryPool<2048 + 256 * 1>(); break;
		case 2048 + 256 * 2: new (_pools[i]) MemoryPool<2048 + 256 * 2>(); break;
		case 2048 + 256 * 3: new (_pools[i]) MemoryPool<2048 + 256 * 3>(); break;
		case 2048 + 256 * 4: new (_pools[i]) MemoryPool<2048 + 256 * 4>(); break;
		case 2048 + 256 * 5: new (_pools[i]) MemoryPool<2048 + 256 * 5>(); break;
		case 2048 + 256 * 6: new (_pools[i]) MemoryPool<2048 + 256 * 6>(); break;
		case 2048 + 256 * 7: new (_pools[i]) MemoryPool<2048 + 256 * 7>(); break;
		case 2048 + 256 * 8: new (_pools[i]) MemoryPool<2048 + 256 * 8>(); break;
		default: std::cout << "there is no size [ " << PoolInfo::consArray[i] << " ]\n"; TODO_TLS_LOG_ERROR;  break;
		}
	}
}

