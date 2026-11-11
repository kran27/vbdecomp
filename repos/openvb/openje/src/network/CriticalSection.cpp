#include "CriticalSection.hpp"
#include <cstdlib>

namespace JE { namespace Net
{
	CriticalSection::CriticalSection()
	{
		section = new CRITICAL_SECTION;
		InitializeCriticalSection(section);
	}
	CriticalSection::~CriticalSection()
	{
		DeleteCriticalSection(section);
		std::free(section); //Original uses free after operator new, not delete.
	}
} }
