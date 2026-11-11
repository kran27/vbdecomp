#include "ResourceSystem.hpp"
#include "ResourceFormats.hpp"
#include "diagnostics/DebugOutput.hpp"

namespace JE
{
	namespace Resources
	{
		ResourceSystem* g_pResourceSystem = NULL;

		unsigned char ResourceSystem::FileDelete(const char* filename) //49A910.
		{
			std::string name;
			name.assign(filename);
			lock.Lock();
			if (!::DeleteFileA(name.c_str()))
			{
				LogDebugString("ResourceSystem::FileDelete - Couldn't delete file %s\n", filename);
				lock.Unlock();
				return 0;
			}
			const std::string::size_type slash = name.find_last_of('\\');
			if (slash != std::string::npos)
			{
				name = name.substr(slash + 1);
				const Net::UInt32 type = ExtractFileType(name);
				temporaryFileIndex.Remove(name.c_str(), type);
			}
			lock.Unlock();
			return 1;
		}

		bool ResourceSystem::Contains(const char* filename) //49ABD0.
		{
			std::string name(filename);
			const Net::UInt32 type = ExtractFileType(name);
			lock.Lock();
			ResourceDescriptor* descriptor = temporaryFileIndex.Find(name.c_str(), type);
			if (!descriptor) descriptor = primary.Find(name.c_str(), type);
			lock.Unlock();
			return descriptor != NULL;
		}

		ResourceSystem::ResourceSystem()
		{
			// Native members construct in original subsystem order. root is
			// intentionally unwritten until Initialize, as in original 0x49C610.
		}

		ResourceSystem::~ResourceSystem()
		{
			// Actual member destructors preserve cache/lock/file-manager/path/
			// temporary-index/primary-index lifetime. No CRT implementation here.
		}

		void InitializeIndex(ResourceIndex& index, Net::UInt32 buckets)
        {
            // Historical source adapter; the actual producer uses the member directly.
            index.Initialize(buckets);
        }
	}
}
