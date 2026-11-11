#include "ResourceOverrides.hpp"
#include "ResourceSystem.hpp"
#include "ResourceFormats.hpp"
#include "configuration/ConfigurationGetters.hpp"

#include <string>

namespace JE
{
	namespace Resources
	{
		void ResourceSystem::ConfigureOverrides(const Configuration::Settings& settings)
		{
			const unsigned char recursive = Configuration::GetBoolean(settings,
				"Resources", "override_recursive", 1);
			std::string remaining = Configuration::GetString(settings,
				"Resources", "override_paths", "\\override\\");
			std::string path;
			while (!remaining.empty())
			{
				const std::string::size_type separator = remaining.find(';');
				if (separator == std::string::npos)
				{
					if (remaining.c_str()[0] != ';')
						ScanOverrideDirectory(remaining.c_str(), recursive);
					break;
				}
				path.assign(remaining.substr(0, separator));
				remaining.assign(remaining.substr(separator + 1, remaining.size() - 1));
				ScanOverrideDirectory(path.c_str(), recursive);
			}
		}

		void ResourceSystem::ScanOverrideDirectory(const char* source, unsigned char recursive)
		{
			std::string path(source);
			// Native retains this separately constructed empty string through enumeration.
			std::string unusedSearch;
			if (path.c_str()[0] != '\\') path = std::string("\\") + path;
			if (path[path.size() - 1] != '\\') path.append(1, '\\');
			WIN32_FIND_DATAA data;
			HANDLE handle = FindFirstFileA((std::string(root) + path + "*.*").c_str(), &data);
			if (handle == INVALID_HANDLE_VALUE) return;
			do
			{
				if (data.cFileName[0] != '.')
				{
					if (data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
					{
						if (recursive)
							ScanOverrideDirectory((path + data.cFileName).c_str(), recursive);
					}
					else
						RegisterOverrideFile((path + data.cFileName).c_str());
				}
			} while (FindNextFileA(handle, &data));
			FindClose(handle);
		}

		void ResourceSystem::RegisterOverrideFile(const char* source)
		{
			std::string name(source);
			const std::string::size_type separator = name.rfind('\\');
			if (separator != std::string::npos) name.assign(name.substr(separator + 1));
			const Net::UInt32 type = ExtractFileType(name);
			lock.Lock();
			primary.Upsert(name.c_str(), type, 1, source, 0xFFFFFFFFU);
			lock.Unlock();
		}

		void ConfigureOverridePaths(ResourceOverrideState& state,
			const Configuration::Settings& settings)
		{
			const unsigned char bRecursive = Configuration::GetBoolean(settings,
				"Resources", "override_recursive", 1);
			std::string remaining = Configuration::GetString(settings,
				"Resources", "override_paths", "\\override\\");
			while (!remaining.empty())
			{
				const std::string::size_type nSeparator = remaining.find(';');
				if (nSeparator == std::string::npos)
				{
					if (remaining.c_str()[0] != ';')
						ScanOverrideDirectory(state, remaining.c_str(), bRecursive);
					break;
				}
				const std::string path = remaining.substr(0, nSeparator);
				remaining.assign(remaining.substr(nSeparator + 1, remaining.size() - 1));
				// Leading/repeated separators scan empty paths; a trailing empty
				// remainder is skipped by the loop's original length test.
				ScanOverrideDirectory(state, path.c_str(), bRecursive);
			}
		}

		void ScanOverrideDirectory(ResourceOverrideState& state, const char* pszPath,
			unsigned char bRecursive)
		{
			std::string path(pszPath);
			if (path.c_str()[0] != '\\') path = "\\" + path;
			if (path[path.size() - 1] != '\\') path.append(1, '\\');

			// The root belongs to the manager. Copy its current value for this
			// search; a child scan must reload that borrowed field after callbacks.
			const std::string pattern = std::string(state.root) + path + "*.*";
			WIN32_FIND_DATAA data;
			HANDLE handle = ResourceOverrideServices::First(pattern.c_str(), &data);
			if (handle == INVALID_HANDLE_VALUE) return;
			do
			{
				if (data.cFileName[0] != '.')
				{
					if (data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
					{
						if (bRecursive)
							ScanOverrideDirectory(state, (path + data.cFileName).c_str(), bRecursive);
					}
					else
						RegisterOverrideFile(state, (path + data.cFileName).c_str());
				}
			} while (ResourceOverrideServices::Next(handle, &data));
			// Original closes only after normal enumeration. Do not add an EH
			// close guard that changes its handle ownership on an exception.
			ResourceOverrideServices::Close(handle);
		}

		void RegisterOverrideFile(ResourceOverrideState& state, const char* pszPath)
		{
			std::string name(pszPath);
			const std::string::size_type nSeparator = name.rfind('\\');
			if (nSeparator != std::string::npos) name.assign(name.substr(nSeparator + 1));
			const Net::UInt32 nType = ExtractFileType(name);
			state.lock.Lock();
			ResourceOverrideServices::Upsert(state.index, name.c_str(), nType, 1,
				pszPath, 0xFFFFFFFFU);
			// No original exceptional-unlock guard exists around this registration.
			state.lock.Unlock();
		}

		namespace ResourceOverrideServices
		{
			HANDLE First(const char* pszPattern, WIN32_FIND_DATAA* pData)
			{
				return FindFirstFileA(pszPattern, pData);
			}
			BOOL Next(HANDLE handle, WIN32_FIND_DATAA* pData)
			{
				return FindNextFileA(handle, pData);
			}
			BOOL Close(HANDLE handle) { return FindClose(handle); }
			void Upsert(ResourceIndex& index, const char* pszName, Net::UInt32 nType,
				Net::UInt32 nFlags, const char* pszPath, Net::UInt32 nOrdinal)
			{
				index.Upsert(pszName, nType, nFlags, pszPath, nOrdinal);
			}
		}
	}
}
