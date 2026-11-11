#include "Manager.hpp"
#include "Font.hpp"

namespace GUI
{
	Font* Manager::GetFont(const std::string& name, DWORD flags, LONG pointSize, DWORD charset)
	{
		for (std::list<Font*>::iterator entry = fonts.begin(); entry != fonts.end(); ++entry)
		{
			Font* font = *entry;
			std::string storedName(font->name);
			if (storedName == name && font->flags == flags && font->pointSize == pointSize &&
				font->charset == charset)
			{
				return font;
			}
		}
		Font* font = new Font;
		font->Configure(name, flags, pointSize, charset);
		fonts.push_front(font);
		return font;
	}
}
