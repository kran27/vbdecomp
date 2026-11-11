#include "Texture.hpp"
#include "D3DState.hpp"

HRESULT Gfx_Texture::Bind(UINT stage, int frame)
{
	if (frame < 0 || frame >= frameCount) return D3DERR_INVALIDCALL;
	return g_D3D.SetTexture(stage, frames[frame].texture);
}

void Gfx_Texture::UpdateMetadata()
{
	for (int frame = 0; frame < frameCount; ++frame)
	{
		if (frames[frame].texture)
		{
			IDirect3DTexture8*& texture = frames[frame].texture;
			frames[frame].mipCount = texture->GetLevelCount();
			frames[frame].totalSize = 0;
			for (UINT level = 0; level < frames[frame].mipCount; ++level)
			{
				D3DSURFACE_DESC description;
				if (SUCCEEDED(frames[frame].texture->GetLevelDesc(level, &description)))
				{
					if (level == 0)
					{
						frames[frame].width = description.Width;
						frames[frame].height = description.Height;
						frames[frame].format = description.Format;
						frames[frame].pool = description.Pool;
						g_D3D.DescribeTextureFormat(frames[frame].format,
							&frames[frame].bitsPerPixelWord, NULL);
						frames[frame].firstMipSize = description.Size;
					}
					frames[frame].totalSize += description.Size;
				}
			}
		}
	}
}
