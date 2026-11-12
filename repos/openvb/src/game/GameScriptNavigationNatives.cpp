#include "GameScriptNatives.hpp"
#include "GameScriptMapContext.hpp"
#include "entity/Map.hpp"
#include "entity/World.hpp"
#include "entity/Trigger.hpp"
#include "scripting/ScriptEnvironment.hpp"
#include <cstring>
#include <cmath>
#include <stdexcept>

namespace F3
{
	namespace
	{
		BYTE ContextMap(AMX* vm, const char* userError, const char* mapError, JE::Map** output)
		{
			void* user;
			if (amx_GetUserData(vm, 0x464C534D, &user))
			{
				LogScriptError(vm, userError);
				return 0;
			}
			DWORD id = ResolveScriptMapID(reinterpret_cast<DWORD>(user));
			if (!id)
			{
				LogScriptError(vm, mapError);
				return 0;
			}
			*output = JE::g_World->GetMap(id);
			return 1;
		}
		JE::NavPoint* NavPointAt(JE::Map& map, DWORD index)
		{
			//56DA40/56DAF0/56DB40 reject index>size, not index>=size.
			if (index > map.navigation.size()) return NULL;
			return map.navigation[index];
		}
		BYTE StoreValues(AMX* vm, cell address, const void* values, DWORD count)
		{
			cell* destination;
			if (amx_GetAddr(vm, address, &destination)) return 0;
			memcpy(destination, values, count * sizeof(cell)); //Actual59AC00/59AC90.
			return 1;
		}
		JE::WayPointPath* FindPath(JE::Map& map, const std::string& name)
		{
			std::map<std::string, JE::WayPointPath>::iterator i = map.waypoints.find(name);
			return i == map.waypoints.end() ? NULL : &i->second; //56F860, no insertion.
		}
		BYTE PathVector(JE::Map& map, const std::string& name, DWORD index, D3DXVECTOR3& output, BYTE orientation)
		{
			JE::WayPointPath* path = FindPath(map, name);
			if (!path || index >= static_cast<DWORD>(path->count)) return 0;
			JE::WayPoint* point = path->points[index];
			output = orientation ? point->orientation : point->position;
			return 1;
		}
	}
	cell AMX_NATIVE_CALL NativeGetNavPointCount(AMX* vm, cell* parameters)
	{
		if (!VerifyScriptParameters(vm, parameters, 0, "GetNavpointCount")) return 0;
		void* user;
		if (amx_GetUserData(vm, 0x464C534D, &user))
		{
			LogScriptError(vm, "GetNavpointCount() - was unable to resolve the userID (internal error)");
			return 0;
		}
		DWORD id = ResolveScriptMapID(reinterpret_cast<DWORD>(user));
		if (!id)
		{
			LogScriptError(vm, "GetNavpointCount() - was unable to resolve the map. Usually internal error.");
			return 0;
		}
		JE::Map* map = JE::GetWorldInstance()->GetMap(id);
		if (map) return map->GetNavPointCount();
		LogScriptError(vm, "GetNavPointCount() - Map pointer was NULL. (internal error.");
		return 0;
	}
	cell AMX_NATIVE_CALL NativeGetNavPointLinks(AMX* vm, cell* parameters)
	{
		if (!VerifyScriptParameters(vm, parameters, 2, "GetNavPointLinks")) return 0;
		void* user;
		if (amx_GetUserData(vm, 0x464C534D, &user))
		{
			LogScriptError(vm, "GetNavpointLinks() - was unable to resolve the userID (internal error)");
			return 0;
		}
		DWORD id = ResolveScriptMapID(reinterpret_cast<DWORD>(user));
		if (!id)
		{
			LogScriptError(vm, "GetNavpointLinks() - was unable to resolve the map. Usually internal error.");
			return 0;
		}
		JE::Map* map = JE::GetWorldInstance()->GetMap(id);
		DWORD count = 0;
		if (map)
		{
			DWORD links[5];
			count = map->GetNavPointLinks(parameters[1], links);
			if (count) WriteScriptCells(vm, parameters[2], links, 5);
			return count;
		}
		LogScriptError(vm, "Unable to resolve map pointer. (internal error.");
		return count;
	}
	cell AMX_NATIVE_CALL NativeGetNavPointPosition(AMX* vm, cell* parameters)
	{
		D3DXVECTOR3 value(0, 0, 0);
		if (!VerifyScriptParameters(vm, parameters, 2, "GetNavPointPosition")) return 0;
		void* user;
		if (amx_GetUserData(vm, 0x464C534D, &user))
		{
			LogScriptError(vm, "GetNavpointPosition() - was unable to resolve the userID (internal error)");
			return 0;
		}
		DWORD id = ResolveScriptMapID(reinterpret_cast<DWORD>(user));
		if (!id)
		{
			LogScriptError(vm, "GetNavpointPosition() - was unable to resolve the map. Usually internal error.");
			return 0;
		}
		JE::Map* map = JE::GetWorldInstance()->GetMap(id);
		if (!map)
		{
			LogScriptError(vm, "GetNavPointPosition() - Unable to resolve map pointer. (internal error)");
			return 0;
		}
		if (!map->GetNavPointPosition(parameters[1], value)) return 0;
		WriteScriptVector(vm, parameters[2], value);
		return 1;
	}
	cell AMX_NATIVE_CALL NativeGetNavPointOrientation(AMX* vm, cell* parameters)
	{
		D3DXVECTOR3 value(0, 0, 0);
		if (!VerifyScriptParameters(vm, parameters, 1, "GetNavPointOrientation")) return 0;
		void* user;
		if (amx_GetUserData(vm, 0x464C534D, &user))
		{
			LogScriptError(vm, "GetNavpointOrientation() - was unable to resolve the userID (internal error)");
			return 0;
		}
		DWORD id = ResolveScriptMapID(reinterpret_cast<DWORD>(user));
		if (!id)
		{
			LogScriptError(vm, "GetNavpointOrientation() - was unable to resolve the map. Usually internal error.");
			return 0;
		}
		JE::Map* map = JE::GetWorldInstance()->GetMap(id);
		if (!map)
		{
			LogScriptError(vm, "GetNavPointOrientation() - Unable to resolve map pointer. (internal error)");
			return 0;
		}
		if (!map->GetNavPointOrientation(parameters[1], value)) return 0;
		return cell(static_cast<__int64>(value.x));
	}
	cell AMX_NATIVE_CALL NativeGetWayPointCount(AMX* vm, cell* parameters)
	{
		if (!VerifyScriptParameters(vm, parameters, 1, "GetWayPointCount")) return 0;
		void* user;
		if (amx_GetUserData(vm, 0x464C534D, &user))
		{
			LogScriptError(vm, "GetWayPointCount() - was unable to resolve the userID (internal error)");
			return 0;
		}
		DWORD id=ResolveScriptMapID(reinterpret_cast<DWORD>(user));
		if (!id)
		{
			LogScriptError(vm, "GetWayPointCount() - was unable to resolve the map. Usually internal error.");
			return 0;
		}
		JE::Map* map=JE::GetWorldInstance()->GetMap(id);
		char name[256];
		if (!JE::ScriptServices::ReadNativeString(vm, parameters[1], name, sizeof(name)))
		{
			LogScriptError(vm, "GetWayPointCount() - Unable to retrieve path name.");
			return 0;
		}
		DWORD result=0;
		if (map) result=map->GetWayPointCount(std::string(name));
		return cell(result);
	}
	cell AMX_NATIVE_CALL NativeGetWayPointPosition(AMX* vm, cell* parameters)
	{
		if (!VerifyScriptParameters(vm, parameters, 3, "GetWayPointPosition")) return 0;
		void* user;
		if (amx_GetUserData(vm, 0x464C534D, &user))
		{
			LogScriptError(vm, "GetWayPointPosition() - was unable to resolve the userID (internal error)");
			return 0;
		}
		DWORD id=ResolveScriptMapID(reinterpret_cast<DWORD>(user));
		if (!id)
		{
			LogScriptError(vm, "GetWayPointPosition() - was unable to resolve the map. Usually internal error.");
			return 0;
		}
		JE::Map* map=JE::GetWorldInstance()->GetMap(id);
		D3DXVECTOR3 value(0,0,0);
		char name[256];
		DWORD result=0;
		if (!JE::ScriptServices::ReadNativeString(vm, parameters[1], name, sizeof(name)))
		{
			LogScriptError(vm, "GetWayPointPosition() - Unable to retrieve path name.");
			return 0;
		}
		if (map)
		{
			result=map->GetWayPointPosition(std::string(name),DWORD(parameters[2]),value);
			if (result) WriteScriptVector(vm,parameters[3],value);
		}
		return cell(result);
	}
	cell AMX_NATIVE_CALL NativeGetWayPointOrientation(AMX* vm, cell* parameters)
	{
		if (!VerifyScriptParameters(vm, parameters, 2, "GetWayPointOrientation")) return 0;
		void* user;
		if (amx_GetUserData(vm, 0x464C534D, &user))
		{
			LogScriptError(vm, "GetWayPointOrientation() - was unable to resolve the userID (internal error)");
			return 0;
		}
		DWORD id=ResolveScriptMapID(reinterpret_cast<DWORD>(user));
		if (!id)
		{
			LogScriptError(vm, "GetWayPointOrientation() - was unable to resolve the map. Usually internal error.");
			return 0;
		}
		JE::Map* map=JE::GetWorldInstance()->GetMap(id);
		D3DXVECTOR3 value(0,0,0);
		char name[256];
		if (!JE::ScriptServices::ReadNativeString(vm, parameters[1], name, sizeof(name)))
		{
			LogScriptError(vm, "GetWayPointOrientation() - Unable to retrieve path name.");
			return 0;
		}
		if (map)
		{
			DWORD result=map->GetWayPointOrientation(std::string(name),DWORD(parameters[2]),value);
			if (result) return cell(static_cast<__int64>(value.x));
		}
		else LogScriptError(vm, "GetWayPointOrientation() - Unable to resolve map pointer. (internal error)");
		return 0;
	}
	cell AMX_NATIVE_CALL NativeGetNearestPath(AMX* vm, cell* parameters)
	{
		if (!VerifyScriptParameters(vm, parameters, 2, "GetNearestPath")) return 0;
		JE::Entity* entity;
		if (!ResolveScriptEntity(vm, DWORD(parameters[1]), &entity, 1))
		{
			LogScriptError(vm, "GetNearestPath() is unable to find the source entity.");
			return 0;
		}
		JE::IWorld* world = JE::g_World;
		JE::Map* map = world->GetMap(entity->mapHandle);
		if (!map)
		{
			LogScriptError(vm, "GetNearestPath() is unable to find the entity's map.");
			return 0;
		}
		JE::WayPointPath* nearest = NULL;
		float nearestDistance;
		const LONG count = LONG(map->GetWayPointPathCount());
		for (LONG index = 0; index < count; ++index)
		{
			JE::WayPointPath* path = map->GetWayPointPath(index);
			if (!path->count) continue;
			D3DXVECTOR3 origin;
			origin.x = entity->position.x;
			origin.y = entity->position.y;
			origin.z = entity->position.z;
			D3DXVECTOR3 first(0, 0, 0);
			map->GetWayPointPosition(path->name, 0, first);
			double x = double(origin.x) - first.x;
			double y = double(origin.y) - first.y;
			double z = double(origin.z) - first.z;
			float distance = float(sqrt(z*z + y*y + x*x));
			if (!nearest || distance < nearestDistance)
			{
				nearestDistance = float(distance);
				nearest = path;
			}
		}
		if (!nearest) return 0;
		JE::ScriptServices::WriteNativeString(vm, parameters[2], nearest->name.c_str());
		return 1;
	}
}
