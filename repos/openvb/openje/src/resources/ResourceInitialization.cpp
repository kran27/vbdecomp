#include "ResourceSystem.hpp"
#include "ResourceInitializationServices.hpp"
#include "ResourceOverrides.hpp"
#include "configuration/ConfigurationGetters.hpp"
#include "diagnostics/DebugOutput.hpp"

#include <new>

namespace JE
{
	namespace Resources
	{
		namespace
		{
			// File field definitions adapted from the supplied VanBurenTools
			// Other Tools/f3_unpacker.cpp F3RHTHeader/Entry. Use fixed DWORD widths
			// at the file boundary; these are not original C++ object layouts.
			struct ResourceHashHeader
			{
				Net::UInt32 vMajor;
				Net::UInt32 vMinor;
				Net::UInt32 nEntries;
				Net::UInt32 offsetPacks;
				Net::UInt32 offsetResources;
			};

			struct ResourceHashEntry
			{
				Net::UInt32 one;
				Net::UInt32 number;
				Net::UInt32 type;
				Net::UInt32 offset;
				Net::UInt32 pack;
			};

			typedef char ResourceHashHeaderWidth[(sizeof(ResourceHashHeader) == 20) ? 1 : -1];
			typedef char ResourceHashEntryWidth[(sizeof(ResourceHashEntry) == 20) ? 1 : -1];
		}

		// 0x49BF20. Original I/O status is ignored except for the open-state test.
		void ResourceSystem::Initialize(const char* root,
			const Configuration::Settings& settings)
		{
			std::ifstream input;
			Net::UInt32 resourceIndex = 0;
			std::string resourceName;
			std::string packPath;
			this->root = root;
			files.Configure(root, settings);
			primary.Initialize( static_cast<Net::UInt32>(Configuration::GetInteger(
				settings, "Resources", "resource_hashtable_size", 4096)));
			temporaryFileIndex.Initialize( static_cast<Net::UInt32>(Configuration::GetInteger(
				settings, "Resources", "temp_hashtable_size", 512)));

			input.open((std::string(this->root) + '\\' + "resource.rht").c_str(),
				std::ios::in | std::ios::binary);
			const bool opened = input.is_open();
			if (opened)
			{
			input.seekg(0, std::ios::end);
			const Net::UInt32 hashTableBytes = static_cast<Net::UInt32>(static_cast<std::streamoff>(input.tellg()));
			input.seekg(0, std::ios::beg);
			void* hashTableBuffer = new unsigned char[hashTableBytes];
			if (hashTableBuffer)
			{
				// Deliberately no raw-buffer cleanup guard: the original does not
				// free this allocation if a row/override child departs exceptionally.
				input.read(static_cast<char*>(hashTableBuffer), static_cast<std::streamsize>(hashTableBytes));
				const ResourceHashHeader* hashTableHeader = static_cast<ResourceHashHeader*>(hashTableBuffer);
				if (hashTableHeader->vMajor != 1 || hashTableHeader->vMinor != 1)
				{
					JE::FatalError("Hashtable file wrong version\n");
					::operator delete[](hashTableBuffer);
					input.close();
					return;
				}

				const char* hashTableData = static_cast<const char*>(hashTableBuffer);
				const ResourceHashEntry* resourceEntry = reinterpret_cast<const ResourceHashEntry*>(hashTableHeader + 1);
				for (; resourceIndex < hashTableHeader->nEntries; ++resourceIndex, ++resourceEntry)
				{
					resourceName.assign(hashTableData + hashTableHeader->offsetResources + resourceEntry->offset);
					packPath.assign(hashTableData + hashTableHeader->offsetPacks + resourceEntry->pack);
					primary.Upsert(resourceName.c_str(), resourceEntry->type, resourceEntry->one,
						packPath.c_str(), resourceEntry->number);
				}
				ConfigureOverrides(settings);
			}
			else
			{
				JE::LogDebugString("ResourceSystem::Init - Couldn't allocate %d for the resource hashtable file\n",
					static_cast<Net::Int32>(hashTableBytes));
			}
			::operator delete[](hashTableBuffer);
			input.close();
			}
			else
				JE::FatalError("Couldn't open hashtable file\n");
			ConfigurePaths(settings);
			const Net::UInt32 cacheBudget = static_cast<Net::UInt32>(Configuration::GetInteger(
				settings, "Resources", "cache_size", 1000000));
			cache.Configure(cacheBudget, this, &temporaryFileIndex);
		}

		namespace ResourceInitializationServices
		{
			void Open(std::ifstream& input, const char* pszPath)
			{
				input.open(pszPath, std::ios::in | std::ios::binary);
			}
			bool IsOpen(const std::ifstream& input) { return input.is_open(); }
			void Seek(std::ifstream& input, Net::Int32 nOffset, int nOrigin)
			{
				const std::ios::seekdir directions[] = {std::ios::beg, std::ios::cur, std::ios::end};
				input.seekg(static_cast<std::streamoff>(nOffset), directions[nOrigin]);
			}
			Net::UInt32 Tell(std::ifstream& input)
			{
				return static_cast<Net::UInt32>(static_cast<std::streamoff>(input.tellg()));
			}
			void Read(std::ifstream& input, void* pOutput, Net::UInt32 nBytes)
			{
				input.read(static_cast<char*>(pOutput), static_cast<std::streamsize>(nBytes));
			}
			void Close(std::ifstream& input) { input.close(); }
			void* Allocate(Net::UInt32 nBytes) { return new unsigned char[nBytes]; }
			void Free(void* pBuffer) { ::operator delete[](pBuffer); }
			void Upsert(ResourceIndex& index, const char* pszName, Net::UInt32 nType,
				Net::UInt32 nFlags, const char* pszPath, Net::UInt32 nOrdinal)
			{
				index.Upsert(pszName, nType, nFlags, pszPath, nOrdinal);
			}
		}
	}
}
