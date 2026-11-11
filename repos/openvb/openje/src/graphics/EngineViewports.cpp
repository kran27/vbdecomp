#include "Engine.hpp"

Gfx_Viewport* Gfx_Engine::DefaultViewport()
{
	return defaultViewport;
}

Gfx_Viewport* Gfx_Engine::CurrentViewport()
{
	return currentViewport;
}

Gfx_Viewport* Gfx_Engine::PostViewport()
{
	return postViewport;
}

void Gfx_Engine::AddViewport(Gfx_Viewport* viewport)
{
	additionalViewports.push_back(viewport);
}
