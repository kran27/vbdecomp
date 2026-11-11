#include "ClientCommandExecution.hpp"
#include "ClientCommands.hpp"
#include "ClientCameraCommands.hpp"
#include "ClientLightingCommands.hpp"
#include "ClientScriptCommands.hpp"
#include "ClientConsoleCommands.hpp"
#include "ClientGraphicsCommands.hpp"
#include "ClientWorldCommands.hpp"
#include "ClientEntityCommands.hpp"
#include "ClientMediaCommands.hpp"
#include "ClientMemoryCommand.hpp"
#include "ClientPipBoyCommand.hpp"
#include "ClientUseSkillCommand.hpp"
#include "ClientNewGameCommand.hpp"
#include "GameClient.hpp"
#include "NetworkComponent.hpp"
#include "network/EngineClient.hpp"
#include "network/EngineServer.hpp"
#include "network/CommandProcessor.hpp"
#include "gui/ConsoleInterface.hpp"
#include "gui/Manager.hpp"
#include "entity/World.hpp"
#include "graphics/Graphics.hpp"
#include "graphics/SystemInterface.hpp"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <stdexcept>

namespace F3
{
	void __cdecl CommandGetGlobal(CommandArguments* arguments) //58DFD0.
	{
		if (arguments->size()!=2)
		{
			static_cast<JE::Client*>(g_pNetworkInstance->client)->child0x88->Print("Usage: GetGlobal <variable name>");
			return;
		}
		if (!JE::GetWorldInstance()->GetGlobalParameters()->HasValue((*arguments)[1],""))
		{
			static_cast<JE::Client*>(g_pNetworkInstance->client)->child0x88->Print("Global variable <%s> not registered.",(*arguments)[1]);
			return;
		}
		static_cast<JE::Client*>(g_pNetworkInstance->client)->child0x88->Print("<%s> = %d",(*arguments)[1],
			JE::GetWorldInstance()->GetGlobalParameters()->GetInteger((*arguments)[1],""));
	}

	void __cdecl CommandSetGlobal(CommandArguments* arguments) //58E1B0.
	{
		if (arguments->size()!=3)
		{
			static_cast<JE::Client*>(g_pNetworkInstance->client)->child0x88->Print("Usage: SetGlobal <variable name> <value>");
			return;
		}
		if (!JE::GetWorldInstance()->GetGlobalParameters()->HasValue((*arguments)[1],""))
			static_cast<JE::Client*>(g_pNetworkInstance->client)->child0x88->Print("Global variable <%s> not registered. It will be created.",(*arguments)[1]);
		static_cast<JE::Client*>(g_pNetworkInstance->client)->RequestSetGlobal((*arguments)[1],std::atol((*arguments)[2]));
	}

	void ExecuteClientCommand(JE::Net::UInt32 originalVA, CommandArguments* arguments)
	{
		if (ExecuteClientCameraCommand(originalVA, arguments)) return;
		if (ExecuteClientLightingCommand(originalVA, arguments)) return;
		if (ExecuteClientScriptCommand(originalVA, arguments)) return;
		if (ExecuteClientConsoleCommand(originalVA, arguments)) return;
		if (ExecuteClientGraphicsCommand(originalVA, arguments)) return;
		if (ExecuteClientWorldCommand(originalVA, arguments)) return;
		if (ExecuteClientEntityCommand(originalVA, arguments)) return;
		if (ExecuteClientMediaCommand(originalVA, arguments)) return;
		if (ExecuteClientMemoryCommand(originalVA, arguments)) return;
		if (ExecuteClientPipBoyCommand(originalVA, arguments)) return;
		if (ExecuteClientUseSkillCommand(originalVA, arguments)) return;
		switch (originalVA)
		{
		case 0x5887B0: JE::Client::CommandFontTest(); return;
		case 0x58DFD0: CommandGetGlobal(arguments); return;
		case 0x58E1B0: CommandSetGlobal(arguments); return;
		case 0x58DBE0:
			RequestNewGame(arguments);
			return;
		case 0x45EC00:
			static_cast<GameClient*>(g_pNetworkInstance->client)->ToggleEscapeMenu();
			return;
		case 0x45EB70:
			static_cast<GameClient*>(g_pNetworkInstance->client)->ToggleOptionsWindow();
			return;
		case 0x45EAC0:
			CommandToggleInventoryWindow(arguments);
			return;
		case 0x45EA50:
			static_cast<GameClient*>(g_pNetworkInstance->client)->ToggleActionbarWindow();
			return;
		case 0x45EE00:
			static_cast<GameClient*>(g_pNetworkInstance->client)->ToggleFeedbackWindow();
			return;
		case 0x45EED0:
			static_cast<GameClient*>(g_pNetworkInstance->client)->ToggleWeapon();
			return;
		case 0x45EE60:
			static_cast<GameClient*>(g_pNetworkInstance->client)->CycleAttackMode();
			return;
		default:
		{
			throw JE::UnrecoveredConsoleCommand(originalVA);
		}
		}
	}
}
