#include "GameCreature.hpp"
#include "entity/EntityEnvironment.hpp"

namespace F3
{
	void GameCreature::OnActivate(DWORD time)
	{
		//4093C0: desired team Entity+17C is applied through Server555890 before
		//the qualified GameEntity467640 activation, and its BYTE result is ignored.
		if (value17C) JE::GetEntityEnvironment().SetEntityTeam(this, value17C);
		GameEntity::OnActivate(time);
	}
}
