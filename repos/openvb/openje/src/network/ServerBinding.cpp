#include "ServerBinding.hpp"
#include <map>

namespace
{
	struct PacketObserver
	{
		JE::Net::Server::UserPacketObserver observer;
		void* context;
	};
	typedef std::map<const JE::Net::Server*, PacketObserver> Bindings;
	Bindings& PacketObservers()
	{
		static Bindings bindings;
		return bindings;
	}
}

namespace JE { namespace Net
{
	void SetServerPacketObserver(Server& server, Server::UserPacketObserver observer, void* context)
	{
		if (!observer) { ClearServerPacketObserver(server); return; }
		PacketObserver binding;
		binding.observer = observer;
		binding.context = context;
		PacketObservers()[&server] = binding;
	}
	bool HasServerPacketObserver(const Server& server)
	{
		return PacketObservers().find(&server) != PacketObservers().end();
	}
	void ClearServerPacketObserver(const Server& server)
	{
		PacketObservers().erase(&server);
	}
	void DispatchServerPacketObserver(Server& server, UInt32 index, const Packet& packet)
	{
		Bindings::const_iterator found = PacketObservers().find(&server);
		if (found == PacketObservers().end()) return;
		const PacketObserver binding = found->second;
		binding.observer(binding.context, index, packet);
	}
} }
