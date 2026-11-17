#include "GameCreature.hpp"
#include "GameInventory.hpp"
#include "GameWeapon.hpp"
#include "scripting/ScriptSmall.hpp"
#include <stdexcept>

namespace F3
{
    BYTE GameCreature::ResolveWeaponAndAttackMode(GameWeapon** weapon, AttackModeIterator* mode, BYTE* unarmed)
    {
        // 40B3A1 captures Client+8C before the active-slot/inventory children.
        void* debugWindow = JE::ScriptServices::GetDebugWindow();
        DWORD slot = GetActiveWeaponSlot();
        GameInventory* items = dynamic_cast<GameInventory*>(inventory);
        if (!items)
        {
            return 0;
        }
        GameItem* item = items->GetItem(slot);
        *unarmed = 0;
        if (item)
        {
            if (item->entityClass != 6)
            {
                if (debugWindow)
                {
                    JE::ScriptServices::DebugOutput(
                        debugWindow,
                        "GameEntity::ResolveWeaponAndAttackMode() - Attack was attempted with non-weapon in hand.", 128,
                        128, 128);
                }
                return 0;
            }
            GameWeapon* selected = dynamic_cast<GameWeapon*>(item);
            *weapon = selected;

            if (!selected)
            {
                return 0;
            }

            *mode = selected->currentAttackMode;
        }
        else
        {
            *unarmed = 1;
            *mode = currentAttackMode;
            // Original deliberately leaves the prior weapon output untouched here.
        }
        // A missing attack mode must not be dereferenced. This can occur
        // before the attack-mode lists are loaded or rebuilt.
        if (!*mode)
        {
            return 0;
        }

        if ((*mode)->kind == 3 && !*unarmed)
        {
            *unarmed = 1;
            *mode = currentAttackMode;
        }

        return *mode ? 1 : 0;
    }
} // namespace F3
