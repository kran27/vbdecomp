#pragma once

#include "EngineRegistry.hpp"
#include "network/Types.hpp"
// The original allocation map uses the supplied VC7 hash container.
#include <hash_map>

namespace F3
{
	struct MemoryPoolNode
	{
		explicit MemoryPoolNode(JE::Net::UInt32 nBytes); // 0x5B3180
		~MemoryPoolNode(); // 0x5B31A0
		void* pData;
		MemoryPoolNode* pNext;
	};

	//Native uses signed32 address keys, list nodes and bucket-vector ownership.
	typedef std::hash_map<JE::Net::Int32, MemoryPoolNode*> MemoryAllocationMap;
	class MemoryPool
	{
	public:
		MemoryPool(JE::Net::UInt32 nBatchCount, JE::Net::UInt32 nBlockBytes); // 0x5B3BC0
		~MemoryPool(); // 0x5B3510
		void Expand(); // 0x5B31F0
		void* Acquire(); // 0x5B3CF0
		void Release(void* pData); // 0x5B3D50; incidental iterator EAX is not an API result

		MemoryAllocationMap m_Allocations;
		MemoryPoolNode* m_pFreeNode;
		JE::Net::UInt32 m_nBatchCount;
		JE::Net::UInt32 m_nBlockBytes;
	};

	class MemorySystem : public GameComponent
	{
	public:
		MemorySystem(); // 0x5B31C0
		~MemorySystem(); // 0x5B3170; does not call Cleanup
		void Initialize(); // 0x5B3DB0; operates on separate canonical object
		void Cleanup(); // 0x5B3870; operates on supplied this
		void Update(); // 0x404B70
		static void operator delete(void* pObject);
		void* GetOrCreatePool(int nSelector, JE::Net::UInt32 nBatchCount,
			JE::Net::UInt32 nBlockBytes); // 0x5B3C70; invalid selectors return canonical object
		void* Allocate(int nSelector); //5B3E00; thiscall, canonical global selects pool.
		void Release(int nSelector, void* pData); //5B3E30; no defined API result.

		// Constructor intentionally leaves these two original fields unwritten.
		JE::Net::UInt32 m_nBatchCount;
		JE::Net::UInt32 m_nBlockBytes;
		MemoryPool* m_pPool[1]; // Native selector-indexed sole slot at +0x0C.
	};

	extern MemorySystem* g_pMemorySystem; // 0x70C950, distinct from registry allocation
	GameComponent* CreateMemorySystem(); // 0x5B3270
	//Compatibility facades for earlier workspace callers. Original direct
	//calls use the member APIs and their callee-cleaned arguments.
	void* AllocateMemory(int nSelector);
	void ReleaseMemory(int nSelector, void* pData);

	// Allocation-kind and key adapters; native VC7 uses its real supplied hash owner.
	namespace MemoryServices
	{
		enum AllocationKind { SystemObject, PoolObject, PoolNode, PoolData };
		void* Allocate(AllocationKind kind, std::size_t nBytes);
		void Free(AllocationKind kind, void* pObject);
		MemoryPoolNode*& Insert(MemoryAllocationMap& entries, void* pData);
		bool Find(MemoryAllocationMap& entries, void* pData, MemoryPoolNode*& pNode);
	}
}
