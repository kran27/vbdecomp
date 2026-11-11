#include "ClientBinding.hpp"
#include <map>

namespace
{
	struct PacketBinding
	{
		JE::Net::Client::GamePacketSink sink;
		void* context;
	};
	typedef std::map<const JE::Net::Client*, PacketBinding> Bindings;
	Bindings& PacketBindings()
	{
		static Bindings bindings;
		return bindings;
	}
}

namespace JE { namespace Net
{
	void SetClientPacketSink(Client& client, Client::GamePacketSink sink, void* context)
	{
		if (!sink) { ClearClientPacketSink(client); return; }
		PacketBinding binding;
		binding.sink = sink;
		binding.context = context;
		PacketBindings()[&client] = binding;
	}
	bool HasClientPacketSink(const Client& client)
	{
		return PacketBindings().find(&client) != PacketBindings().end();
	}
	void ClearClientPacketSink(const Client& client)
	{
		PacketBindings().erase(&client);
	}
	bool DispatchClientPacketSink(Client& client, Packet& packet)
	{
		Bindings::const_iterator found = PacketBindings().find(&client);
		if (found == PacketBindings().end()) return false;
		const PacketBinding binding = found->second;
		binding.sink(client, packet, binding.context);
		return true;
	}
} }
