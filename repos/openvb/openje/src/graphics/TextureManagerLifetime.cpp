#include "TextureManager.hpp"
#include "Texture.hpp"

Gfx_TextureManager::Gfx_TextureManager()
{
	defaultFrameCount = 0;
	defaultTotalSize = 0;
	defaultFirstMipSize = 0;
	managedFrameCount = 0;
	managedTotalSize = 0;
	managedFirstMipSize = 0;
	systemFrameCount = 0;
	systemTotalSize = 0;
	systemFirstMipSize = 0;
	textureCount = 0;
	totalSize = 0;
	nextHandle = 1;
	freeHandles.reserve(100);
}

Gfx_TextureManager::~Gfx_TextureManager()
{
	Clear();
}

void Gfx_TextureManager::ClearPool(std::map<DWORD, std::list<int> >& pool)
{
	for (std::map<DWORD, std::list<int> >::iterator entry = pool.begin();
		entry != pool.end(); ++entry)
	{
		Gfx_Texture* texture = Lookup(entry->first);
		for (std::list<int>::iterator frame = entry->second.begin();
			frame != entry->second.end(); ++frame)
		{
			texture->ClearFrame(*frame);
		}
	}
	pool.clear();
}

void Gfx_TextureManager::ClearObjects()
{
	for (std::map<DWORD, Gfx_Texture*>::iterator entry = entries.begin();
		entry != entries.end(); ++entry)
	{
		if (entry->second)
		{
			delete entry->second;
		}
	}
	entries.clear();
}

void Gfx_TextureManager::Clear()
{
	ClearPool(defaultFrames);
	defaultFrameCount = 0;
	defaultTotalSize = 0;
	defaultFirstMipSize = 0;
	ClearPool(managedFrames);
	managedFrameCount = 0;
	managedTotalSize = 0;
	managedFirstMipSize = 0;
	ClearPool(systemFrames);
	systemFrameCount = 0;
	systemTotalSize = 0;
	systemFirstMipSize = 0;
	ClearObjects();
	textureCount = 0;
	totalSize = 0;
}
