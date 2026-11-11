#include "Client.hpp"
#include "Connection.hpp"
#include "DebugPrint.hpp"
#include <cstdio>

namespace JE
{
	namespace Net
	{
		Client::~Client()
		{
			Client::Cleanup();
			// Normal lifetime then destroys serverAddress and the Base owner.
		}

		void Client::Send(Packet& packet, JE::Net::UInt32 channel)
		{
			char destination[64];
			debugStream.Print(1, "[client %u] Sending packet to server at %s\n",
				address.port, packet.address.Format(destination, sizeof(destination), 1));
			connection->Send(packet, channel);
		}
	}
}
