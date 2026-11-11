#include "ConfigurationGetters.hpp"

#include <cstdlib>

namespace JE
{
	namespace Configuration
	{
		const char* GetString(const Settings& settings, const char* pszSection,
			const char* pszKey, const char* pszFallback)
		{
			Settings::const_iterator section = settings.find(std::string(pszSection));
			if (section != settings.end())
			{
				Values::const_iterator value;
				value = section->second.find(std::string(pszKey));
				if (value != section->second.end())
					return value->second.c_str();
			}
			return pszFallback;
		}

		int GetInteger(const Settings& settings, const char* pszSection,
			const char* pszKey, int nFallback)
		{
			Settings::const_iterator section = settings.find(std::string(pszSection));
			if (section != settings.end())
			{
				Values::const_iterator value;
				value = section->second.find(std::string(pszKey));
				if (value != section->second.end())
					return static_cast<int>(std::atol(value->second.c_str()));
			}
			return nFallback;
		}

		unsigned char GetBoolean(const Settings& settings, const char* pszSection,
			const char* pszKey, unsigned char nFallback)
		{
			Settings::const_iterator section = settings.find(std::string(pszSection));
			if (section != settings.end())
			{
				Values::const_iterator value;
				value = section->second.find(std::string(pszKey));
				if (value != section->second.end())
					return static_cast<unsigned char>(std::atol(value->second.c_str()) > 0);
			}
			return nFallback;
		}
	}
}
