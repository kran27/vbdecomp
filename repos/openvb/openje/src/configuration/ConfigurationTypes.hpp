#pragma once

#include <cstring>
#include <map>
#include <string>

namespace JE
{
	namespace Configuration
	{
		// Shared real-library types, usable by C++98 callers independently of
		// the separate reader/resource implementation's remaining host headers.
		struct CaseInsensitiveLess
		{
			unsigned char operator()(const std::string& left, const std::string& right) const
			{
				return ::_stricmp(left.c_str(), right.c_str()) < 0;
			}
		};
		typedef std::map<std::string, std::string, CaseInsensitiveLess> Values;
		typedef std::map<std::string, Values, CaseInsensitiveLess> Settings;
	}
}
