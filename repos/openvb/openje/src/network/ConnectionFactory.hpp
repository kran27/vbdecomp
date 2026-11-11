#pragma once

#include "Connection.hpp"
#include <cstddef>
#include <new>

namespace JE
{
	namespace Net
	{
		namespace ConnectionFactoryServices
		{
			void* Allocate(std::size_t size);
			void Free(void* block);
		}

		// Source-level new-expression policy: the original game factories keep
		// their NULL result and constructor-failure ownership decisions. MSVC
		// supplies construction, member unwinding and object storage services.
		template<class ConnectionType> ConnectionType* ConstructConnection()
		{
			void* block = ConnectionFactoryServices::Allocate(sizeof(ConnectionType));
			if (!block) return NULL;
			try { return ::new (block) ConnectionType; }
			catch (...) { ConnectionFactoryServices::Free(block); throw; }
		}
	}
}
