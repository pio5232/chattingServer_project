#pragma once

#include <queue>
#include <vector>

namespace C_Memory
{
	enum : uint
	{
		EXTRA_CHUNK_MOVE_SIZE = 100,
		MEMORY_ALLOC_SIZE = 500, // 
		POOL_COUNT = (1024 / 32) + (1024 / 128) + (2048 / 256),
		MAX_ALLOC_SIZE = 4096
	};

	enum : MemoryGuard
	{
		GUARD_VALUE = 0xcccc,
	};

	/*--------------------------
		   MemoryProtector
	--------------------------*/
	class MemoryProtector // Use Header + Guard 
	{
	public:
		MemoryProtector(uint size) : _size(size),_frontGuard(GUARD_VALUE) {}
		/*---------------------------------------------------------------------------------------
		  Mempool::Alloc : [header + Guard(2bytes) + data + Guard(2bytes) + NextPtr] => [data]
							 ↑
							ptr
		---------------------------------------------------------------------------------------*/
		static void* Attach(void* ptr, uint size)
		{
			new (static_cast<MemoryProtector*>(ptr)) MemoryProtector(size);
			
			MemoryGuard* pRightGuard = reinterpret_cast<MemoryGuard*>(reinterpret_cast<char*>(ptr) + size - MemoryProtector::_rightGuardSpace);
			
			*pRightGuard = GUARD_VALUE;

			return static_cast<MemoryProtector*>(ptr) + 1;
		}

		/*-------------------------------------------------------------------------------
												   [ptr]
													↓
		  Mempool::Free :[header + Guard(2bytes) + data + Guard(2bytes) + NextPtr]
		-------------------------------------------------------------------------------*/
		static MemoryProtector* Detach(void* ptr)
		{
			MemoryProtector* base = static_cast<MemoryProtector*>(ptr) - 1;

			if (base->_frontGuard != GUARD_VALUE) // left guard
			{
				std::cout << "Left Guard is Not 0xcccc\n";
				TODO_TLS_LOG_ERROR
			}

			if (*reinterpret_cast<MemoryGuard*>(reinterpret_cast<char*>(ptr) + base->_size - MemoryProtector::_rightGuardSpace) != GUARD_VALUE)
			{
				std::cout << "Right Guard is Not 0xcccc\n";
				TODO_TLS_LOG_ERROR
			}
				
			return base;
		}
		inline uint GetSize() const { _size; }
	private:
		uint _size; // header->size

		MemoryGuard _frontGuard; // 

		static constexpr uint _rightGuardSpace = sizeof(MemoryGuard) + sizeof(void*);
	};

	/*-----------------
			Node
	------------------*/
	template <int AllocSize>
	struct Node
	{
		char c[AllocSize]{};
		Node<AllocSize>* next;
	};


	/*-------------------
		  NodeChunk
	-------------------*/
	template <int AllocSize>
	struct alignas (16) NodeChunk
	{
	public:
		using PoolNode = Node<AllocSize>;
		NodeChunk() : _head(nullptr), _size(0) {}
		~NodeChunk()
		{
			while (_head)
			{
				PoolNode* next = _head->next;

				_aligned_free(_head);

				_head = next;

				--_size;
			}

			if (_size == 0)
			{
				std::cout << "NodeChunk Size is 0 ~ Destructor call successed\n";
				TODO_TLS_LOG_SUCCESS;
			}
			else
			{
				std::cout << "NodeChunk Size is Not 0 - Error\n";

				TODO_TLS_LOG_ERROR;
			}
		}
		inline uint GetSize() const{ return _size; }

		void* Export() // == Alloc
		{
			if (!_head)
				return nullptr;

			PoolNode* ret = _head;
			_head = _head->next;
			
			--_size;
			return ret;
		}

		void Regist(void* ptr) // == Free
		{
			PoolNode* newTop = static_cast<PoolNode*>(ptr);
			newTop->next = _head;
			_head = newTop;

			++_size;
		}


	private:
		inline PoolNode* GetHead() { return _head; }
		PoolNode* _head;
		uint _size;
	};


	/*-----------------------------------
		   IntegrationChunkManager
	-----------------------------------*/
	class IntegrationChunkManager // has Chunk Pool
	{
	public:
		static IntegrationChunkManager& GetInstance()
		{
			static IntegrationChunkManager instance;
			return instance;
		}

		// TODO_추가
		uint GetPoolSize() { return 1; } // 추가

	private:
		~IntegrationChunkManager() {}
		IntegrationChunkManager() {}
		SRWLOCK _lock;
	};



	/*---------------------------------------
		Memory Pool (memoryHeader + size)
	---------------------------------------*/
	template <int AllocSize> // pool Allocation Size
	class alignas (32) MemoryPool final
	{
		
	public:
		MemoryPool(uint managedCnt = 0) : _managedCount(managedCnt)
		{
			MakeChunk(_pInnerChunk);
			MakeChunk(_pExtraChunk);

			for (int i = 0; i < _managedCount; i++)
			{
				_pInnerChunk->Regist(_aligned_malloc(AllocSize, 64));
			}

			TODO_TLS_LOG_SUCCESS; 
		}
		~MemoryPool() 
		{
			DeleteChunk(_pInnerChunk);
			DeleteChunk(_pExtraChunk);
		}
		inline void MakeChunk(NodeChunk<AllocSize>* nodeChunk)
		{
			nodeChunk = static_cast<NodeChunk<AllocSize>*>(_aligned_malloc(sizeof(NodeChunk), 16));
			new (nodeChunk) NodeChunk();
		}

		inline void DeleteChunk(NodeChunk<AllocSize>* nodeChunk)
		{
			nodeChunk->~NodeChunk();
			_aligned_free(nodeChunk);
		}
		inline void* Alloc()
		{
			if (_pInnerChunk->GetSize() > 0 )
			{
				return _pInnerChunk->Export();
			}

			if (_pExtraChunk->GetSize() > 0)
			{
				return _pExtraChunk->Export();
			}

			if (IntegrationChunkManager::GetInstance().GetPoolSize() > 0)
			{
				// TODO_추가
				DeleteChunk(_pInnerChunk);

				_pInnerChunk = IntegrationChunkManager::GetInstance().GetNodeChunk();

				return _pInnerChunk->Export();
			}
			else
				return _aligned_malloc(AllocSize, 64);
		}

		inline void Free(void* ptr)
		{
			if (_pInnerChunk->GetSize() < _managedCount)
			{
				_pInnerChunk->Regist(ptr);
				return;
			}

			_pExtraChunk->Regist(ptr); // Managing NodeChunk, if (NodeChunk Size -> 100, Regist NodeChunk to Global Manager)
			if (_pExtraChunk->GetSize() > EXTRA_CHUNK_MOVE_SIZE)
			{
				IntegrationChunkManager::GetInstance().RegistChunk(_pExtraChunk);
				
				MakeChunk(_pExtraChunk);

				return;
			}
		}
	private:

		NodeChunk<AllocSize>* _pInnerChunk;
		NodeChunk<AllocSize>* _pExtraChunk;

		const uint _managedCount;

	};



	/*---------------------------------------
			 Pool Manager ( 1 per Thread )
	---------------------------------------*/

	class PoolManager final
	{
	public:
		PoolManager() {}
		~PoolManager() {}
		void* Alloc()
		{

		}

		void Free()
		{

		}
	private:

		// 5,6,7 -> 24 
		// 24 -> hash table
	};

	thread_local PoolManager poolMgr;
}