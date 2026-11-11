#include <winsock2.h>
#include "GameScriptNatives.hpp"
#include "GameScriptMapContext.hpp"
#include "scripting/ScriptEnvironment.hpp"
#include "../startup/NetworkComponent.hpp"
#include "entity/Entity.hpp"
#include "entity/EntityEnvironment.hpp"
#include "entity/Map.hpp"
#include "entity/MapFog.hpp"
#include "entity/World.hpp"
#include "graphics/SystemInterface.hpp"
#include "network/EngineServer.hpp"
#include <string.h>
#include <stdexcept>

namespace F3
{
	cell AMX_NATIVE_CALL NativeGetCameraFog(AMX* vm, cell* parameters) //5C2BB0.
	{
		if (!VerifyScriptParameters(vm, parameters, 5, "GetCameraFog")) return 0;
		BYTE enabled = 1;
		void* owner;
		if (amx_GetUserData(vm, 0x464C534D, &owner))
		{
			LogScriptError(vm, "GetCameraFog() was unable to resolve user ID. (internal error)");
			return 0;
		}
		DWORD mapID = ResolveScriptMapID(reinterpret_cast<DWORD>(owner));
		if (!mapID)
		{
			LogScriptError(vm, "GetCameraFog() was unable to resolve the map.");
			return 0;
		}
		JE::Map* map = JE::GetWorldInstance()->GetMap(mapID);
		if (!map)
		{
			LogScriptError(vm, "GetCameraFog() hit an internal error resolving the map.");
			return 0;
		}
		float first, second, density;
		DWORD color;
		map->GetCameraFog(first, second, color, density, enabled);
		cell rgb[3] = { cell((color >> 16) & 0xFFu),
			cell((color >> 8) & 0xFFu), cell(color & 0xFFu) };
		JE::ScriptServices::WriteNativeFloat(vm, parameters[1], first);
		JE::ScriptServices::WriteNativeFloat(vm, parameters[2], second);
		if (!WriteScriptCells(vm, parameters[3], rgb, 3))
		{
			LogScriptError(vm, "GetDistanceFog() was unable to set the color vector parameter.");
			return 0;
		}
		JE::ScriptServices::WriteNativeFloat(vm, parameters[4], density);
		JE::ScriptServices::WriteNativeInteger(vm, parameters[5], enabled);
		return 1;
	}

	cell AMX_NATIVE_CALL NativeSetCameraFog(AMX* vm, cell* parameters) //5C2D20.
	{
		if (!VerifyScriptParameters(vm, parameters, 5, "SetCameraFog")) return 0;
		float nearLimit, farLimit, density;
		JE::ScriptServices::ReadNativeFloat(vm, parameters[1], nearLimit);
		JE::ScriptServices::ReadNativeFloat(vm, parameters[2], farLimit);
		cell rgb[3];
		if (!ReadScriptCells(vm, parameters[3], rgb, 3))
		{
			LogScriptError(vm, "SetCameraFog() was unable to retrieve the color vector.");
			return 0;
		}
		JE::ScriptServices::ReadNativeFloat(vm, parameters[4], density);
		if (farLimit <= nearLimit)
		{
			LogScriptError(vm, "SetCameraFog() - farLimit can not be <= the nearLimit.");
			return 0;
		}
		if (density <= 0.0 || density > 1.0)
		{
			LogScriptError(vm, "SetCameraFog() - density must greater than 0.0 and no more than 1.0");
			return 0;
		}
		const DWORD color = (((DWORD(rgb[0]) | 0xFFFFFF00u) << 8)
			| (DWORD(rgb[1]) & 0xFFu)) << 8 | (DWORD(rgb[2]) & 0xFFu);
		void* owner;
		if (amx_GetUserData(vm, 0x464C534D, &owner))
		{
			LogScriptError(vm, "SetCameraFog() was unable to resolve user ID. (internal error)");
			return 0;
		}
		DWORD mapID = ResolveScriptMapID(reinterpret_cast<DWORD>(owner));
		if (!mapID)
		{
			LogScriptError(vm, "SetCameraFog() was unable to resolve the map.");
			return 0;
		}
		static_cast<JE::Server*>(g_pNetworkInstance->server)->SetCameraFog(mapID,
			nearLimit, farLimit, color, density, 1);
		return 1;
	}

	cell AMX_NATIVE_CALL NativeResetCameraFog(AMX* vm, cell* parameters) //5C2EB0.
	{
		if (!VerifyScriptParameters(vm, parameters, 0, "ResetCameraFog")) return 0;
		void* owner;
		if (amx_GetUserData(vm, 0x464C534D, &owner))
		{
			LogScriptError(vm, "ResetCameraFog() was unable to resolve user ID. (internal error)");
			return 0;
		}
		DWORD mapID = ResolveScriptMapID(reinterpret_cast<DWORD>(owner));
		if (!mapID)
		{
			LogScriptError(vm, "ResetCameraFog() was unable to resolve the map.");
			return 0;
		}
		static_cast<JE::Server*>(g_pNetworkInstance->server)->ResetCameraFog(mapID);
		return 1;
	}

	cell AMX_NATIVE_CALL NativeGetHeightFog(AMX* vm, cell* parameters) //5C2F40.
	{
		if (!VerifyScriptParameters(vm, parameters, 5, "GetHeightFog")) return 0;
		BYTE enabled = 1;
		void* owner;
		if (amx_GetUserData(vm, 0x464C534D, &owner))
		{
			LogScriptError(vm, "GetHeightFog() was unable to resolve user ID. (internal error)");
			return 0;
		}
		DWORD mapID = ResolveScriptMapID(reinterpret_cast<DWORD>(owner));
		if (!mapID)
		{
			LogScriptError(vm, "GetHeightFog() was unable to resolve the map.");
			return 0;
		}
		JE::Map* map = JE::GetWorldInstance()->GetMap(mapID);
		if (!map)
		{
			LogScriptError(vm, "GetHeightFog() hit an internal error resolving the map.");
			return 0;
		}
		float first, second, density;
		DWORD color;
		map->GetHeightFog(first, second, color, density, enabled);
		cell rgb[3] = { cell((color >> 16) & 0xFFu),
			cell((color >> 8) & 0xFFu), cell(color & 0xFFu) };
		JE::ScriptServices::WriteNativeFloat(vm, parameters[1], first);
		JE::ScriptServices::WriteNativeFloat(vm, parameters[2], second);
		if (!WriteScriptCells(vm, parameters[3], rgb, 3))
		{
			LogScriptError(vm, "GetHeightFog() was unable to set the color vector parameter.");
			return 0;
		}
		JE::ScriptServices::WriteNativeFloat(vm, parameters[4], density);
		JE::ScriptServices::WriteNativeInteger(vm, parameters[5], enabled);
		return 1;
	}

	cell AMX_NATIVE_CALL NativeSetHeightFog(AMX* vm, cell* parameters) //5C30B0.
	{
		if (!VerifyScriptParameters(vm, parameters, 5, "SetHeightFog")) return 0;
		BYTE enabled = 1;
		float upperLimit, lowerLimit, density;
		JE::ScriptServices::ReadNativeFloat(vm, parameters[1], upperLimit);
		JE::ScriptServices::ReadNativeFloat(vm, parameters[2], lowerLimit);
		cell rgb[3];
		if (!ReadScriptCells(vm, parameters[3], rgb, 3))
		{
			LogScriptError(vm, "SetHeightFog() expected float vector.");
			return 0;
		}
		JE::ScriptServices::ReadNativeFloat(vm, parameters[4], density);
		const cell enabledArgument = parameters[5];
		if (upperLimit <= lowerLimit)
		{
			LogScriptError(vm, "SetHeightFog() - upperLimit can not be <= the lowerLimit.");
			return 0;
		}
		if (density <= 0.0 || density > 1.0)
		{
			LogScriptError(vm, "SetHeightFog() - density must greater than 0.0 and no more than 1.0");
			return 0;
		}
		if (!enabledArgument) enabled = 0;
		const DWORD color = (((DWORD(rgb[0]) | 0xFFFFFF00u) << 8)
			| (DWORD(rgb[1]) & 0xFFu)) << 8 | (DWORD(rgb[2]) & 0xFFu);
		void* owner;
		if (amx_GetUserData(vm, 0x464C534D, &owner))
		{
			LogScriptError(vm, "SetHeightFog() was unable to resolve user ID. (internal error)");
			return 0;
		}
		DWORD mapID = ResolveScriptMapID(reinterpret_cast<DWORD>(owner));
		if (!mapID)
		{
			LogScriptError(vm, "SetHeightFog() was unable to resolve the map.");
			return 0;
		}
		static_cast<JE::Server*>(g_pNetworkInstance->server)->SetHeightFog(mapID,
			upperLimit, lowerLimit, color, density, enabled);
		return 1;
	}

	cell AMX_NATIVE_CALL NativeResetHeightFog(AMX* vm, cell* parameters) //5C3260.
	{
		if (!VerifyScriptParameters(vm, parameters, 0, "ResetHeightFog")) return 0;
		void* owner;
		if (amx_GetUserData(vm, 0x464C534D, &owner))
		{
			LogScriptError(vm, "ResetHeightFog() was unable to resolve user ID. (internal error)");
			return 0;
		}
		DWORD mapID = ResolveScriptMapID(reinterpret_cast<DWORD>(owner));
		if (!mapID)
		{
			LogScriptError(vm, "ResetHeightFog() was unable to resolve the map.");
			return 0;
		}
		static_cast<JE::Server*>(g_pNetworkInstance->server)->ResetHeightFog(mapID);
		return 1;
	}

	cell AMX_NATIVE_CALL NativeIsUnderFOW(AMX* vm, cell* parameters) //5C7D10.
	{
		if (!VerifyScriptParameters(vm, parameters, 1, "IsUnderFOW")) return 0;
		JE::Entity* entity;
		if (!ResolveScriptEntity(vm, DWORD(parameters[1]), &entity, 1))
		{
			LogScriptError(vm, "IsUnderFOW() is unable to find the source entity.");
			return 0;
		}
		if (!g_GfxSystem->FogOfWarEnabled(NULL)) return 0;
		JE::Map* map = JE::GetWorldInstance()->GetMap(entity->mapHandle);
		JE::AreaMap* areaMap = map->areaMap;
		if (!areaMap) return 0;
		D3DXVECTOR3 position;
		position = entity->position;
		LONG first, row;
		areaMap->WorldToCell(position.x, position.y, position.z, &first, &row);
		return !areaMap->IsVisible(first, row);
	}
}
