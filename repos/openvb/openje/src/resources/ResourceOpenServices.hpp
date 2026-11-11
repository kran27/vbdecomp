#pragma once

#include "ResourceSystem.hpp"
#include "ResourceFormats.hpp"

namespace JE
{
	namespace Resources
	{
		namespace ResourceOpenServices
		{
			Net::UInt32 Classify(std::string& name);
			void Lock(ResourceSystem& system);
			void Unlock(ResourceSystem& system);
			ResourceDescriptor* Find(ResourceIndex& index, const char* name, Net::UInt32 type);
			ResourceDescriptor* AllocateDescriptor();
			void DestroyDescriptor(ResourceDescriptor* descriptor);
			ResourceFileHandle* AllocateHandle();
			ResourceFileEntry* Open(ResourceFileManager& files, const char* name,
				const char* directory, unsigned char mode);
			unsigned char Range(ResourceDescriptor& descriptor, ResourceFileHandle& handle,
				GroupEntryRange& range);
			void Seek(ResourceFileEntry& file, Net::Int32 offset, int origin);
			Net::UInt32 Tell(ResourceFileEntry& file);
			void Release(ResourceSystem& system, ResourceFileHandle* handle, unsigned char flag);
			void DescriptorAllocationFailure();
			void HandleAcquisitionFailure();
		}
	}
}
