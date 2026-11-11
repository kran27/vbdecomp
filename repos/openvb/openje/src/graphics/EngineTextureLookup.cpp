#include "Engine.hpp"
#include "TextureManager.hpp"

// 0x5C0930: the Engine receiver is unused; reload the published manager each call.
Gfx_Texture* Gfx_Engine::LookupTexture(DWORD handle)
{
	return g_TextureManager->Lookup(handle);
}
