#include "EngineServer.hpp"
#include "entity/Entity.hpp"

namespace JE
{
	Server::~Server()
	{
		// Qualified call: original destructor does not dispatch derived Cleanup.
		Server::Cleanup();
	}

	void Server::DeleteEntity(Entity* entity) //5555A0 ->553790.
	{
		if (!entity) return;
		DWORD id = entity->entityID;
		delete entity;
		SendEntityDeletion(id);
	}

	void Server::SendEntityDeletion(DWORD entityID) //553790.
	{
		// Construct/publish only after successful virtual destruction.
		Net::Packet notification;
		notification.field0x30 = 10;
		notification.WriteDword(entityID);
		Broadcast(notification, 1);
	}
}
