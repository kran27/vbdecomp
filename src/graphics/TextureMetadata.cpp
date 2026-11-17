#include "Texture.hpp"
#include "D3DState.hpp"

namespace
{
    static DWORD EstimateSurfaceSize(const D3DSURFACE_DESC& desc)
    {
        DWORD bytesPerPixel = 0;

        switch (desc.Format)
        {
        case D3DFMT_A8R8G8B8:
        case D3DFMT_X8R8G8B8:
        case D3DFMT_D32:
        case D3DFMT_D24S8:
        case D3DFMT_D24X8:
        case D3DFMT_D24X4S4:
            bytesPerPixel = 4;
            break;

        case D3DFMT_R5G6B5:
        case D3DFMT_X1R5G5B5:
        case D3DFMT_A1R5G5B5:
        case D3DFMT_A4R4G4B4:
        case D3DFMT_D16:
        case D3DFMT_D16_LOCKABLE:
        case D3DFMT_D15S1:
            bytesPerPixel = 2;
            break;

        default:
            return 0;
        }

        return desc.Width * desc.Height * bytesPerPixel;
    }
} // namespace

HRESULT Gfx_Texture::Bind(UINT stage, int frame)
{
    if (frame < 0 || frame >= frameCount)
    {
        return D3DERR_INVALIDCALL;
    }
    return g_D3D.SetTexture(stage, frames[frame].texture);
}

void Gfx_Texture::UpdateMetadata()
{
    for (int frame = 0; frame < frameCount; ++frame)
    {
        if (frames[frame].texture)
        {
            IDirect3DTexture9*& texture = frames[frame].texture;
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
                        g_D3D.DescribeTextureFormat(frames[frame].format, &frames[frame].bitsPerPixelWord, NULL);
                        frames[frame].firstMipSize = EstimateSurfaceSize(description);
                    }
                    frames[frame].totalSize += EstimateSurfaceSize(description);
                }
            }
        }
    }
}
