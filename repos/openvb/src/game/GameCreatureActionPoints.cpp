#include "GameCreature.hpp"
#include "diagnostics/DebugOutput.hpp"

namespace F3
{
	LONG GameCreature::GetDerivedAttribute(LONG attribute)
	{
		// Original+760 is the seven-value copy populated by40B6A0. The existing
		// source calls this array baseSpecial; do not replace it with sheet inputs.
		switch (attribute)
		{
		case 0: return LONG(DWORD(baseSpecial[5]) + 20);
		case 1: return LONG(25 * (DWORD(baseSpecial[0]) + 1));
		case 2: return LONG(2 * DWORD(baseSpecial[1]));
		case 3: return baseSpecial[6];
		case 4: return LONG(value7FC);
		case 5: return baseSpecial[2];
		case 6: return baseSpecial[0];
		case 7: return LONG(3 * DWORD(baseSpecial[1]) + 40);
		case 8: return 3;
		case 9: case 10: case 11: return 0;
		case 12: return LONG(2 * DWORD(baseSpecial[4]) + 5);
		case 13: return LONG(3 * DWORD(baseSpecial[0]));
		case 14:
			{
				double rate = double(GetDerivedAttribute(0)) * 0.0001666666666666667;
				return rate == 0.0 ? 1000 : LONG(static_cast<__int64>(1.0 / rate));
			}
		default:
			JE::LogDebugString("GameCreature::GetDerivedAttribute() - Unknown attribute requested: %d", attribute);
			return 0;
		}
	}

	void GameCreature::UpdateClient(DWORD time)
	{
		JE::Entity::UpdateClient(time);
		if (moving)
		{
			creationServerTime = time;
			return;
		}
		LONG period = GetDerivedAttribute(14);
		LONG elapsed = LONG(time - creationServerTime);
		if (elapsed >= period)
		{
			DWORD maximum = DWORD(GetDerivedAttribute(0));
			if (actionBudget < maximum)
			{
				actionBudget += elapsed / period; //Original signed IDIV; no invented zero-period policy.
				if (actionBudget > maximum) actionBudget = maximum;
			}
			creationServerTime = time;
		}
	}
}
