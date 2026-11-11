#include "Video.hpp"
#include "Bink.hpp"
#include "SystemInterface.hpp"
#include "Texture.hpp"
#include "gui/Picture.hpp"

void Video::Frame() //5B5990.
{
	if (movie)
	{
		BinkDoFrame(movie);
		if (textureHandle)
		{
			Gfx_Texture* texture = g_GfxSystem->LookupTexture(textureHandle);
			if (texture)
			{
				D3DLOCKED_RECT locked;
				texture->LockRect(0, &locked, NULL, 0, 0);
				BinkCopyToBuffer(movie, locked.pBits, locked.Pitch, movie->Height, 0, 0, 0xF0000003);
				texture->UnlockRect(0);
				activeWindow->SetTexture(textureHandle, 0, movie->Width, movie->Height, 0, 0, -1.0f, -1.0f, 1, 0);
			}
		}
		if (movie->FrameNum == movie->Frames) Stop();
		else BinkNextFrame(movie);
	}
}

void Video::Update() //5B5AE0.
{
	if (movie && !BinkWait(movie)) Frame();
}

