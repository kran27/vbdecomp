#include "EngineServer.hpp"
#include "entity/Entity.hpp"
#include "diagnostics/DebugOutput.hpp"

namespace JE
{
	void Server::AssignEntityClient(JE::Net::UInt32 entityID, JE::Net::UInt32 clientIndex)
	{
		//555750 captures the connection before resolving Entity. Its unique ID
		//is reloaded after SetClientOwner before publishing packet14.
		Net::EngineServerClientConnection* client=
			static_cast<Net::EngineServerClientConnection*>(connections[clientIndex]);
		Entity* entity=GetEntityPtr(entityID);
		entity->SetClientOwner(client->field0x4168);
		SendEntityClientAssignment(entityID,client->field0x4168);
	}
	void Server::WriteClientIdentity(JE::Net::UInt32 clientIndex, Net::Packet& reply)
	{
		//552CA0 prefix only. In the complete OnNewClient callback this is
		//followed by the server-World guard and World serialization into reply.
		//Keep packet write before publication: overflow must not assign the ID.
		const JE::Net::UInt32 identity=AllocateClientID();
		reply.WriteDword(identity);
		static_cast<Net::EngineServerClientConnection*>(connections[clientIndex])->field0x4168=identity;
		LogDebugString("Server: New client connected with an id of %u\n",identity);
	}
}
