#include "graphics/ClientCursor.hpp"
#include "game/GameItem.hpp"
#include "resources/ResourceSystem.hpp"
#include "entity/Entity.hpp"
#include "entity/Inventory.hpp"

namespace F3
{
	BYTE GameCursorManager::CancelSpecialContext() //437B90.
	{
		BYTE handled = 0;
		if (static_cast<LONG>(context) >= 15 && static_cast<LONG>(context) <= 16)
		{
			SetContext(1);
			handled = 1;
		}
		return handled;
	}
	void GameCursorManager::BeginItemDrag(DWORD item,DWORD owner) //437940.
	{
		if (!item)
		{
			draggedItemID = 0;
			draggedOwnerID = owner;
			ClearDragChoices();
			SetContext(1);
			return;
		}
		JE::Inventory* inventory = JE::GetEntityPtr(owner)->inventory;
		JE::Entity* entity = inventory->FindByID(item);
		draggedItemID = item;
		draggedOwnerID = owner;
		SetContext(14);
		GameItem* gameItem = dynamic_cast<GameItem*>(entity);
		if (gameItem)
		{
			//The captured Entity supplies the string after the immediate cursor refresh.
			std::string name = entity->inventoryTexture;
			if (!JE::Resources::g_pResourceSystem->Contains(name.c_str()))
			{
				name = "icon_unknown.bmp";
			}
			RequestImage(name.c_str(),0,0,1);
		}
		else
		{
			RequestImage("icon_unknown.bmp",0,0,1);
		}
	}
    void GameCursorManager::AppendDragChoices(const std::list<DWORD>& source)
    {
        for (std::list<DWORD>::const_iterator i=source.begin();i!=source.end();++i)
            choices.push_back(*i);
    }
    BYTE GameCursorManager::CancelSpecialContext(JE::ClientInputState& input,HWND window)
    {
        if (context<15 || context>16) return 0;
        SetContext(1,input,window);
        return 1;
    }
    void GameCursorManager::BeginItemDrag(DWORD item,DWORD owner,JE::ClientInputState& input,HWND window)
    {
        if (!item)
        {
            draggedItemID=0; draggedOwnerID=owner;
            ClearDragChoices();
            SetContext(1,input,window);
            return;
        }
        JE::Entity* entity=JE::GetEntityPtr(owner)->inventory->FindByID(item);
        draggedItemID=item; draggedOwnerID=owner;
        SetContext(14,input,window);
        GameItem* gameItem=dynamic_cast<GameItem*>(entity);
        if (gameItem)
        {
            //Original4379DE uses the Entity pointer+250 (inventoryTexture),
            //not the returned GameItem primary pointer to choose the string.
            std::string name=entity->inventoryTexture;
            if (!JE::Resources::g_pResourceSystem->Contains(name.c_str())) name="icon_unknown.bmp";
            RequestImage(name.c_str(),0,0,1);
        }
        else RequestImage("icon_unknown.bmp",0,0,1);
    }
}

