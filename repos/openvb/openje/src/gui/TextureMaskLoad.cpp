#include "TextureMask.hpp"
#include "graphics/Texture.hpp"
#include "graphics/TextureManager.hpp"
#include <cstring>
#include <new>

namespace GUI
{
	BYTE TextureMask::Load(DWORD handle)
	{
		LONG height = 0;
		BYTE* oldBits = bits;
		textureHandle = 0;
		rowWidth = 0;
		delete[] oldBits;
		bits = NULL;
		Gfx_Texture* texture = g_TextureManager->Lookup(handle);
		if (!texture) return 0;
		int count = texture->frameCount;
		LONG width = count > 0 ? static_cast<LONG>(texture->frames[0].width) : 0;
		if (count > 0) height = static_cast<LONG>(texture->frames[0].height);
		LONG pixels = static_cast<LONG>(static_cast<DWORD>(width) * static_cast<DWORD>(height));
		LONG byteCount = pixels / 8;
		if (pixels % 8) ++byteCount;
		D3DLOCKED_RECT locked; // Only LockRect output initializes this storage.
		if (FAILED(texture->LockRect(0, &locked, NULL, D3DLOCK_READONLY | D3DLOCK_NO_DIRTY_UPDATE, 0)))
			return 0;
		bits = new BYTE[static_cast<size_t>(byteCount)];
		std::memset(bits, 0xFF, static_cast<size_t>(byteCount));
		DWORD byteIndex = 0;
		BYTE bit = 0x80;
		if (texture->frameCount > 0)
		{
			D3DFORMAT format = texture->frames[0].format;
			BYTE* row = static_cast<BYTE*>(locked.pBits);
			if (format == D3DFMT_A8R8G8B8)
			{
				if (height > 0) do
				{
					const DWORD* input = reinterpret_cast<const DWORD*>(row);
					for (LONG x = 0; x < width; ++x)
					{
						if (!(input[x] & 0xFF000000)) bits[byteIndex] &= static_cast<BYTE>(~bit);
						bit >>= 1;
						if (!bit) { ++byteIndex; bit = 0x80; }
					}
					row += (locked.Pitch / 4) * 4;
				} while (--height);
			}
			else
			{
				WORD alpha;
				if (format == D3DFMT_A1R5G5B5) alpha = 0x8000;
				else if (format == D3DFMT_A4R4G4B4) alpha = 0xF000;
				else if (format == D3DFMT_A8R3G3B2) alpha = 0xFF00;
				else
				{
					if (format == D3DFMT_A8)
					{
						if (height > 0) do
						{
							for (LONG x = 0; x < width; ++x)
							{
								if (!row[x]) bits[byteIndex] &= static_cast<BYTE>(~bit);
								bit >>= 1;
								if (!bit) { ++byteIndex; bit = 0x80; }
							}
							row += locked.Pitch;
						} while (--height);
					}
					goto loaded;
				}
				if (height > 0) do
				{
					const WORD* input = reinterpret_cast<const WORD*>(row);
					for (LONG x = 0; x < width; ++x)
					{
						if (!(input[x] & alpha)) bits[byteIndex] &= static_cast<BYTE>(~bit);
						bit >>= 1;
						if (!bit) { ++byteIndex; bit = 0x80; }
					}
					row += (locked.Pitch / 2) * 2;
				} while (--height);
			}
		}
	loaded:
		texture->UnlockRect(0); // Original ignores this HRESULT.
		textureHandle = handle;
		rowWidth = width;
		return 1;
	}
}
