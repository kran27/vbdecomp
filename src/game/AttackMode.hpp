#pragma once

#include "entity/Chunk.hpp"
#include <list>

namespace F3
{
    struct AttackMode
    {
        AttackMode()
        {
            flag66 = 0;
            calledShot = 0;
            kind = 3;
        } // 40DBA0 inline local defaults; no zero fill.
        enum Owner
        {
            Weapon
        };
        explicit AttackMode(Owner)
        {
            flag66 = 0;
            calledShot = 0;
        } // 4148A0 inline local defaults; kind remains unwritten.
        // 40A980 uses implicit memberwise copy construction, including the real string.
        AttackMode& operator=(const AttackMode& source);
        BYTE Read(JE::Common::Chunk& chunk,
                  JE::Resources::ResourceMemoryStream*
                      stream); // Source-only factored helper; native40DBA0 belongs to GameCreature.
        BYTE ReadWeapon(JE::Common::Chunk& chunk, JE::Resources::ResourceMemoryStream* stream,
                        DWORD weaponClass); // Source-only factored helper; native4148A0 belongs to GameWeapon.
        void MakeFailsafe(); // Local record built by40D9E0.
        DWORD kind, animation, damageType, value0C;
        BYTE unwritten10[4];
        DWORD shots, values18[3], range, values28[2], minimumDamage, maximumDamage;
        BYTE flag38, calledShot, consumed, thrown;
        DWORD actionPoints, pose, nameRef;
        std::string effectName;
        BYTE flag64, flag65, flag66;

      private:
        BYTE ReadData(JE::Common::Chunk& chunk, JE::Resources::ResourceMemoryStream* stream, BYTE weapon,
                      DWORD weaponClass); // Factored identical proprietary read sequence.
    };

    // A default-constructed iterator was used as a nullable selection throughout
    // the VC7 engine. Comparing that iterator to real std::list iterators is
    // invalid with today's checked STL. Keep null separate from an STL iterator,
    // and retain the owning list so different weapons/creatures cannot have
    // their iterators compared accidentally.
    class AttackModeIterator
    {
      public:
        using Container = std::list<AttackMode>;
        using Iterator = Container::iterator;

        AttackModeIterator() = default;

        static AttackModeIterator First(Container& modes)
        {
            if (modes.empty())
            {
                return {};
            }

            return AttackModeIterator(modes, modes.begin());
        }

        static AttackModeIterator From(Container& modes, Iterator position)
        {
            if (position == modes.end())
            {
                return {};
            }

            return AttackModeIterator(modes, position);
        }

        explicit operator bool() const noexcept { return m_owner != nullptr; }

        bool BelongsTo(const Container& modes) const noexcept { return m_owner == &modes; }

        AttackMode& operator*() const { return *m_position; }

        AttackMode* operator->() const { return &**this; }

        AttackModeIterator& operator++()
        {
            ++m_position;

            if (m_position == m_owner->end())
            {
                *this = {};
            }

            return *this;
        }

        AttackModeIterator operator++(int)
        {
            AttackModeIterator previous = *this;
            ++*this;
            return previous;
        }

        friend bool operator==(const AttackModeIterator& lhs, const AttackModeIterator& rhs)
        {
            if (lhs.m_owner != rhs.m_owner)
            {
                return false;
            }

            if (!lhs.m_owner)
            {
                return true;
            }

            return lhs.m_position == rhs.m_position;
        }

        friend bool operator!=(const AttackModeIterator& lhs, const AttackModeIterator& rhs) { return !(lhs == rhs); }

      private:
        AttackModeIterator(Container& modes, Iterator position) : m_owner(&modes), m_position(position) {}

        Container* m_owner = nullptr;
        Iterator m_position{};
    };
} // namespace F3
