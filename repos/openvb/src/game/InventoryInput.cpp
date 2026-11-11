#include "network/EngineClient.hpp"
#include "audio/Sound.hpp"
#include "game/InventoryInterface.hpp"
#include "game/InventoryInputServices.hpp"
#include "game/GameInventory.hpp"
#include "game/GameItem.hpp"
#include "resources/ResourceSystem.hpp"
#include "game/GameEntity.hpp"
#include "graphics/ClientCursor.hpp"
#include "game/ClientInventoryEquipmentRequests.hpp"
#include "gui/Picture.hpp"
#include "gui/Label.hpp"
#include "gui/ScrollBar.hpp"
#include "gui/Event.hpp"
#include "startup/EngineStringSystem.hpp"
#include <list>

namespace F3
{
	void InventorySlot::Bind(DWORD item, DWORD owner) //43AE90.
	{
		if (!picture) return;
		if (!item)
		{
			itemID = 0;
			icon = "icon_none.bmp";
			picture->LoadTexture(icon, 0, 0, 0, -1, -1, 1);
			SetState(0);
			return;
		}
		JE::Inventory* inventory = JE::GetEntityPtr(owner)->inventory;
		JE::Entity* entity = inventory->FindByID(item);
		GameItem* gameItem = dynamic_cast<GameItem*>(entity);
		itemID = item;
		if (gameItem)
		{
			icon = gameItem->inventoryTexture;
			if (!JE::Resources::g_pResourceSystem->Contains(icon.c_str())) icon = "icon_unknown.bmp";
			picture->LoadTexture(icon, 0, 0, 0, -1, -1, 1);
			SetState(2);
		}
		else
		{
			icon = "icon_unknown.bmp";
			picture->LoadTexture(icon, 0, 0, 0, -1, -1, 1);
			SetState(2);
		}
	}

    namespace
    {
        GameCursorManager& Cursor()
        { return *static_cast<GameCursorManager*>(InventoryInputServices::GetClient()->child0xA4); }
        void SetDraggedItem(GameCursorManager& cursor,DWORD item,DWORD owner)
        { InventoryInputServices::BeginItemDrag(cursor,item,owner); }
        BYTE EquippedSlotsByID(GameInventory* inventory,DWORD id,std::list<DWORD>& output) //46A8E0.
        {
            BYTE found=0;
            for (DWORD i=0;i<6;++i)
                if (inventory->equipped[i] && inventory->equipped[i]->entityID==id)
                { found=1; output.push_front(i); }
            return found; //Does not clear the incoming list.
        }
    }

    void InventoryInterface::PickupSound() //43A960.
    {
        JE::CSound* sound = new JE::CSound;
        sound->Load("InventoryPickUp.wav",0);
        sound->Play(2,0,0);
    }
    void InventoryInterface::PutDownSound() //43A9E0.
    {
        JE::CSound* sound = new JE::CSound;
        sound->Load("InventoryPutDown.wav",0);
        sound->Play(2,0,0);
    }

    LONG InventoryInterface::FindEquipmentIndex(::GUI::Window* window) //43A550.
    {
        for (LONG i=0;i<6;++i)
            if (equipmentSlots[i]->picture==window || equipmentSlots[i]->highlight==window) return i;
        return -1;
    }
    void InventoryInterface::FindBackpackIndex(::GUI::Window* window,LONG& row,LONG& column) //43A580.
    {
        for (LONG r=0;r<33;++r)
            for (LONG c=0;c<3;++c)
                if (backpackSlots[r][c]->picture==window
                    || backpackSlots[r][c]->highlight==window)
                {
                    row=r;
                    column=c;
                    return;
                }
        row=-1;
        column=-1;
    }
    void InventoryInterface::ToggleSelection(LONG row,LONG column) //43E6D0.
    {
        InventorySlot* slot=backpackSlots[row][column];
        if (slot->visible==3)
        { slot->SetVisible(0); selectedItemIDs.remove(slot->itemID); }
        else
        { slot->SetVisible(3); selectedItemIDs.push_back(slot->itemID); }
    }
    void InventoryInterface::BeginEquip(DWORD slot) //43A7A0, captures both drag IDs before sending.
    {
        JE::Client* client=InventoryInputServices::GetClient();
        GameCursorManager* cursor=static_cast<GameCursorManager*>(client->child0xA4);
        DWORD item=cursor->draggedItemID,source=cursor->draggedOwnerID;
        flags2AC=1;
        client->RequestEquip(actorID,item,source,slot);
    }
    void InventoryInterface::SetUnequippedIndex(DWORD item,LONG index) //43A7F0.
    {
        GameInventory* inventory=GetInventory();
        JE::Client* client=InventoryInputServices::GetClient();
        dynamic_cast<GameEntity*>(inventory->FindByID(item))->value8C=DWORD(index);
        client->RequestUnequip(actorID,item);
        flags2AD=1;
    }
    void InventoryInterface::BindEquipmentDrop(DWORD slot) //43B4C0, local presentation before deferred packet reply.
    {
        GameCursorManager& cursor=Cursor();
        if (equipmentSlots[slot]->visible==1)
        {
            equipmentSlots[slot]->Bind(cursor.draggedItemID,actorID);
            SetDraggedItem(cursor,0,0);
        }
        else
        {
            for (LONG row=0;row<33;++row)
                for (LONG col=0;col<3;++col)
                    if (backpackSlots[row][col] && backpackSlots[row][col]->state==1)
                        backpackSlots[row][col]->SetState(2);
            SetDraggedItem(cursor,0,0);
        }
    }
    void InventoryInterface::BindBackpackDrop(LONG index) //43B570, original valid index domain.
    {
        GameCursorManager& cursor=Cursor();
        backpackSlots[index/3][index%3]->Bind(cursor.draggedItemID,actorID);
        SetDraggedItem(cursor,0,0);
    }
    void InventoryInterface::DescribeItem(DWORD item,DWORD) //43B5D0 ignores supplied owner and reloads actorID.
    {
        if (!descriptionLabel) return;
        JE::Entity* actor=JE::GetEntityPtr(actorID);
        GameEntity* entity=dynamic_cast<GameEntity*>(actor->inventory->FindByID(item));
        std::string title,description;
        GetString(entity->stringRefs[0],title);
        GetString(entity->stringRefs[1],description);
        SetDescription(title+"\n"+"\n"+description);
    }
    void InventoryInterface::ShowSort() //43A870.
    {
        if (!sortPicture) return;
        sortPicture->SetHidden(0);
        if (sortLabels[1]) sortLabels[1]->SetColor(0xB45C4E14);
        if (sortLabels[2]) sortLabels[2]->SetColor(0xB45C4E14);
        if (sortLabels[3]) sortLabels[3]->SetColor(0xB45C4E14);
    }

    int InventoryInterface::OnEvent(::GUI::Window* window,::GUI::Event* event) //43FF30 ->43F620.
    {
        JE::Client* client=NULL;
        if (GetHidden() || !(client=InventoryInputServices::GetClient()))
            return JE::EngineInterface::OnEvent(window,event);
        if (backpackScroll->Process(window,event))
        {
            RefreshGridBindings();
            PopulateItems();
            return JE::EngineInterface::OnEvent(window,event);
        }
        GameCursorManager* cursor=static_cast<GameCursorManager*>(client->child0xA4);
        GameInventory* inventory=GetInventory();
        if (event->type==4 || event->type==7)
        {
            bool gridItem=false;
            LONG equipment=FindEquipmentIndex(window);
            if (equipment!=-1 && cursor->context==1)
            {
                DWORD item=equipmentSlots[equipment]->itemID;
                if (item)
                {
                    SetDraggedItem(*cursor,item,actorID);
                    PickupSound();
                    DescribeItem(equipmentSlots[equipment]->itemID,actorID);
                    ResetGrid();
                    equipmentSlots[equipment]->SetState(1);
                    RefreshEquipmentAvailability();
                }
            }
            LONG row,column;
            FindBackpackIndex(window,row,column);
            if (row!=-1 && column!=-1 && cursor->context==1)
            {
                InventorySlot* slot=backpackSlots[row][column];
                if (slot->itemID)
                {
                    if (GetAsyncKeyState(VK_CONTROL)&0x8000)
                        ToggleSelection(row,column);
                    else
                    {
                        SetDraggedItem(*cursor,slot->itemID,actorID);
                        PickupSound();
                        if (selectedItemIDs.size()<=1)
                        {
                            DescribeItem(slot->itemID,actorID);
                            if (slot->picture) slot->SetState(1);
                        }
                        else
                        {
                            cursor->ClearDragChoices();
                            cursor->AppendDragChoices(selectedItemIDs);
                            //Live held nodes, no snapshot/recovery after callbacks.
                            for (std::list<DWORD>::iterator i=selectedItemIDs.begin();i!=selectedItemIDs.end();++i)
                                for (LONG r=0;r<33;++r)
                                    for (LONG c=0;c<3;++c)
                                        if (backpackSlots[r][c] && backpackSlots[r][c]->itemID==*i)
                                            backpackSlots[r][c]->SetState(1);
                        }
                        RefreshEquipmentAvailability();
                    }
                    gridItem=true;
                }
            }
            if (window==sortLabels[0]) ShowSort();
            DWORD descriptionReference=window->valueCD4;
            if (window->valueCF4!=DWORD(-1))
            {
                std::string title,description;
                GetString(window->valueCF4,title);
                GetString(descriptionReference,description);
                SetDescription(title+"\n"+"\n"+description);
            }
            if (!gridItem) ResetGrid();
        }
        else if (event->type==5)
        {
            BYTE unhandled=1;
            LONG equipment=FindEquipmentIndex(window);
            if (equipment!=-1 && cursor->context==14 && equipmentSlots[equipment]->visible==1)
            {
                if (cursor->draggedItemID==equipmentSlots[equipment]->itemID) PutDownSound();
                else
                {
                    std::list<DWORD> slots;
                    if (EquippedSlotsByID(inventory,cursor->draggedItemID,slots))
                        client->RequestUnequip(actorID,cursor->draggedItemID);
                    BeginEquip(DWORD(equipment));
                    PutDownSound();
                    BindEquipmentDrop(DWORD(equipment));
                    unhandled=0;
                    ResetGrid();
                    RefreshEquipmentAvailability();
                    GameEntity* item=dynamic_cast<GameEntity*>(inventory->FindByID(equipmentSlots[equipment]->itemID));
                    if (item) item->value8C=DWORD(-1);
                }
            }
            LONG row,column;
            FindBackpackIndex(window,row,column);
            if (window==backpack)
            {
                LONG index=MaxBackpackIndex()+1;
                row=index/3; column=index%3;
            }
            if (row!=-1 && column!=-1 && cursor->context==14)
            {
                if (cursor->draggedItemID!=backpackSlots[row][column]->itemID)
                {
                    DWORD item=cursor->draggedItemID,source=cursor->draggedOwnerID;
                    std::list<DWORD> slots;
                    if (EquippedSlotsByID(inventory,item,slots))
                    {
                        LONG index=MaxBackpackIndex()+1;
                        SetUnequippedIndex(item,index);
                        PutDownSound();
                        SortItems(4,0);
                        BindBackpackDrop(index);
                        unhandled=0;
                    }
                    else if (source!=actorID)
                    {
                        client->RequestInventoryTransfer(actorID,item,source);
                        SetDraggedItem(*cursor,0,0);
                    }
                    ResetGrid();
                    RefreshEquipmentAvailability();
                }
            }
            if (window==dropPicture && cursor->context==14)
            {
                DWORD item=cursor->draggedItemID;
                if (cursor->draggedOwnerID==actorID)
                {
                    if (selectedItemIDs.size()<=1)
                    {
                        dynamic_cast<GameEntity*>(inventory->FindByID(item))->value8C=DWORD(-1);
                        client->RequestDropItem(actorID,item);
                    }
                    else
                    {
                        for (std::list<DWORD>::iterator i=selectedItemIDs.begin();i!=selectedItemIDs.end();++i)
                        {
                            DWORD id=*i;
                            dynamic_cast<GameEntity*>(inventory->FindByID(id))->value8C=DWORD(-1);
                            client->RequestDropItem(actorID,id);
                        }
                    }
                    SetDraggedItem(*cursor,0,0);
                }
                ResetGrid();
                RefreshEquipmentAvailability();
            }
            if (window==sortLabels[1]) SortItems(1,0);
            else if (window==sortLabels[2]) SortItems(2,0);
            else if (window==sortLabels[3]) SortItems(3,0);
            if (cursor->context==14 && unhandled)
            {
                SetDraggedItem(*cursor,0,0);
                PopulateItems();
                RefreshEquipmentText();
                RefreshEquipmentAvailability();
            }
            HideSort();
        }
        else if (event->type==1)
        {
            if (window==sortLabels[1] || window==sortLabels[2] || window==sortLabels[3])
                static_cast< ::GUI::Label*>(window)->SetTextColor(0xB4808000);
        }
        else if (event->type==3)
        {
            if (window==sortLabels[1] || window==sortLabels[2] || window==sortLabels[3])
                static_cast< ::GUI::Label*>(window)->SetTextColor(0xB45C4E14);
        }
        else if (event->type==17 && window==background && event->x==1)
        {
            GameCursorManager* current=static_cast<GameCursorManager*>(client->child0xA4);
            if (!current || !InventoryInputServices::CancelSpecialContext(*current))
                SetHidden(1);
            return 1; //43FBB7: close path bypasses EngineInterface tail.
        }
        return JE::EngineInterface::OnEvent(window,event);
    }
}

