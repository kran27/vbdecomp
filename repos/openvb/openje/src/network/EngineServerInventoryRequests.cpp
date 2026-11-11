#include "network/EngineServer.hpp"
#include "network/Packet.hpp"
#include "entity/Entity.hpp"
#include "entity/Inventory.hpp"

namespace JE
{
	//Original user-packet49 branch and its actual reliable response publisher.
	void Server::ReadInventoryTransferRequest(Net::Packet& request) //556EA0 case49 at557389.
	{
		JE::Net::UInt32 destinationID, itemID, sourceID; //Actual Buffer output-reference type.
		request.ReadDword(destinationID);
		request.ReadDword(itemID);
		request.ReadDword(sourceID);
		BYTE failure = 0;
		Entity* destination = GetEntityPtr(destinationID);
		if (!destination) failure = 1;
		Entity* source = GetEntityPtr(sourceID);
		if (!source) failure = 1;
		Entity* item = NULL;
		if (source)
		{
			item = source->inventory->FindByID(itemID);
			if (!item) failure = 1;
		}
		if (!failure)
			destination->inventory->Add(item); //Actual virtualAdd: transfer/removal/equipment callbacks.
		//Always publish captured original request IDs, including lookup failures.
		//Do not replace this with555640's post-Add live item.inventoryOwnerID.
		PublishInventoryTransfer(destinationID, itemID, sourceID, failure);
	}

	void Server::PublishInventoryTransfer(DWORD destinationID, DWORD itemID, DWORD sourceID,
		BYTE failure) //553850.
	{
		Net::Packet reply;
		reply.field0x30 = 49;
		reply.WriteByte(failure);
		reply.WriteDword(destinationID);
		reply.WriteDword(itemID);
		reply.WriteDword(sourceID);
		Broadcast(reply, 1);
	}
}
