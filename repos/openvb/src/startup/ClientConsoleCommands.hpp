#pragma once

#include <winsock2.h>
#include "ConsoleCommands.hpp"

namespace F3
{
	void __cdecl CommandConnect(CommandArguments* arguments); //58DC50.
	void __cdecl CommandDisconnect(CommandArguments* arguments); //588290.
	void __cdecl CommandDebugWindow(CommandArguments* arguments); //588690.
	void __cdecl CommandStackWindow(CommandArguments* arguments); //5886F0.
	void __cdecl CommandDebugTools(CommandArguments* arguments); //588750.
	void __cdecl CommandEditToken(CommandArguments* arguments); //58BB30.
	void __cdecl CommandCameraAzimuthSpeed(CommandArguments* arguments); //58B4A0.
	void __cdecl CommandCameraElevationSpeed(CommandArguments* arguments); //58B550.
	void __cdecl CommandCameraDistanceSpeed(CommandArguments* arguments); //58B600.
	void __cdecl CommandCameraScrollSpeed(CommandArguments* arguments); //58B7D0.
	void __cdecl CommandCameraFOV(CommandArguments* arguments); //58B660.
	BYTE ExecuteClientConsoleCommand(DWORD originalVA, CommandArguments* arguments);
}
