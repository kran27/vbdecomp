#include "ResourceSettings.hpp"
#include "ResourceSystem.hpp"
#include "configuration/ConfigurationGetters.hpp"

namespace JE
{
	namespace Resources
	{
		// Standalone policy adapter. Empty CD settings retain previous paths.
		static void ConfigurePathArray(std::string* paths, const char* root,
			const Configuration::Settings& settings)
		{
			static const char* const keys[8] =
			{
				"HD", "CD1", "CD2", "CD3", "CD4", "CD5", "CD6", "CD7"
			};
			for (std::size_t nIndex = 0; nIndex < ResourcePaths::PathCount; ++nIndex)
			{
				std::string sPath(Configuration::GetString(settings, "Resources", keys[nIndex], ""));
				if (nIndex == 0 && sPath.empty())
					sPath = root;
				else if (sPath.empty())
					continue;
				// Original tests the terminal NUL, not size()-1. A trailing slash
				// therefore gains another slash; preserve that path decision.
				if (sPath.c_str()[sPath.size()] != '\\')
					sPath.append(1, '\\');
				paths[nIndex] = sPath + "data\\";
			}
		}

		void ConfigureResourcePaths(ResourcePaths& paths, const char* root,
			const Configuration::Settings& settings)
		{
			ConfigurePathArray(paths.paths, root, settings);
		}

		void ResourceSystem::ConfigurePaths(const Configuration::Settings& settings)
		{
			std::string path;
			path = Configuration::GetString(settings, "Resources", "HD", "");
			if (path.size() > 0)
			{
				if (path.c_str()[path.size()] != '\\')
					path.append(1, '\\');
				media[0] = path + "data\\";
			}
			else
			{
				path = root;
				if (path.c_str()[path.size()] != '\\')
					path.append(1, '\\');
				media[0] = path + "data\\";
			}
			path = Configuration::GetString(settings, "Resources", "CD1", "");
			if (path.size() > 0)
			{
				if (path.c_str()[path.size()] != '\\')
					path.append(1, '\\');
				media[1] = path + "data\\";
			}
			path = Configuration::GetString(settings, "Resources", "CD2", "");
			if (path.size() > 0)
			{
				if (path.c_str()[path.size()] != '\\')
					path.append(1, '\\');
				media[2] = path + "data\\";
			}
			path = Configuration::GetString(settings, "Resources", "CD3", "");
			if (path.size() > 0)
			{
				if (path.c_str()[path.size()] != '\\')
					path.append(1, '\\');
				media[3] = path + "data\\";
			}
			path = Configuration::GetString(settings, "Resources", "CD4", "");
			if (path.size() > 0)
			{
				if (path.c_str()[path.size()] != '\\')
					path.append(1, '\\');
				media[4] = path + "data\\";
			}
			path = Configuration::GetString(settings, "Resources", "CD5", "");
			if (path.size() > 0)
			{
				if (path.c_str()[path.size()] != '\\')
					path.append(1, '\\');
				media[5] = path + "data\\";
			}
			path = Configuration::GetString(settings, "Resources", "CD6", "");
			if (path.size() > 0)
			{
				if (path.c_str()[path.size()] != '\\')
					path.append(1, '\\');
				media[6] = path + "data\\";
			}
			path = Configuration::GetString(settings, "Resources", "CD7", "");
			if (path.size() > 0)
			{
				if (path.c_str()[path.size()] != '\\')
					path.append(1, '\\');
				media[7] = path + "data\\";
			}
		}

		// 0x50CD40. The native receiver is the manager, with two stack arguments.
		void ResourceFileManager::Configure(const char* root,
			const Configuration::Settings& configuration)
		{
			settings.root = root;
			settings.managed = Configuration::GetBoolean(configuration, "Resources",
				"use_managed_file", 0);
			settings.count = 0;
			settings.bytes = static_cast<Net::UInt32>(Configuration::GetInteger(configuration,
				"Resources", "file_cache_size", 0x20000000));
			settings.maxCount = static_cast<Net::UInt32>(Configuration::GetInteger(configuration,
				"Resources", "file_max_count", 10));
		}

		// Preserve the standalone settings API without fabricating a manager lifetime.
		void ConfigureFileManagerSettings(FileManagerSettings& fileManager,
			const char* root, const Configuration::Settings& settings)
		{
			fileManager.root = root;
			fileManager.managed = Configuration::GetBoolean(settings, "Resources",
				"use_managed_file", 0);
			fileManager.count = 0;
			fileManager.bytes = static_cast<Net::UInt32>(Configuration::GetInteger(settings,
				"Resources", "file_cache_size", 0x20000000));
			fileManager.maxCount = static_cast<Net::UInt32>(Configuration::GetInteger(settings,
				"Resources", "file_max_count", 10));
		}
	}
}
