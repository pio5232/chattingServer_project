#pragma once

#include <queue>
#include <vector>

namespace C_MEMORY
{
	enum : int
	{
		LEFT_GUARD_SIZE = 2,
		RIGHT_GUARD_SIZE = 2,
		MEMORY_ALLOC_SIZE = 500, // 
		POOL_COUNT = (1024 / 32) + (1024 / 128) + (2048 / 256),
		MAX_ALLOC_SIZE = 4096
	};

	enum : uint16
	{
		GUARD_VALUE = 0xcccc,
	};

	using MemoryGuardSize = unsigned short;

	/*--------------------
		 Memory Guard
	--------------------*/
	class MemoryGuard
	{
	public:

		/*------------------------------------------------------------------------
			[ptr] => [Guard(2bytes) + pureData + Guard(2bytes) + nextPtr]  
										ก่
									   ptr
									 [               size               ]
		------------------------------------------------------------------------*/
		inline static void Attach(void* ptr,uint size)
		{
			memset(static_cast<char*>(ptr)-LEFT_GUARD_SIZE, GUARD_VALUE, LEFT_GUARD_SIZE); // set LeftGuard
			memset(static_cast<char*>(ptr) + size - _rightGuardSpace, GUARD_VALUE, RIGHT_GUARD_SIZE); // set RightGuard
		}

		/*------------------------------------------------------------------------
			[ptr] => [Guard(2bytes) + pureData + Guard(2bytes) + nextPtr]  
										ก่
									   ptr
									 [               size               ]
		------------------------------------------------------------------------*/
		inline static void Detach(MemoryGuardSize* ptr, uint size)
		{
			if (*(ptr - 1) != GUARD_VALUE)  // Left Guard Check
			{
				// ERROR LOGGING
				TODO_TLS_LOG_ERROR; 
			}
			
			if (*reinterpret_cast<MemoryGuardSize*>(reinterpret_cast<char*>(ptr) + size - _rightGuardSpace)) // Right Guard Check
			{
				// ERROR LOGGING
				TODO_TLS_LOG_ERROR;
			}

		}
		static constexpr uint _rightGuardSpace = RIGHT_GUARD_SIZE + sizeof(void*);
	};

	class MemoryProtector // Use Header + Guard 
	{
	public:
		MemoryProtector(uint size) : _size(size),usV(0) {}
		/*---------------------------------------------------------------------------------------
		  Mempool::Alloc : [header + Guard(2bytes) + data + Guard(2bytes) + NextPtr] => [data]
							 ก่
							ptr
		---------------------------------------------------------------------------------------*/
		static void* Attach(MemoryProtector* ptr, uint size)
		{
			new (ptr) MemoryProtector(size);
			
			MemoryGuard::Attach(++ptr, size - sizeof(MemoryProtector));
			
			return ptr;
		}

		/*-------------------------------------------------------------------------------
												   [ptr]
													ก้
		  Mempool::Free :[header + Guard(2bytes) + data + Guard(2bytes) + NextPtr]
		-------------------------------------------------------------------------------*/
		static MemoryProtector* Detach(void* ptr)
		{
			MemoryProtector* base = static_cast<MemoryProtector*>(ptr) - 1;

			MemoryGuard::Detach(static_cast<MemoryGuardSize*>(ptr), base->_size - sizeof(MemoryProtector));
				
			return base;
		}
		inline uint GetSize() const { _size; }
	private:
		uint _size; // header->size

		MemoryGuardSize usV; // Guard size, meaningless var
	};

	template <int N>
	struct Node
	{
		char c[N]{};
		Node* next;
	};

	/*---------------------------------------
		Memory Pool (memoryHeader + size)
	---------------------------------------*/
	template <int AllocSize> // pool Allocation Size
	class MemoryPool final
	{
		using PoolNode = Node<AllocSize>;

		MemoryPool(uint managedCnt = 0) : _useCount(0), _managedCnt(managedCnt), _top(nullptr)
		{
			for (int i = 0; i < _managedCnt; i++)
			{
				RegistNode(static_cast<PoolNode*>(_aligned_malloc(AllocSize,64)));
			}
			_freeCount = _managedCnt;

			TODO_TLS_LOG_SUCCESS; 
		}
		~MemoryPool()
		{

		}

		void* Alloc()
		{
			
			--_useCount;
		}

		void Free(void* ptr)
		{
			++_freeCount;
		}
	private:
		inline void RegistNode(PoolNode* newNode)
		{
			newNode->next = _top;
			_top = newNode;
		}

		PoolNode* _top;

		uint _useCount; // Alloced Count
		uint _freeCount; // free Node Count
		const uint _managedCnt;

	};



	/*---------------------------------------
			 Pool Manager ( 1 per Thread )
	---------------------------------------*/

	class PoolManager final
	{
	public:
		PoolManager() {}
		~PoolManager() {}
	private:

	};

	thread_local PoolManager poolMgr;


	class IntegrationPoolManager // has Chunk Pool
	{
	public:
		static IntegrationPoolManager& GetInstance()
		{
			static IntegrationPoolManager instance;
			return instance;
		}

	private:
		IntegrationPoolManager();
		~IntegrationPoolManager();
		SRWLOCK _lock;
	};
}