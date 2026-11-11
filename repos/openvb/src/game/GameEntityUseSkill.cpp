#include "GameEntity.hpp"
#include "scripting/ScriptSmall.hpp"
#include "diagnostics/DebugOutput.hpp"

namespace F3
{
	void GameEntity::UseSkill(DWORD skill, JE::Entity* actor) //467340.
	{
		cell arguments[2] = {cell(actor->entityID), cell(skill)};
		JE::Script_Small* script = GetScript();
		cell result; //The script result is ignored; only Run's success byte is tested.
		if (!script || !script->Run("UseSkill", &result, 2, arguments))
			JE::LogDebugString("Couldn't execute GetSkillDifficulty on the entity %s\r\n",
				scriptName.c_str()); //Original mismatched diagnostic, not a difficulty fallback.
	}
}
