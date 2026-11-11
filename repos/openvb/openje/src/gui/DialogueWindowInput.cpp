#include "network/EngineClient.hpp"
#include "startup/NetworkComponent.hpp"
#include "gui/DialogueWindow.hpp"
#include "network/Packet.hpp"

namespace JE
{
	void DialogueWindow::SelectReply(DWORD replyID)
	{
		Net::Packet packet; //Actual45F300, real stack owner/destructor45F280.
		packet.WriteByte(dialogueID); //Original+D60.
		packet.WriteByte(dialogueFlag); //Original+D61, wire revision, not bool.
		packet.WriteDword(replyID);
		packet.WriteDword(listenerID); //Original+D70, selected participant.
		// Resolve global Network/client only after all four writes, as5924FD.
		static_cast<Client*>(F3::g_pNetworkInstance->client)->SendDialogueReply(packet);
	}
}
