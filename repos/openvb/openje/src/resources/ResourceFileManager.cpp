#include "ResourceFileManager.hpp"
#include "diagnostics/DebugOutput.hpp"
#include "filesystem/FileOperations.hpp"
#include <cstring>

namespace JE
{
	namespace Resources
	{
		bool ResourceFileNameLess::operator()(const char* left, const char* right) const
		{
			return ::_stricmp(left, right) < 0;
		}

		ResourceFileManager::ResourceFileManager()
		{
			settings.managed = 0;
			settings.maxCount = 0;
			settings.bytes = 0;
			settings.count = 0;
			settings.root = NULL;
		}

		ResourceFileEntry* ResourceFileManager::Open(const char* szName,
			const char* szDirectory, unsigned char mode)
		{
			std::string path;
			bool addedCache = false;
			ResourceFileMap::iterator existing = m_Entries.find(szName);
			ResourceFileEntry* entry;
			if (settings.managed == 1 && mode == 1 && existing != m_Entries.end())
			{
				entry = existing->second;
				++entry->refs;
			}
			else
			{
				entry = new ResourceFileEntry;
				if (entry)
				{
					path.assign(szDirectory);
					path.append(szName);
					if (!mode)
						entry->input.open(path.c_str(), std::ios::binary | std::ios::out);
					else if (static_cast<unsigned char>(JE::FileExists(path.c_str())) == 1)
						entry->input.open(path.c_str(), std::ios::binary | std::ios::in);
					else if (UncompressToCache(szDirectory, szName, path) == 1)
					{
						entry->input.open(path.c_str(), std::ios::binary | std::ios::in);
						if (entry->input.is_open())
						{
							entry->input.seekg(0, std::ios::end);
							const Net::UInt32 length = static_cast<Net::UInt32>(static_cast<std::streamoff>(entry->input.tellg()));
							entry->input.seekg(0, std::ios::beg);
							settings.count += length;
							addedCache = true;
						}
					}
					if (!entry->input.is_open())
					{
						JE::LogDebugString("ResourceFileManager::OpenFile - Couldn't open file %s\n", szName);
						delete entry;
						entry = NULL;
					}
					else
					{
						entry->flags = 0;
						if (!settings.managed || !mode) entry->flags = 1;
						if (mode == 1) entry->flags |= 2;
						entry->path = path;
						entry->name = szName;
						entry->refs = 1;
						if (existing != m_Entries.end()) ++existing->second->refs;
						else Add(entry);
						if (addedCache) CleanupUnusedCache();
					}
				}
				else JE::LogDebugString("ResourceFileManager::OpenFile - Memory allocation error opening file %s\n", szName);
			}
			return entry;
		}

		void ResourceFileManager::Add(ResourceFileEntry* entry)
		{
			if (settings.managed == 1 && m_Entries.size() >= settings.maxCount)
			{
				ResourceFileMap::iterator unused = m_Entries.begin();
				while (unused != m_Entries.end())
				{
					if (!unused->second->refs)
					{
						Remove(unused);
						break;
					}
					++unused;
				}
				// Erasing the last entry also returns end and increases the limit originally.
				if (unused == m_Entries.end())
				{
					++settings.maxCount;
					FileManagerServices::Report(FileManagerServices::IncreasedLimit,
						entry->name.c_str(), settings.maxCount);
				}
			}
			m_Entries[entry->name.c_str()] = entry;
		}

		void ResourceFileManager::Remove(ResourceFileMap::iterator& found)
		{
			FileManagerServices::CloseInput(*found->second);
			ResourceFileEntry* entry = found->second;
			if (entry) FileManagerServices::DestroyEntry(entry);
			found = m_Entries.erase(found);
		}

		void ResourceFileManager::Release(ResourceFileEntry* entry)
		{
			ResourceFileMap::iterator found = m_Entries.find(entry->name.c_str());
			if (found == m_Entries.end())
			{
				FileManagerServices::Report(FileManagerServices::MissingRelease, entry->name.c_str());
				FileManagerServices::CloseInput(*entry);
				FileManagerServices::DestroyEntry(entry);
			}
			else --found->second->refs;
			if (!settings.managed)
			{
				// The original does not guard the missing-entry path before this dereference.
				if (!found->second->refs) Remove(found);
				else
				{
					FileManagerServices::CloseInput(*entry);
					FileManagerServices::DestroyEntry(entry);
				}
			}
		}

		ResourceFileManager::~ResourceFileManager()
		{
			for (ResourceFileMap::const_iterator pair = m_Entries.begin();
				pair != m_Entries.end(); ++pair)
			{
				ResourceFileEntry* entry = pair->second;
				if (entry->refs > 0)
					FileManagerServices::Report(FileManagerServices::OutstandingReferences,
						entry->name.c_str(), entry->refs);
				pair->second->input.close();
				entry = pair->second;
				if (entry) FileManagerServices::DestroyEntry(entry);
			}
		}

		unsigned char ResourceFileManager::UncompressToCache(const char* szDirectory,
			const char* szName, std::string& path)
		{
			std::string error;
			std::string source(szDirectory);
			source.append(szName);
			std::string destination(settings.root);
			destination.append("\\cache\\");
			destination.append(szName);
			if (FileManagerServices::Exists(destination.c_str()) != 1)
			{
				source.append(1, '.');
				source.append("z");
				if (!FileManagerServices::Decode(source.c_str(), destination.c_str(), error))
				{
					FileManagerServices::Report(FileManagerServices::DecodeFailure, error.c_str());
					return 0;
				}
			}
			path = destination;
			return 1;
		}

		void ResourceFileManager::CleanupUnusedCache()
		{
			std::string directory(settings.root);
			directory.append("\\cache\\");
			WIN32_FIND_DATAA found;
			HANDLE search = FileManagerServices::First((directory + "*.*").c_str(), found);
			// Preserve the original search-handle leak when cleaning is below its budget.
			if (settings.bytes > settings.count) return;
			if (search == INVALID_HANDLE_VALUE)
			{
				FileManagerServices::Report(FileManagerServices::SearchFailure, directory.c_str());
				return;
			}
			do
			{
				if (found.cFileName[0] == '.' || (found.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
					continue;
				ResourceFileMap::iterator entry;
				entry = m_Entries.find(found.cFileName);
				if (entry == m_Entries.end() || !entry->second->refs)
				{
					if (!FileManagerServices::DeleteCacheFile((directory + found.cFileName).c_str()))
						settings.count -= found.nFileSizeLow;
					else
						FileManagerServices::Report(FileManagerServices::DeleteFailure, found.cFileName);
				}
			} while (FileManagerServices::Next(search, found));
			FileManagerServices::CloseSearch(search);
			if (settings.count > settings.bytes)
				FileManagerServices::Report(FileManagerServices::CacheTooBig, "", settings.count);
		}

		namespace FileManagerServices
		{
			ResourceFileEntry* AllocateEntry() { return new ResourceFileEntry; }
			void DestroyEntry(ResourceFileEntry* entry) { delete entry; }
			unsigned char Exists(const char* path) { return static_cast<unsigned char>(JE::FileExists(path)); }
			void OpenInput(ResourceFileEntry& entry, const char* path, unsigned int mode)
			{
				entry.input.open(path, std::ios::binary | (mode == 0x21 ? std::ios::in : std::ios::out));
			}
			bool IsOpen(const ResourceFileEntry& entry) { return entry.input.is_open(); }
			void CloseInput(ResourceFileEntry& entry) { entry.input.close(); }
			void SeekEnd(ResourceFileEntry& entry) { entry.input.seekg(0, std::ios::end); }
			Net::UInt32 Position(ResourceFileEntry& entry)
			{
				return static_cast<Net::UInt32>(static_cast<std::streamoff>(entry.input.tellg()));
			}
			void SeekBegin(ResourceFileEntry& entry) { entry.input.seekg(0, std::ios::beg); }
			unsigned char Decode(const char* source, const char* destination, std::string& error)
			{
				return DecodeCompressedFile(source, destination, error);
			}
			HANDLE First(const char* pattern, WIN32_FIND_DATAA& found) { return ::FindFirstFileA(pattern, &found); }
			bool Next(HANDLE search, WIN32_FIND_DATAA& found) { return ::FindNextFileA(search, &found) != 0; }
			void CloseSearch(HANDLE search) { ::FindClose(search); }
			bool DeleteCacheFile(const char* path) { return ::DeleteFileA(path) != 0; }
			void Report(Diagnostic kind, const char* text, Net::UInt32 number)
			{
				switch (kind)
				{
				case OpenFailure: JE::LogDebugString("ResourceFileManager::OpenFile - Couldn't open file %s\n", text); break;
				case AllocationFailure: JE::LogDebugString("ResourceFileManager::OpenFile - Memory allocation error opening file %s\n", text); break;
				case IncreasedLimit: JE::LogDebugString("ResourceFileManager::Add - Had to increase the number of files open to %d  because of file stream->name\n", static_cast<int>(number), text); break;
				case MissingRelease: JE::LogDebugString("ResourceFileManager::ReleaseFile - Couldn't find %s\n", text); break;
				case OutstandingReferences: JE::LogDebugString("ResourceFileManager::~ResourceFileManager - %s  is still damanded %d times\n", text, static_cast<int>(number)); break;
				case DecodeFailure: JE::LogDebugString("ResourceFileManager::UncompressToCache - %s\n", text); break;
				case SearchFailure: JE::LogDebugString("ResourceFileManager::CleanupUnusedCache - Couldn't get a search handle for %s\n", text); break;
				case DeleteFailure: JE::LogDebugString("ResourceFileManager::CleanupUnusedCache - Couldn't delete file from cache %s\n", text); break;
				case CacheTooBig: JE::LogDebugString("ResourceFileManager::CleanupUnusedCache - Cache is still to big after cleaning - %d\n", static_cast<int>(number)); break;
				}
			}
		}
	}
}
