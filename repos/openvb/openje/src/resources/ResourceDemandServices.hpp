#pragma once

#include "ResourceCache.hpp"

namespace JE
{
	namespace Resources
	{
		namespace ResourceDemandServices
		{
			Net::UInt32 Classify(std::string& name);
			ResourceDescriptor* Find(ResourceIndex& index, const char* name, Net::UInt32 type);
			ResourceData* Get(ResourceCache& cache, ResourceDescriptor* descriptor);
			void Add(ResourceCache& cache, ResourceData* resource, ResourceDescriptor* descriptor);
			void* AllocateData(Net::UInt32 size);
			ResourceData* AllocateResource();
			void AssignName(ResourceData& resource, const char* name);
			void FreeData(void* data);
			void DestroyResource(ResourceData* resource);

			// Existing fixture hooks use actual native cache-node identities.
			void Promote(ResourceCache& cache, ResourceCacheNode* item);
			ResourceCacheNode* Append(ResourceCache& cache, ResourceData* resource);
			void Erase(ResourceCache& cache, ResourceCacheNode* item);
		}
	}
}
