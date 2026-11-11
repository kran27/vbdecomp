#include "Engine.hpp"
#include "Viewport.hpp"
#include "RenderManager.hpp"

void Gfx_Engine::QueueMeshInstances(Gfx_MeshInstance* instances, Gfx_Viewport* viewport,
	UINT count)
{
	if (viewport)
	{
		viewport->renderer->QueueMeshInstances(instances, count);
	}
	else
	{
		defaultViewport->renderer->QueueMeshInstances(instances, count);
	}
}
