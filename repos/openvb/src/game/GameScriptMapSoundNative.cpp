#include "GameScriptNatives.hpp"
#include "entity/Map.hpp"
#include "entity/MapSound.hpp"
#include "entity/World.hpp"
#include "scripting/ScriptEnvironment.hpp"

namespace F3
{
	cell AMX_NATIVE_CALL NativeMapSoundSetActive(AMX* vm, cell* parameters) //5C9C50.
	{
		if (!VerifyScriptParameters(vm, parameters, 3, "MapSoundSetActive")) return 0;
		const DWORD mapID = DWORD(parameters[1]);
		char name[260];
		if (!JE::ScriptServices::ReadNativeString(vm, parameters[2], name, sizeof(name)))
		{
			LogScriptError(vm, "MapSoundSetActive() is unable to retrieve sound instance name.");
			return 0;
		}
		JE::Map* map = JE::GetWorldInstance()->GetMap(mapID);
		if (!map)
		{
			LogScriptError(vm, "MapSoundSetActive() is unable to find the specified map.");
			return 0;
		}
		std::list<JE::MapSound*> sounds;
		map->FindSounds(std::string(name), sounds);
		if (sounds.empty())
		{
			//The original diagnostic says effect here, despite the sound lookup.
			LogScriptError(vm, "MapSoundSetActive() is unable to find the specified effect.");
			return 0;
		}
		for (std::list<JE::MapSound*>::iterator i = sounds.begin(); i != sounds.end(); ++i)
		{
			cell enabled = parameters[3];
			JE::MapSound* sound = *i;
			sound->SetEnabled(enabled != 0);
		}
		return 1; //Explicit MOV EAX,1 at5C9DC2, after the owned list cleanup.
	}
}
