#include "TextureManager.hpp"
#include "../diagnostics/DebugOutput.hpp"

Gfx_Texture* Gfx_TextureManager::Lookup(DWORD handle)
{
	Gfx_Texture* result = NULL;
	std::map<DWORD, Gfx_Texture*>::iterator found = entries.find(handle);
	if (found != entries.end()) result = found->second;
	else JE::LogDebugString("TextureManager: requesting an invalid Texture Handle\n");
	return result;
}
