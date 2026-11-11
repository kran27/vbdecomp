#include "game/ActionbarInterface.hpp"
#include "game/CounterControl.hpp"
#include "game/GameCreature.hpp"
#include "gui/Label.hpp"
#include "gui/Picture.hpp"
#include <cstdio>

namespace F3
{
    void ActionbarInterface::SetPartyHealth(LONG index, LONG percent) //416C60.
    {
        if (index < 0 || index >= 6) return;
        if (percent <= 0)
            partyHealth[index]->SetHidden(1);
        else
        {
            partyHealth[index]->SetHidden(0);
            //Original IMUL retains low32 before signed division by100.
            LONG width = LONG(DWORD(percent) * 88u) / 100;
            if (width > 88) width = 88;
            partyHealth[index]->Resize(width, 4); //Reload after SetHidden callback.
        }
    }

    void ActionbarInterface::RefreshHealthDigits(BYTE force) //416CD0.
    {
        GameCreature* creature = dynamic_cast<GameCreature*>(JE::GetEntityPtr(entityID));
        const DWORD mode = force ? 2 : 0;
        if (creature)
        {
            healthDigits->SetValue(creature->currentHealth, mode);
            fatigueDigits->SetValue(0, mode); //Actual literal0, not an inferred stat.
            evasionDigits->SetValue(creature->GetDerivedAttribute(4), mode);
        }
        else
        {
            healthDigits->SetValue(0, mode);
            fatigueDigits->SetValue(0, mode);
            evasionDigits->SetValue(0, mode);
        }
    }

    void ActionbarInterface::RefreshHealthBar() //416D80.
    {
        GameCreature* creature = dynamic_cast<GameCreature*>(JE::GetEntityPtr(entityID));
        if (!creature) return; //Original retains the previous bar on failed cast.
        const DWORD maximum = DWORD(creature->GetMaximumHealth());
        //416DC2 XOR EDX,EDX /416DC4 DIV ECX: original unsigned ratio,
        //including low32 IMUL and negative-health bit-pattern quirks.
        if (maximum)
            SetPartyHealth(0, LONG((DWORD(creature->currentHealth) * 100u) / maximum));
        else SetPartyHealth(0, 0);
    }

    void ActionbarInterface::SetActionPoints(LONG current, LONG maximum,
        LONG reserved, BYTE force) //418360, exactly4 stack args (RETN10h).
    {
        if (!force && current == cachedCurrentAP && maximum == cachedMaximumAP
            && reserved == cachedReservedAP) return;
        //Original publication order precedes all texture/GUI child calls.
        cachedMaximumAP = maximum;
        cachedCurrentAP = current;
        cachedReservedAP = reserved;
        LONG index = 0;
        while (index < reserved && index < 30)
        {
            actionPointIcons[index]->LoadTexture("f3_actionbar_apfine_blue.tga",
                0, 0.0f, 0.0f, -1.0f, -1.0f, 1);
            actionPointIcons[index]->SetHidden(0);
            ++index;
        }
        while (index < current && index < 30)
        {
            actionPointIcons[index]->LoadTexture("f3_actionbar_apfine_green.tga",
                0, 0.0f, 0.0f, -1.0f, -1.0f, 1);
            actionPointIcons[index]->SetHidden(0);
            ++index;
        }
        while (index < maximum && index < 30)
        {
            actionPointIcons[index]->LoadTexture("f3_actionbar_apfine_grey.tga",
                0, 0.0f, 0.0f, -1.0f, -1.0f, 1);
            actionPointIcons[index]->SetHidden(0);
            ++index;
        }
        for (; index < 30; ++index) actionPointIcons[index]->SetHidden(1);
        if (actionPointsValue)
        {
            char buffer[32];
            std::sprintf(buffer, "%d", static_cast<int>(cachedCurrentAP));
            actionPointsValue->SetText(buffer);
        }
    }

    void ActionbarInterface::RefreshActionPoints(BYTE force) //418820.
    {
        GameCreature* creature = dynamic_cast<GameCreature*>(JE::GetEntityPtr(entityID));
        if (creature)
        {
            //418857 uses the real Entity virtual-base+174 actionBudget.
            const LONG current = LONG(creature->actionBudget);
            const LONG maximum = creature->GetDerivedAttribute(0);
            const LONG reserved = LONG(creature->value7FC);
            SetActionPoints(current, maximum, reserved, force);
        }
        else SetActionPoints(0, 0, 0, force);
    }
}
