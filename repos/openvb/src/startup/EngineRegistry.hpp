#pragma once

#include "entity/IComponent.hpp"
#include <cstddef>

namespace F3
{
	// All eight original owners share this actual offset-zero IComponent base.

	typedef JE::IComponent GameComponent;

	typedef GameComponent* (*ComponentFactory)();
	struct ComponentEntry
	{
		ComponentFactory pFactory;
		GameComponent* pObject;
	};

	const std::size_t COMPONENT_COUNT = 8;
	extern ComponentEntry g_Components[COMPONENT_COUNT]; // original 0x6FF6C8
	void Startup(); // 0x5ACD60
	void Shutdown(); // 0x5ACDA0
	void UpdateComponents(); // 0x5ACDE0
}
