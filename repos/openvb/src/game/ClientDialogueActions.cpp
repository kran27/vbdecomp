#include "network/Types.hpp"
#include "ClientDialogueActions.hpp"
#include "GameActionInitiateDialogue.hpp"
#include "entity/Entity.hpp"
#include "network/Client.hpp"
#include "network/EngineClient.hpp"
#include "network/EngineServer.hpp"
#include "network/Packet.hpp"
#include <new>
#include <stdexcept>

namespace JE
{
	void Client::RequestTalk(DWORD actorID, DWORD targetID) //45F5D0.
	{
		JE::Net::Packet packet;
		packet.field0x30 = 73;
		packet.WriteDword(actorID);
		packet.WriteDword(targetID);
		Send(packet, 1);
	}

}

namespace F3
{
	BYTE ReceiveGameServerDialogueRequest(JE::Server& server, JE::Net::Packet& packet) //46DF38..46DFD7.
	{
		if (packet.field0x30 != 73) return 0;
		JE::Net::UInt32 actorID, targetID;
		packet.ReadDword(actorID);
		packet.ReadDword(targetID);
		JE::Entity* actor = JE::GetEntityPtr(actorID);
		JE::Entity* target = JE::GetEntityPtr(targetID);
		JE::DialogueManager* dialogue = static_cast<JE::DialogueManager*>(server.dialogue);
		if (!dialogue)
			throw std::logic_error("GameServer46DF71: original unchecked dialogue manager required");
		//55BDB0 returns the actual entries size, before either entity-pointer test.
		if (!dialogue->entries.empty() || !actor || !target) return 1;
		actor->ClearActions();
		GameActionInitiateDialogue* action = new GameActionInitiateDialogue;
		action->target = target;
		action->useTargetDialogue = 1;
		actor->QueueAction(action);
		return 1;
	}
}
