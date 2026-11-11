#include "network/EngineClient.hpp"
#include "gui/WorldContainerInterface.hpp"
#include "gui/Window.hpp"
#include "gui/Picture.hpp"
#include "gui/Label.hpp"
#include "gui/Button.hpp"
#include "gui/ScrollBar.hpp"
#include "game/GameItem.hpp"
#include "game/GameInventory.hpp"
#include "startup/EngineStringSystem.hpp"
#include "graphics/SystemInterface.hpp"
#include "graphics/Camera.hpp"
#include "gui/Interfaces.hpp"
#include "startup/NetworkComponent.hpp"
#include <cstdio>
#include <stdexcept>

namespace JE
{
	void WorldContainerRow::Bind(F3::GameItem* value) //457460.
	{
		item = value;
		if (picture && value)
			picture->LoadTexture(value->inventoryTexture, 0, 0.0f, 0.0f, -1.0f, -1.0f, 1);
	}

	WorldContainerInterface::WorldContainerInterface(const std::string& name) //457DB0.
		: ::GUI::Interface(name)
	{
		WorldContainerRow* row = rows;
		for (int i = 0; i < 10; ++i, ++row)
		{
			row->picture = NULL;
			row->item = NULL;
		}
		anchor.x = 0.0f;
		anchor.y = 0.0f;
		anchor.z = 0.0f;
		LoadResource("Container.INT"); //Original ignores the reader result.
		targetID = 0;
		actorID = 0;
		scrollbar = NULL;
		upButton = NULL;
		downButton = NULL;
		baseWidth = 0;
		baseHeight = 0;
		rootWindow = FindWindow("Main");
		rootWindow->GetSize(baseWidth, baseHeight); //Original requires the authored root.
		for (int i = 0; i < 10; ++i)
		{
			char name[256];
			std::sprintf(name, "Slot%d", i);
			rows[i].picture = static_cast< ::GUI::Picture*>(rootWindow->FindWindow(name));
		}
		clickSound.Load("btnclick01.wav", 0);
	}

	WorldContainerInterface::~WorldContainerInterface() //457D20.
	{
		Clear();
		// Member vector releases its storage before the actual CSound lifetime,
		// then the qualified Interface base performs its original root cleanup.
	}

	void WorldContainerInterface::Clear() //457A10.
	{
		items.clear(); //Actual VC7 vector::clear calls _Tidy and releases storage.
		if (scrollbar) { delete scrollbar; scrollbar = NULL; }
		if (upButton) { delete upButton; upButton = NULL; }
		if (downButton) { delete downButton; downButton = NULL; }
	}

	F3::GameInventory* WorldContainerInterface::GetInventory() //4574B0.
	{
		Entity* target = GetEntityPtr(targetID);
		if (!target) return NULL;
		return dynamic_cast<F3::GameInventory*>(target->inventory);
	}

	F3::GameItem* WorldContainerInterface::FindRowItem(::GUI::Window* window) //4574F0.
	{
		for (int i = 0; i < 10; ++i)
			if (rows[i].picture == window) return rows[i].item;
		return NULL;
	}

	void WorldContainerInterface::Populate(DWORD target, DWORD actor) //4580C0.
	{
		targetID = target;
		actorID = actor;
		Clear();
		F3::GameInventory* inventory = GetInventory();
		if (!inventory) return;
		::GUI::Label* nameLabel = static_cast< ::GUI::Label*>(rootWindow->FindWindow("Name"));
		F3::GameEntity* targetEntity = dynamic_cast<F3::GameEntity*>(GetEntityPtr(targetID));
		if (targetEntity && nameLabel)
		{
			std::string text;
			const DWORD stringRef = targetEntity->stringRefs[0]; //4581CC precedes output-address setup.
			F3::g_EngineStringSystem.GetString(stringRef, text);
			nameLabel->SetText(text);
			const D3DXVECTOR3& position = targetEntity->position;
			anchor.x = position.x;
			anchor.y = position.y;
			anchor.z = position.z;
		}
		else anchor.x = anchor.y = anchor.z = 0.0f;
		const BYTE showEquipped = targetEntity->entityClass == 1;
		LONG displayed = 0;
		for (std::list<Entity*>::iterator it = inventory->items.begin(); it != inventory->items.end(); ++it)
		{
			Entity* entity = *it;
			if (entity)
			{
				std::list<DWORD> slots;
				if (showEquipped == 1 || !inventory->GetEquippedSlots(entity, slots))
				{
					F3::GameItem* item = dynamic_cast<F3::GameItem*>(entity);
					if (item)
					{
						if (displayed < 10)
						{
							rows[displayed].picture->SetHidden(0);
							rows[displayed].Bind(item);
							++displayed;
						}
						items.push_back(item);
					}
				}
			}
		}
		// Raw458309 increments ONLY the first-ten binding counter, not vector
		// count. Thus the original displayed>10 ScrollBar creation path is not
		// reached by this producer, even when items contains more than10 items.
		if (displayed > 10)
		{
			scrollbar = new ::GUI::ScrollBar;
			if (scrollbar)
			{
				rootWindow->Resize(baseWidth + 21, baseHeight);
				upButton = new ::GUI::Button(1);
				rootWindow->AddChild(upButton);
				upButton->SetName("Up");
				upButton->SetAbsolutePosition(baseWidth, 1);
				upButton->SetFrameVisible(0);
				upButton->Resize(20, 30);
				upButton->ApplySkin("DefaultInterface.ini");
				downButton = new ::GUI::Button(1);
				rootWindow->AddChild(downButton);
				downButton->SetName("Down");
				downButton->SetAbsolutePosition(baseWidth, baseHeight - 31);
				downButton->SetFrameVisible(0);
				downButton->Resize(20, 30);
				downButton->ApplySkin("DefaultInterface.ini");
				scrollbar->BindArrows(upButton, downButton);
				scrollbar->SetPageSize(2);
				scrollbar->SetMaximum(displayed / 5 + (displayed % 5 > 0));
				scrollbar->SetStep(1);
				scrollbar->SetValue(0, 1);
			}
			return;
		}
		while (displayed < 10)
		{
			rows[displayed].picture->SetHidden(1);
			rows[displayed].Bind(NULL);
			++displayed;
		}
		rootWindow->Resize(baseWidth, baseHeight);
	}

	void WorldContainerInterface::Refresh() //458710.
	{
		Populate(targetID, actorID);
	}

	void WorldContainerInterface::RefreshRows() //457520.
	{
		const LONG first = 5 * scrollbar->GetValue();
		LONG displayed = 0;
		while (displayed < 10)
		{
			if (displayed + first >= LONG(items.size())) break;
			rows[displayed].Bind(items[displayed + first]);
			rows[displayed++].picture->SetHidden(0);
		}
		while (displayed < 10)
		{
			rows[displayed].Bind(NULL);
			rows[displayed++].picture->SetHidden(1);
		}
	}

	void WorldContainerInterface::SetAbsolutePosition(LONG x, LONG y) //4574A0.
	{
		rootWindow->SetAbsolutePosition(x, y);
	}

	void WorldContainerInterface::BeforeUpdate() //4577D0.
	{
		D3DXVECTOR3 position = anchor; //Capture before camera callbacks,4577DC..457803.
		D3DXMATRIX view, projection, combined;
		g_GfxSystem->Camera()->GetViewMatrix(view);
		g_GfxSystem->Camera()->GetProjectionMatrix(projection);
		D3DXMatrixMultiply(&combined, &view, &projection);
		D3DXVECTOR4 clip;
		D3DXVec3Transform(&clip, &position, &combined);
		const float inverseW = 1.0f / clip.w;
		const float screenX = float((clip.x * inverseW + 1.0f) * 512.0f);
		const float screenY = float((2.0f - (inverseW * clip.y + 1.0f)) * 384.0f);
		::GUI::Window* root = GetRoots()->front(); //Original requires a first root.
		LONG width, height;
		root->GetSize(width, height);
		LONG x = LONG(static_cast<LONGLONG>(screenX));
		LONG y = LONG(static_cast<LONGLONG>(screenY)) - height; //4578D1..4578F0.
		LONG screenLeft, screenTop, screenWidth = 1024, screenHeight = 768;
		static_cast<Client*>(F3::g_pNetworkInstance->client)->child0xA0->GetScreenRectangle(
			screenLeft, screenTop, screenWidth, screenHeight); //Actual unchecked Client+A0 virtual20.
		if (x + width < 0 || x > screenWidth || y + height < 0 || y > screenHeight)
		{
			SetHidden(1);
			return;
		}
		if (x < 0) x = 0;
		if (x + width > screenWidth) x = screenWidth - width;
		if (y < 0) y = 0;
		if (y + height > screenHeight) y = screenHeight - height;
		root->SetAbsolutePosition(x, y);
	}
}
