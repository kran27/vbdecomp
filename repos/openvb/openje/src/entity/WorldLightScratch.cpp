#include <winsock2.h>
#include "World.hpp"
#include "graphics/Graphics.hpp"
#include "network/EngineClient.hpp"
#include "startup/NetworkComponent.hpp"
#include "graphics/OctreeRender.hpp"
#include <cstdlib>
#include <new>

namespace JE
{
	void World::PrepareLightIndices() //544D10, actual zero-argument client entry.
	{
		if (lightIndices)
		{
			delete[] lightIndices;
			lightIndices = NULL;
		}
		DWORD count = 0;
		Graphics* graphics = static_cast<Client*>(F3::g_pNetworkInstance->client)->child0x74;
		graphics->GetStaticLights(&count);
		lightIndices = new WORD[count];
	}

	void World::PrepareLightIndices(Octree2Gfx* renderer) //544D10, World+0C interface.
	{
		if (lightIndices)
		{
			delete[] lightIndices;
			lightIndices = NULL;
		}
		DWORD count = 0;
		// Graphics5811A0 writes count only when its actual Octree2Gfx owner exists.
		// The returned lights pointer is unused by544D10.
		if (renderer) count = renderer->lightCount;
		lightIndices = new WORD[count];
	}
}
