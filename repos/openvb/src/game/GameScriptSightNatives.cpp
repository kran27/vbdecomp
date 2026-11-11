#include "GameScriptNatives.hpp"
#include "entity/Entity.hpp"
#include "scripting/ScriptEnvironment.hpp"

namespace F3
{
	cell AMX_NATIVE_CALL NativeGetLOS(AMX* vm, cell* parameters)
	{
		if (!VerifyScriptParameters(vm, parameters, 2, "GetLOS")) return 0;
		JE::Entity* source;
		if (!ResolveScriptEntity(vm, DWORD(parameters[1]), &source, 1))
		{
			LogScriptError(vm, "GetLOS() is unable to find the source entity.");
			return 0;
		}
		JE::Entity* target;
		if (!ResolveScriptEntity(vm, DWORD(parameters[2]), &target, 1))
		{
			LogScriptError(vm, "GetLOS() is unable to find the target entity.");
			return 0;
		}
		return cell(source->GetLOS(target));
	}
	cell AMX_NATIVE_CALL NativeGetLOSPoint(AMX* vm, cell* parameters)
	{
		if (!VerifyScriptParameters(vm, parameters, 2, "GetLOSPoint")) return 0;
		JE::Entity* source;
		if (!ResolveScriptEntity(vm, DWORD(parameters[1]), &source, 1))
		{
			LogScriptError(vm, "GetLOSPoint() is unable to find the source entity.");
			return 0;
		}
		D3DXVECTOR3 point(0,0,0);
		if (!JE::ScriptServices::ReadNativeVector(vm, parameters[2], point))
		{
			LogScriptError(vm, "GetLOSPoint() is unable to retrieve the point.");
			return 0;
		}
		return cell(source->GetLOSPoint(point.x, point.y, point.z));
	}
}
