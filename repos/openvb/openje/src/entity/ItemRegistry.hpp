#pragma once

#include <windows.h>
#include "configuration/ConfigurationTypes.hpp"
#include <map>
#include <string>

namespace JE
{
	// Original IWorld+AC owner at World+158; actual map storage, no extra fields/vptr.
	class ItemRegistry : public std::map<std::string,DWORD,Configuration::CaseInsensitiveLess>
	{
	public:
		typedef std::map<std::string,DWORD,Configuration::CaseInsensitiveLess> Base;
		BYTE Contains(const std::string& key); //5A0000, byte membership result.
		void RegisterName(const std::string& key); //5A04B0, assigns zero even if present.
	};
#if defined(_MSC_VER) && _MSC_VER == 1300 && defined(_M_IX86)
	typedef char ItemRegistrySize[(sizeof(ItemRegistry) == sizeof(ItemRegistry::Base)) ? 1 : -1];
	typedef char ItemRegistryAlignment[(__alignof(ItemRegistry) == __alignof(ItemRegistry::Base)) ? 1 : -1];
#endif
}
