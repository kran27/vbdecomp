#pragma once

#include "ResourceIndex.hpp"
#include "ResourceLock.hpp"
#include "configuration/ConfigurationTypes.hpp"

namespace JE
{
	namespace Resources
	{
		struct ResourceOverrideState
		{
			ResourceOverrideState(ResourceIndex& value, const char*& path, ResourceLock& guard)
				: index(value), root(path), lock(guard) {}
			ResourceIndex& index;
			const char*& root; // Borrowed owner field, reloaded by each recursive scan.
			ResourceLock& lock;
		};

		void ConfigureOverridePaths(ResourceOverrideState& state,
			const Configuration::Settings& settings); // 0x49BC60
		void ScanOverrideDirectory(ResourceOverrideState& state, const char* pszPath,
			unsigned char bRecursive); // 0x49B240
		void RegisterOverrideFile(ResourceOverrideState& state,
			const char* pszPath); // 0x49AA90

		namespace ResourceOverrideServices
		{
			HANDLE First(const char* pszPattern, WIN32_FIND_DATAA* pData);
			BOOL Next(HANDLE handle, WIN32_FIND_DATAA* pData);
			BOOL Close(HANDLE handle);
			void Upsert(ResourceIndex& index, const char* pszName, Net::UInt32 nType,
				Net::UInt32 nFlags, const char* pszPath, Net::UInt32 nOrdinal);
		}
	}
}
