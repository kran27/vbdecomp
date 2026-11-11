#pragma once

#include <windows.h>
#include <list>
#include <map>

namespace JE
{
	// Names are inferred; the original has distinct team/squad value and map APIs.
	struct WorldTeam
	{
		WorldTeam(); //56A4C0.
		WorldTeam(const WorldTeam& other); //56A5E0.
		~WorldTeam(); //56A540.
		std::list<DWORD> order;
		std::map<DWORD,DWORD> values;
	};
	struct WorldSquad
	{
		WorldSquad(); //569560.
		WorldSquad(const WorldSquad& other); //569680.
		~WorldSquad(); //5695E0.
		std::list<DWORD> order;
		std::map<DWORD,DWORD> values;
	};
	struct TeamSystem
	{
		TeamSystem(); //56AE70, prepopulate0..1999.
		~TeamSystem(); //56AE40.
		DWORD GetAttitude(DWORD source,DWORD target); //56A240, ordinary ECX/RET8.
		BYTE RemoveEntityTeam(DWORD id); //56A1A0.
		BYTE AddEntityTeam(DWORD id,DWORD team); //56A330.
		BYTE SetTeamAttitude(DWORD source,DWORD target,DWORD attitude); //56A3E0.
		DWORD CreateGroup(); //56AD40.
		std::list<DWORD>* GetTeamMembers(DWORD id); //56A2F0, borrowed.
		typedef std::map<DWORD,WorldTeam> Groups;
		Groups values;
		DWORD next;
	};
	struct SquadSystem
	{
		SquadSystem(); //569F10, prepopulate0..1999.
		~SquadSystem(); //569EE0.
		DWORD GetAttitude(DWORD source,DWORD target); //5692E0, ordinary ECX/RET8.
		BYTE RemoveEntitySquad(DWORD id); //569240.
		BYTE AddEntitySquad(DWORD id,DWORD squad); //5693D0.
		BYTE SetSquadAttitude(DWORD source,DWORD target,DWORD attitude); //569480.
		DWORD CreateGroup(); //569DE0; original also uses the CreateTeam diagnostic.
		std::list<DWORD>* GetSquadMembers(DWORD id); //569390, borrowed.
		typedef std::map<DWORD,WorldSquad> Groups;
		Groups values;
		DWORD next;
	};
#if defined(_MSC_VER) && _MSC_VER == 1300 && defined(_M_IX86)
	typedef char WorldTeamOriginalExtent[(sizeof(WorldTeam)==24) ? 1 : -1];
	typedef char WorldSquadOriginalExtent[(sizeof(WorldSquad)==24) ? 1 : -1];
	typedef char TeamSystemOriginalExtent[(sizeof(TeamSystem)==16) ? 1 : -1];
	typedef char SquadSystemOriginalExtent[(sizeof(SquadSystem)==16) ? 1 : -1];
	typedef char TeamAttitudesOriginalOffset[(offsetof(WorldTeam,values)==12) ? 1 : -1];
	typedef char SquadAttitudesOriginalOffset[(offsetof(WorldSquad,values)==12) ? 1 : -1];
	typedef char TeamCounterOriginalOffset[(offsetof(TeamSystem,next)==12) ? 1 : -1];
	typedef char SquadCounterOriginalOffset[(offsetof(SquadSystem,next)==12) ? 1 : -1];
#endif
}
