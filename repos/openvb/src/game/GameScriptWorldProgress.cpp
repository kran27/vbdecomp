#include <winsock2.h>
#include "GameScriptNatives.hpp"
#include "network/EngineServer.hpp"
#include "scripting/ScriptEnvironment.hpp"
#include "../startup/NetworkComponent.hpp"

namespace F3
{
	cell AMX_NATIVE_CALL NativeSetWorldMapAreaVisible(AMX* vm, cell* parameters)
	{
		if (!VerifyScriptParameters(vm, parameters, 3, "WorldMapSetAreaVisible")) return 0;
		char filename[260];
		if (!JE::ScriptServices::ReadNativeString(vm, parameters[1], filename, 260))
		{
			LogScriptError(vm, "NativeWorldMapSetAreaVisible() was unable to retrieve filename.");
			return 0;
		}
		static_cast<JE::Server*>(g_pNetworkInstance->server)->SetWorldMapAreaVisible(filename, DWORD(parameters[2]), parameters[3] != 0);
		return 1;
	}
	cell AMX_NATIVE_CALL NativeSetWorldMapAreaExplorable(AMX* vm, cell* parameters)
	{
		if (!VerifyScriptParameters(vm, parameters, 3, "WorldMapSetAreaExplorable")) return 0;
		char filename[260];
		if (!JE::ScriptServices::ReadNativeString(vm, parameters[1], filename, 260))
		{
			LogScriptError(vm, "NativeWorldMapSetAreaExplorable() was unable to retrieve filename.");
			return 0;
		}
		static_cast<JE::Server*>(g_pNetworkInstance->server)->SetWorldMapAreaExplorable(filename, DWORD(parameters[2]), parameters[3] != 0);
		return 1;
	}
	cell AMX_NATIVE_CALL NativeSetWorldMapAreaExplored(AMX* vm, cell* parameters)
	{
		if (!VerifyScriptParameters(vm, parameters, 3, "WorldMapSetAreaExplored")) return 0;
		char filename[260];
		if (!JE::ScriptServices::ReadNativeString(vm, parameters[1], filename, 260))
		{
			LogScriptError(vm, "NativeWorldMapSetAreaExplored() was unable to retrieve filename.");
			return 0;
		}
		static_cast<JE::Server*>(g_pNetworkInstance->server)->SetWorldMapAreaExplored(filename, DWORD(parameters[2]), parameters[3] != 0);
		return 1;
	}
	cell AMX_NATIVE_CALL NativeSetEncyclopediaData(AMX* vm, cell* parameters)
	{
		if (!VerifyScriptParameters(vm, parameters, 3, "SetEncyclopediaData")) return 0;
		const DWORD clientIndex = DWORD(parameters[1]);
		const DWORD dataID = DWORD(parameters[2]);
		char filename[260];
		if (!JE::ScriptServices::ReadNativeString(vm, parameters[3], filename, 260))
		{
			LogScriptError(vm, "NativeSetEncyclopediaData() was unable to retrieve filename.");
			return 0;
		}
		static_cast<JE::Server*>(g_pNetworkInstance->server)->SetEncyclopediaData(clientIndex, dataID, filename);
		return 1;
	}
}
