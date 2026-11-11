#include <winsock2.h>
#include "ClientPipBoyCommand.hpp"
#include "NetworkComponent.hpp"
#include "network/EngineClient.hpp"
#include "game/GameInterfaces.hpp"
#include "game/InventoryInterface.hpp"
#include "game/OptionsInterface.hpp"
#include "game/PipBoyInterface.hpp"
#include "gui/DialogueInterface.hpp"

namespace F3
{
	void __cdecl CommandTogglePipBoy(CommandArguments*) //45EC70.
	{
		JE::Client* client = static_cast<JE::Client*>(g_pNetworkInstance->client);
		GameInterfaces* interfaces = static_cast<GameInterfaces*>(client->child0xA0);
		PipBoyInterface* pipBoy = interfaces->pipBoy;
		InventoryInterface* inventory = interfaces->inventory;
		OptionsInterface* options = interfaces->options;
		if (pipBoy)
		{
			if (pipBoy->GetHidden())
			{
				JE::DialogueInterface* dialogue = client->child0x9C;
				if (!dialogue || dialogue->AllWindowsHidden())
				{
					DWORD entity = client->field0xE0;
					pipBoy->SetHidden(0);
					pipBoy->Open(entity);
					if (inventory) inventory->SetHidden(1);
					if (options)
					{
						options->CloseChildren();
						options->SetHidden(1);
					}
				}
			}
			else pipBoy->Close();
		}
	}

	unsigned char ExecuteClientPipBoyCommand(JE::Net::UInt32 originalVA,
		CommandArguments* arguments)
	{
		if (originalVA != 0x45EC70) return 0;
		CommandTogglePipBoy(arguments);
		return 1;
	}
}
