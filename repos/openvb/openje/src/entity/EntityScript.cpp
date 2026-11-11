#include "startup/NetworkComponent.hpp"
#include "Entity.hpp"
#include "Trigger.hpp"
#include "EntityEnvironment.hpp"
#include "World.hpp"
#include "scripting/ScriptSmall.hpp"
#include "model/CommonException.hpp"
#include "diagnostics/DebugOutput.hpp"
#include <stdexcept>

namespace JE
{
	const char* Entity::GetScriptName() const
	{
		return scriptName.c_str();
	}

	void Entity::SetScriptName(const char* name)
	{
		scriptName.assign(name);
	}

	BYTE ScriptTrigger::RunScript(const char* name,cell* result,int count,cell* arguments)
	{
		if (!script) return 0;
		return script->Run(name,result,count,arguments);
	}

	Script_Small* Entity::GetScript()
	{
		if (scriptDisabled) return NULL;
		if (script) return script;
		if (scriptName.empty())
		{
			Script_Small* current = script;
			scriptDisabled = 1;
			return current;
		}
		IWorldServer* server = GetWorldServerInstance();
		Script_Small* created = server->NewScript(entityID);
		script = created;
		if (created->Load(scriptName.c_str(), 1))
		{
			if (!variables.empty()) script->PreLoad(variables);
			return script;
		}
		LogDebugString("Entity::GetScript() Unable to load script: %s\n", scriptName.c_str());
		Script_Small* failed = script;
		scriptDisabled = 1;
		delete failed;
		script = NULL;
		return script;
	}

	BYTE Entity::RunScript(const char* name, cell* result, int count, cell* arguments)
	{
		if (!(F3::g_pNetworkInstance->GetMode() & 1))
			throw ::Common::Exception("Entity::RunScript() method may only be called by the server.");
		if (!GetScript()) return 0;
		return script->Run(name, result, count, arguments); // Reload the published member,54B02B.
	}
}
