#pragma once
#include <windows.h>
namespace JE { struct TeamSystem; struct SquadSystem; }
namespace F3
{
	DWORD GetTeamAttitude(const JE::TeamSystem& table, DWORD source, DWORD target); //Compatibility adapter; native56A240 is TeamSystem::GetAttitude.
	DWORD GetSquadAttitude(const JE::SquadSystem& table, DWORD source, DWORD target); //Compatibility adapter; native5692E0 is SquadSystem::GetAttitude.
}
