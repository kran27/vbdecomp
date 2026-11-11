#include "ResourceCache.hpp"
#include "diagnostics/DebugOutput.hpp"

#include <new>

namespace JE
{
	namespace Resources
	{
		ResourceData::ResourceData()
		{
			name = "";
			size = 0;
			refs = 0;
			data = NULL;
			// The actual producer fills type/cached; original 0x499860 does not.
		}

		ResourceData::~ResourceData()
		{
			if (refs)
				JE::LogDebugString("Res::~Res - Resource %s still demanded %d times\r\n",
					name.c_str(), static_cast<Net::Int32>(refs));
			if (data)
			{
				::operator delete[](data);
			}
		}

		void ResourceData::operator delete(void* pObject) throw()
		{
			ResourceCacheServices::FreeObject(pObject);
		}

		void ResourceCacheList::Detach(ResourceCacheNode* node)
		{
			if (node->previous) node->previous->next=node->next;
			else
			{
				head=node->next;
				if (head) head->previous=NULL;
			}
			if (node->next) node->next->previous=node->previous;
			else
			{
				tail=node->previous;
				if (tail) tail->next=NULL;
			}
		}
		ResourceCacheNode* ResourceCacheList::Append(ResourceData* data)
		{
			ResourceCacheNode* node=new ResourceCacheNode;
			if (!node) return NULL;
			node->data=data;
			node->next=NULL;
			if (!head)
			{
				node->previous=NULL;
				head=node;
				tail=node;
			}
			else
			{
				tail->next=node;
				node->previous=tail;
				tail=node;
			}
			return node;
		}
		void ResourceCacheList::Promote(ResourceCacheNode* node)
		{
			if (node!=head)
			{
				Detach(node);
				node->previous=NULL;
				node->next=head;
				head->previous=node;
				head=node;
			}
		}
		ResourceCacheNode* ResourceCacheList::Remove(ResourceCacheNode* node, unsigned char forward)
		{
			ResourceCacheNode* next=forward==1?node->next:node->previous;
			Detach(node);
			ResourceData* data=node->data;
			if (data)
			{
				data->~ResourceData();
				::operator delete(data);
			}
			::operator delete(node);
			return next;
		}
		void ResourceCacheList::Clear()
		{
			ResourceCacheNode* current=head;
			while (current) current=Remove(current,1);
		}
		void ResourceCache::Flush()
		{
			items.Clear();
			JE::LogDebugString("ResourceCache::Flush\n");
		}
		ResourceCache::~ResourceCache()
		{
			Flush(); //Automatic ResourceCacheList destruction is the second Clear.
		}

		void ResourceCache::Configure(Net::UInt32 nBudget, ResourceSystem* pOwner,
			ResourceIndex* pTemporary)
		{
			Flush();
			used = 0;
			budget = nBudget;
			owner = pOwner;
			temporary = pTemporary;
		}

		namespace ResourceCacheServices
		{
			void FreeData(void* pData) { ::operator delete[](pData); }
			void FreeObject(void* pObject) throw() { ::operator delete(pObject); }
		}
	}
}
