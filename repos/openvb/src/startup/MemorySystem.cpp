#include "MemorySystem.hpp"
#include "ClientMemoryCommand.hpp"
#include <new>

namespace F3
{
	MemorySystem* g_pMemorySystem = NULL;
	LARGE_INTEGER g_memoryTestFrequency = {0};
	int g_memoryTestCounter = 42;


	MemoryPoolNode::~MemoryPoolNode()
	{
		if (pData)
		{
			MemoryServices::Free(MemoryServices::PoolData, pData);
			pData = NULL;
		}
	}

	MemoryPool::MemoryPool(JE::Net::UInt32 nBatchCount, JE::Net::UInt32 nBlockBytes)
	{
		m_nBlockBytes = nBlockBytes;
		m_nBatchCount = nBatchCount;
		m_pFreeNode = NULL;
		Expand();
		m_Allocations.clear();
	}

	MemoryPool::~MemoryPool()
	{
		while (m_pFreeNode)
		{
			MemoryPoolNode* pNode = m_pFreeNode;
			m_pFreeNode = pNode->pNext;
			pNode->~MemoryPoolNode();
			MemoryServices::Free(MemoryServices::PoolNode, pNode);
		}
		// The real map destructor frees its own storage, never the pointed-to nodes.
		// Outstanding allocations are not visited by the original pool destructor.
	}

	void MemoryPool::Expand()
	{
		for (JE::Net::UInt32 nIndex = 0; nIndex < m_nBatchCount; ++nIndex)
		{
			MemoryPoolNode* pNode = new MemoryPoolNode(m_nBlockBytes);
			// The original has no graceful null-node allocation path.
			pNode->pNext = m_pFreeNode;
			m_pFreeNode = pNode;
		}
	}

	MemoryPoolNode::MemoryPoolNode(JE::Net::UInt32 nBytes)
	{
		pData = MemoryServices::Allocate(MemoryServices::PoolData, nBytes);
		pNext = NULL;
	}

	void* MemoryPool::Acquire()
	{
		if (!m_pFreeNode)
			Expand();
		MemoryPoolNode* pNode = m_pFreeNode;
		if (!pNode)
			return NULL;
		m_pFreeNode = pNode->pNext;
		MemoryServices::Insert(m_Allocations, pNode->pData) = pNode;
		return pNode->pData;
	}

	void MemoryPool::Release(void* pData)
	{
		MemoryPoolNode* pNode;
		if (MemoryServices::Find(m_Allocations, pData, pNode))
		{
			MemoryServices::Insert(m_Allocations, pData) = NULL;
			// A repeated release retains a null map value and faults here originally.
			pNode->pNext = m_pFreeNode;
			m_pFreeNode = pNode;
		}
	}

	MemorySystem::MemorySystem()
	{
		m_pPool[0] = NULL;
	}

	MemorySystem::~MemorySystem()
	{
		// Original destructor resets its compiler table only; cleanup is separate.
	}

	void MemorySystem::operator delete(void* pObject)
	{
		MemoryServices::Free(MemoryServices::SystemObject, pObject);
	}

	GameComponent* CreateMemorySystem()
	{
		return new MemorySystem;
	}

	void MemorySystem::Initialize()
	{
		if (!g_pMemorySystem)
			g_pMemorySystem = static_cast<MemorySystem*>(CreateMemorySystem());
		g_pMemorySystem->m_nBatchCount = 64;
		g_pMemorySystem->m_nBlockBytes = 1;
		g_pMemorySystem->GetOrCreatePool(0, g_pMemorySystem->m_nBatchCount,
			g_pMemorySystem->m_nBlockBytes);
	}

	void* MemorySystem::GetOrCreatePool(int nSelector, JE::Net::UInt32 nBatchCount,
		JE::Net::UInt32 nBlockBytes)
	{
		if (!g_pMemorySystem || nSelector < 0 || nSelector >= 1)
			return g_pMemorySystem;
		MemoryPool* pPool = m_pPool[nSelector];
		if (!pPool)
		{
			pPool = new MemoryPool(nBatchCount, nBlockBytes);
			// Reload the canonical owner; do not replace this with m_pPool[nSelector] = pPool.
			g_pMemorySystem->m_pPool[nSelector] = pPool;
		}
		return pPool;
	}

	void MemorySystem::Cleanup()
	{
		MemoryPool* pPool = m_pPool[0];
		if (pPool)
		{
			pPool->~MemoryPool();
			MemoryServices::Free(MemoryServices::PoolObject, pPool);
			m_pPool[0] = NULL;
		}
	}

	void MemorySystem::Update()
	{
	}

	void* MemorySystem::Allocate(int nSelector)
	{
		if (!g_pMemorySystem || nSelector < 0 || nSelector >= 1)
			return NULL;
		return g_pMemorySystem->m_pPool[nSelector]->Acquire();
	}

	void MemorySystem::Release(int nSelector, void* pData)
	{
		if (g_pMemorySystem && nSelector >= 0 && nSelector < 1)
			g_pMemorySystem->m_pPool[nSelector]->Release(pData);
	}

	void* AllocateMemory(int nSelector)
	{
		return g_pMemorySystem ? g_pMemorySystem->Allocate(nSelector) : NULL;
	}

	void ReleaseMemory(int nSelector, void* pData)
	{
		if (g_pMemorySystem) g_pMemorySystem->Release(nSelector,pData);
	}

	namespace MemoryServices
	{
		void* Allocate(AllocationKind kind, std::size_t nBytes)
		{
			return kind == PoolData ? ::operator new[](nBytes) : ::operator new(nBytes);
		}
		void Free(AllocationKind kind, void* pObject)
		{
			if (kind == PoolData) ::operator delete[](pObject);
			else ::operator delete(pObject);
		}
		MemoryPoolNode*& Insert(MemoryAllocationMap& entries, void* pData)
		{
			return entries.insert(std::make_pair(reinterpret_cast<MemoryAllocationMap::key_type>(pData), static_cast<MemoryPoolNode*>(NULL)))
				.first->second;
		}
		bool Find(MemoryAllocationMap& entries, void* pData, MemoryPoolNode*& pNode)
		{
			MemoryAllocationMap::iterator found = entries.find(reinterpret_cast<MemoryAllocationMap::key_type>(pData));
			if (found == entries.end())
				return false;
			pNode = found->second;
			return true;
		}
	}
}
