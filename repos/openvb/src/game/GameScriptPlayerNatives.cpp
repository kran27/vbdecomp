#include "GameScriptNatives.hpp"
#include "GameWorldCollections.hpp"
#include "entity/World.hpp"
#include <stdexcept>

namespace F3
{
	cell AMX_NATIVE_CALL NativeProtagonist(AMX* vm, cell* parameters)
	{
		if (!VerifyScriptParameters(vm, parameters, 0, "Protagonist")) return 0;
		GameParty* party = dynamic_cast<GameParty*>(JE::GetWorldInstance()->GetParty());
		if (!party)
		{
			LogScriptError(vm, "Protagonist() could not find GameParty. Returning INVALID_ID.");
			return 0;
		}
		JE::Entity* protagonist = party->selectedEntity;
		if (!protagonist)
		{
			LogScriptError(vm, "Protagonist() Protagonist is NULL. Returning INVALID_ID.");
			return 0;
		}
		return cell(protagonist->entityID);
	}
}
