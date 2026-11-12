#include "FrameLookupCaches.hpp"
#include "entity/Entity.hpp"

#include <stdexcept>

namespace F3
{
	FrameLookupCaches::ObjectMap g_FrameObjectResults;
	FrameLookupCaches::MemberMap g_FrameSquadResults;
	FrameLookupCaches::MemberMap g_FrameTeamResults;

	void ClearFrameObjects()
	{
		for (FrameLookupCaches::ObjectMap::iterator current = g_FrameObjectResults.begin(); current != g_FrameObjectResults.end(); ++current)
		{
			FrameLookupCaches::ObjectList* const pList = current->second;
			if (pList) FrameLookupServices::DestroyObjects(pList);
		}
		g_FrameObjectResults.clear();
	}

	void ClearFrameTeams()
	{
		for (FrameLookupCaches::MemberMap::iterator current = g_FrameTeamResults.begin(); current != g_FrameTeamResults.end(); ++current)
		{
			FrameLookupCaches::MemberList* const pList = current->second;
			if (pList) FrameLookupServices::DestroyMembers(pList);
		}
		g_FrameTeamResults.clear();
	}

	void ClearFrameSquads()
	{
		for (FrameLookupCaches::MemberMap::iterator current = g_FrameSquadResults.begin(); current != g_FrameSquadResults.end(); ++current)
		{
			FrameLookupCaches::MemberList* const pList = current->second;
			if (pList) FrameLookupServices::DestroyMembers(pList);
		}
		g_FrameSquadResults.clear();
	}

	const FrameLookupCaches::ObjectList* GetFrameObjects(JE::Net::UInt32 handle)
	{
		if (!handle) return NULL;
		const FrameLookupCaches::ObjectMap::iterator found=g_FrameObjectResults.find(handle);
		return found==g_FrameObjectResults.end() ? NULL : found->second;
	}








	JE::Net::UInt32 GetFrameObjectId(JE::Net::UInt32 handle,JE::Net::UInt32 index)
	{
		if (!handle) return 0;
		const FrameLookupCaches::ObjectMap::iterator found=g_FrameObjectResults.find(handle);
		if (found==g_FrameObjectResults.end()) return 0;
		const FrameLookupCaches::ObjectList& values=*found->second;
		if (index>=values.size()) return 0;
		return FrameLookupServices::EntityId(values[index]);
	}

	JE::Net::UInt32 GetFrameTeamMember(JE::Net::UInt32 handle,JE::Net::UInt32 index)
	{
		if (!handle) return 0;
		const FrameLookupCaches::MemberMap::iterator found=g_FrameTeamResults.find(handle);
		if (found==g_FrameTeamResults.end()) return 0;
		const FrameLookupCaches::MemberList& values=*found->second;
		if (index>=values.size()) return 0;
		return values[index];
	}

	JE::Net::UInt32 GetFrameSquadMember(JE::Net::UInt32 handle,JE::Net::UInt32 index)
	{
		if (!handle) return 0;
		const FrameLookupCaches::MemberMap::iterator found=g_FrameSquadResults.find(handle);
		if (found==g_FrameSquadResults.end()) return 0;
		const FrameLookupCaches::MemberList& values=*found->second;
		if (index>=values.size()) return 0;
		return values[index];
	}

	namespace FrameLookupServices
	{
		void DestroyObjects(FrameLookupCaches::ObjectList* pList) { delete pList; }
		void DestroyMembers(FrameLookupCaches::MemberList* pList) { delete pList; }
		JE::Net::UInt32 EntityId(void* entity)
		{
			return static_cast<JE::Entity*>(entity)->entityID; //5593EA, real borrowed Entity+1DC.
		}
	}
}
