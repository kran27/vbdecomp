#include "game/GameWeapon.hpp"
#include "game/GameCreature.hpp"

namespace F3
{
#if defined(_MSC_VER) && _MSC_VER == 1300
	typedef char AttackModeIteratorHasNativeWord[sizeof(AttackModeIterator) == 4 ? 1 : -1];
#endif
	namespace
	{
		//409BB0 accepts one iterator word and always returns AL=1/RET4.
		//Its unused receiver identity is unresolved; this is an explicit ABI spelling.
		__declspec(noinline) BYTE __stdcall IsAttackModeAvailable(AttackModeIterator)
		{
			return 1;
		}
	}

	AttackMode* GameWeapon::CurrentAttackMode()
	{
		return currentAttackMode == AttackModeIterator() ? NULL : &*currentAttackMode;
	}

	AttackModeIterator GameWeapon::PeekNextAttackMode() //412E60, RET4 hidden output.
	{
		AttackModeIterator candidate = currentAttackMode;
		AttackModeIterator end = activeAttackModes.end();
		BYTE accepted = 0;
		DWORD visited = 0;
		for (;;)
		{
			if (candidate == AttackModeIterator()) candidate = activeAttackModes.begin();
			else ++candidate;
			if (candidate == end) break;
			if (IsAttackModeAvailable(candidate)) accepted = 1;
			if (++visited > activeAttackModes.size()) break;
			if (accepted) return candidate;
		}
		return AttackModeIterator();
	}

	AttackModeIterator GameWeapon::PeekAttackMode(LONG step) //412ED0, RET8.
	{
		AttackModeIterator candidate = currentAttackMode;
		for (LONG i = 0; i < step; ++i)
		{
			BYTE accepted = 0;
			DWORD visited = 0;
			for (;;)
			{
				if (candidate == AttackModeIterator()) candidate = activeAttackModes.begin();
				else ++candidate;
				if (candidate == activeAttackModes.end())
				{
					candidate = AttackModeIterator();
					break;
				}
				if (IsAttackModeAvailable(candidate)) accepted = 1;
				if (++visited > activeAttackModes.size()) return AttackModeIterator();
				if (accepted) break;
			}
		}
		return candidate;
	}

	void GameWeapon::CycleAttackMode() //4134A0; publish before every test/call.
	{
		BYTE accepted = 0;
		DWORD visited = 0;
		for (;;)
		{
			if (currentAttackMode == AttackModeIterator()) currentAttackMode = activeAttackModes.begin();
			else ++currentAttackMode;
			if (currentAttackMode == activeAttackModes.end()) break;
			if (IsAttackModeAvailable(currentAttackMode)) accepted = 1;
			if (++visited > activeAttackModes.size()) break;
			if (accepted) return;
		}
		currentAttackMode = AttackModeIterator();
	}

	void GameWeapon::SelectAttackMode(LONG step) //413500.
	{
		currentAttackMode = PeekAttackMode(step);
	}

	AttackMode* GameCreature::CurrentAttackMode()
	{
		return currentAttackMode == AttackModeIterator() ? NULL : &*currentAttackMode;
	}

	AttackModeIterator GameCreature::PeekNextAttackMode() //409D10, RET4.
	{
		AttackModeIterator candidate = currentAttackMode;
		DWORD count = DWORD(activeAttackModes.size());
		BYTE accepted = 0;
		DWORD visited = 0;
		if (!count) return AttackModeIterator();
		AttackModeIterator end = activeAttackModes.end();
		for (;;)
		{
			if (candidate == AttackModeIterator()) candidate = activeAttackModes.begin();
			else ++candidate;
			if (candidate == end) candidate = activeAttackModes.begin();
			if (IsAttackModeAvailable(candidate)) accepted = 1;
			if (++visited > count) break;
			if (accepted) return candidate;
		}
		return AttackModeIterator();
	}

	void GameCreature::CycleAttackMode() //409DA0, live member and count.
	{
		if (!activeAttackModes.size())
		{
			currentAttackMode = AttackModeIterator();
			return;
		}
		BYTE accepted = 0;
		DWORD visited = 0;
		for (;;)
		{
			if (currentAttackMode == AttackModeIterator()) currentAttackMode = activeAttackModes.begin();
			else ++currentAttackMode;
			if (currentAttackMode == activeAttackModes.end()) currentAttackMode = activeAttackModes.begin();
			if (IsAttackModeAvailable(currentAttackMode)) accepted = 1;
			if (++visited > activeAttackModes.size()) break;
			if (accepted) return;
		}
		currentAttackMode = AttackModeIterator();
	}
}
