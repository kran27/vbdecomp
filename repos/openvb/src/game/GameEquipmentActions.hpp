#pragma once

#include "entity/ActionEquipment.hpp"

namespace F3
{
	class GameActionEquip : public JE::ActionEquip
	{
	public:
		GameActionEquip(); //405060.
		virtual int Start(DWORD time); //4050A0.
		virtual int Update(DWORD time, DWORD budget); //4051F0.
		virtual void Cancel(); //4051C0, commits the operation before base no-op.
		void OnEquipCue(); //405160, also original deadline/cancellation fallback.
		DWORD recoveryEnd, itemID, slot, value28;
		BYTE flag2C, equipped;
	};
	class GameActionUnequip : public JE::ActionUnequip
	{
	public:
		GameActionUnequip(); //407AF0.
		virtual int Start(DWORD time); //407B20.
		virtual int Update(DWORD time, DWORD budget); //407C50.
		virtual void Cancel(); //407C20.
		void OnUnequipCue(); //407BC0, also original deadline/cancellation fallback.
		DWORD recoveryEnd, itemID;
		BYTE unequipped;
	};
	class GameActionSetActiveWeapon : public JE::ActionSetActiveWeapon
	{
	public:
		GameActionSetActiveWeapon(); //4075D0.
		virtual int Start(DWORD time); //407620.
		virtual int Update(DWORD time, DWORD budget); //407910.
		virtual void Cancel(); //407850, then actual optional GameClient HUD tail.
		void OnEquipCue(); //407710.
		void OnUnequipCue(); //4077B0.
		DWORD previousSlot, nextSlot, phase, nextItemID, previousItemID;
		BYTE waitingUnequip;
		DWORD unequipEnd;
		BYTE waitingEquip;
		DWORD equipEnd;
		BYTE equipped, unequipped;
	};
}
