#include <winsock2.h>
#include "ClientConsoleCommands.hpp"
#include "NetworkComponent.hpp"
#include "network/EngineClient.hpp"
#include "gui/ConsoleInterface.hpp"
#include "gui/DebugWindow.hpp"
#include "gui/Manager.hpp"
#include "gui/DebugToolsInterface.hpp"
#include "diagnostics/DebugOutput.hpp"
#include "graphics/CameraControl.hpp"
#include "graphics/Camera.hpp"
#include "graphics/SystemInterface.hpp"
#include <cstdlib>
#include <string>

namespace F3
{
	void CommandToggleConsoleWindow() //588630.
	{
		JE::ConsoleInterface* console = static_cast<JE::Client*>(g_pNetworkInstance->client)->child0x88;
		if (console->GetHidden())
		{
			console->ClearInput();
			console->SetHidden(0);
			console->BringToFront();
			::GUI::g_Manager.PushModal(console);
		}
	}

	void __cdecl CommandConnect(CommandArguments* arguments) //58DC50.
	{
		int count = static_cast<int>(arguments->size());
		if (count >= 2)
		{
			std::string host;
			host.clear();
			for (int i = 1; i < count; ++i)
			{
				host.append((*arguments)[i]);
				host.append(" ");
			}
			JE::Client* client=static_cast<JE::Client*>(g_pNetworkInstance->client);
			client->Connect(host.c_str());
		}
	}

	void __cdecl CommandDisconnect(CommandArguments*) //588290.
	{
		static_cast<JE::Client*>(g_pNetworkInstance->client)->Disconnect();
	}

	void __cdecl CommandDebugWindow(CommandArguments*) //588690.
	{
		JE::DebugWindow* window = static_cast<JE::Client*>(g_pNetworkInstance->client)->child0x8C;
		if (!window)
		{
			JE::LogDebugString("Client::CommandToggleDebugWindow - No Debug window was created in this client.\n");
			return;
		}
		if (window->GetHidden())
		{
			window->SetHidden(0);
			window->BringToFront();
		}
		else window->SetHidden(1);
	}

	void __cdecl CommandStackWindow(CommandArguments*) //5886F0.
	{
		JE::DebugWindow* window = static_cast<JE::Client*>(g_pNetworkInstance->client)->child0x90;
		if (!window)
		{
			JE::LogDebugString("Client::CommandToggleStackWindow - No Stack Window was created in this client.\n");
			return;
		}
		if (window->GetHidden())
		{
			window->SetHidden(0);
			window->BringToFront();
		}
		else window->SetHidden(1);
	}

	void __cdecl CommandDebugTools(CommandArguments*) //588750.
	{
		JE::DebugToolsInterface* window = static_cast<JE::Client*>(g_pNetworkInstance->client)->child0x94;
		if (!window)
		{
			JE::LogDebugString("Client::CommandToggleDebugTools - No Debug Tools window was created in this client.\n");
			return;
		}
		if (window->GetHidden())
		{
			window->SetHidden(0);
			window->BringToFront();
		}
		else window->SetHidden(1);
	}

	void __cdecl CommandEditToken(CommandArguments* arguments) //58BB30.
	{
		JE::Client* client = static_cast<JE::Client*>(g_pNetworkInstance->client);
		if (client->child0x74)
		{
			JE::ConsoleInterface* console = client->child0x88;
			if (console)
			{
				if (arguments->size() < 1) console->Print("No token passed in\r\n");
				else
				{
					client->EditToken(1, (*arguments)[0]);
					console->SetHidden(1);
				}
			}
		}
	}

	void __cdecl CommandCameraAzimuthSpeed(CommandArguments* arguments) //58B4A0.
	{
		JE::Client* client = static_cast<JE::Client*>(g_pNetworkInstance->client);
		JE::CameraControl* camera = client->child0x84;
		if (arguments->size() > 1)
		{
			const float value = static_cast<float>(atof((*arguments)[1]));
			camera->azimuthSpeed = value;
		}
		else client->child0x88->Print("Camera azimuth speed is %f", camera->azimuthSpeed);
	}

	void __cdecl CommandCameraElevationSpeed(CommandArguments* arguments) //58B550.
	{
		JE::Client* client = static_cast<JE::Client*>(g_pNetworkInstance->client);
		JE::CameraControl* camera = client->child0x84;
		if (arguments->size() > 1)
		{
			const float value = static_cast<float>(atof((*arguments)[1]));
			camera->elevationSpeed = value;
		}
		else client->child0x88->Print("Camera elevation speed is %f", camera->elevationSpeed);
	}

	void __cdecl CommandCameraDistanceSpeed(CommandArguments* arguments) //58B600.
	{
		JE::Client* client = static_cast<JE::Client*>(g_pNetworkInstance->client);
		JE::CameraControl* camera = client->child0x84;
		if (arguments->size() > 1)
		{
			const float value = static_cast<float>(atof((*arguments)[1]));
			camera->distanceSpeed = value;
		}
		else client->child0x88->Print("Camera distance speed is %f", camera->distanceSpeed);
	}

	void __cdecl CommandCameraScrollSpeed(CommandArguments* arguments) //58B7D0.
	{
		JE::Client* client = static_cast<JE::Client*>(g_pNetworkInstance->client);
		JE::CameraControl* camera = client->child0x84;
		if (arguments->size() > 1)
		{
			const float value = static_cast<float>(atof((*arguments)[1]));
			camera->scrollSpeed = value;
		}
		else client->child0x88->Print("Camera scroll speed is %f", camera->scrollSpeed);
	}

	void __cdecl CommandCameraFOV(CommandArguments* arguments) //58B660.
	{
		JE::CameraControl* camera = static_cast<JE::Client*>(g_pNetworkInstance->client)->child0x84;
		if (arguments->size() > 1) camera->SetFieldOfView(static_cast<float>(atof((*arguments)[1])));
		Gfx_Camera* active = g_GfxSystem->Camera();
		const float radians = active->fieldOfViewY;
		JE::ConsoleInterface* console = static_cast<JE::Client*>(g_pNetworkInstance->client)->child0x88;
		console->Print("Camera FOV is %f", radians * 57.295776f);
	}

	BYTE ExecuteClientConsoleCommand(DWORD originalVA, CommandArguments* arguments)
	{
		switch (originalVA)
		{
		case 0x588630: CommandToggleConsoleWindow(); return 1;
		case 0x58DC50: CommandConnect(arguments); return 1;
		case 0x588290: CommandDisconnect(arguments); return 1;
		case 0x588690: CommandDebugWindow(arguments); return 1;
		case 0x5886F0: CommandStackWindow(arguments); return 1;
		case 0x588750: CommandDebugTools(arguments); return 1;
		case 0x58BB30: CommandEditToken(arguments); return 1;
		case 0x58B4A0: CommandCameraAzimuthSpeed(arguments); return 1;
		case 0x58B550: CommandCameraElevationSpeed(arguments); return 1;
		case 0x58B600: CommandCameraDistanceSpeed(arguments); return 1;
		case 0x58B7D0: CommandCameraScrollSpeed(arguments); return 1;
		case 0x58B660: CommandCameraFOV(arguments); return 1;
		default: return 0;
		}
	}
}
