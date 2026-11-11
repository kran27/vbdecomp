#pragma once

#include "entity/Inventory.hpp"

namespace JE { class ModelInstance; }

namespace F3
{
	class GameItem;
	class GameCreature;
	struct EquipmentEffectTarget
	{
		DWORD entityID;
		std::list<DWORD> effects;
	};
	struct InventoryPendingRecord { DWORD values[6]; }; // Native469530 node32; node+16 is entityID.
	class GameInventory : public JE::Inventory
	{
		friend class GameEntity;
		friend class GameActionSetActiveWeapon;
	public:
		virtual void Add(JE::Entity* entity); // 0x4690A0.
		virtual void Remove(JE::Entity* entity); //469550; preserves discarded-erase-iterator bug.
		virtual BYTE ContainsID(DWORD id); //4692D0, original+0C; party registry then base ID search.
		virtual BYTE Contains(JE::Entity* entity); // 0x469240; real pointer list or party lookup.
		virtual BYTE ContainsResource(const std::string& name); //4691C0, party lookup then base.
		virtual BYTE IsEquipped(JE::Entity* entity); // 0x469670; source defined with client-frame family.
		virtual BYTE GetEquippedSlots(JE::Entity* entity, std::list<DWORD>& slots); // 0x46A860; pushes front.
		virtual void PackState(JE::Net::Packet& packet); //469370, original+20; qualified empty base hook.
		virtual void UnpackState(JE::Net::Packet& packet); //469370, original+24; qualified inbound base hook.
		void FindItemsByResource(const std::string& name, std::list<JE::Entity*>& equippedItems,
			std::list<JE::Entity*>& unequippedItems); //46A6C0, clear and append full matches.
		virtual BYTE Equip(JE::Entity* entity, DWORD requestedSlot, BYTE notify); // 0x46BE40.
		virtual void Unequip(JE::Entity* entity); // 0x46B350.
		void SetItem(JE::Entity* entity, DWORD slot); // 0x469380.
		void ClearSlot(DWORD slot); // 0x4693C0.
		GameItem* GetItem(DWORD slot); // 0x4693D0; borrowed item, actual RTTI.
		void ReapplyEquipment(); // 0x46A970.
		void ApplyItemAppearance(GameItem* item, DWORD bone, DWORD slot); // 0x469CF0.
		void RemoveItemAppearance(GameItem* item, DWORD group); // 0x469F90.
		BYTE SlotCompatible(DWORD slot, DWORD kind); // 0x469430.
		DWORD DefaultSlot(DWORD kind); // 0x469490.
		DWORD SelectEquipmentGroup(const std::list<DWORD>& slots); // 0x4694B0.
		static void __stdcall BuildItemIndices(DWORD* indices, GameItem* item,
			JE::ModelInstance* model); //469900, three stack arguments/RET0C.
		static void __stdcall BuildCreatureIndices(DWORD* indices, GameCreature* creature,
			JE::ModelInstance* model); //469A90, three stack arguments/RET0C.
		JE::Entity* equipped[6]; // Borrowed; inventory's owning list is distinct.
		DWORD value44;
		LONG value48;
		std::map<DWORD,std::list<EquipmentEffectTarget> > equippedEffects; // Original+34, borrowed effect IDs.
		std::list<InventoryPendingRecord> pendingRecords; // Original+40, producers remain unresolved.
	protected:
		GameInventory(); // 0x46B970; populated base Destroy lifetime still requires actual Create/state.
		virtual ~GameInventory();
		virtual void OnRemove(JE::Entity* entity); // 0x469410, then55FD10.
		virtual void OnAdd(JE::Entity* entity); //469400, original+30; actual55FCD0 forwarding thunk.
		//Nonvirtual source services; the original inventory table ends at Unequip+3C.
		BYTE IsPartyOwner(); // Actual IWorld+68 precedes owner lookup and party+0C.
		BYTE PartyContains(JE::Entity* entity); // Actual5A0000 with Entity+1C0 string.
		void AddPartyItem(GameItem* item); // Actual5A04B0 registry assignment and deletion branch.
		void RemovePartyItem(JE::Entity* entity); // Actual5A00F0 registry removal branch.
		DWORD NetworkMode(); // Actual SecondElement(HudBase); live caller input required.
		void ApplyEquipEffects(GameItem* item); //46BA30; clone/target/activate and retain effect IDs.
		void RemoveEquipEffects(GameItem* item); //46B100; mark effects complete, then release IDs.
	};
}
