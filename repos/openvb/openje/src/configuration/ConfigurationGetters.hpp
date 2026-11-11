#pragma once

#include "ConfigurationTypes.hpp"

namespace JE
{
	namespace Configuration
	{
		const char* GetString(const Settings& settings, const char* pszSection,
			const char* pszKey, const char* pszFallback); // 0x4DF640
		int GetInteger(const Settings& settings, const char* pszSection,
			const char* pszKey, int nFallback); // 0x4DF770
		unsigned char GetBoolean(const Settings& settings, const char* pszSection,
			const char* pszKey, unsigned char nFallback); // 0x4DF8B0
	}
}
