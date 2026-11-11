#include "ResourceOpenServices.hpp"
#include "diagnostics/DebugOutput.hpp"

namespace JE
{
	namespace Resources
	{
		ResourceFileHandle* ResourceSystem::OpenStream(const char* szName, unsigned char mode)
		{
			std::string name(szName);
			const Net::UInt32 type = ResourceOpenServices::Classify(name);
			ResourceOpenServices::Lock(*this);
			ResourceDescriptor* descriptor;
			if (mode == 1)
			{
				descriptor = ResourceOpenServices::Find(temporaryFileIndex, name.c_str(), type);
				if (!descriptor)
					descriptor = ResourceOpenServices::Find(primary, name.c_str(), type);
			}
			else
			{
				descriptor = ResourceOpenServices::AllocateDescriptor();
				if (!descriptor)
				{
					ResourceOpenServices::DescriptorAllocationFailure();
					ResourceOpenServices::Unlock(*this);
					return NULL;
				}
				const std::size_t separator = name.find_last_of('\\');
				if (separator != std::string::npos) name = name.substr(separator + 1);
				descriptor->ordinal = 0xFFFFFFFF;
				descriptor->name = name;
				descriptor->path = szName;
				descriptor->flags = 1;
				// The temporary descriptor's type is unwritten in the original.
			}
			ResourceFileHandle* handle = OpenDescriptor(descriptor, mode);
			if (!mode && descriptor) ResourceOpenServices::DestroyDescriptor(descriptor);
			ResourceOpenServices::Unlock(*this);
			return handle;
		}

		ResourceFileHandle* ResourceSystem::OpenDescriptor(ResourceDescriptor* descriptor,
			unsigned char mode)
		{
			ResourceFileHandle* handle = OpenFile(descriptor, mode);
			if (!handle) return NULL;
			if (descriptor->ordinal == 0xFFFFFFFF)
			{
				handle->begin = 0;
				ResourceOpenServices::Seek(*handle->file, 0, 2);
				handle->end = ResourceOpenServices::Tell(*handle->file);
				ResourceOpenServices::Seek(*handle->file, 0, 0);
				handle->cursor = 0;
			}
			else
			{
				GroupEntryRange range;
				if (!ReadGroupEntryRange(descriptor, handle, &range))
				{
					ResourceOpenServices::Release(*this, handle, 0);
					return NULL;
				}
				ResourceOpenServices::Seek(*handle->file, static_cast<Net::Int32>(range.offset), 0);
				handle->begin = ResourceOpenServices::Tell(*handle->file);
				handle->end = handle->begin + range.length;
				handle->cursor = handle->begin;
			}
			return handle;
		}

		ResourceFileHandle* ResourceSystem::OpenFile(ResourceDescriptor* descriptor, unsigned char mode)
		{
			std::list<std::string> resources; //49AEBE owns the original unused36-byte sentinel.
			std::string name;
			std::string directory;
			if (!descriptor) return NULL;
			ResourceFileHandle* handle = ResourceOpenServices::AllocateHandle();
			if (handle)
			{
				if (descriptor->ordinal == 0xFFFFFFFF)
				{
					if (descriptor->path.c_str()[0] == '\\')
					{
						directory = root;
						directory += descriptor->path;
					}
					else directory = descriptor->path;
					// npos+1 wraps to zero: no backslash means an empty directory.
					const std::size_t split = directory.find_last_of('\\');
					name = directory.substr(split + 1);
					directory = directory.substr(0, split + 1);
				}
				else
				{
					directory = media[0];
					name = descriptor->path;
					name += '.';
					name += "grp";
				}
				ResourceFileEntry* file = ResourceOpenServices::Open(files,
					name.c_str(), directory.c_str(), mode);
				if (file)
				{
					handle->file = file;
					return handle;
				}
				// The original leaves this raw allocated handle leaked on failed Open.
			}
			ResourceOpenServices::HandleAcquisitionFailure();
			return NULL;
		}

		namespace ResourceOpenServices
		{
			Net::UInt32 Classify(std::string& name) { return ExtractFileType(name); }
			void Lock(ResourceSystem& system) { system.lock.Lock(); }
			void Unlock(ResourceSystem& system) { system.lock.Unlock(); }
			ResourceDescriptor* Find(ResourceIndex& index, const char* name, Net::UInt32 type)
			{
				return index.Find(name, type);
			}
			ResourceDescriptor* AllocateDescriptor() { return new ResourceDescriptor; }
			void DestroyDescriptor(ResourceDescriptor* descriptor) { delete descriptor; }
			ResourceFileHandle* AllocateHandle() { return new ResourceFileHandle; }
			ResourceFileEntry* Open(ResourceFileManager& files, const char* name,
				const char* directory, unsigned char mode) { return files.Open(name, directory, mode); }
			unsigned char Range(ResourceDescriptor& descriptor, ResourceFileHandle& handle,
				GroupEntryRange& range)
			{
				return ReadGroupEntryRange(handle.file->input, handle.file->name,
					descriptor.name, descriptor.ordinal, range);
			}
			void Seek(ResourceFileEntry& file, Net::Int32 offset, int origin)
			{
				file.input.seekg(offset, origin == 2 ? std::ios::end : std::ios::beg);
			}
			Net::UInt32 Tell(ResourceFileEntry& file)
			{
				return static_cast<Net::UInt32>(static_cast<std::streamoff>(file.input.tellg()));
			}
			void Release(ResourceSystem& system, ResourceFileHandle* handle, unsigned char flag)
			{
				system.Release(handle, flag);
			}
			void DescriptorAllocationFailure()
			{
				JE::LogDebugString("ResourceSystem::FileStreamOpen - Couldn't allocate %d\n", 72);
			}
			void HandleAcquisitionFailure()
			{
				JE::LogDebugString("ResourceSystem::OpenFile - Couldn't allocate enough memory for a ResourceFileStream_struct\n");
			}
		}
	}
}
