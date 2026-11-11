#include "FontPage.hpp"
#include "graphics/SystemInterface.hpp"
#include "graphics/Texture.hpp"
#include <cstdlib>
#include <cstring>

namespace GUI
{
	FontPage::~FontPage()
	{
		Clear();
	}

	void FontPage::GetGlyph(BYTE character, FontPageGlyph& output) const
	{
		output = glyphs[character];
	}

	// The pinned DX8.1 CD3DFont sample uses this GDI atlas mechanism too.
	// This is the game's distinct page policy: 256 glyphs, 32-bit alpha, charset,
	// four style bits, Fallouty adjustment and Graphics texture-handle ownership.
	BYTE FontPage::Create(const std::string& name, DWORD flags, LONG pointSize,
		LONG firstCharacter, DWORD charset)
	{
		Clear();
		if (pointSize > 40)
		{
			return 0;
		}
		LONG width;
		if (pointSize > 22) width = 1024;
		else if (pointSize > 10) width = 512;
		else width = 256;
		LONG height = width;
		scale = 1.0f;
		DWORD maximumWidth, maximumHeight;
		g_GfxSystem->MaximumTextureSize(maximumWidth, maximumHeight);
		if (static_cast<DWORD>(width) > maximumWidth)
		{
			scale = static_cast<float>(static_cast<double>(maximumWidth) / width);
			width = static_cast<LONG>(maximumWidth);
			height = static_cast<LONG>(maximumWidth);
		}

		BITMAPINFO bitmapInfo;
		std::memset(&bitmapInfo.bmiHeader, 0, sizeof(BITMAPINFOHEADER));
		bitmapInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
		bitmapInfo.bmiHeader.biWidth = width;
		bitmapInfo.bmiHeader.biHeight = -height;
		bitmapInfo.bmiHeader.biPlanes = 1;
		bitmapInfo.bmiHeader.biCompression = BI_RGB;
		bitmapInfo.bmiHeader.biBitCount = 32;
		HDC dc = CreateCompatibleDC(NULL);
		if (!dc)
		{
			return 0;
		}
		void* bitmapBits;
		HBITMAP bitmap = CreateDIBSection(dc, &bitmapInfo, DIB_RGB_COLORS, &bitmapBits, NULL, 0);
		if (!bitmap)
		{
			DeleteDC(dc);
			return 0;
		}
		if (!SetMapMode(dc, MM_TEXT))
		{
			DeleteObject(bitmap);
			DeleteDC(dc);
			return 0;
		}
		const LONG fontHeight = -MulDiv(pointSize, static_cast<LONG>(scale * 96.0), 72);
		HFONT font = CreateFontA(fontHeight, 0, 0, 0, flags & 1 ? FW_BOLD : FW_NORMAL,
			(flags >> 1) & 1, (flags >> 2) & 1, (flags >> 3) & 1, charset,
			OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, PROOF_QUALITY, VARIABLE_PITCH, name.c_str());
		if (!font)
		{
			DeleteObject(bitmap);
			DeleteDC(dc);
			return 0;
		}
		SelectObject(dc, bitmap);
		SelectObject(dc, font);
		SetTextColor(dc, RGB(255, 255, 255));
		SetBkColor(dc, RGB(0, 0, 0));
		SetTextAlign(dc, 0);
		LONG x = 0;
		LONG y = 0;
		LONG rowHeight = 0;
		char text[2] = "x";
		SIZE extent;
		for (LONG character = 0; character < 256; ++character)
		{
			text[0] = static_cast<char>(character + firstCharacter);
			GetTextExtentPoint32A(dc, text, 1, &extent);
			if (!_stricmp(name.c_str(), "fallouty") && character == 32)
			{
				extent.cx += 3;
			}
			if (extent.cy > rowHeight)
			{
				rowHeight = extent.cy;
			}
			if (extent.cx + x + 2 > width)
			{
				x = 0;
				y += rowHeight + 1;
				rowHeight = 0;
			}
			if (!ExtTextOutA(dc, x, y, ETO_OPAQUE, NULL, text, 1, NULL))
			{
				DeleteObject(font);
				DeleteObject(bitmap);
				DeleteDC(dc);
				return 0;
			}
			FontPageGlyph& glyph = glyphs[character];
			glyph.width = static_cast<DWORD>(static_cast<double>(extent.cx) / scale);
			glyph.height = static_cast<DWORD>(static_cast<double>(extent.cy) / scale);
			glyph.u0 = static_cast<float>(static_cast<double>(x) / width);
			glyph.v0 = static_cast<float>(static_cast<double>(y) / height);
			glyph.u1 = static_cast<float>(static_cast<double>(x + extent.cx) / width);
			glyph.v1 = static_cast<float>(static_cast<double>(y + extent.cy) / height);
			x += extent.cx + 2;
		}

		std::string textureName(name);
		char number[32];
		textureName += _itoa(static_cast<int>(flags), number, 10);
		textureName += _itoa(pointSize, number, 10);
		textureName += _itoa(firstCharacter, number, 10);
		textureName += _itoa(static_cast<int>(charset), number, 10);
		Gfx_Texture image(1);
		if (FAILED(image.CreateSystemTexture(textureName, width, height, 1, D3DFMT_A8R8G8B8, 0)))
		{
			DeleteObject(font);
			DeleteObject(bitmap);
			DeleteDC(dc);
			return 0;
		}
		D3DLOCKED_RECT locked;
		image.LockRect(0, &locked, NULL, 0, 0);
		BYTE* destinationRow = static_cast<BYTE*>(locked.pBits);
		const DWORD* sourceRow = static_cast<const DWORD*>(bitmapBits);
		for (LONG row = 0; row < height; ++row)
		{
			DWORD* destination = reinterpret_cast<DWORD*>(destinationRow);
			for (LONG column = 0; column < width; ++column)
			{
				const DWORD intensity = sourceRow[column] & 0xFF;
				destination[column] = intensity ? (intensity << 24) | 0xFFFFFF : 0;
			}
			destinationRow += locked.Pitch;
			sourceRow += width;
		}
		image.UnlockRect(0);
		if (FAILED(g_GfxSystem->AddTexture(&texture, &image)))
		{
			DeleteObject(font);
			DeleteObject(bitmap);
			DeleteDC(dc);
			return 0;
		}
		DeleteObject(font);
		DeleteObject(bitmap);
		DeleteDC(dc);
		valid = 1;
		return 1;
	}
}
