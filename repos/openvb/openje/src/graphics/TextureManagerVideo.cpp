#include "TextureManager.hpp"
#include "Texture.hpp"

DWORD Gfx_TextureManager::ReleaseVideoResources()
{
	for (std::map<DWORD, std::list<int> >::iterator pool = defaultFrames.begin();
		pool != defaultFrames.end(); ++pool)
	{
		Gfx_Texture* texture = Lookup(pool->first);
		for (std::list<int>::iterator frame = pool->second.begin();
			frame != pool->second.end(); ++frame)
			texture->ClearFrame(*frame);
	}
	return 0;
}

DWORD Gfx_TextureManager::RestoreVideoResources()
{
	for (std::map<DWORD, std::list<int> >::iterator pool = defaultFrames.begin();
		pool != defaultFrames.end(); ++pool)
	{
		Gfx_Texture* texture = Lookup(pool->first);
		if (texture->value30)
		{
			Gfx_Texture* source = Lookup(texture->value30);
			if (source)
			{
				for (std::list<int>::iterator frame = pool->second.begin();
					frame != pool->second.end(); ++frame)
				{
					const int index = *frame;
					D3DFORMAT format = index >= 0 && index < source->frameCount
						? source->frames[index].format : D3DFMT_UNKNOWN;
					UINT levels = index >= 0 && index < source->frameCount
						? source->frames[index].mipCount : 0;
					UINT height = index >= 0 && index < source->frameCount
						? source->frames[index].height : 0;
					UINT width = index >= 0 && index < source->frameCount
						? source->frames[index].width : 0;
					texture->CreateTexture(width, height, levels, format, D3DPOOL_DEFAULT, 0, index);
				}
				texture->UpdateDefaultTexture(source, 0, 0);
			}
		}
	}
	return 0;
}
