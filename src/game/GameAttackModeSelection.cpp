#include "game/GameWeapon.hpp"
#include "game/GameCreature.hpp"

namespace F3
{
    namespace
    {
        // The original availability check always returned true. Preserve its
        // separate call boundary for a future implementation.
        __declspec(noinline) BYTE __stdcall IsAttackModeAvailable(AttackModeIterator)
        {
            return 1;
        }

        // The weapon sequence terminates at the end; the creature sequence
        // wraps to the first unarmed attack. A null or foreign-list selection
        // starts from the first entry in either case.
        AttackModeIterator FindNextAvailableMode(std::list<AttackMode>& modes, AttackModeIterator selected, bool wrap)
        {
            if (modes.empty())
            {
                return {};
            }

            if (!selected.BelongsTo(modes))
            {
                selected = {};
            }

            for (std::size_t visited = 0; visited < modes.size(); ++visited)
            {
                if (selected)
                {
                    ++selected;

                    if (!selected && !wrap)
                    {
                        return {};
                    }
                }

                if (!selected)
                {
                    selected = AttackModeIterator::First(modes);
                }

                if (IsAttackModeAvailable(selected))
                {
                    return selected;
                }
            }

            return {};
        }
    } // namespace

    AttackMode* GameWeapon::CurrentAttackMode()
    {
        return currentAttackMode ? &*currentAttackMode : nullptr;
    }

    AttackModeIterator GameWeapon::PeekNextAttackMode()
    {
        return FindNextAvailableMode(activeAttackModes, currentAttackMode, false);
    }

    AttackModeIterator GameWeapon::PeekAttackMode(LONG step)
    {
        AttackModeIterator candidate = currentAttackMode;

        for (LONG i = 0; i < step; ++i)
        {
            candidate = FindNextAvailableMode(activeAttackModes, candidate, false);
        }

        return candidate;
    }

    void GameWeapon::CycleAttackMode()
    {
        currentAttackMode = PeekNextAttackMode();
    }

    void GameWeapon::SelectAttackMode(LONG step)
    {
        currentAttackMode = PeekAttackMode(step);
    }

    AttackMode* GameCreature::CurrentAttackMode()
    {
        return currentAttackMode ? &*currentAttackMode : nullptr;
    }

    AttackModeIterator GameCreature::PeekNextAttackMode()
    {
        return FindNextAvailableMode(activeAttackModes, currentAttackMode, true);
    }

    void GameCreature::CycleAttackMode()
    {
        currentAttackMode = PeekNextAttackMode();
    }
} // namespace F3
