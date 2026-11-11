#include "ResourceSystem.hpp"
#include "ResourceDemandServices.hpp"
#include "ResourceFormats.hpp"
#include "diagnostics/DebugOutput.hpp"

#include <algorithm>
#include <new>

namespace JE
{
	namespace Resources
	{
		ResourceData* ResourceSystem::Demand(const char* source, unsigned char cached)
		{
			ResourceData* resource = NULL;
			std::string name(source);
			const Net::UInt32 type = ExtractFileType(name);
			lock.Lock();
			ResourceDescriptor* descriptor = temporaryFileIndex.Find(name.c_str(), type);
			if (!descriptor) descriptor = primary.Find(name.c_str(), type);
			if (cached == 1)
			{
				resource = cache.Get(descriptor);
				if (resource)
				{
					lock.Unlock();
					return resource;
				}
			}
			ResourceFileHandle* handle = OpenDescriptor(descriptor, 1);
			lock.Unlock();
			if (!handle)
			{
				JE::LogDebugString("ResourceSystem::Demand - Couldn't open file %s\n", source);
				return NULL;
			}
			Seek(handle, 0, 2);
			const Net::UInt32 size = Tell(handle);
			Seek(handle, 0, 0);
			void* data = new unsigned char[size];
			if (data)
			{
				Read(data, size, handle); // The original ignores status, including short reads.
				resource = new ResourceData;
				if (!resource)
				{
					JE::LogDebugString("ResourceSystem::Demand - allocation error trying to new a res\n");
					::operator delete(data);
					Release(handle, 0);
					return NULL;
				}
				resource->name.assign(name.c_str());
				resource->size = size;
				const Net::UInt32 refs = resource->refs + 1;
				resource->type = type;
				resource->data = data;
				resource->refs = refs;
				resource->cached = cached;
				if (cached == 1)
				{
					lock.Lock();
					cache.Add(resource, descriptor);
					lock.Unlock();
				}
			}
			else
				JE::LogDebugString("ResourceSystem::Demand - couldn't allocate %d for %s\n",
					static_cast<Net::Int32>(size), source);
			Release(handle, 0);
			return resource;
		}

		void ResourceSystem::ReleaseResource(ResourceData* resource)
		{
			if (!resource)
			{
				JE::LogDebugString("ResourceSystem::Release - Bad Res pointer passed in\n");
				return;
			}
			lock.Lock();
			if (static_cast<Net::Int32>(resource->refs) <= 0)
				JE::LogDebugString("ResourceSystem::Release - Resource %s isn't demanded, current demand count is %d\n",
					resource->name.c_str(), static_cast<Net::Int32>(resource->refs));
			const unsigned char cached = resource->cached;
			--resource->refs;
			if (!cached)
			{
				resource->~ResourceData();
				::operator delete(resource);
			}
			lock.Unlock();
		}

		ResourceData* ResourceCache::Get(ResourceDescriptor* descriptor)
		{
			if (!descriptor || !(descriptor->flags & 0x200)) return NULL;
			items.Promote(descriptor->cached);
			++descriptor->cached->data->refs;
			return descriptor->cached->data;
		}

		void ResourceCache::Add(ResourceData* resource, ResourceDescriptor* descriptor)
		{
			if (!descriptor)
			{
				JE::LogDebugString("ResourceCache::Add - Bucket entry is NULL - %s\n", resource->name.c_str());
				return;
			}
			if (descriptor->flags & 0x200)
			{
				JE::LogDebugString("ResourceCache::Add - Res already cached - %s\n", resource->name.c_str());
				return;
			}
			if (resource->size + used > budget) Cleanup(resource->size);
			descriptor->cached = items.Append(resource);
			used += resource->size;
			descriptor->flags |= 0x200;
		}

		void ResourceCache::Remove(ResourceCacheNode* item)
		{
			ResourceDescriptor* descriptor = temporary->Find(
				item->data->name.c_str(), item->data->type);
			if (!descriptor) descriptor = owner->primary.Find(
				item->data->name.c_str(), item->data->type);
			if (!descriptor)
			{
				JE::LogDebugString("ResourceCache::Remove - entry is NULL - %s\n", item->data->name.c_str());
				return;
			}
			descriptor->flags &= ~0x200U;
			used -= item->data->size;
			items.Remove(item,1);
		}

		void ResourceCache::Cleanup(Net::UInt32 requested)
		{
			if (requested > budget)
			{
				JE::LogDebugString("ResourceCache::Cleanup - Item to big for cache - %d\n",
					static_cast<Net::Int32>(requested));
				return;
			}
			ResourceCacheNode* current=items.tail;
			while (requested+used>budget)
			{
				if (!current)
				{
					JE::LogDebugString("ResourceCache::Cleanup - Not enough room in the cache with all the demanded items, changing max size - %d - to - %d\n",
						static_cast<Net::Int32>(budget), static_cast<Net::Int32>(requested+used));
					budget=requested+used;
					break;
				}
				ResourceCacheNode* previous=current->previous;
				if (!current->data->refs) Remove(current);
				current=previous;
			}

		}

		namespace ResourceDemandServices
		{
			Net::UInt32 Classify(std::string& name) { return ExtractFileType(name); }
			ResourceDescriptor* Find(ResourceIndex& index, const char* name, Net::UInt32 type)
			{
				return index.Find(name, type);
			}
			ResourceData* Get(ResourceCache& cache, ResourceDescriptor* descriptor) { return cache.Get(descriptor); }
			void Add(ResourceCache& cache, ResourceData* resource, ResourceDescriptor* descriptor)
			{
				cache.Add(resource, descriptor);
			}
			void* AllocateData(Net::UInt32 size) { return new unsigned char[size]; }
			ResourceData* AllocateResource() { return new ResourceData; }
			void AssignName(ResourceData& resource, const char* name) { resource.name.assign(name); }
			void FreeData(void* data) { ::operator delete(data); }
			void DestroyResource(ResourceData* resource)
			{
				resource->~ResourceData();
				::operator delete(resource);
			}
			void Promote(ResourceCache& cache, ResourceCacheNode* item)
			{
				cache.items.Promote(item);
			}
			ResourceCacheNode* Append(ResourceCache& cache, ResourceData* resource)
			{
				return cache.items.Append(resource);
			}
			void Erase(ResourceCache& cache, ResourceCacheNode* item)
			{
				cache.items.Remove(item,1);
			}

		}
	}
}
