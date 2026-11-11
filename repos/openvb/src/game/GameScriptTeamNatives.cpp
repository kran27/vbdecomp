#include "network/Types.hpp"
#include <winsock2.h>
#include "GameScriptNatives.hpp"
#include "entity/Entity.hpp"
#include "entity/World.hpp"
#include "entity/EntityEnvironment.hpp"
#include "model/CommonException.hpp"
#include "diagnostics/DebugOutput.hpp"
#include "../startup/FrameLookupCaches.hpp"
#include "network/EngineServer.hpp"
#include "../startup/NetworkComponent.hpp"
#include <new>
#include <stdexcept>

namespace F3
{
	namespace
	{
		DWORD nextTeamList = 100; //Original6FF4C0.
		DWORD nextSquadList = 100; //Original6FF4BC.
		JE::Server& Server()
		{
			return *static_cast<JE::Server*>(g_pNetworkInstance->server);
		}
		DWORD CaptureTeamMembers(DWORD* handle, DWORD id)
		{
			if (!(g_pNetworkInstance->GetMode() & 1))
				throw ::Common::Exception("Invalid attempt to retrieve world server instance.");
			JE::IWorldServer* world = JE::g_World;
			std::list<DWORD>* members = world->GetTeams()->GetTeamMembers(id);
			FrameLookupCaches::MemberList* list = new FrameLookupCaches::MemberList;
			for (std::list<DWORD>::iterator i = members->begin(); i != members->end(); ++i)
			{
				const JE::Net::UInt32 member = *i;
				list->push_back(member);
			}
			if (list->empty())
			{
				*handle = 0;
				delete list;
				return 0;
			}
			*handle = nextTeamList ? nextTeamList : 1;
			nextTeamList = *handle + 1;
			g_FrameTeamResults[*handle] = list;
			return DWORD(list->size());
		}
		DWORD CaptureSquadMembers(DWORD* handle, DWORD id)
		{
			if (!(g_pNetworkInstance->GetMode() & 1))
				throw ::Common::Exception("Invalid attempt to retrieve world server instance.");
			JE::IWorldServer* world = JE::g_World;
			std::list<DWORD>* members = world->GetSquads()->GetSquadMembers(id);
			FrameLookupCaches::MemberList* list = new FrameLookupCaches::MemberList;
			for (std::list<DWORD>::iterator i = members->begin(); i != members->end(); ++i)
			{
				const JE::Net::UInt32 member = *i;
				list->push_back(member);
			}
			if (list->empty())
			{
				*handle = 0;
				delete list;
				return 0;
			}
			*handle = nextSquadList ? nextSquadList : 1;
			nextSquadList = *handle + 1;
			g_FrameSquadResults[*handle] = list;
			return DWORD(list->size());
		}

	}
	cell AMX_NATIVE_CALL NativeFindTeamMembers(AMX* vm, cell* parameters)
	{
		if (!VerifyScriptParameters(vm, parameters, 2, "FindTeamMembers")) return 0;
		DWORD handle;
		DWORD count = CaptureTeamMembers(&handle, DWORD(parameters[2]));
		if (count)
		{
			cell* destination;
			if (amx_GetAddr(vm, parameters[1], &destination))
			{
				LogScriptError(vm, "FindTeamMembers() was unable to resolve list handle address.");
				return 0;
			}
			*destination = cell(handle);
		}
		return cell(count); //Empty result leaves the caller's output cell unchanged.
	}
	cell AMX_NATIVE_CALL NativeGetTeamMember(AMX* vm, cell* parameters)
	{
		if (!VerifyScriptParameters(vm, parameters, 2, "GetTeamMember")) return 0;
		const DWORD handle=DWORD(parameters[1]);
		const DWORD index=DWORD(parameters[2]);
		return cell(GetFrameTeamMember(handle,index));
	}
	cell AMX_NATIVE_CALL NativeGetTeam(AMX* vm, cell* parameters)
	{
		if (!VerifyScriptParameters(vm, parameters, 1, "GetTeam")) return 0;
		JE::Entity* entity = NULL;
		if (!ResolveScriptEntity(vm, DWORD(parameters[1]), &entity, 1))
		{
			LogScriptError(vm, "GetTeam() was unable to find the requested entity.");
			return 0;
		}
		return cell(entity->value368);
	}
	cell AMX_NATIVE_CALL NativeGetTeamRank(AMX* vm, cell* parameters)
	{
		if (!VerifyScriptParameters(vm, parameters, 1, "GetTeamRank")) return 0;
		JE::Entity* entity = NULL;
		if (!ResolveScriptEntity(vm, DWORD(parameters[1]), &entity, 1))
		{
			LogScriptError(vm, "GetTeamRank() was unable to find the requested entity.");
			return 0;
		}
		return cell(entity->value36C);
	}
	cell AMX_NATIVE_CALL NativeFindSquadMembers(AMX* vm, cell* parameters)
	{
		if (!VerifyScriptParameters(vm, parameters, 2, "FindSquadMembers")) return 0;
		DWORD handle;
		DWORD count = CaptureSquadMembers(&handle, DWORD(parameters[2]));
		if (count)
		{
			cell* destination;
			if (amx_GetAddr(vm, parameters[1], &destination))
			{
				LogScriptError(vm, "FindSquadMembers() was unable to resolve list handle address.");
				return 0;
			}
			*destination = cell(handle);
		}
		return cell(count);
	}
	cell AMX_NATIVE_CALL NativeGetSquadMember(AMX* vm, cell* parameters)
	{
		if (!VerifyScriptParameters(vm, parameters, 2, "GetSquadMember")) return 0;
		const DWORD handle=DWORD(parameters[1]);
		const DWORD index=DWORD(parameters[2]);
		return cell(GetFrameSquadMember(handle,index));
	}
	cell AMX_NATIVE_CALL NativeGetSquadID(AMX* vm, cell* parameters)
	{
		if (!VerifyScriptParameters(vm, parameters, 1, "GetSquad")) return 0;
		JE::Entity* entity = NULL;
		if (!ResolveScriptEntity(vm, DWORD(parameters[1]), &entity, 1))
		{
			LogScriptError(vm, "GetSquad() was unable to find the requested entity.");
			return 0;
		}
		return cell(entity->squadID);
	}
	cell AMX_NATIVE_CALL NativeGetSquadRank(AMX* vm, cell* parameters)
	{
		if (!VerifyScriptParameters(vm, parameters, 1, "GetSquad")) return 0;
		JE::Entity* entity = NULL;
		if (!ResolveScriptEntity(vm, DWORD(parameters[1]), &entity, 1))
		{
			LogScriptError(vm, "GetSquadRank() was unable to find the requested entity.");
			return 0;
		}
		return cell(entity->squadRank);
	}
	cell AMX_NATIVE_CALL NativeAddToTeam(AMX* vm, cell* parameters)
	{
		if (!VerifyScriptParameters(vm, parameters, 2, "AddToTeam")) return 0;
		return Server().AssignEntityTeam(DWORD(parameters[1]), DWORD(parameters[2]));
	}
	cell AMX_NATIVE_CALL NativeRemoveFromTeam(AMX* vm, cell* parameters)
	{
		if (!VerifyScriptParameters(vm, parameters, 1, "RemoveFromTeam")) return 0;
		return Server().RemoveEntityTeam(DWORD(parameters[1]));
	}
	cell AMX_NATIVE_CALL NativeSetTeamRank(AMX* vm, cell* parameters)
	{
		if (!VerifyScriptParameters(vm, parameters, 2, "SetTeamRank")) return 0;
		Server().SetEntityTeamRank(DWORD(parameters[1]), DWORD(parameters[2]));
		return 1; //Original returns1 even when the entity lookup fails.
	}
	cell AMX_NATIVE_CALL NativeSetTeamAttitude(AMX* vm, cell* parameters)
	{
		if (!VerifyScriptParameters(vm, parameters, 3, "SetTeamAttitude")) return 0;
		return Server().SetTeamAttitude(DWORD(parameters[1]), DWORD(parameters[2]), DWORD(parameters[3]));
	}
	cell AMX_NATIVE_CALL NativeCreateTeam(AMX* vm, cell* parameters)
	{
		if (!VerifyScriptParameters(vm, parameters, 0, "CreateTeam")) return 0;
		return cell(Server().CreateTeam());
	}
	cell AMX_NATIVE_CALL NativeCreateSquad(AMX* vm, cell* parameters)
	{
		if (!VerifyScriptParameters(vm, parameters, 0, "CreateSquad")) return 0;
		return cell(Server().CreateSquad());
	}
	cell AMX_NATIVE_CALL NativeRemoveFromSquad(AMX* vm, cell* parameters)
	{
		if (!VerifyScriptParameters(vm, parameters, 1, "RemoveFromSquad")) return 0;
		return Server().RemoveEntitySquad(DWORD(parameters[1]));
	}
	cell AMX_NATIVE_CALL NativeSetSquadRank(AMX* vm, cell* parameters)
	{
		if (!VerifyScriptParameters(vm, parameters, 2, "SetSquadRank")) return 0;
		Server().SetEntitySquadRank(DWORD(parameters[1]), DWORD(parameters[2]));
		return 1;
	}
	cell AMX_NATIVE_CALL NativeAddToSquad(AMX* vm, cell* parameters)
	{
		if (!VerifyScriptParameters(vm, parameters, 2, "AddToSquad")) return 0;
		return Server().AssignEntitySquad(DWORD(parameters[1]), DWORD(parameters[2]));
	}
	cell AMX_NATIVE_CALL NativeSetSquadAttitude(AMX* vm, cell* parameters)
	{
		if (!VerifyScriptParameters(vm, parameters, 3, "SetSquadAttitude")) return 0;
		return Server().SetSquadAttitude(DWORD(parameters[1]), DWORD(parameters[2]), DWORD(parameters[3]));
	}
}
