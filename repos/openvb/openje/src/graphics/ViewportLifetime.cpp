#include "Viewport.hpp"
#include "RenderManager.hpp"

Gfx_Viewport::Gfx_Viewport()
	: flag1C(0), clearFlags(3), clearColor(0xFF000100), clearDepth(1.0f),
	clearStencil(0), cameraController(0), fogEnabled(0), fogColor(0),
	fogDensity(1.0f), fogStart(0.0f), fogEnd(1.0f)
{
	// All actual camera, mesh and instance members are constructed first. Neither
	// pointer is published until renderer construction returns, including NULL.
	Gfx_RenderManager* created = new Gfx_RenderManager(this);
	activeCamera = &camera;
	renderer = created;
	viewport.X = 0;
	viewport.Y = 0;
	viewport.Width = 0;
	viewport.Height = 0;
	viewport.MinZ = 0.0f;
	viewport.MaxZ = 1.0f;
	aspect = 1.0f;
}

Gfx_Viewport::~Gfx_Viewport()
{
	if (renderer)
	{
		delete renderer;
		renderer = 0;
	}
	// Instance, mesh and camera then destroy in reverse declaration order.
}
