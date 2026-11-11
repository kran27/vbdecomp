#include "Font.hpp"
#include "FontPage.hpp"
#include "graphics/SystemInterface.hpp"
#include <cstring>

namespace GUI
{
	void FontPage::Clear()
	{
		valid = 0;
		if (texture)
		{
			g_GfxSystem->DeleteTexture(texture);
		}
		texture = 0;
		scale = 1.0f;
		std::memset(glyphs, 0, sizeof(glyphs));
	}

	Font::Font()
	{
		// flags, pointSize and charset are deliberately unwritten.
	}

	Font::~Font()
	{
		ClearPages();
	}

	void Font::ClearPages()
	{
		for (std::map<DWORD, FontPage*>::iterator page = pages.begin();
			page != pages.end(); ++page)
		{
			if (page->second)
			{
				delete page->second;
			}
		}
		pages.clear();
	}

	void Font::Configure(const std::string& face, DWORD style, LONG size, DWORD characterSet)
	{
		ClearPages();
		name = face;
		flags = style;
		pointSize = size;
		charset = characterSet;
	}

	void Font::GetGlyph(char character, FontGlyph& output)
	{
		const int code = static_cast<signed char>(character);
		const DWORD pageIndex = static_cast<DWORD>(code / 256);
		std::map<DWORD, FontPage*>& cache = pages;
		std::map<DWORD, FontPage*>::iterator found = cache.find(pageIndex);
		FontPage* page;
		if (found != cache.end())
		{
			page = found->second;
		}
		else
		{
			page = new FontPage;
			// Publication precedes GDI/texture creation and all fallbacks.
			cache[pageIndex] = page;
			if (!page->Create(name, flags, pointSize, static_cast<LONG>(pageIndex), charset))
			{
				BYTE created;
				{
					std::string fallback("symbol");
					created = page->Create(fallback, 0, pointSize, 0, 1);
				}
				if (!created)
				{
					std::string fallback("system");
					page->Create(fallback, 0, pointSize, 0, 1);
				}
			}
		}
		FontPageGlyph glyph;
		page->GetGlyph(static_cast<BYTE>(code % 256), glyph);
		output.height = glyph.height;
		output.width = glyph.width;
		output.texture = page->texture;
		std::memcpy(&output.u0, &glyph.u0, 4 * sizeof(float));
	}
}
