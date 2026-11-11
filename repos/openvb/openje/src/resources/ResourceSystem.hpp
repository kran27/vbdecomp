#pragma once

#include "ResourceCache.hpp"
#include "ResourceFileManager.hpp"
#include "ResourceIndex.hpp"
#include "ResourceLock.hpp"
#include "ResourceMemoryStream.hpp"
#include "ResourceSettings.hpp"

namespace JE
{
	namespace Resources
	{
		struct GroupEntryRange;

		struct ResourceFileHandle
		{
			Net::UInt32 begin;
			Net::UInt32 end;
			Net::UInt32 cursor;
			ResourceFileEntry* file;
		};

		class ResourceSystem
		{
		public:
			ResourceSystem(); // 0x49C610
			~ResourceSystem(); // 0x49C530
			void ConfigureOverrides(const Configuration::Settings& settings); // 0x49BC60
			void ConfigurePaths(const Configuration::Settings& settings); // 0x499900, actual receiver.
			void Initialize(const char* root, const Configuration::Settings& settings); // 0x49BF20
			ResourceFileHandle* OpenStream(const char* name, unsigned char mode); // 0x49B9F0
			unsigned char Seek(ResourceFileHandle* handle, Net::Int32 offset, int origin); // 0x498800
			Net::UInt32 Tell(ResourceFileHandle* handle); // 0x4989C0
			unsigned char Read(void* output, Net::UInt32 count, ResourceFileHandle* handle); // 0x499220
			unsigned char Write(const void* input, Net::UInt32 count, ResourceFileHandle* handle); //4993B0.
			unsigned char FileDelete(const char* filename); //49A910, actual Win32 path then temp descriptor.
			void Release(ResourceFileHandle* handle, unsigned char flag); // 0x49B1D0
			void RegisterTemporaryFile(const char* path); // 0x49ACD0
			ResourceData* Demand(const char* name, unsigned char cached); // 0x49B750
			void ReleaseResource(ResourceData* resource); // 0x498EB0
			bool Contains(const char* name); //49ABD0, descriptor presence without acquisition.
			ResourceMemoryStream* OpenMemoryStream(const char* name, BYTE cached); // 0x49BEB0.
			void CloseMemoryStream(ResourceMemoryStream* stream); // 0x498F30.
			BYTE ReadMemory(void* output, Net::UInt32 count, ResourceMemoryStream* stream); //498170.
			Net::UInt32 TellMemory(ResourceMemoryStream* stream); //4981D0, callers preserve the actual owner receiver.
			BYTE SeekMemory(ResourceMemoryStream* stream, Net::Int32 offset, int origin); //498A00.

			// Declaration order retains reverse destruction: cache, lock, file
			// manager, paths, temporary index, primary index. These are native
			// C++ objects, not storage overlaid at old compiler offsets.
			ResourceIndex primary;
			ResourceIndex temporaryFileIndex;
			std::string media[ResourcePaths::PathCount]; // Native direct array; original EH owns all8 strings.
			ResourceFileManager files;
			ResourceLock lock;
			ResourceCache cache;
			const char* root; // Borrowed cwd, native +0x144; left unwritten by constructor.

		private:
			void ScanOverrideDirectory(const char* path, unsigned char recursive); // 0x49B240
			void RegisterOverrideFile(const char* path); // 0x49AA90
			unsigned char ReadGroupEntryRange(ResourceDescriptor* descriptor,
				ResourceFileHandle* handle, GroupEntryRange* range); // 0x499160
			ResourceFileHandle* OpenDescriptor(ResourceDescriptor* descriptor, unsigned char mode); // 0x49B670
			ResourceFileHandle* OpenFile(ResourceDescriptor* descriptor, unsigned char mode); // 0x49AE90
		};

		extern ResourceSystem* g_pResourceSystem; // original 0x707DA8


		void InitializeIndex(ResourceIndex& index, Net::UInt32 buckets);
	}
}

#if defined(_MSC_VER) && _MSC_VER == 1300 && defined(_M_IX86)
#include <stddef.h>
typedef char NativeResourcePaths224[(sizeof(JE::Resources::ResourcePaths)==224)?1:-1];
typedef char NativeResourceSystem328[(sizeof(JE::Resources::ResourceSystem)==328)?1:-1];
typedef char NativeResourceTemporary20[(offsetof(JE::Resources::ResourceSystem,temporaryFileIndex)==0x14)?1:-1];
typedef char NativeResourceMedia40[(offsetof(JE::Resources::ResourceSystem,media)==0x28)?1:-1];
typedef char NativeResourceFiles264[(offsetof(JE::Resources::ResourceSystem,files)==0x108)?1:-1];
typedef char NativeResourceLock296[(offsetof(JE::Resources::ResourceSystem,lock)==0x128)?1:-1];
typedef char NativeResourceCache300[(offsetof(JE::Resources::ResourceSystem,cache)==0x12c)?1:-1];
typedef char NativeResourceRoot324[(offsetof(JE::Resources::ResourceSystem,root)==0x144)?1:-1];
#endif
