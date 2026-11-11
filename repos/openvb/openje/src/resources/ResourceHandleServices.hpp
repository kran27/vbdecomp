#pragma once

#include "network/Types.hpp"

namespace JE
{
	namespace Resources
	{
		struct ResourceFileEntry;
		struct ResourceFileHandle;
		class ResourceFileManager;
		class ResourceSystem;

		// Native stream and ownership APIs. Original object offsets exist only in
		// the comparison transport, never in these C++ resource objects.
		namespace ResourceHandleServices
		{
			bool IsOpen(const ResourceFileEntry& entry);
			void Seek(ResourceFileEntry& entry, Net::Int32 offset, int origin);
			Net::UInt32 Tell(ResourceFileEntry& entry);
			void Read(ResourceFileEntry& entry, void* output, Net::UInt32 count);
			bool Failed(const ResourceFileEntry& entry);
			void RegisterTemporary(ResourceSystem& system, const char* path);
			void ReleaseFile(ResourceFileManager& manager, ResourceFileEntry* entry);
			void DestroyHandle(ResourceFileHandle* handle);
		}
	}
}
