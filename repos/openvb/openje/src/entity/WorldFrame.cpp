#include "network/EngineServer.hpp"
#include "World.hpp"
#include "EntityEnvironment.hpp"
#include "diagnostics/DebugOutput.hpp"

namespace JE
{
	void World::UpdateServer(DWORD frameClock)
	{
		if (value38 == 4) return;
		time48 += frameClock - value4C;
		if (value5C)
		{
			time48 -= value5C;
			value5C = 0;
		}
		value4C = frameClock;
		if (!IsServerFrameDue(time48)) return;
		switch (value38)
		{
		case 0:
			FatalError("Trying to update world when state is invalid");
			break;
		case 1:
			LoadMainmenu(time48);
			break;
		case 2:
			StartGame(time48);
			break;
		default:
			UpdateServerWorld(time48);
			break;
		}
		value68 = time48;
		value6C += GetUpdateInterval();
	}

	void World::StartGame(DWORD)
	{
		//5455A0 replaces the incoming argument with3 before ChangeState.
		ChangeState(3);
	}

	DWORD World::StartCutscene(DWORD id, DWORD mapID)
	{
		return cutscene154->Start(id, mapID);
	}
	DWORD World::StopCutscene()
	{
		return cutscene154->Stop();
	}

	void World::UpdateClientFrame(DWORD frameClock)
	{
		if (value38 == 4) return;
		DWORD previous = value50;
		value50 += frameClock - value54;
		if (value60)
		{
			value50 -= value60;
			value60 = 0;
		}
		value54 = frameClock;
		UpdateClient(value50 - previous);
	}
}
