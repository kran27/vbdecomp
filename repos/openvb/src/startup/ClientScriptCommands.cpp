#include <winsock2.h>
#include "ClientScriptCommands.hpp"
#include "gui/DebugWindow.hpp"
#include "NetworkComponent.hpp"
#include "network/EngineClient.hpp"
#include "network/EngineServer.hpp"
#include <string>

namespace F3
{
	void CommandToggleScripts() //588880.
	{
		if (g_pNetworkInstance->GetMode() & 1)
		{
			JE::Server* server = static_cast<JE::Server*>(g_pNetworkInstance->server);
			server->field0x70E4 = !server->field0x70E4;
		}
	}

	void __cdecl CommandDisableScripts(CommandArguments*) //58E4A0.
	{
		//Capture the original Client+8C receiver before inspecting mode. The
		//message targets that same borrowed window after the server mutation.
		NetworkComponent* const network = g_pNetworkInstance;
		JE::DebugWindow* window = static_cast<JE::Client*>(network->client)->child0x8C;
		if (network->GetMode() & 1)
		{
			static_cast<JE::Server*>(g_pNetworkInstance->server)->field0x70E5 = 1;
			std::string message("Scripts disabled.");
			window->Write(message, 128, 0, 0);
		}
		else
		{
			std::string message("Unable to disable scripts from remote client.");
			window->Write(message, 128, 0, 0);
		}
	}

	void __cdecl CommandEnableScripts(CommandArguments*) //58E5B0.
	{
		NetworkComponent* const network = g_pNetworkInstance;
		JE::DebugWindow* window = static_cast<JE::Client*>(network->client)->child0x8C;
		if (network->GetMode() & 1)
		{
			static_cast<JE::Server*>(g_pNetworkInstance->server)->field0x70E5 = 0;
			std::string message("Scripts enabled.");
			window->Write(message, 0, 128, 0);
		}
		else
		{
			std::string message("Unable to enable scripts from remote client.");
			window->Write(message, 128, 0, 0);
		}
	}

	void __cdecl CommandOneScriptPass(CommandArguments*) //58E6C0.
	{
		NetworkComponent* const network = g_pNetworkInstance;
		JE::DebugWindow* window = static_cast<JE::Client*>(network->client)->child0x8C;
		if (network->GetMode() & 1)
		{
			static_cast<JE::Server*>(g_pNetworkInstance->server)->SetScriptStep(1);
			std::string message("Executing scripts for one game round.");
			window->Write(message, 128, 128, 0);
		}
		else
		{
			std::string message("Unable to control script execution from remote client.");
			window->Write(message, 128, 0, 0);
		}
	}

	unsigned char ExecuteClientScriptCommand(JE::Net::UInt32 originalVA, CommandArguments* arguments)
	{
		switch (originalVA)
		{
		case 0x588880: CommandToggleScripts(); return 1;
		case 0x58E4A0: CommandDisableScripts(arguments); return 1;
		case 0x58E5B0: CommandEnableScripts(arguments); return 1;
		case 0x58E6C0: CommandOneScriptPass(arguments); return 1;
		default: return 0;
		}
	}
}
