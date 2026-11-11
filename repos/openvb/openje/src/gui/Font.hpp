#pragma once

#include <windows.h>
#include <map>
#include <string>

namespace GUI
{
	class FontPage;

	struct FontGlyph
	{
		DWORD width, height;
		DWORD texture;
		float u0, v0, u1, v1;
	};

	class Font
	{
	public:
		Font(); // 4B5500; only string and map acquire initial state.
		virtual ~Font(); // 4B55A0; sole virtual operation.
		void Configure(const std::string& face, DWORD style, LONG size, DWORD characterSet); // 51EBD0.
		void ClearPages(); // 51E970.
		void GetGlyph(char character, FontGlyph& output); // 51ECD0.

		std::string name; // +04.
		DWORD flags; // +20; unwritten until Configure.
		LONG pointSize; // +24; unwritten until Configure.
		DWORD charset; // +28; unwritten until Configure.
		std::map<DWORD, FontPage*> pages; // +2C; owns page pointers.
	};
}
