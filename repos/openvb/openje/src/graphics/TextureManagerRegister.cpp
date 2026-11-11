#include "TextureManager.hpp"
#include "Texture.hpp"

HRESULT Gfx_TextureManager::Register(DWORD* output, Gfx_Texture* texture)
{
	if (texture)
	{
		++textureCount;
		*output = AllocateHandle();
		entries.insert(std::make_pair(*output, texture));
		for (int index = 0; index < texture->frameCount; ++index)
		{
			D3DPOOL pool = index >= 0 && index < texture->frameCount
				? texture->frames[index].pool : D3DPOOL_DEFAULT;
			switch (pool)
			{
			case D3DPOOL_DEFAULT:
				{
					std::map<DWORD, std::list<int> >::iterator entry = defaultFrames.find(*output);
					if (entry != defaultFrames.end()) entry->second.push_back(index);
					else
					{
						std::list<int> frames;
						frames.push_back(index);
						defaultFrames.insert(std::map<DWORD, std::list<int> >::value_type(*output, frames));
					}
				}
				defaultTotalSize += index >= 0 && index < texture->frameCount
					? texture->frames[index].totalSize : 0;
				defaultFirstMipSize += index >= 0 && index < texture->frameCount
					? texture->frames[index].firstMipSize : 0;
				++defaultFrameCount;
				if (!texture->value30)
				{
					DWORD shadow;
					Register(&shadow, texture->CloneDefaultToSystem());
					texture->value30 = shadow;
				}
				break;
			case D3DPOOL_MANAGED:
				{
					std::map<DWORD, std::list<int> >::iterator entry = managedFrames.find(*output);
					if (entry != managedFrames.end()) entry->second.push_back(index);
					else
					{
						std::list<int> frames;
						frames.push_back(index);
						managedFrames.insert(std::map<DWORD, std::list<int> >::value_type(*output, frames));
					}
				}
				managedTotalSize += index >= 0 && index < texture->frameCount
					? texture->frames[index].totalSize : 0;
				managedFirstMipSize += index >= 0 && index < texture->frameCount
					? texture->frames[index].firstMipSize : 0;
				++managedFrameCount;
				break;
			case D3DPOOL_SYSTEMMEM:
				{
					std::map<DWORD, std::list<int> >::iterator entry = systemFrames.find(*output);
					if (entry != systemFrames.end()) entry->second.push_back(index);
					else
					{
						std::list<int> frames;
						frames.push_back(index);
						systemFrames.insert(std::map<DWORD, std::list<int> >::value_type(*output, frames));
					}
				}
				systemTotalSize += index >= 0 && index < texture->frameCount
					? texture->frames[index].totalSize : 0;
				systemFirstMipSize += index >= 0 && index < texture->frameCount
					? texture->frames[index].firstMipSize : 0;
				++systemFrameCount;
				break;
			}
		}
		totalSize = defaultTotalSize + managedTotalSize + systemTotalSize;
	}
	// The apparent guard above does not make a NULL texture a successful input.
	texture->value20 = *output;
	return S_OK;
}
