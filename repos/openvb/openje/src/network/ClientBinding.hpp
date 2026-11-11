#pragma once

#include "Client.hpp"

namespace JE { namespace Net
{
	// Nonowning workspace transport capture. The external owner explicitly
	// registers and clears it; canonical Client lifetime has no binding calls.
	void SetClientPacketSink(Client& client, Client::GamePacketSink sink, void* context);
	bool HasClientPacketSink(const Client& client);
	void ClearClientPacketSink(const Client& client);
	bool DispatchClientPacketSink(Client& client, Packet& packet);
} }
