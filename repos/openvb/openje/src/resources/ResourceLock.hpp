#pragma once

#include <windows.h>

namespace JE
{
	namespace Resources
	{
		// Thin ownership of the game's recursive Win32 critical section. This
		// is independent of any CRT lock or legacy object layout.
		class ResourceLock
		{
		public:
			ResourceLock();
			~ResourceLock();
			void Lock();
			void Unlock();

		private:
			ResourceLock(const ResourceLock&);
			ResourceLock& operator=(const ResourceLock&);
			CRITICAL_SECTION* m_pSection;
		};

		namespace ResourceLockServices
		{
			void Enter(CRITICAL_SECTION* pSection);
			void Leave(CRITICAL_SECTION* pSection);
		}
	}
}
