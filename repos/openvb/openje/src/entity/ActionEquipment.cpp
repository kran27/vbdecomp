#include "ActionEquipment.hpp"

namespace JE
{
	//The three original base tables share404B60/404B70/404BB0/5638D0.
	//Their constructors leave the inherited owner and minimumBudget unwritten.
	ActionEquip::ActionEquip() : completed(0) { kind=1; }
	BYTE ActionEquip::IsComplete() const { return completed; }
	void ActionEquip::Cancel() {}
	int ActionEquip::Start(DWORD) { return 0; }
	int ActionEquip::Update(DWORD, DWORD) { return 0; }
	int ActionEquip::Finish(DWORD) { return 0; }

	ActionUnequip::ActionUnequip() : completed(0) { kind=1; }
	BYTE ActionUnequip::IsComplete() const { return completed; }
	void ActionUnequip::Cancel() {}
	int ActionUnequip::Start(DWORD) { return 0; }
	int ActionUnequip::Update(DWORD, DWORD) { return 0; }
	int ActionUnequip::Finish(DWORD) { return 0; }

	ActionSetActiveWeapon::ActionSetActiveWeapon() : completed(0) { kind=1; }
	BYTE ActionSetActiveWeapon::IsComplete() const { return completed; }
	void ActionSetActiveWeapon::Cancel() {}
	int ActionSetActiveWeapon::Start(DWORD) { return 0; }
	int ActionSetActiveWeapon::Update(DWORD, DWORD) { return 0; }
	int ActionSetActiveWeapon::Finish(DWORD) { return 0; }
}
