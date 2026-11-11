#pragma once

#include "network/Types.hpp"
#include <map>
#include <vector>

namespace F3
{
	// Script-visible object/squad/team search results live until Network Update
	// clears them. Vectors are owned here; their entities/member IDs are borrowed.
	class FrameLookupCaches
	{
	public:
		typedef std::vector<JE::Net::UInt32> MemberList;
		typedef std::vector<void*> ObjectList;
		typedef std::map<JE::Net::UInt32, ObjectList*> ObjectMap;
		typedef std::map<JE::Net::UInt32, MemberList*> MemberMap;
		ObjectMap objects; // original 0x70BED8
		MemberMap squads; // original 0x70C6C4
		MemberMap teams; // original 0x70C6D0

		void ClearObjects(); // Local-instance compatibility operation.
		void ClearTeams(); // Local-instance compatibility operation.
		void ClearSquads(); // Local-instance compatibility operation.
		JE::Net::UInt32 GetSquadMember(JE::Net::UInt32 nHandle, JE::Net::UInt32 nIndex) const; // Local-cache compatibility API.
		JE::Net::UInt32 GetTeamMember(JE::Net::UInt32 nHandle, JE::Net::UInt32 nIndex) const; // Local-cache compatibility API.
		const ObjectList* GetObjects(JE::Net::UInt32 nHandle) const; // Local-instance compatibility lookup.
		JE::Net::UInt32 GetObjectId(JE::Net::UInt32 nHandle, JE::Net::UInt32 nIndex) const; // Local-cache compatibility API; native uses free function below
	};

	//Independent original global maps, each with its own initializer/atexit lifetime.
	extern FrameLookupCaches::ObjectMap g_FrameObjectResults; //70BED8.
	extern FrameLookupCaches::MemberMap g_FrameSquadResults; //70C6C4.
	extern FrameLookupCaches::MemberMap g_FrameTeamResults; //70C6D0.
	void ClearFrameObjects(); //559B30, free global operation.
	void ClearFrameTeams(); //5A34A0.
	void ClearFrameSquads(); //5A2B60, complete shared tail of543770.
	const FrameLookupCaches::ObjectList* GetFrameObjects(JE::Net::UInt32 handle); //559400, cdecl/RET0.
	JE::Net::UInt32 GetFrameObjectId(JE::Net::UInt32 handle,JE::Net::UInt32 index); //5593A0, cdecl/RET0.
	JE::Net::UInt32 GetFrameTeamMember(JE::Net::UInt32 handle,JE::Net::UInt32 index); //5A3450, cdecl/RET0.
	JE::Net::UInt32 GetFrameSquadMember(JE::Net::UInt32 handle,JE::Net::UInt32 index); //5A2B10, cdecl/RET0.

	namespace FrameLookupServices
	{
		void DestroyObjects(FrameLookupCaches::ObjectList* pList);
		void DestroyMembers(FrameLookupCaches::MemberList* pList);
		// Original5593EA reads the borrowed Entity ID; production uses the real
		// reconstructed Entity member, without an original-layout overlay.
		JE::Net::UInt32 EntityId(void* pEntity);
	}
}
