#include "TextureMask.hpp"

namespace GUI
{
	TextureMaskCache::~TextureMaskCache()
	{
		Clear();
		// The real map destructor then releases its empty tree/header storage.
	}

	void TextureMaskCache::Clear()
	{
		for (std::map<DWORD, TextureMask*>::iterator entry = entries.begin();
			entry != entries.end(); ++entry)
		{
			TextureMask* mask = entry->second;
			if (mask)
			{
				delete[] mask->bits;
				::operator delete(mask);
			}
		}
		entries.clear();
	}
}
