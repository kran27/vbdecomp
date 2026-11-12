#include "GameScriptNatives.hpp"
#include "entity/World.hpp"
#include "diagnostics/DebugOutput.hpp"
#include <stdexcept>

namespace F3
{
	cell AMX_NATIVE_CALL NativeGetTeamAttitude(AMX* vm, cell* parameters)
	{
		//45AAF0: identical IDs return1 before the server guard, including0/0.
		if (!VerifyScriptParameters(vm,parameters,2,"GetTeamAttitude")) return 0;
		if (parameters[1] == parameters[2]) return 1;
		JE::IWorldServer* world = JE::GetWorldServerInstance();
		return static_cast<cell>(world->GetTeams()->GetAttitude(DWORD(parameters[1]),DWORD(parameters[2])));
	}

	cell AMX_NATIVE_CALL NativeGetSquadAttitude(AMX* vm, cell* parameters)
	{
		//45AA90: IWorld+88 retrieves the separate squad table atWorld+118.
		if (!VerifyScriptParameters(vm,parameters,2,"GetSquadAttitude")) return 0;
		if (parameters[1] == parameters[2]) return 1;
		JE::IWorldServer* world = JE::GetWorldServerInstance();
		return static_cast<cell>(world->GetSquads()->GetAttitude(DWORD(parameters[1]),DWORD(parameters[2])));
	}
}
