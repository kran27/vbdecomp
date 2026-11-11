#include <winsock2.h>
#include "EngineRegistry.hpp"
#include "GameScriptingComponent.hpp"
#include "MemorySystem.hpp"
#include "ResourcesComponent.hpp"
#include "StringsComponent.hpp"
#include "NetworkComponent.hpp"
#include "model/ModelCache.hpp"
#include "game/GameAnimationCache.hpp"
#include "game/GameWorld.hpp"

namespace F3
{
	namespace
	{
		GameComponent* CreateModelCache() //45E270, original registry index5.
		{
			return new JE::ModelCache;
		}
		GameComponent* CreateGameAnimationCache() //45E2D0, original registry index6.
		{
			return new GameAnimationCache;
		}
		GameComponent* CreateGameWorld() //45E210, original registry index7.
		{
			return new GameWorld;
		}
	}

	// Original 0x6FF6C8 order, with the actual eight factory identities.
	ComponentEntry g_Components[COMPONENT_COUNT] =
	{
		{ &CreateMemorySystem, NULL },
		{ &CreateResources, NULL },
		{ &CreateGameScripting, NULL },
		{ &CreateStrings, NULL },
		{ &CreateGameNetwork, NULL },
		{ &CreateModelCache, NULL },
		{ &CreateGameAnimationCache, NULL },
		{ &CreateGameWorld, NULL }
	};
}
