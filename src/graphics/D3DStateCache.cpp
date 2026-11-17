#include "D3DState.hpp"
#include "DX9SamplerStates.hpp"
#include <cstring>

namespace
{
    // Actual immutable SDK state-ID tables at0x673BC0/0x673CF0.
    const D3DRENDERSTATETYPE RenderStateIds[] = { D3DRS_ZENABLE,
                                                  D3DRS_FILLMODE,
                                                  D3DRS_SHADEMODE,
                                                  D3DRS_ZWRITEENABLE,
                                                  D3DRS_ALPHATESTENABLE,
                                                  D3DRS_LASTPIXEL,
                                                  D3DRS_SRCBLEND,
                                                  D3DRS_DESTBLEND,
                                                  D3DRS_CULLMODE,
                                                  D3DRS_ZFUNC,
                                                  D3DRS_ALPHAREF,
                                                  D3DRS_ALPHAFUNC,
                                                  D3DRS_DITHERENABLE,
                                                  D3DRS_ALPHABLENDENABLE,
                                                  D3DRS_FOGENABLE,
                                                  D3DRS_SPECULARENABLE,
                                                  D3DRS_FOGCOLOR,
                                                  D3DRS_FOGTABLEMODE,
                                                  D3DRS_FOGSTART,
                                                  D3DRS_FOGEND,
                                                  D3DRS_FOGDENSITY,
                                                  D3DRS_RANGEFOGENABLE,
                                                  D3DRS_STENCILENABLE,
                                                  D3DRS_STENCILFAIL,
                                                  D3DRS_STENCILZFAIL,
                                                  D3DRS_STENCILPASS,
                                                  D3DRS_STENCILFUNC,
                                                  D3DRS_STENCILREF,
                                                  D3DRS_STENCILMASK,
                                                  D3DRS_STENCILWRITEMASK,
                                                  D3DRS_TEXTUREFACTOR,
                                                  D3DRS_WRAP0,
                                                  D3DRS_WRAP1,
                                                  D3DRS_WRAP2,
                                                  D3DRS_WRAP3,
                                                  D3DRS_WRAP4,
                                                  D3DRS_WRAP5,
                                                  D3DRS_WRAP6,
                                                  D3DRS_WRAP7,
                                                  D3DRS_CLIPPING,
                                                  D3DRS_LIGHTING,
                                                  D3DRS_AMBIENT,
                                                  D3DRS_FOGVERTEXMODE,
                                                  D3DRS_COLORVERTEX,
                                                  D3DRS_LOCALVIEWER,
                                                  D3DRS_NORMALIZENORMALS,
                                                  D3DRS_DIFFUSEMATERIALSOURCE,
                                                  D3DRS_SPECULARMATERIALSOURCE,
                                                  D3DRS_AMBIENTMATERIALSOURCE,
                                                  D3DRS_EMISSIVEMATERIALSOURCE,
                                                  D3DRS_VERTEXBLEND,
                                                  D3DRS_CLIPPLANEENABLE,
                                                  D3DRS_POINTSIZE,
                                                  D3DRS_POINTSIZE_MIN,
                                                  D3DRS_POINTSPRITEENABLE,
                                                  D3DRS_POINTSCALEENABLE,
                                                  D3DRS_POINTSCALE_A,
                                                  D3DRS_POINTSCALE_B,
                                                  D3DRS_POINTSCALE_C,
                                                  D3DRS_MULTISAMPLEANTIALIAS,
                                                  D3DRS_MULTISAMPLEMASK,
                                                  D3DRS_PATCHEDGESTYLE,
                                                  D3DRS_DEBUGMONITORTOKEN,
                                                  D3DRS_POINTSIZE_MAX,
                                                  D3DRS_INDEXEDVERTEXBLENDENABLE,
                                                  D3DRS_COLORWRITEENABLE,
                                                  D3DRS_TWEENFACTOR,
                                                  D3DRS_BLENDOP };

    const D3DTEXTURESTAGESTATETYPE TextureStateIds[] = { D3DTSS_COLOROP,        D3DTSS_COLORARG1,
                                                         D3DTSS_COLORARG2,      D3DTSS_ALPHAOP,
                                                         D3DTSS_ALPHAARG1,      D3DTSS_ALPHAARG2,
                                                         D3DTSS_BUMPENVMAT00,   D3DTSS_BUMPENVMAT01,
                                                         D3DTSS_BUMPENVMAT10,   D3DTSS_BUMPENVMAT11,
                                                         D3DTSS_TEXCOORDINDEX,  D3DTSS_BUMPENVLSCALE,
                                                         D3DTSS_BUMPENVLOFFSET, D3DTSS_TEXTURETRANSFORMFLAGS,
                                                         D3DTSS_COLORARG0,      D3DTSS_ALPHAARG0,
                                                         D3DTSS_RESULTARG };
} // namespace

HRESULT Gfx_D3D::InitializeDefaultRenderStates() // 0x502CB0.
{
    std::memset(defaultRenderStates, 0, sizeof(defaultRenderStates));
    std::memset(g_D3DRenderResetBytes, 0, sizeof(g_D3DRenderResetBytes));
    defaultRenderStates[D3DRS_FOGSTART] = 0;
    defaultRenderStates[D3DRS_ALPHAFUNC] = D3DCMP_ALWAYS;
    defaultRenderStates[D3DRS_STENCILFUNC] = D3DCMP_ALWAYS;
    defaultRenderStates[D3DRS_FOGEND] = 0x3F800000;
    defaultRenderStates[D3DRS_FOGDENSITY] = 0x3F800000;
    Gfx_D3DDeviceInfo* capturedDevice = currentDevice;
    defaultRenderStates[D3DRS_ZENABLE] = D3DZB_TRUE;
    defaultRenderStates[D3DRS_FILLMODE] = D3DFILL_SOLID;
    defaultRenderStates[D3DRS_SHADEMODE] = D3DSHADE_GOURAUD;
    defaultRenderStates[D3DRS_ZWRITEENABLE] = 1;
    defaultRenderStates[D3DRS_ALPHATESTENABLE] = 0;
    defaultRenderStates[D3DRS_LASTPIXEL] = 1;
    defaultRenderStates[D3DRS_SRCBLEND] = D3DBLEND_ONE;
    defaultRenderStates[D3DRS_DESTBLEND] = D3DBLEND_ZERO;
    defaultRenderStates[D3DRS_CULLMODE] = D3DCULL_CCW;
    defaultRenderStates[D3DRS_ZFUNC] = D3DCMP_LESSEQUAL;
    defaultRenderStates[D3DRS_ALPHAREF] = 0;
    defaultRenderStates[D3DRS_DITHERENABLE] = 0;
    defaultRenderStates[D3DRS_ALPHABLENDENABLE] = 0;
    defaultRenderStates[D3DRS_FOGENABLE] = 0;
    defaultRenderStates[D3DRS_SPECULARENABLE] = 0;
    defaultRenderStates[D3DRS_FOGCOLOR] = 0;
    defaultRenderStates[D3DRS_FOGTABLEMODE] = D3DFOG_NONE;
    defaultRenderStates[D3DRS_RANGEFOGENABLE] = 0;
    defaultRenderStates[D3DRS_STENCILENABLE] = 0;
    defaultRenderStates[D3DRS_STENCILFAIL] = D3DSTENCILOP_KEEP;
    defaultRenderStates[D3DRS_STENCILZFAIL] = D3DSTENCILOP_KEEP;
    defaultRenderStates[D3DRS_STENCILPASS] = D3DSTENCILOP_KEEP;
    defaultRenderStates[D3DRS_STENCILREF] = 0;
    defaultRenderStates[D3DRS_STENCILMASK] = 0xFFFFFFFF;
    defaultRenderStates[D3DRS_STENCILWRITEMASK] = 0xFFFFFFFF;
    defaultRenderStates[D3DRS_TEXTUREFACTOR] = 0;
    defaultRenderStates[D3DRS_WRAP0] = 0;
    defaultRenderStates[D3DRS_WRAP1] = 0;
    defaultRenderStates[D3DRS_WRAP2] = 0;
    defaultRenderStates[D3DRS_WRAP3] = 0;
    defaultRenderStates[D3DRS_WRAP4] = 0;
    defaultRenderStates[D3DRS_WRAP5] = 0;
    defaultRenderStates[D3DRS_WRAP6] = 0;
    defaultRenderStates[D3DRS_WRAP7] = 0;
    defaultRenderStates[D3DRS_CLIPPING] = 1;
    defaultRenderStates[D3DRS_LIGHTING] = 1;
    defaultRenderStates[D3DRS_AMBIENT] = 0;
    defaultRenderStates[D3DRS_FOGVERTEXMODE] = D3DFOG_NONE;
    defaultRenderStates[D3DRS_COLORVERTEX] = 1;
    defaultRenderStates[D3DRS_LOCALVIEWER] = 1;
    defaultRenderStates[D3DRS_NORMALIZENORMALS] = 0;
    defaultRenderStates[D3DRS_DIFFUSEMATERIALSOURCE] = D3DMCS_COLOR1;
    defaultRenderStates[D3DRS_SPECULARMATERIALSOURCE] = D3DMCS_COLOR2;
    defaultRenderStates[D3DRS_AMBIENTMATERIALSOURCE] = D3DMCS_COLOR2;
    defaultRenderStates[D3DRS_EMISSIVEMATERIALSOURCE] = D3DMCS_MATERIAL;
    defaultRenderStates[D3DRS_VERTEXBLEND] = D3DVBF_DISABLE;
    defaultRenderStates[D3DRS_CLIPPLANEENABLE] = 0;
    defaultRenderStates[D3DRS_POINTSIZE] = 0x3F800000;
    defaultRenderStates[D3DRS_POINTSIZE_MIN] = 0x3F800000;
    defaultRenderStates[D3DRS_POINTSCALE_A] = 0x3F800000;
    defaultRenderStates[D3DRS_POINTSCALE_B] = 0;
    defaultRenderStates[D3DRS_POINTSCALE_C] = 0;
    defaultRenderStates[D3DRS_POINTSPRITEENABLE] = 0;
    defaultRenderStates[D3DRS_POINTSCALEENABLE] = 0;
    defaultRenderStates[D3DRS_MULTISAMPLEMASK] = 0xFFFFFFFF;
    defaultRenderStates[D3DRS_PATCHEDGESTYLE] = D3DPATCHEDGE_DISCRETE;
    defaultRenderStates[D3DRS_DEBUGMONITORTOKEN] = D3DDMT_ENABLE;
    defaultRenderStates[D3DRS_MULTISAMPLEANTIALIAS] = 1;
    // The original uses integer loads/stores for the caps float, including NaN bits.
    std::memcpy(&defaultRenderStates[D3DRS_POINTSIZE_MAX], &capturedDevice->caps.MaxPointSize, sizeof(DWORD));
    defaultRenderStates[D3DRS_INDEXEDVERTEXBLENDENABLE] = 0;
    defaultRenderStates[D3DRS_COLORWRITEENABLE] = 15;
    defaultRenderStates[D3DRS_TWEENFACTOR] = 0;
    defaultRenderStates[D3DRS_BLENDOP] = D3DBLENDOP_ADD;
    return 0;
}

DWORD Gfx_D3D::InitializeDefaultTextureStates() // 0x502F40.
{
    g_D3DTextureResetValue0 = 0;
    g_D3DTextureResetValue1 = 0;
    std::memset(g_D3DTextureResetBytes, 0, sizeof(g_D3DTextureResetBytes));
    DWORD stage = 0;
    while (stage < textureStageCount)
    {
        std::memset(&defaultTextureStates[stage], 0, sizeof(Gfx_D3DTextureState));
        defaultTextureStates[stage].values[D3DTSS_COLOROP] = stage ? D3DTOP_DISABLE : D3DTOP_MODULATE;
        defaultTextureStates[stage].values[D3DTSS_COLORARG1] = D3DTA_TEXTURE;
        defaultTextureStates[stage].values[D3DTSS_COLORARG2] = D3DTA_CURRENT;
        defaultTextureStates[stage].values[D3DTSS_ALPHAOP] = stage ? D3DTOP_DISABLE : D3DTOP_SELECTARG1;
        defaultTextureStates[stage].values[D3DTSS_ALPHAARG1] = D3DTA_TEXTURE;
        defaultTextureStates[stage].values[D3DTSS_ALPHAARG2] = D3DTA_CURRENT;
        defaultTextureStates[stage].values[D3DTSS_BUMPENVMAT00] = 0;
        defaultTextureStates[stage].values[D3DTSS_BUMPENVMAT01] = 0;
        defaultTextureStates[stage].values[D3DTSS_BUMPENVMAT10] = 0;
        defaultTextureStates[stage].values[D3DTSS_BUMPENVMAT11] = 0;
        defaultTextureStates[stage].values[D3DTSS_TEXCOORDINDEX] = stage;
        defaultTextureStates[stage].samplerValues[D3DSAMP_ADDRESSU] = D3DTADDRESS_WRAP;
        defaultTextureStates[stage].samplerValues[D3DSAMP_ADDRESSV] = D3DTADDRESS_WRAP;
        defaultTextureStates[stage].samplerValues[D3DSAMP_BORDERCOLOR] = 0;
        defaultTextureStates[stage].samplerValues[D3DSAMP_MAGFILTER] = D3DTEXF_POINT;
        defaultTextureStates[stage].samplerValues[D3DSAMP_MINFILTER] = D3DTEXF_POINT;
        defaultTextureStates[stage].samplerValues[D3DSAMP_MIPFILTER] = D3DTEXF_NONE;
        defaultTextureStates[stage].samplerValues[D3DSAMP_MIPMAPLODBIAS] = 0;
        defaultTextureStates[stage].samplerValues[D3DSAMP_MAXMIPLEVEL] = 0;
        defaultTextureStates[stage].samplerValues[D3DSAMP_MAXANISOTROPY] = 1;
        defaultTextureStates[stage].values[D3DTSS_BUMPENVLSCALE] = 0;
        defaultTextureStates[stage].values[D3DTSS_BUMPENVLOFFSET] = 0;
        defaultTextureStates[stage].values[D3DTSS_TEXTURETRANSFORMFLAGS] = D3DTTFF_DISABLE;
        defaultTextureStates[stage].samplerValues[D3DSAMP_ADDRESSW] = D3DTADDRESS_WRAP;
        if (currentDevice->caps.TextureOpCaps & (D3DTEXOPCAPS_MULTIPLYADD | D3DTEXOPCAPS_LERP))
        {
            defaultTextureStates[stage].values[D3DTSS_COLORARG0] = D3DTA_DIFFUSE;
            defaultTextureStates[stage].values[D3DTSS_ALPHAARG0] = D3DTA_DIFFUSE;
        }
        if (currentDevice->caps.PrimitiveMiscCaps & D3DPMISCCAPS_TSSARGTEMP)
        {
            defaultTextureStates[stage].values[D3DTSS_RESULTARG] = D3DTA_CURRENT;
        }
        ++stage;
    }
    return stage;
}

DWORD Gfx_D3D::ApplyCachedStates(BYTE defaults) // 0x503220.
{
    if (defaults)
    {
        ApplyDefaultRenderStates();
        return ApplyDefaultTextureStates();
    }

    DWORD result;
    for (DWORD i = 0; i < sizeof(RenderStateIds) / sizeof(RenderStateIds[0]); ++i)
    {
        D3DRENDERSTATETYPE state = RenderStateIds[i];
        result = device->SetRenderState(state, renderStates[state]);
    }
    DWORD stage = 0;
    while (stage < textureStageCount)
    {
        for (DWORD i = 0; i < sizeof(TextureStateIds) / sizeof(TextureStateIds[0]); ++i)
        {
            D3DTEXTURESTAGESTATETYPE state = TextureStateIds[i];
            device->SetTextureStageState(stage, state, textureStates[stage].values[state]);
        }
        for (D3DSAMPLERSTATETYPE sampler : Gfx_DX9SamplerStates)
        {
            device->SetSamplerState(stage, sampler, textureStates[stage].samplerValues[sampler]);
        }
        result = textureStageCount;
        ++stage;
        if (stage >= result)
        {
            break;
        }
    }
    return result;
}

HRESULT Gfx_D3D::ApplyDefaultRenderStates() // 0x503120.
{
    std::memcpy(renderStates, defaultRenderStates, sizeof(renderStates));
    HRESULT result;
    for (DWORD i = 0; i < sizeof(RenderStateIds) / sizeof(RenderStateIds[0]); ++i)
    {
        result = device->SetRenderState(RenderStateIds[i], renderStates[RenderStateIds[i]]);
    }
    return result;
}

DWORD Gfx_D3D::ApplyDefaultTextureStates() // 0x503170.
{
    DWORD bytes = textureStageCount * static_cast<DWORD>(sizeof(Gfx_D3DTextureState));
    std::memcpy(textureStates, defaultTextureStates, bytes);
    DWORD result = 0;
    DWORD stage = 0;
    if (stage < textureStageCount)
    {
        do
        {
            for (DWORD i = 0; i < sizeof(TextureStateIds) / sizeof(TextureStateIds[0]); ++i)
            {
                device->SetTextureStageState(stage, TextureStateIds[i],
                                             textureStates[stage].values[TextureStateIds[i]]);
            }
            for (D3DSAMPLERSTATETYPE sampler : Gfx_DX9SamplerStates)
            {
                device->SetSamplerState(stage, sampler, textureStates[stage].samplerValues[sampler]);
            }
            result = textureStageCount;
            ++stage;
        } while (stage < result);
    }
    return result;
}
