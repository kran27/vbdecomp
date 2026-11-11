#include "TextureManager.hpp"
#include "Texture.hpp"

BYTE Gfx_TextureManager::AcquireByName(DWORD* output, const std::string& name)
{
	if (!name.empty())
	{
		for (std::map<DWORD, Gfx_Texture*>::iterator entry = entries.begin();
			entry != entries.end(); ++entry)
		{
			bool equal = name == entry->second->GetDiagnosticName();
			// The returned temporary is destroyed before the match branch.
			if (equal)
			{
				*output = entry->first;
				++Lookup(*output)->referenceCount;
				return 1;
			}
		}
	}
	*output = 0;
	return 0;
}

DWORD Gfx_TextureManager::AllocateHandle()
{
	DWORD result = 0;
	const size_t count = freeHandles.size();
	if (count)
	{
		result = freeHandles[count - 1];
		freeHandles.pop_back();
	}
	else if (nextHandle != 0xFFFFFFFF) result = nextHandle++;
	return result;
}

void Gfx_TextureManager::RecycleHandle(DWORD* handle)
{
	if (*handle && *handle < nextHandle)
	{
		std::vector<DWORD>::iterator found = freeHandles.begin();
		while (found != freeHandles.end() && *found != *handle) ++found;
		if (found == freeHandles.end()) freeHandles.push_back(*handle);
		*handle = 0;
	}
}

BYTE Gfx_TextureManager::Unregister(DWORD handle)
{
	std::map<DWORD, Gfx_Texture*>::iterator entry = entries.find(handle);
	if (entry != entries.end())
	{
		entries.erase(entry);
		--textureCount;
		RecycleHandle(&handle);
		return 1;
	}
	return 0;
}
