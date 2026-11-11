#pragma once

#include "network/EngineServer.hpp"
#include "configuration/Configuration.hpp"

namespace JE { class VFX_VEGInstance; class VisualEffect; }

namespace F3
{
	class GameServer : public JE::Server
	{
	public:
		explicit GameServer(JE::Net::UInt16 port); // 0x46EE70.
		~GameServer(); // 0x46CE90; no derived Cleanup.
		void Cleanup(); // 0x46CEB0.
		void OnUserPacket(JE::Net::UInt32 index,JE::Net::Packet& packet); //46DD70.
		void ToggleWeather(DWORD message); //46DD70 cases110/111/112, no packet payload.
		BYTE EquipItem(JE::Entity* owner,JE::Entity* item,DWORD slot,BYTE notify); //46EBD0.
		void UnequipItem(JE::Entity* owner,JE::Entity* item); //46EC30.
		void RemoveItemFromInventory(JE::Entity* owner,JE::Entity* item); //46CF10.
		void SendActiveWeapon(DWORD ownerID, DWORD slot); //46D390, reliable89.
		void SendPrimaryAmmo(DWORD entityID, LONG modifier, DWORD count); //46D700, reliable102.
		void SendSecondaryAmmo(DWORD entityID, LONG modifier, DWORD count); //46D7F0, reliable103.
		void SendNewAmmoStack(DWORD ownerID, DWORD stackID, LONG caliber, LONG modifier); //46D8E0, reliable92.
		void SendEntityDamage(DWORD entityID, DWORD damage, DWORD damageType); //46D020, reliable85.
		LONG SetEntityHealth(DWORD entityID, LONG health); //46EE10, real thiscall mutation.
		void SendEntityHealth(DWORD entityID, LONG health); //46D620, requested value in packet88.
		void SetMoveMode(DWORD entityID, DWORD mode, float speed); //46EC70, actual mutation member.
		void SendMoveMode(DWORD entityID, DWORD mode, float speed); //46D470, reliable86.
		void HideInterfaces(); //46EDF0, local client hide then reliable packet104.
		void OpenTask(DWORD id, DWORD value); //46ECD0, mutate then notify97.
		void CloseTask(DWORD id, DWORD value); //46ED30, mutate then notify98.
		void AddEntry(DWORD task, DWORD entry); //46ED90, mutate then notify99.
		void NotifyPerkRank(DWORD entityID, LONG perk, DWORD rank); //46D9E0, reliable94 with three DWORDs.

		JE::VFX_VEGInstance* child0x70F0; // Rain, directly deleted by Cleanup.
		JE::VisualEffect* field0x70F4; // Borrowed map-owned binding.
		JE::VFX_VEGInstance* child0x70F8; // Snow.
		JE::VisualEffect* field0x70FC;
		JE::VFX_VEGInstance* child0x7100; // Wind.
		JE::VisualEffect* field0x7104;
	private:
		void NotifyTaskOpen(DWORD id, DWORD value); //46DAD0.
		void NotifyTaskClosed(DWORD id, DWORD value); //46DBB0.
		void NotifyEntryAdded(DWORD task, DWORD entry); //46DC90.
		void SendInterfaceHide(); //46D560, reliable104.
		void SendEquippedItem(DWORD ownerID, DWORD itemID, DWORD sourceID, DWORD slot, BYTE add); //46D110.
		void SendUnequippedItem(DWORD ownerID, DWORD itemID, BYTE add); //46D260.
	};

	namespace GameServerServices
	{
		JE::Configuration::Settings* GetSettings();
		const JE::Configuration::Values* FindSection(const JE::Configuration::Settings& settings,
			const char* name);
		const std::string* FindValue(const JE::Configuration::Values& values, const char* name);
		long ToLong(const char* value);
		void AssignString(std::string& target, const char* value);
		void MissingStartMap();
		void DeleteChild(JE::VFX_VEGInstance* child);
	}
}
