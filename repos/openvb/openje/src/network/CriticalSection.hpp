#pragma once

#include <windows.h>

namespace JE { namespace Net
{
	// Original net::critical_section, embedded at Base+04; distinct from
	// the nonpolymorphic Resources::ResourceLock owner.
	class CriticalSection
	{
	public:
		CriticalSection(); //5B9270.
		virtual ~CriticalSection(); //5B92A0.
		void Lock() { EnterCriticalSection(section); }
		void Unlock() { LeaveCriticalSection(section); }
		CRITICAL_SECTION* section;
	private:
		CriticalSection(const CriticalSection&);
		CriticalSection& operator=(const CriticalSection&);
	};
} }
