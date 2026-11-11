#pragma once

#include "ResourceIndex.hpp"

#include "network/Types.hpp"
#include <string>

namespace JE
{
	namespace Resources
	{
		class ResourceSystem;

		struct ResourceData
		{
			ResourceData(); // 0x499860
			std::string name;
			Net::UInt32 type;
			Net::UInt32 size;
			unsigned char cached; // Original +36 BYTE; constructor leaves it unwritten.
			Net::UInt32 refs;
			void* data;
			~ResourceData(); // 0x498D60
			static void operator delete(void* pObject) throw();
		};

		struct ResourceCacheNode
		{
			ResourceCacheNode* previous;
			ResourceCacheNode* next;
			ResourceData* data;
		};
		// Native resource-cache owner, not an SDK std::list specialization.
		class ResourceCacheList
		{
		public:
			ResourceCacheList() { head=tail=NULL; } // Native writes tail then head.
			~ResourceCacheList() { Clear(); } //498160 delegates to50BA30.
			void Detach(ResourceCacheNode* node); //50B910; no payload destruction.
			ResourceCacheNode* Append(ResourceData* data); //50B960.
			void Promote(ResourceCacheNode* node); //50B9B0.
			ResourceCacheNode* Remove(ResourceCacheNode* node, unsigned char forward); //50B9E0.
			void Clear(); //50BA30, owning payload/node removal.
			ResourceCacheNode* head;
			ResourceCacheNode* tail;
		};

		class ResourceCache
		{
		public:
			ResourceCache()
			{
				used=0; budget=0; owner=NULL; temporary=NULL;
			} // Native manager inlines these writes after automatic list construction.
			~ResourceCache(); //50CAC0: Flush then automatic list destruction.
			void Flush(); //50CAA0.
			void Configure(Net::UInt32 nBudget, ResourceSystem* pOwner,
				ResourceIndex* pTemporary); //50CB10.
			ResourceData* Get(ResourceDescriptor* descriptor); //50CA60.
			void Add(ResourceData* resource, ResourceDescriptor* descriptor); //50CC80.
			void Cleanup(Net::UInt32 requested); //50CBF0.
			void Remove(ResourceCacheNode* item); //50CB40, actual node pointer.

			ResourceSystem* owner;
			ResourceIndex* temporary;
			ResourceCacheList items;
			Net::UInt32 used;
			Net::UInt32 budget;
		};

		namespace ResourceCacheServices
		{
			void FreeData(void* pData);
			void FreeObject(void* pObject) throw();
		}
	}
}

#if defined(_MSC_VER) && _MSC_VER == 1300 && defined(_M_IX86)
#include <stddef.h>
typedef char NativeCacheNode12[(sizeof(JE::Resources::ResourceCacheNode)==12)?1:-1];
typedef char NativeCacheList8[(sizeof(JE::Resources::ResourceCacheList)==8)?1:-1];
typedef char NativeCache24[(sizeof(JE::Resources::ResourceCache)==24)?1:-1];
typedef char NativeCacheHead8[(offsetof(JE::Resources::ResourceCache,items)==8)?1:-1];
typedef char NativeCacheUsed16[(offsetof(JE::Resources::ResourceCache,used)==16)?1:-1];
typedef char NativeCacheBudget20[(offsetof(JE::Resources::ResourceCache,budget)==20)?1:-1];
typedef char NativeDescriptor72[(sizeof(JE::Resources::ResourceDescriptor)==72)?1:-1];
typedef char NativeDescriptorCached68[(offsetof(JE::Resources::ResourceDescriptor,cached)==68)?1:-1];
#endif
