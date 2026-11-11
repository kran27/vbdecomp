#include "ResourceLock.hpp"

#include <new>

namespace JE
{
	namespace Resources
	{
		ResourceLock::ResourceLock()
		{
			m_pSection = static_cast<CRITICAL_SECTION*>(::operator new(sizeof(CRITICAL_SECTION)));
			InitializeCriticalSection(m_pSection);
		}

		ResourceLock::~ResourceLock()
		{
			DeleteCriticalSection(m_pSection);
			::operator delete(m_pSection);
		}

		void ResourceLock::Lock() { ResourceLockServices::Enter(m_pSection); }
		void ResourceLock::Unlock() { ResourceLockServices::Leave(m_pSection); }

		namespace ResourceLockServices
		{
			void Enter(CRITICAL_SECTION* pSection) { EnterCriticalSection(pSection); }
			void Leave(CRITICAL_SECTION* pSection) { LeaveCriticalSection(pSection); }
		}
	}
}
