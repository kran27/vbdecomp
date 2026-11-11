#include "ResourceSystem.hpp"
#include "ResourceHandleServices.hpp"
#include "ResourceFormats.hpp"
#include "diagnostics/DebugOutput.hpp"

namespace JE
{
	namespace Resources
	{
		unsigned char ResourceSystem::Seek(ResourceFileHandle* handle,
			Net::Int32 offset, int origin)
		{
			lock.Lock();
			if (!handle || !ResourceHandleServices::IsOpen(*handle->file))
			{
				JE::LogDebugString("ResourceSystem::FileStreamRead - file not open\n");
				lock.Unlock();
				return 0;
			}
			ResourceFileEntry* entry = handle->file;
			const Net::UInt32 flags = entry->flags;
			if (!(flags & 2))
			{
				ResourceHandleServices::Seek(*entry, offset, origin);
				lock.Unlock();
				return 1;
			}
			if (!(flags & 1))
				ResourceHandleServices::Seek(*entry, static_cast<Net::Int32>(handle->cursor), 0);

			if (origin == 0)
			{
				if (offset >= 0)
				{
					const Net::UInt32 candidate = handle->begin + static_cast<Net::UInt32>(offset);
					if (candidate <= handle->end)
					{
						ResourceFileEntry* target = handle->file;
						handle->cursor = candidate;
						ResourceHandleServices::Seek(*target, static_cast<Net::Int32>(candidate), 0);
						lock.Unlock();
						return 1;
					}
				}
				JE::LogDebugString("ResourceSystem::FileStreamSeek - Tried a bad seek from the begining of file %s - endpos = %d tried = %d\n",
					handle->file->name.c_str(), static_cast<Net::Int32>(handle->end), offset);
				lock.Unlock();
				return 0;
			}
			else if (origin == 2)
			{
				if (offset <= 0)
				{
					const Net::UInt32 candidate = handle->end + static_cast<Net::UInt32>(offset);
					if (candidate >= handle->begin)
					{
						ResourceFileEntry* target = handle->file;
						handle->cursor = candidate;
						ResourceHandleServices::Seek(*target, static_cast<Net::Int32>(candidate), 0);
						lock.Unlock();
						return 1;
					}
				}
				JE::LogDebugString("ResourceSystem::FileStreamSeek - Tried a bad seek from the end of file %s - endpos = %d tried = %d\n",
					handle->file->name.c_str(), static_cast<Net::Int32>(handle->end), offset);
				lock.Unlock();
				return 0;
			}
			else if (origin == 1)
			{
				const Net::UInt32 candidate = handle->cursor + static_cast<Net::UInt32>(offset);
				if (candidate >= handle->begin && candidate <= handle->end)
				{
					ResourceFileEntry* target = handle->file;
					handle->cursor = candidate;
					ResourceHandleServices::Seek(*target, static_cast<Net::Int32>(candidate), 0);
					lock.Unlock();
					return 1;
				}
				JE::LogDebugString("ResourceSystem::FileStreamSeek - Tried a bad seek from the current position of file %s - endpos = %d tried = %d\n",
					handle->file->name.c_str(), static_cast<Net::Int32>(handle->end), offset);
				lock.Unlock();
				return 0;
			}
			JE::LogDebugString("ResourceSystem::FileStreamSeek - Incorrect seek position on file %s - tried = %d\n",
				handle->file->name.c_str(), origin);
			lock.Unlock();
			return 1;
		}

		Net::UInt32 ResourceSystem::Tell(ResourceFileHandle* handle)
		{
			if (handle && (handle->file->flags & 2))
				return handle->cursor - handle->begin;
			// The original null test also falls through to this dereference.
			return ResourceHandleServices::Tell(*handle->file);
		}

		unsigned char ResourceSystem::Read(void* output, Net::UInt32 count,
			ResourceFileHandle* handle)
		{
			lock.Lock();
			if (!handle || !ResourceHandleServices::IsOpen(*handle->file))
			{
				JE::LogDebugString("ResourceSystem::FileStreamRead - file not open\n");
				lock.Unlock();
				return 0;
			}
			ResourceFileEntry* entry = handle->file;
			const Net::UInt32 flags = entry->flags;
			if (!(flags & 2))
			{
				ResourceHandleServices::Read(*entry, output, count);
				entry = handle->file;
				if (entry->input.bad() || (entry->input.rdstate() &
					(std::ios::badbit | std::ios::eofbit | std::ios::failbit)) != std::ios::goodbit)
				{
					JE::LogDebugString("ResourceSystem::FileStreamRead - Bad read on file %s\n",
						handle->file->name.c_str());
					lock.Unlock();
					return 0;
				}
				lock.Unlock();
				return 1;
			}
			if (!(flags & 1))
				ResourceHandleServices::Seek(*entry, static_cast<Net::Int32>(handle->cursor), 0);
			if (handle->cursor + count > handle->end)
			{
				lock.Unlock();
				return 0;
			}
			ResourceHandleServices::Read(*handle->file, output, count);
			entry = handle->file;
			if (!(entry->input.bad() || (entry->input.rdstate() &
					(std::ios::badbit | std::ios::eofbit | std::ios::failbit)) != std::ios::goodbit))
			{
				handle->cursor += count;
				lock.Unlock();
				return 1;
			}
			JE::LogDebugString("ResourceSystem::FileStreamRead - Bad read on file %s\n",
				handle->file->name.c_str());
			lock.Unlock();
			handle->cursor = static_cast<Net::UInt32>(
				static_cast<std::streamoff>(handle->file->input.tellg()));
			return 0;
		}

		void ResourceSystem::Release(ResourceFileHandle* handle, unsigned char flag)
		{
			lock.Lock();
			if (handle)
			{
				if (flag == 1)
					ResourceHandleServices::RegisterTemporary(*this, handle->file->path.c_str());
				ResourceHandleServices::ReleaseFile(files, handle->file);
				ResourceHandleServices::DestroyHandle(handle);
			}
			lock.Unlock();
		}

		// 0x49ACD0 registers directly in the actual manager, selecting
		// the temporary index. The incoming path remains the stored path.
		void ResourceSystem::RegisterTemporaryFile(const char* path)
		{
			std::string name(path);
			const std::string::size_type separator = name.rfind('\\');
			if (separator != std::string::npos) name.assign(name.substr(separator + 1));
			const Net::UInt32 type = ExtractFileType(name);
			lock.Lock();
			temporaryFileIndex.Upsert(name.c_str(), type, 1, path, 0xFFFFFFFFU);
			lock.Unlock();
		}

		namespace ResourceHandleServices
		{
			bool IsOpen(const ResourceFileEntry& entry) { return entry.input.is_open(); }
			void Seek(ResourceFileEntry& entry, Net::Int32 offset, int origin)
			{
				entry.input.seekg(static_cast<std::streamoff>(offset),
					static_cast<std::ios_base::seekdir>(origin));
			}
			Net::UInt32 Tell(ResourceFileEntry& entry)
			{
				return static_cast<Net::UInt32>(static_cast<std::streamoff>(entry.input.tellg()));
			}
			void Read(ResourceFileEntry& entry, void* output, Net::UInt32 count)
			{
				entry.input.read(static_cast<char*>(output), static_cast<std::streamsize>(count));
			}
			bool Failed(const ResourceFileEntry& entry)
			{
				return (entry.input.rdstate() & (std::ios::badbit | std::ios::eofbit | std::ios::failbit))
					!= std::ios::goodbit;
			}
			void RegisterTemporary(ResourceSystem& system, const char* path)
			{
				system.RegisterTemporaryFile(path);
			}
			void ReleaseFile(ResourceFileManager& manager, ResourceFileEntry* entry)
			{
				manager.Release(entry);
			}
			void DestroyHandle(ResourceFileHandle* handle) { delete handle; }
		}
	}
}
