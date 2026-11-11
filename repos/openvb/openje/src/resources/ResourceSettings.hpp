#pragma once

#include "configuration/ConfigurationTypes.hpp"

#include "network/Types.hpp"
#include <string>

namespace JE
{
	namespace Resources
	{
		struct ResourcePaths
		{
			enum { PathCount = 8 };
			std::string paths[PathCount];
					};

		struct FileManagerSettings
		{
			unsigned char managed;
			Net::UInt32 maxCount;
			Net::UInt32 bytes;
			Net::UInt32 count;
			const char* root;
		};

		// Standalone policy adapter; actual manager receiver is ConfigurePaths.
		void ConfigureResourcePaths(ResourcePaths& paths, const char* root,
			const Configuration::Settings& settings); // 0x499900
		void ConfigureFileManagerSettings(FileManagerSettings& fileManager,
			const char* root, const Configuration::Settings& settings); // Standalone settings adapter; no native receiver identity.
	}
}
