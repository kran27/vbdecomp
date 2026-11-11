#include <winsock2.h>
#include "EngineServer.hpp"
#include "startup/NetworkComponent.hpp"
#include "scripting/ScriptSmall.hpp"
#include "Packet.hpp"
#include "Connection.hpp"

#include <cstring>

namespace JE
{
	namespace Net
	{
		Packet::Packet() : Buffer(payload, sizeof(payload)), field0x24(0), field0x28(0xFFFFFFFF), field0x2C(0), field0x30(0)
		{
		}

		ConnectionStatistics::ConnectionStatistics()
		{
			meanLatency = 0;
			echoedTick = 0;
			for (unsigned int index=0;index<16;++index) latencySamples[index]=0;
			sequenceGap = 0;
			traffic[0].Reset();
			traffic[1].Reset();
			traffic[2].Reset();
		}

		Connection::Connection() : state(0) {}


		Packet::Packet(const Packet& source)
			: Buffer(payload, sizeof(payload))
		{
			*this = source;
		}

		void Packet::Reset()
		{
			field0x10 = 0;
			field0x0C = 0;
			address.bytes[0] = 0;
			address.bytes[1] = 0;
			address.bytes[2] = 0;
			address.bytes[3] = 0;
			address.port = 0;
			field0x24 = 0;
			field0x28 = 0xFFFFFFFF;
			field0x2C = 0;
			field0x30 = 0;
			field0x34 = 0;
		}
	}
}

namespace JE
{
	void DialogueManager::EndAllDialogues() //55F1D0.
	{
		DialogueEntries::iterator entry = entries.begin();
		while (entry != entries.end())
		{
			Net::Packet notification;
			Net::UInt8 id = entry->second.field0x0;
			F3::NetworkComponent* network = F3::g_pNetworkInstance;
			notification.WriteByte(id);
			static_cast<Server*>(network->server)->SendDialogueEnd(notification);
			if (entry->second.script)
			{
				delete entry->second.script;
				entry->second.script = NULL;
			}
			entry = entries.erase(entry);
		}
	}

}
