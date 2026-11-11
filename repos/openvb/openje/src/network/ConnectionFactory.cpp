#include "ConnectionFactory.hpp"

namespace JE { namespace Net
{
		Connection::~Connection()
		{
			Connection::Reset();
			// Normal reverse lifetime: packet1, packet0, queue, then Address.
			// The real std::deque owns its values and storage; no old template
			// destructor, block map or allocator layout is reconstructed.
		}

} }

namespace JE { namespace Net { namespace ConnectionFactoryServices
{
	void* Allocate(std::size_t size) { return ::operator new(size); }
	void Free(void* block) { ::operator delete(block); }
} } }
