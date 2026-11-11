#include "game/InventoryInterface.hpp"
#include "game/GameInventory.hpp"
#include "game/GameItem.hpp"
#include "gui/Window.hpp"
#include "gui/Picture.hpp"
#include "gui/Label.hpp"
#include "gui/Button.hpp"
#include "gui/ScrollBar.hpp"
#include "gui/Manager.hpp"
#include "graphics/Viewport.hpp"
#include "graphics/CameraController.hpp"
#include "resources/ResourceSystem.hpp"
#include <mmsystem.h>
#include <cstdio>

namespace F3
{
	InventorySlot::InventorySlot() //43AE10; real icon defaults before body stores.
	{
		picture = NULL;
		highlight = NULL;
		itemID = 0;
		icon = "icon_none.bmp";
		state = 0;
		visible = 0;
	}

	void InventorySlot::SetState(DWORD value) //43A420.
	{
		if (!picture) return;
		state = value;
		switch (value)
		{
		case 0: picture->SetAlpha(0); picture->SetMaterialModes(0); break;
		case 1: picture->SetAlpha(100); picture->SetMaterialModes(1); break;
		case 2: picture->SetAlpha(255); picture->SetMaterialModes(0); break;
		}
	}
	void InventorySlot::SetVisible(DWORD value) //43A490; highlight state, not Window.hidden.
	{
		if (!highlight) return;
		visible = value;
		switch (value)
		{
		case 0: highlight->SetAlpha(0); highlight->SetMaterialModes(0); break;
		case 1: highlight->SetColor(0x80008000); highlight->SetAlpha(64); highlight->SetMaterialModes(1); break;
		case 2: highlight->SetColor(0x80800000); highlight->SetAlpha(64); highlight->SetMaterialModes(1); break;
		case 3: highlight->SetColor(0x80000040); highlight->SetAlpha(64); highlight->SetMaterialModes(1); break;
		}
	}
	void InventorySlot::Clear() { Bind(0, 0); } //43AFF0.

	InventoryInterface::InventoryInterface(const std::string& name) : JE::EngineInterface(name), GameInterface(name) //43CD50.
	{
		LoadResource("Inventory.INT");
		viewport = NULL; orbit = NULL; actorID = 0; flags2AC = flags2AD = 0;
		selectedItemIDs.clear();
		rotation = 0; elevation = 10; distance = 26;
		lastModelTick = timeGetTime();
		background = static_cast< ::GUI::Picture*>(FindWindow("picBackground"));
		backpack = FindWindow("winBackpack");
		::GUI::Window* modelWindow = FindWindow("winModel");
		if (modelWindow)
		{
			viewport = new Gfx_Viewport;
			orbit = new Gfx_CamCtrl_Orbit;
			LONG width, height, x, y;
			modelWindow->GetSize(width, height);
			modelWindow->GetAbsolutePosition(x, y);
			float sx, sy; ::GUI::g_Manager.GetScale(sx, sy);
			const LONG scaledWidth = LONG(static_cast<LONGLONG>(float(width) * sx));
			const LONG scaledHeight = LONG(static_cast<LONGLONG>(float(height) * sy));
			D3DVIEWPORT8 descriptor;
			descriptor.X = DWORD(static_cast<LONGLONG>(float(x) * sx));
			descriptor.Y = DWORD(static_cast<LONGLONG>(float(y) * sy));
			descriptor.Width = scaledWidth ? scaledWidth : viewport->viewport.Width;
			descriptor.Height = scaledHeight ? scaledHeight : viewport->viewport.Height;
			descriptor.MinZ = 0; descriptor.MaxZ = 1;
			viewport->SetViewport(descriptor);
			viewport->clearColor = 0;
			//Raw43D13F..43D151 zero each vertex's U, not its diffuse word.
			for (int i = 0; i < 4; ++i) viewport->vertices[i].u = 0;
			viewport->SetCameraController(orbit);
		}
		const char* attributes[7] = {"ST", "PE", "EN", "CH", "IN", "AG", "LK"};
		char buffer[256];
		for (int i = 0; i < 7; ++i)
		{
			std::sprintf(buffer, "lbl%sValue", attributes[i]);
			statLabels[i] = static_cast< ::GUI::Label*>(FindWindow(buffer));
		}
		for (int i = 0; i < 13; ++i)
		{
			std::sprintf(buffer, "lblSkill%02dValue", i + 1);
			skillLabels[i] = static_cast< ::GUI::Label*>(FindWindow(buffer));
		}
		weightLabel = static_cast< ::GUI::Label*>(FindWindow("lblWeightValue"));
		const char* armorFields[7] = {"Name", "BallValue", "BioValue", "ElecValue", "EMPValue", "HeatValue", "NormValue"};
		for (int i = 0; i < 7; ++i)
		{
			std::sprintf(buffer, "lblHead%s", armorFields[i]);
			headLabels[i] = static_cast< ::GUI::Label*>(FindWindow(buffer));
		}
		for (int i = 0; i < 7; ++i)
		{
			std::sprintf(buffer, "lblBody%s", armorFields[i]);
			bodyLabels[i] = static_cast< ::GUI::Label*>(FindWindow(buffer));
		}
		const char* handFields[4] = {"Name", "DamageValue", "AmmoValue", "RangeValue"};
		for (int hand = 0; hand < 2; ++hand)
			for (int i = 0; i < 4; ++i)
			{
				std::sprintf(buffer, "lblHand%d%s", hand + 1, handFields[i]);
				handLabels[hand][i] = static_cast< ::GUI::Label*>(FindWindow(buffer));
			}
		sortPicture = static_cast< ::GUI::Picture*>(FindWindow("picSortPulldown"));
		const char* sorts[4] = {"Name", "Type", "Weight", "Number"};
		for (int i = 0; i < 4; ++i)
		{
			std::sprintf(buffer, "lblSort%s", sorts[i]);
			sortLabels[i] = static_cast< ::GUI::Label*>(FindWindow(buffer));
		}
		backpackUp = static_cast< ::GUI::Button*>(FindWindow("btnBackpackUp"));
		backpackDown = static_cast< ::GUI::Button*>(FindWindow("btnBackpackDown"));
		descriptionUp = static_cast< ::GUI::Button*>(FindWindow("btnDescriptionUp"));
		descriptionDown = static_cast< ::GUI::Button*>(FindWindow("btnDescriptionDown"));
		backpackScroll = new ::GUI::ScrollBar;
		backpackScroll->BindArrows(backpackUp, backpackDown);
		backpackScroll->SetPageSize(6);
		backpackScroll->SetMaximum(33);
		backpackScroll->SetStep(1);
		backpackScroll->SetValue(0, 1);
		const char* inactive[28] = {"lblHead", "lblBody", "lblHand1", "lblHand2", "lblST", "lblPE",
			"lblEN", "lblCH", "lblIN", "lblAG", "lblLK", "lblSkill01", "lblSkill02", "lblSkill03",
			"lblSkill04", "lblSkill05", "lblSkill06", "lblSkill07", "lblSkill08", "lblSkill09",
			"lblSkill10", "lblSkill11", "lblSkill12", "lblSkill13", "lblSkill14",
			"lblSortName", "lblSortType", "lblSortWeight"};
		for (int i = 0; i < 28; ++i)
		{
			::GUI::Window* label = FindWindow(inactive[i]);
			if (label) label->SetValue3B(0); //43B1B0, only this flag is changed.
		}
		::GUI::Window* number = FindWindow("lblSortNumber");
		if (number) number->SetValue3B(0);
		descriptionLabel = static_cast< ::GUI::Label*>(FindWindow("lblDescriptionText"));
		dropPicture = static_cast< ::GUI::Picture*>(FindWindow("picDropToWorld"));
		const char* slotNames[6] = {"picBodySlot", "picHeadSlot", "picHand1Slot", "picHand2Slot", "picHand3Slot", "picHand4Slot"};
		const char* highlightNames[6] = {"picBodySlotHighlight", "picHeadSlotHighlight", "picHand1SlotHighlight", "picHand2SlotHighlight", "picHand3SlotHighlight", "picHand4SlotHighlight"};
		for (int i = 0; i < 6; ++i)
		{
			equipmentSlots[i] = new InventorySlot;
			equipmentSlots[i]->picture = static_cast< ::GUI::Picture*>(FindWindow(slotNames[i]));
			equipmentSlots[i]->highlight = static_cast< ::GUI::Picture*>(FindWindow(highlightNames[i]));
			equipmentSlots[i]->Clear();
		}
		for (int row = 0; row < 33; ++row)
			for (int column = 0; column < 3; ++column)
				backpackSlots[row][column] = new InventorySlot; //43E5DC, binds no GUI pictures yet.
	}

	InventoryInterface::~InventoryInterface() //43B000.
	{
		if (viewport) { delete viewport; viewport = NULL; }
		if (orbit) { delete orbit; orbit = NULL; }
		for (int i = 0; i < 6; ++i)
			if (equipmentSlots[i]) { delete equipmentSlots[i]; equipmentSlots[i] = NULL; }
		for (int row = 0; row < 33; ++row)
			for (int column = 0; column < 3; ++column)
			{
				InventorySlot*& slot = backpackSlots[row][column];
				if (slot)
				{
					if (slot->picture) delete slot->picture;
					delete slot; slot = NULL;
				}
			}
		if (backpackScroll) delete backpackScroll;
		selectedItemIDs.clear();
	}
	GameInventory* InventoryInterface::GetInventory() //43A5E0.
	{
		if (!actorID) return NULL;
		JE::Entity* actor = JE::GetEntityPtr(actorID);
		return actor ? dynamic_cast<GameInventory*>(actor->inventory) : NULL;
	}
	JE::Entity* InventoryInterface::FindEquipped(DWORD slot) //43AAC0.
	{
		GameInventory* inventory = GetInventory();
		if (!inventory) return NULL;
		for (std::list<JE::Entity*>::iterator it = inventory->items.begin(); it != inventory->items.end(); ++it)
		{
			JE::Entity* item = *it;
			if (!item) continue;
			std::list<DWORD> slots;
			if (inventory->GetEquippedSlots(item, slots) && !slots.empty())
				for (std::list<DWORD>::iterator i = slots.begin(); i != slots.end(); ++i)
					if (*i == slot) return item;
		}
		return NULL;
	}
	void InventoryInterface::SetDescription(const std::string& text) //43A850.
	{ if (descriptionLabel) descriptionLabel->SetText(text); }
	void InventoryInterface::HideSort() //43A8D0.
	{ if (sortPicture) sortPicture->SetHidden(1); }
	void InventoryInterface::ResetGrid() //43AA60.
	{
		for (int row = 0; row < 33; ++row)
			for (int column = 0; column < 3; ++column) backpackSlots[row][column]->SetVisible(0);
		selectedItemIDs.clear();
	}
	void InventoryInterface::ClearSlotItems() //43B270.
	{
		for (int i = 0; i < 6; ++i) if (equipmentSlots[i]) equipmentSlots[i]->Clear();
		for (int row = 0; row < 33; ++row)
			for (int column = 0; column < 3; ++column)
				if (backpackSlots[row][column]) backpackSlots[row][column]->Clear();
	}
}
