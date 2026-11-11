#pragma once

#include <windows.h>
#include <string>

namespace GUI
{
	struct FontPageGlyph
	{
		DWORD width, height;
		float u0, v0, u1, v1;
	};

	class FontPage
	{
	public:
		FontPage() : valid(0), texture(0), scale(1.0f) {} //51ECD0; glyphs unwritten.
		virtual ~FontPage(); // 51E940 deleting wrapper calls Clear.
		void Clear(); // 5382A0.
		BYTE Create(const std::string& name, DWORD flags, LONG pointSize,
			LONG firstCharacter, DWORD charset); // 5382E0.
		void GetGlyph(BYTE character, FontPageGlyph& output) const; // 538280.

		BYTE valid; // +04.
		DWORD texture; // +08; owns a Graphics texture handle.
		float scale; // +0C.
		FontPageGlyph glyphs[256]; // +10; initialized by Clear before Create.
	};
}
