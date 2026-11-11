#include "gui/Label.hpp"
#include "gui/Manager.hpp"
#include "gui/ResourceIO.hpp"
#include "gui/FileString.hpp"
#include <cstdio>

namespace GUI
{
	BYTE Label::ReadFile(FILE* file, int version) //4AA3E0; distinct from4AA640 resource input.
	{
		ClearGeometry();
		const BYTE baseRead = Window::ReadFile(file, version);
		if (!baseRead) return baseRead;
		if (std::fread(&stringReference, 4, 1, file) != 1) return 0;
		std::string resolved;
		if (g_Manager.ResolveString(stringReference, resolved)) text = resolved;
		if (!ReadFileString(fontName, file)) return 0;
		if (std::fread(&fontFlags, 4, 1, file) != 1) return 0;
		if (std::fread(&fontSize, 4, 1, file) != 1) return 0;
		if (std::fread(&fontCharset, 4, 1, file) != 1) return 0;
		if (std::fread(&textColor, 4, 1, file) != 1) return 0;
		if (std::fread(&textFrame, 1, 1, file) != 1) return 0;
		if (std::fread(&wrap, 1, 1, file) != 1) return 0;
		if (std::fread(&alignment, 4, 1, file) != 1) return 0;
		if (std::fread(&paddingLeft, 4, 1, file) != 1) return 0;
		if (std::fread(&paddingRight, 4, 1, file) != 1) return 0;
		if (std::fread(&paddingTop, 4, 1, file) != 1) return 0;
		if (std::fread(&paddingBottom, 4, 1, file) != 1) return 0;
		if (version >= 1 && std::fread(&verticalAlignment, 4, 1, file) != 1) return 0;
		RebuildText();
		return 1;
	}
	void Label::SetStringReference(DWORD value) //4AA150.
	{
		std::string resolved;
		if (g_Manager.ResolveString(value, resolved))
		{
			stringReference = value;
			text = resolved;
		}
		else
		{
			stringReference = 0;
			text.erase(text.begin(), text.end());
		}
		RebuildText();
	}

	//4AA640: actual serialized Label child of Interface::LoadResource495B50.
	//The FILE reader/writer remain separately addressed; no successful alias.
	BYTE Label::ReadResource(JE::Resources::ResourceFileHandle* file, int version)
	{
		ClearGeometry();
		if (!Window::ReadResource(file, version)
			|| !ResourceIO::Read(&stringReference, 4, file)) return 0;
		std::string resolved;
		if (g_Manager.ResolveString(stringReference, resolved)) text = resolved;
		if (!ReadResourceString(fontName, file)
			|| !ResourceIO::Read(&fontFlags, 4, file)
			|| !ResourceIO::Read(&fontSize, 4, file)
			|| !ResourceIO::Read(&fontCharset, 4, file)
			|| !ResourceIO::Read(&textColor, 4, file)
			|| !ResourceIO::Read(&textFrame, 1, file)
			|| !ResourceIO::Read(&wrap, 1, file)
			|| !ResourceIO::Read(&alignment, 4, file)
			|| !ResourceIO::Read(&paddingLeft, 4, file)
			|| !ResourceIO::Read(&paddingRight, 4, file)
			|| !ResourceIO::Read(&paddingTop, 4, file)
			|| !ResourceIO::Read(&paddingBottom, 4, file)
			|| (version >= 1 && !ResourceIO::Read(&verticalAlignment, 4, file))) return 0;
		RebuildText();
		return 1;
	}

	//4A8AB0, original virtual+14C.
	void Label::GetFont(std::string& face, DWORD* flags, LONG* pointSize, DWORD* charset)
	{
		face = fontName;
		if (flags) *flags = fontFlags;
		if (pointSize) *pointSize = fontSize;
		if (charset) *charset = fontCharset;
	}

	LONG Label::GetFontSize() const { return fontSize; } //4A7750, original+154.
}
