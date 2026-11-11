#include "game/ActionbarInterface.hpp"
#include "game/ActionbarServices.hpp"
#include "game/GameInventory.hpp"
#include "game/GameItem.hpp"
#include "game/GameWeapon.hpp"
#include "game/GameCreature.hpp"
#include "game/ClientAttackModeRequests.hpp"
#include "startup/EngineStringSystem.hpp"
#include "startup/GameClient.hpp"
#include "resources/ResourceSystem.hpp"
#include "gui/Button.hpp"
#include "gui/Picture.hpp"
#include "gui/Label.hpp"
#include <cstdio>

namespace F3
{
    void ActionbarInterface::WeaponIcon(std::string& output, GameItem* item) //417BC0.
    {
        std::string name=item->activeTexture;
        if (!name.empty()) output=name;
        else
        {
            name=item->inventoryTexture;
            if (name.size()>8)
            {
                output=name;
                output.replace(name.size()-8,4,"_ACT");
            }
            else output="";
        }
    }

    GameInventory* ActionbarInterface::GetInventory() //416DF0.
    {
        if (!entityID) return NULL;
        JE::Entity* entity=JE::GetEntityPtr(entityID);
        if (!entity) return NULL;
        return dynamic_cast<GameInventory*>(entity->inventory);
    }

    BYTE ActionbarInterface::IsWeaponActive(DWORD index) //416E80.
    {
        if (LONG(index)<0 || LONG(index)>=2) return 0;
        return weaponMasks[index] && !BYTE(weaponMasks[index]->GetAlpha());
    }

    void ActionbarInterface::SetWeaponText(DWORD index,const std::string& text) //416E30.
    {
        if (LONG(index)<0 || LONG(index)>=2) return;
        ::GUI::Label* up=weaponModeUp[index];
        ::GUI::Label* down=weaponModeDown[index];
        if (up && down) { up->SetText(text); down->SetText(text); }
    }

    void ActionbarInterface::SetWeaponTextReference(DWORD index,DWORD reference) //417450.
    {
        std::string text;
        GetString(reference,text);
        SetWeaponText(index,text);
    }

    void ActionbarInterface::SetWeaponActive(DWORD index,BYTE active) //416EB0.
    {
        if (LONG(index)<0 || LONG(index)>=2) return;
        ::GUI::Label* mode=weaponModeUp[index];
        ::GUI::Button* button=weaponButtons[index];
        ::GUI::Picture* mask=weaponMasks[index];
        ::GUI::Label* ap=weaponAPUp[index];
        if (!button || !mode || !ap || !mask) return;
        if (active)
        {
            mode->SetTextColor(0xB46E6E3C);
            ap->SetTextColor(0xB46E6E3C);
            mask->SetAlpha(0); mask->SetMaterialModes(0);
            button->SetKeyboardEnabled(1);
        }
        else
        {
            mode->SetTextColor(0xB43C3C3C);
            ap->SetTextColor(0xB43C3C3C);
            mask->SetAlpha(100); mask->SetMaterialModes(1);
            button->SetKeyboardEnabled(0);
        }
    }

    void ActionbarInterface::SetWeaponIcon(DWORD index,DWORD itemID,DWORD ownerID) //417CF0.
    {
        if (LONG(index)<0 || LONG(index)>=2) return;
        ::GUI::Picture* up=weaponIconsUp[index];
        ::GUI::Picture* down=weaponIconsDown[index];
        if (!up || !down) return;
        if (!itemID)
        {
            up->SetAlpha(0); up->SetMaterialModes(0);
            down->SetAlpha(0); down->SetMaterialModes(0);
            return;
        }
        const DWORD capturedItemID=itemID;
        JE::Entity* owner=JE::GetEntityPtr(ownerID);
        JE::Entity* entity=owner->inventory->FindByID(capturedItemID);
        GameItem* item=dynamic_cast<GameItem*>(entity);
        if (item)
        {
            std::string name;
            WeaponIcon(name,item);
            if (name.empty() || !JE::Resources::g_pResourceSystem->Contains(name.c_str()))
                name="icon_unknown.bmp";
            up->SetAlpha(255); up->SetMaterialModes(0);
            up->LoadTexture(name,0,0,0,-1,-1,1);
            down->SetAlpha(255); down->SetMaterialModes(0);
            down->LoadTexture(name,0,0,0,-1,-1,1);
        }
        else
        {
            up->SetAlpha(255); up->SetMaterialModes(0);
            up->LoadTexture("icon_unknown.bmp",0,0,0,-1,-1,1);
            down->SetAlpha(255); down->SetMaterialModes(0);
            down->LoadTexture("icon_unknown.bmp",0,0,0,-1,-1,1);
        }
    }

    void ActionbarInterface::SetWeaponAP(DWORD index,LONG value) //417F50.
    {
        if (LONG(index)<0 || LONG(index)>=2) return;
        ::GUI::Label* up=weaponAPUp[index];
        ::GUI::Label* down=weaponAPDown[index];
        if (!up || !down) return;
        char text[32];
        std::sprintf(text,"AP%d",value);
        up->SetText(std::string(text));
        down->SetText(std::string(text));
    }

    void ActionbarInterface::SetAmmoPercent(LONG index,LONG percent) //418040, actual RET8.
    {
        if (percent<0) percent=0;
        else if (percent>100) percent=100;
        const LONG count=16*percent/100;
        if (index<0 || index>=2) return;
        LONG i=0;
        for (;i<count;++i)
        {
            reloadIcons[0][index][i]->LoadTexture(std::string("f3_actionbar_ammo_on.tga"),0,0,0,-1,-1,1);
            reloadIcons[1][index][i]->LoadTexture(std::string("f3_actionbar_ammo_on.tga"),0,0,0,-1,-1,1);
        }
        for (;i<16;++i)
        {
            reloadIcons[0][index][i]->LoadTexture(std::string("f3_actionbar_ammo_off.tga"),0,0,0,-1,-1,1);
            reloadIcons[1][index][i]->LoadTexture(std::string("f3_actionbar_ammo_off.tga"),0,0,0,-1,-1,1);
        }
    }

    void ActionbarInterface::SetAmmoAmount(LONG index,LONG amount,LONG maximum) //41C000, actual RET0C.
    {
        if (index<0 || index>=2) return;
        if (!maximum) SetAmmoPercent(index,0);
        else
        {
            //Raw IMUL32 then signed IDIV; avoid a source signed-overflow operation.
            const LONG numerator=LONG(DWORD(amount)*100u);
            SetAmmoPercent(index,numerator/maximum);
        }
    }

    void ActionbarInterface::SetCalledShot(DWORD index,BYTE value) //416FF0.
    {
        if (LONG(index)<0 || LONG(index)>=2) return;
        if (calledShotUp[index]) calledShotUp[index]->SetHidden(value==0);
        if (calledShotDown[index]) calledShotDown[index]->SetHidden(value==0);
    }

    void ActionbarInterface::RefreshInventory() //41C4C0.
    {
        SetWeaponIcon(0,0,0); SetWeaponIcon(1,0,0);
        SetWeaponText(0,std::string(""));
        SetWeaponText(0,std::string("")); //Original41C58D pushes0 again, not panel1.
        SetWeaponAP(0,0); SetWeaponAP(1,0);
        SetAmmoPercent(0,0); SetAmmoPercent(1,0);
        SetCalledShot(0,0); SetCalledShot(1,0);
        GameInventory* inventory=GetInventory();
        if (!inventory) return;
        for (std::list<JE::Entity*>::iterator i=inventory->items.begin();i!=inventory->items.end();++i)
        {
            JE::Entity* item=*i;
            if (!item) continue;
            std::list<DWORD> slots;
            if (inventory->GetEquippedSlots(item,slots))
            {
                BYTE first=0,second=0,both=0;
                if (!slots.empty())
                {
                    do
                    {
                        DWORD slot=slots.front();
                        if (slot==2) { first=1; if (second) both=1; }
                        else if (slot==3) { second=1; if (first) both=1; }
                        slots.pop_front(); //41C6A9..6D2 consumes nodes before icon call.
                    } while (!slots.empty());
                    if (!both)
                    {
                        if (first) SetWeaponIcon(0,item->entityID,entityID);
                        else if (second) SetWeaponIcon(1,item->entityID,entityID);
                    }
                }
            }
        }
        RefreshAttackMode(0);
        RefreshAttackMode(1);
    }

    void ActionbarInterface::RefreshAttackMode(DWORD index) //41C2E0.
    {
        GameInventory* inventory=GetInventory();
        if (!inventory) return;
        GameItem* item=NULL;
        if (!index) item=inventory->GetItem(2);
        else if (index==1) item=inventory->GetItem(3);
        GameWeapon* weapon=dynamic_cast<GameWeapon*>(item);
        GameCreature* creature=dynamic_cast<GameCreature*>(JE::GetEntityPtr(entityID));
        if (weapon)
        {
            AttackMode* mode=weapon->CurrentAttackMode();
            if (mode)
            {
                SetWeaponTextReference(index,mode->nameRef);
                SetWeaponAP(index,LONG(mode->actionPoints));
                SetAmmoAmount(LONG(index),LONG(weapon->primaryAmmoLoaded),LONG(weapon->primaryAmmoCapacity));
                SetCalledShot(index,mode->calledShot);
                return;
            }
            SetWeaponTextReference(index,389);
            SetWeaponAP(index,6);
            SetAmmoAmount(LONG(index),LONG(weapon->primaryAmmoLoaded),LONG(weapon->primaryAmmoCapacity));
        }
        else
        {
            if (item)
            {
                SetWeaponTextReference(index,378);
                SetWeaponAP(index,3);
            }
            else
            {
                if (creature)
                {
                    AttackMode* mode=creature->CurrentAttackMode();
                    if (mode)
                    {
                        SetWeaponTextReference(index,mode->nameRef);
                        SetWeaponAP(index,LONG(mode->actionPoints));
                        SetAmmoPercent(LONG(index),0);
                        SetCalledShot(index,mode->calledShot);
                        return;
                    }
                }
                SetWeaponText(index,std::string());
                SetWeaponAP(index,0);
            }
            SetAmmoPercent(LONG(index),0);
        }
        SetCalledShot(index,0);
    }

    void ActionbarInterface::SetAttackMode(DWORD index,LONG step) //41C040.
    {
        GameInventory* inventory=GetInventory();
        if (!inventory) return;
        GameItem* item=NULL;
        if (!index) item=inventory->GetItem(2);
        else if (index==1) item=inventory->GetItem(3);
        GameWeapon* weapon=dynamic_cast<GameWeapon*>(item);
        GameCreature* creature=dynamic_cast<GameCreature*>(JE::GetEntityPtr(entityID));
        if (weapon)
        {
            AttackModeIterator previous=weapon->currentAttackMode;
            AttackModeIterator next=step<=1 ? weapon->PeekNextAttackMode() : weapon->PeekAttackMode(step);
            if (previous!=next)
            {
                //Original performs actual GameClient RTTI here before either branch.
                GameClient* client=dynamic_cast<GameClient*>(ActionbarServices::GetClient());
                if (step<=1) client->RequestWeaponModeCycle(weapon->entityID);
                else weapon->SelectAttackMode(step);
                if (next != AttackModeIterator())
                {
                    SetWeaponTextReference(index,next->nameRef);
                    SetWeaponAP(index,LONG(next->actionPoints));
                    SetAmmoAmount(LONG(index),LONG(weapon->primaryAmmoLoaded),LONG(weapon->primaryAmmoCapacity));
                    SetCalledShot(index,next->calledShot);
                }
                else
                {
                    SetWeaponTextReference(index,389);
                    SetWeaponAP(index,6);
                    SetAmmoAmount(LONG(index),LONG(weapon->primaryAmmoLoaded),LONG(weapon->primaryAmmoCapacity));
                    SetCalledShot(index,0);
                }
            }
        }
        else if (!item && creature)
        {
            AttackModeIterator previous=creature->currentAttackMode;
            AttackModeIterator next=creature->PeekNextAttackMode();
            if (previous!=next)
            {
                GameClient* client=dynamic_cast<GameClient*>(ActionbarServices::GetClient());
                DWORD id=creature->entityID;
                client->RequestUnarmedModeCycle(id);
                if (next != AttackModeIterator())
                {
                    SetWeaponTextReference(index,next->nameRef);
                    SetWeaponAP(index,LONG(next->actionPoints));
                    SetAmmoPercent(LONG(index),0);
                    SetCalledShot(index,next->calledShot);
                }
                else
                {
                    SetWeaponText(index,std::string());
                    SetWeaponAP(index,0);
                    SetAmmoPercent(LONG(index),0);
                    SetCalledShot(index,0);
                }
            }
        }
    }
}

