#pragma once

#include "game/GameInterface.hpp"
#include <list>

namespace GUI { class Picture; class Label; class Button; class ScrollBar; }
class Gfx_Viewport;
class Gfx_CamCtrl_Orbit;
namespace JE { class Entity; }

namespace F3
{
	class GameInventory;
	class GameItem;
	struct InventorySlot
	{
		InventorySlot(); //43AE10, no GUI node ownership.
		void Bind(DWORD item, DWORD owner); //43AE90.
		void Clear(); //43AFF0; NULL item icon/state.
		void SetState(DWORD value); //43A420; item alpha/material policy.
		void SetVisible(DWORD value); //43A490; highlight states0..3.
		::GUI::Picture* picture;
		::GUI::Picture* highlight;
		DWORD itemID;
		std::string icon;
		DWORD state, visible;
	};

	class InventoryInterface : public GameInterface
	{
	public:
		explicit InventoryInterface(const std::string& name); //43CD50.
		virtual ~InventoryInterface(); //43B000, then actual base lifetime.
		virtual int OnEvent(::GUI::Window* window, ::GUI::Event* event); //43FF30 ->43F620.
		virtual void BeforeUpdate(); //43B180 ->43A620.
		virtual void BeforeDraw(); //43B190 ->43A750.
		void Open(DWORD actor); //43F4C0; one actual stack argument.
		void Refresh(); //43F5E0; visible only.
		GameInventory* GetInventory(); //43A5E0.
		JE::Entity* FindEquipped(DWORD slot); //43AAC0.
		void SetDescription(const std::string& text); //43A850.
		void HideSort(); //43A8D0.
		void ResetGrid(); //43AA60; highlights and selection IDs.
		void ClearSlotItems(); //43B270.
		void RefreshGridBindings(); //43B2C0.
		void PopulateItems(); //43EE60.
		void SortItems(DWORD kind, LONG firstOrder); //43E760.
		void RefreshEquipmentText(); //43EAE0.
		void RefreshWeight(); //43B810.
		void RefreshCharacterName(); //43B920.
		void RefreshPrimaryAttributes(); //43BA90.
		void RefreshSkills(); //43BE70.
		void RefreshEquipmentAvailability(); //43F280.
		LONG MaxBackpackIndex(); //43AC40.
		LONG FindEquipmentIndex(::GUI::Window* window); //43A550.
		void FindBackpackIndex(::GUI::Window* window, LONG& row, LONG& column); //43A580.
		void ToggleSelection(LONG row, LONG column); //43E6D0.
		void BeginEquip(DWORD slot); //43A7A0.
		void SetUnequippedIndex(DWORD item, LONG index); //43A7F0.
		void BindEquipmentDrop(DWORD slot); //43B4C0.
		void BindBackpackDrop(LONG index); //43B570.
		void DescribeItem(DWORD item, DWORD owner); //43B5D0.
		void ShowSort(); //43A870.

		::GUI::Picture* background; //Original+04.
		::GUI::Window* backpack; //+0C.
		::GUI::Label* statLabels[7]; //+10..28.
		::GUI::Label* skillLabels[13]; //+2C..5C.
		::GUI::Label* weightLabel; //+60.
		::GUI::Label* headLabels[7]; //+64..7C: name,Ball,Bio,Elec,EMP,Heat,Norm.
		::GUI::Label* bodyLabels[7]; //+80..98.
		::GUI::Label* handLabels[2][4]; //+9C..B8: name,damage,ammo,range.
		::GUI::Picture* sortPicture; //+BC, picSortPulldown.
		::GUI::Label* sortLabels[4]; //+C0..CC: Name,Type,Weight,Number.
		::GUI::Button* backpackUp;
		::GUI::Button* backpackDown; //+D0/D4.
		::GUI::Button* descriptionUp;
		::GUI::Button* descriptionDown; //+D8/DC.
		::GUI::ScrollBar* backpackScroll; //+E0, actual helper owner.
		::GUI::Label* descriptionLabel; //+E4.
		::GUI::Picture* dropPicture; //+E8.
		InventorySlot* equipmentSlots[6]; //+EC..100; own records, borrow GUI nodes.
		InventorySlot* backpackSlots[33][3]; //+104..28C; own records and dynamic Pictures.
		Gfx_Viewport* viewport; //+290.
		Gfx_CamCtrl_Orbit* orbit; //+294.
		float rotation, elevation, distance; //+298/29C/2A0.
		DWORD lastModelTick, actorID; //+2A4/2A8.
		BYTE flags2AC, flags2AD;
		std::list<DWORD> selectedItemIDs; //+2B0; owns nodes, not entities.
	private:
		void PickupSound(); //43A960, unused native receiver.
		void PutDownSound(); //43A9E0, unused native receiver.
		void DamageTypeName(DWORD type, std::string& text); //43C990.
		void WeaponDamage(JE::Entity* item, std::string& text); //43CA70.
		void ItemName(JE::Entity* item, std::string& text); //43A8F0.
		void ArmorValue(JE::Entity* item, DWORD damageType, std::string& text, DWORD part); //43C570, RET16.
		void WeaponName(JE::Entity* item, std::string& text); //43C6F0, RET8.
		void WeaponAmmo(JE::Entity* item, std::string& text); //43C7B0, RET8.
		void WeaponRange(JE::Entity* item, std::string& text); //43C8A0, RET8.
		InventoryInterface(const InventoryInterface&);
		InventoryInterface& operator=(const InventoryInterface&);
	};
}
