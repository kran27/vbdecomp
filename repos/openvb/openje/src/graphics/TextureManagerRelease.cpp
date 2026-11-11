#include "TextureManager.hpp"
#include "Texture.hpp"
#include "../JE.hpp"
#include <algorithm>

DWORD Gfx_TextureManager::Release(DWORD handle)
{
	if (!handle)
	{
		JE::LogDebugString("Gfx_Texture_Manager::DeleteTexture - NULL texture handle ignored\n");
		return 0;
	}
	Gfx_Texture* texture = Lookup(handle);
	if (!texture)
	{
		JE::LogDebugString("Gfx_Texture_Manager::DeleteTexture - Texture %08x already deleted\n", handle);
		return 0;
	}
	DWORD references = --texture->referenceCount;
	if (static_cast<LONG>(references) > 0) return references;
	for (int frame = 0; frame < texture->frameCount; ++frame)
	{
		D3DPOOL pool = frame >= 0 && frame < texture->frameCount
			? texture->frames[frame].pool : D3DPOOL_DEFAULT;
		switch (pool)
		{
		case D3DPOOL_DEFAULT:
			{
				std::map<DWORD, std::list<int> >::iterator entry = defaultFrames.find(handle);
				if (entry != defaultFrames.end())
				{
					std::list<int>::iterator found = entry->second.begin();
					while (found != entry->second.end() && *found != frame) ++found;
					if (found != entry->second.end()) entry->second.erase(found);
					if (entry->second.empty()) defaultFrames.erase(entry);
					--defaultFrameCount;
					// Native accounting intentionally reads frame zero, not the removed frame.
					defaultTotalSize -= texture->frameCount > 0 ? texture->frames[0].totalSize : 0;
					defaultFirstMipSize -= texture->frameCount > 0 ? texture->frames[0].firstMipSize : 0;
				}
			}
			if (texture->value30 && Lookup(texture->value30)) Release(texture->value30);
			break;
		case D3DPOOL_MANAGED:
			{
				std::map<DWORD, std::list<int> >::iterator entry = managedFrames.find(handle);
				if (entry != managedFrames.end())
				{
					std::list<int>::iterator found = entry->second.begin();
					while (found != entry->second.end() && *found != frame) ++found;
					if (found != entry->second.end()) entry->second.erase(found);
					if (entry->second.empty()) managedFrames.erase(entry);
					--managedFrameCount;
					// Native accounting intentionally reads frame zero, not the removed frame.
					managedTotalSize -= texture->frameCount > 0 ? texture->frames[0].totalSize : 0;
					managedFirstMipSize -= texture->frameCount > 0 ? texture->frames[0].firstMipSize : 0;
				}
			}
			break;
		case D3DPOOL_SYSTEMMEM:
			{
				std::map<DWORD, std::list<int> >::iterator entry = systemFrames.find(handle);
				if (entry != systemFrames.end())
				{
					std::list<int>::iterator found = entry->second.begin();
					while (found != entry->second.end() && *found != frame) ++found;
					if (found != entry->second.end()) entry->second.erase(found);
					if (entry->second.empty()) systemFrames.erase(entry);
					--systemFrameCount;
					// Native accounting intentionally reads frame zero, not the removed frame.
					systemTotalSize -= texture->frameCount > 0 ? texture->frames[0].totalSize : 0;
					systemFirstMipSize -= texture->frameCount > 0 ? texture->frames[0].firstMipSize : 0;
				}
			}
			break;
		}
	}
	Unregister(handle);
	delete texture;
	RecycleHandle(&handle);
	totalSize = defaultTotalSize + managedTotalSize + systemTotalSize;
	return 0;
}
