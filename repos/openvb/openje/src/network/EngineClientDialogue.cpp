#include "network/EngineClient.hpp"
#include "gui/DialogueInterface.hpp"

namespace JE
{
	void Client::ReceiveDialogueRemoval(Net::Packet& packet) //588230.
	{
		BYTE id;
		packet.ReadByte(id);
		child0x9C->RemoveDialogue(id);
	}

	void Client::SendDialogueReply(Net::Packet& packet) //587850.
	{
		packet.field0x30 = 54;
		Send(packet, 0);
	}
	void Client::SendDialogueClose(Net::Packet& packet) //587830.
	{
		packet.field0x30 = 57;
		Send(packet, 1);
	}
}
