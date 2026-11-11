#pragma once

#include "ResourceSettings.hpp"
#include <windows.h>
#include "network/Types.hpp"
#include <fstream>
#include <map>
#include <string>

namespace JE
{
	namespace Resources
	{
		struct ResourceFileEntry
		{
			std::string name;
			std::string path;
			Net::UInt32 refs; // Constructor leaves refs/flags unwritten until successful Open.
			std::fstream input;
			Net::UInt32 flags;
		};

		struct ResourceFileNameLess
		{
			bool operator()(const char* left, const char* right) const;
		};
		// Keys borrow the published entry.name character buffer.
		typedef std::map<const char*, ResourceFileEntry*, ResourceFileNameLess> ResourceFileMap;

		class ResourceFileManager
		{
		private:
			ResourceFileMap m_Entries;

		public:
			ResourceFileManager(); // 0x50DBB0
			~ResourceFileManager(); // 0x50DD90
			void Configure(const char* root, const Configuration::Settings& settings); // 0x50CD40
			ResourceFileEntry* Open(const char* szName, const char* szDirectory,
				unsigned char mode); // 0x50DFA0
			void Release(ResourceFileEntry* entry); // 0x50DC60
			unsigned char UncompressToCache(const char* szDirectory, const char* szName,
				std::string& path); // 0x50D4C0
			void CleanupUnusedCache(); // 0x50D740
			FileManagerSettings settings; // Configured by actual 0x50CD40 before use.

		private:
			void Add(ResourceFileEntry* entry); // 0x50DEE0
			void Remove(ResourceFileMap::iterator& entry); // 0x50DBF0
			friend struct FileManagerVerification;
		};

		// Root provides the classified compressor integration using available libraries.
		unsigned char DecodeCompressedFile(const char* szSource, const char* szDestination,
			std::string& error);

		namespace FileManagerServices
		{
			enum Diagnostic
			{
				OpenFailure, AllocationFailure, IncreasedLimit, MissingRelease,
				OutstandingReferences, DecodeFailure, SearchFailure, DeleteFailure, CacheTooBig
			};
			ResourceFileEntry* AllocateEntry();
			void DestroyEntry(ResourceFileEntry* entry);
			unsigned char Exists(const char* szPath);
			void OpenInput(ResourceFileEntry& entry, const char* szPath, unsigned int mode);
			bool IsOpen(const ResourceFileEntry& entry);
			void CloseInput(ResourceFileEntry& entry);
			void SeekEnd(ResourceFileEntry& entry);
			Net::UInt32 Position(ResourceFileEntry& entry);
			void SeekBegin(ResourceFileEntry& entry);
			unsigned char Decode(const char* szSource, const char* szDestination, std::string& error);
			HANDLE First(const char* szPattern, WIN32_FIND_DATAA& found);
			bool Next(HANDLE search, WIN32_FIND_DATAA& found);
			void CloseSearch(HANDLE search);
			bool DeleteCacheFile(const char* szPath);
			void Report(Diagnostic kind, const char* szText, Net::UInt32 number = 0);
		}
	}
}
