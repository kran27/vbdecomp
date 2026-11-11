#include "Texture.hpp"
#include <dxerr8.h>
#include "diagnostics/DebugOutput.hpp"
#include <string.h>

void Gfx_Texture::Clear() //535170, distinct from releasing a frame535620.
{
	for (int frame = 0; frame < frameCount; ++frame)
	{
		D3DLOCKED_RECT lock;
		HRESULT result = frames[frame].texture->LockRect(0, &lock, NULL, 0);
		if (SUCCEEDED(result))
		{
			DWORD bytes = frames[frame].bitsPerPixelWord >> 3;
			BYTE* destination = static_cast<BYTE*>(lock.pBits);
			for (DWORD row = 0; row < frames[frame].height; ++row)
			{
				memset(destination, 0, bytes * frames[frame].width);
				destination += lock.Pitch;
			}
			(frames + frame)->texture->UnlockRect(0);
		}
		else
		{
			const char* textureName = name.c_str();
			const char* error = DXGetErrorDescription8A(result);
			JE::LogDebugString("Gfx_Texture::Clear - LockRect() failed: %s (%x %s)\n", textureName, result, error);
		}
	}
}
