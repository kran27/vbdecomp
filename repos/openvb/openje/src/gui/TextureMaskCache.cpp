#include "TextureMask.hpp"
#include <new>

namespace GUI
{
	TextureMask* TextureMaskCache::Get(DWORD handle)
	{
		std::map<DWORD, TextureMask*>::iterator found = entries.find(handle);
		if (found != entries.end()) return found->second;
		TextureMask* mask = new TextureMask;
		if (mask)
		{
			mask->textureHandle = 0;
			mask->rowWidth = 0;
			mask->bits = NULL;
		}
		if (mask->Load(handle))
		{
			// Reentrant insertion is overwritten without deleting its pointer.
			entries.insert(std::make_pair(handle, static_cast<TextureMask*>(NULL))).first->second = mask;
			return mask;
		}
		if (mask)
		{
			delete[] mask->bits;
			::operator delete(mask);
		}
		return NULL;
	}
}
