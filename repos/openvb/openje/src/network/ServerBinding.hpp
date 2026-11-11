#pragma once

#include "Server.hpp"

namespace JE { namespace Net
{
	// Borrowed workspace observation, external to the original5064h owner.
	// The scene registers/clears it; canonical Server lifetime has no hooks.
	void SetServerPacketObserver(Server& server, Server::UserPacketObserver observer, void* context);
	bool HasServerPacketObserver(const Server& server);
	void ClearServerPacketObserver(const Server& server);
	void DispatchServerPacketObserver(Server& server, UInt32 index, const Packet& packet);
} }
