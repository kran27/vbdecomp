#include "RSBaseLightmapFogOfWar.hpp"
#include "Camera.hpp"
#include "D3DState.hpp"
#include "SystemInterface.hpp"
#include "TextureManager.hpp"
#include "Texture.hpp"
#include "Viewport.hpp"
#include "TextureTransform.hpp"
#include "diagnostics/DebugOutput.hpp"
#include <cstring>

//Original zero-initialized shared storage; lifetime remains explicit in callers.
Gfx_RS_B_LM_FOWState g_RSBaseLightmapFogOfWarState = {0};

//Exact2761-byte original effect at683898, supplied directly to D3DX8.1.
#define GFX_RS_B_LM_FOW_SOURCE \
"DWORD\t\tBcop;\nDWORD\t\tBaop;\nDWORD\t\tLMcop;\nDWORD\t\tLMaop;\nDWORD\t\tTFcop;\nDWORD\t\tTFaop;\nDWORD\t\tFOWcop;\nDWORD\t\tFOWaop;\nDWORD\t\talphatest;\nDWORD\t\tbLighting;\nDWORD\t\tbFog;\nDWORD\t\tDiffuseSource;\nMATRIX\t\tFOWmatrix;\nTexture\tFOW;\nTechnique\tffp\n{\nPass\tp0\n{\nAlphaTestEnable\t\t\t= < alphatest >;\nLighting\t\t\t\t\t= < bLighting >;\nFogEnable\t\t\t\t\t= < bFog >;\nDiffuseMaterialSource\t\t= < DiffuseSource >;\nAmbientMaterialSource\t\t= < DiffuseSource >;\nTexture[ 3 ]\t\t\t\t= < FOW >;\nColorOp[ 0 ]\t\t\t\t= < Bcop >;\nColorArg1[ 0 ]\t\t\t\t= Texture;\nColorArg2[ 0 ]\t\t\t\t= Diffuse;\nAlphaOp[ 0 ]\t\t\t\t= < Baop >;\nAlphaArg1[ 0 ]\t\t\t\t= Texture;\nAlphaArg2[ 0 ]\t\t\t\t= Diffuse;\nColorOp[ 1 ]\t\t\t\t= < LMcop >;\nColorArg1[ 1 ]\t\t\t\t= Texture;\nColorArg2[ 1 ]\t\t\t\t= Current;\nAlphaOp[ 1 ]\t\t\t\t= < LMaop >;\nAlphaArg1[ 1 ]\t\t\t\t= Texture;\nAlphaArg2[ 1 ]\t\t\t\t= Current;\nColorOp[ 2 ]\t\t\t\t= < TFcop >;\nColorArg1[ 2 ]\t\t\t\t= Current;\nColorArg2[ 2 ]\t\t\t\t= Tfactor;\nAlphaOp[ 2 ]\t\t\t\t= < TFaop >;\nAlphaArg1[ 2 ]\t\t\t\t= Current;\nAlphaArg2[ 2" \
" ]\t\t\t\t= Tfactor;\nTextureTransform[ 3 ]\t\t= < FOWmatrix >;\nTexCoordIndex[ 3 ]\t\t\t= CameraSpacePosition | 3;\nTextureTransformFlags[ 3 ]\t= Count2;\nAddressU[ 3 ]\t\t\t\t= Clamp;\nAddressV[ 3 ]\t\t\t\t= Clamp;\nMipfilter[ 3 ]\t\t\t\t= None;\nColorOp[ 3 ]\t\t\t\t= < FOWcop >;\nColorArg1[ 3 ]\t\t\t\t= Current;\nColorArg2[ 3 ]\t\t\t\t= Texture;\nAlphaOp[ 3 ]\t\t\t\t= < FOWaop >;\nAlphaArg1[ 3 ]\t\t\t\t= Current;\nAlphaArg2[ 3 ]\t\t\t\t= Texture;\nColorOp[ 4 ]\t\t\t\t= Disable;\nAlphaOp[ 4 ]\t\t\t\t= Disable;\n}\n}\nTechnique\tffp_2pass\n{\nPass\tp0\n{\nAlphaTestEnable\t\t\t= < alphatest >;\nLighting\t\t\t\t\t= < bLighting >;\nFogEnable\t\t\t\t\t= < bFog >;\nDiffuseMaterialSource\t\t= < DiffuseSource >;\nAmbientMaterialSource\t\t= < DiffuseSource >;\nColorOp[ 0 ]\t\t\t\t= < Bcop >;\nColorArg1[ 0 ]\t\t\t\t= Texture;\nColorArg2[ 0 ]\t\t\t\t= Diffuse;\nAlphaOp[ 0 ]\t\t\t\t= <Baop>;\nAlphaArg1[ 0 ]\t\t\t\t= Texture;\nAlphaArg2[ 0 ]\t\t\t\t= Diffuse;\nColorOp[ 1 ]\t\t\t\t= < LMcop >;\nColorArg1[ 1 ]\t\t\t\t= Texture;\nColorArg2[ 1 ]\t\t\t\t= Current;\nAlphaOp[ 1 ]\t\t\t\t= < LMaop >;" \
"\nAlphaArg1[ 1 ]\t\t\t\t= Texture;\nAlphaArg2[ 1 ]\t\t\t\t= Current;\nColorOp[ 2 ]\t\t\t\t= Disable;\nAlphaOp[ 2 ]\t\t\t\t= Disable;\n}\nPass\tp1\n{\nAlphaBlendEnable\t\t\t= TRUE;\nSrcBlend\t\t\t\t\t= DestColor;\nDestBlend\t\t\t\t\t= SrcColor;\nBlendOp\t\t\t\t\t= Add;\nTexture[ 1 ]\t\t\t\t= < FOW >;\nColorOp[ 0 ]\t\t\t\t= < TFcop >;\nColorArg1[ 0 ]\t\t\t\t= Current;\nColorArg2[ 0 ]\t\t\t\t= Tfactor;\nAlphaOp[ 0 ]\t\t\t\t= < TFaop >;\nAlphaArg1[ 0 ]\t\t\t\t= Current;\nAlphaArg2[ 0 ]\t\t\t\t= Tfactor;\nTextureTransform[ 1 ]\t\t= < FOWmatrix >;\nTexCoordIndex[ 1 ]\t\t\t= CameraSpacePosition | 1;\nTextureTransformFlags[ 1 ]\t= Count2;\nAddressU[ 1 ]\t\t\t\t= Clamp;\nAddressV[ 1 ]\t\t\t\t= Clamp;\nMipfilter[ 1 ]\t\t\t\t= None;\nColorOp[ 1 ]\t\t\t\t= < FOWcop >;\nColorArg1[ 1 ]\t\t\t\t= Current;\nColorArg2[ 1 ]\t\t\t\t= Texture;\nAlphaOp[ 1 ]\t\t\t\t= < FOWaop >;\nAlphaArg1[ 1 ]\t\t\t\t= Current;\nAlphaArg2[ 1 ]\t\t\t\t= Texture;\nColorOp[ 2 ]\t\t\t\t= Disable;\nAlphaOp[ 2 ]\t\t\t\t= Disable;\n}\n}\n"
typedef char Source2761[(sizeof(GFX_RS_B_LM_FOW_SOURCE) - 1 == 2761) ? 1 : -1];

Gfx_RS_B_LM_FOW::Gfx_RS_B_LM_FOW(DWORD color, DWORD alpha,
	DWORD lightmapColor, DWORD lightmapAlpha, DWORD test)
	: colorOperation(color), alphaOperation(alpha),
	lightmapColorOperation(lightmapColor), lightmapAlphaOperation(lightmapAlpha),
	alphaTest(test)
{
}

Gfx_RS_B_LM_FOW::~Gfx_RS_B_LM_FOW()
{
	Release();
}

HRESULT Gfx_RS_B_LM_FOW::Initialize()
{
	HRESULT result = D3DERR_INVALIDCALL;
	++g_RSBaseLightmapFogOfWarState.users;
	if (!g_RSBaseLightmapFogOfWarState.effect)
	{
		ID3DXBuffer* errors;
		result = D3DXCreateEffect(g_D3D.device, GFX_RS_B_LM_FOW_SOURCE, sizeof(GFX_RS_B_LM_FOW_SOURCE) - 1,
			&g_RSBaseLightmapFogOfWarState.effect, &errors);
		if (SUCCEEDED(result))
		{
			result = g_RSBaseLightmapFogOfWarState.effect->GetDesc(
				&g_RSBaseLightmapFogOfWarState.description);
			if (FAILED(result))
				JE::FatalError("Gfx_RS_B_LM_FOW::Gfx_RS_B_LM_FOW - Failed GetDesc (%x)\n", result);
			result = g_RSBaseLightmapFogOfWarState.effect->SetTechnique(0);
			if (FAILED(result))
				JE::FatalError("Gfx_RS_B_LM_FOW::Gfx_RS_B_LM_FOW - Failed to initialize technique (%x)\n", result);
			D3DXMATRIX identity = g_D3D.identity;
			g_RSBaseLightmapFogOfWarState.effect->SetDword("Bcop", colorOperation);
			g_RSBaseLightmapFogOfWarState.effect->SetDword("Baop", alphaOperation);
			g_RSBaseLightmapFogOfWarState.effect->SetDword("LMcop", lightmapColorOperation);
			g_RSBaseLightmapFogOfWarState.effect->SetDword("LMaop", lightmapAlphaOperation);
			g_RSBaseLightmapFogOfWarState.effect->SetDword("TFcop", 2);
			g_RSBaseLightmapFogOfWarState.effect->SetDword("TFaop", 2);
			g_RSBaseLightmapFogOfWarState.effect->SetDword("FOWcop", 2);
			g_RSBaseLightmapFogOfWarState.effect->SetDword("FOWaop", 2);
			g_RSBaseLightmapFogOfWarState.effect->SetDword("alphatest", 0);
			g_RSBaseLightmapFogOfWarState.effect->SetDword("bLighting", 0);
			g_RSBaseLightmapFogOfWarState.effect->SetDword("bFog", 0);
			g_RSBaseLightmapFogOfWarState.effect->SetDword("DiffuseSource", 1);
			g_RSBaseLightmapFogOfWarState.effect->SetMatrix("FOWmatrix", &identity);
			g_RSBaseLightmapFogOfWarState.effect->SetTexture("FOW", NULL);
			D3DXTECHNIQUE_DESC selected;
			result = g_RSBaseLightmapFogOfWarState.effect->FindNextValidTechnique(
				g_RSBaseLightmapFogOfWarState.technique, &selected);
			if (SUCCEEDED(result))
			{
				g_RSBaseLightmapFogOfWarState.technique = selected.Index;
				result = g_RSBaseLightmapFogOfWarState.effect->SetTechnique(selected.Index);
				if (selected.Name[0] == 's' && selected.Name[1] == 'h')
					g_RSBaseLightmapFogOfWarState.usesShader = 1;
				g_RSBaseLightmapFogOfWarState.passes = selected.Passes;
			}
			else
				JE::FatalError("Gfx_RS_B_LM_FOW::SetMaterial - Failed to find valid technique (%x)\n", result);
		}
		else
		{
			JE::LogDebugString("Gfx_RS_B_LM_FOW::Gfx_RS_B_LM_FOW - Failed to compile (%x)\n", result);
			JE::FatalError(static_cast<const char*>(errors->GetBufferPointer()));
		}
		if (errors) errors->Release();
	}
	return result;
}

void Gfx_RS_B_LM_FOW::Release()
{
	if (g_RSBaseLightmapFogOfWarState.users)
	{
		--g_RSBaseLightmapFogOfWarState.users;
		if (!g_RSBaseLightmapFogOfWarState.users && g_RSBaseLightmapFogOfWarState.effect)
		{
			g_RSBaseLightmapFogOfWarState.effect->Release();
			g_RSBaseLightmapFogOfWarState.effect = NULL;
		}
	}
}

HRESULT Gfx_RS_B_LM_FOW::OnLostDevice()
{
	HRESULT result = D3DERR_INVALIDCALL;
	if (g_RSBaseLightmapFogOfWarState.effect)
		result = g_RSBaseLightmapFogOfWarState.effect->OnLostDevice();
	return result;
}

HRESULT Gfx_RS_B_LM_FOW::OnResetDevice()
{
	HRESULT result = D3DERR_INVALIDCALL;
	if (g_RSBaseLightmapFogOfWarState.effect)
		result = g_RSBaseLightmapFogOfWarState.effect->OnResetDevice();
	return result;
}

DWORD Gfx_RS_B_LM_FOW::TechniqueCount()
{
	return g_RSBaseLightmapFogOfWarState.description.Techniques;
}

DWORD Gfx_RS_B_LM_FOW::SelectTechnique(DWORD index)
{
	if (index < g_RSBaseLightmapFogOfWarState.description.Techniques &&
		index != reinterpret_cast<DWORD>(g_RSBaseLightmapFogOfWarState.technique))
	{
		g_RSBaseLightmapFogOfWarState.techniqueChanged = 1;
		g_RSBaseLightmapFogOfWarState.technique = reinterpret_cast<LPCSTR>(index);
	}
	return index;
}

DWORD Gfx_RS_B_LM_FOW::NextTechnique()
{
	DWORD next = reinterpret_cast<DWORD>(g_RSBaseLightmapFogOfWarState.technique) + 1;
	if (next >= g_RSBaseLightmapFogOfWarState.description.Techniques) next = 0;
	return SelectTechnique(next);
}

BYTE Gfx_RS_B_LM_FOW::UsesVertexShader()
{
	return g_RSBaseLightmapFogOfWarState.usesShader;
}

DWORD Gfx_RS_B_LM_FOW::PassCount()
{
	return g_RSBaseLightmapFogOfWarState.passes;
}

HRESULT Gfx_RS_B_LM_FOW::Begin(UINT* passes)
{
	return g_RSBaseLightmapFogOfWarState.effect->Begin(passes, 0);
}

HRESULT Gfx_RS_B_LM_FOW::End()
{
	return g_RSBaseLightmapFogOfWarState.effect->End();
}

HRESULT Gfx_RS_B_LM_FOW::Pass(UINT pass)
{
	g_RSBaseLightmapFogOfWarState.currentPass = pass;
	return g_RSBaseLightmapFogOfWarState.effect->Pass(pass);
}

HRESULT Gfx_RS_B_LM_FOW::SetMaterial(Gfx_Material* material, DWORD)
{
	HRESULT result = D3DERR_INVALIDCALL;
	if (g_RSBaseLightmapFogOfWarState.effect)
	{
		const DWORD baseTexture = material->textureHandles[0];
		const DWORD lightmapTexture = material->textureHandles[1];
		if (baseTexture)
		{
			Gfx_Texture* texture = g_TextureManager->Lookup(baseTexture);
			if (texture) texture->Bind(0, 0);
			else
			{
				g_D3D.SetTexture(0, NULL);
				JE::LogDebugString("Gfx_RS_B_LM_FOW: Cannot find Base Texture %i\n", baseTexture);
			}
		}
		else
		{
			g_D3D.SetTexture(0, NULL);
			JE::LogDebugString("Gfx_RS_B_LM_FOW: NULL Base Texture handle\n");
		}
		if (lightmapTexture)
		{
			Gfx_Texture* texture = g_TextureManager->Lookup(lightmapTexture);
			if (texture) texture->Bind(1, 0);
			else g_D3D.SetTexture(1, NULL);
		}
		else g_D3D.SetTexture(1, NULL);

		IDirect3DTexture8* fogTexture = g_GfxSystem->FogOfWarRenderTexture();
		if (fogTexture)
		{
			g_RSBaseLightmapFogOfWarState.effect->SetTexture("FOW", fogTexture);
			g_RSBaseLightmapFogOfWarState.effect->SetDword("FOWcop", 4);
			g_RSBaseLightmapFogOfWarState.effect->SetDword("FOWaop", 2);
		}
		else
		{
			g_RSBaseLightmapFogOfWarState.effect->SetTexture("FOW", NULL);
			g_RSBaseLightmapFogOfWarState.effect->SetDword("FOWcop", 1);
			g_RSBaseLightmapFogOfWarState.effect->SetDword("FOWaop", 1);
		}
		g_RSBaseLightmapFogOfWarState.effect->SetDword("alphatest", alphaTest);
		BOOL lighting = FALSE;
		if (material->flagBC)
			lighting = g_GfxSystem->Lighting() != 0;
		g_RSBaseLightmapFogOfWarState.effect->SetDword("bLighting", lighting);
		BOOL fog = FALSE;
		if (material->flagBD)
			fog = g_GfxSystem->CurrentViewport()->fogEnabled != 0;
		g_RSBaseLightmapFogOfWarState.effect->SetDword("bFog", fog);
		BYTE useRGB = 0, useAlpha = 0;
		DWORD color;
		material->GetColor(useRGB, useAlpha, color);
		g_RSBaseLightmapFogOfWarState.effect->SetDword("DiffuseSource", material->flagB8 != 0);
		g_RSBaseLightmapFogOfWarState.effect->SetDword("Bcop", colorOperation);
		g_RSBaseLightmapFogOfWarState.effect->SetDword("Baop", alphaOperation);
		g_RSBaseLightmapFogOfWarState.effect->SetDword("LMcop", lightmapColorOperation);
		g_RSBaseLightmapFogOfWarState.effect->SetDword("LMaop", lightmapAlphaOperation);
		D3DMATERIAL8 values;
		std::memcpy(&values, &material->values, sizeof(values));
		g_D3D.device->SetMaterial(&values);
		if (!useRGB && !useAlpha)
		{
			g_RSBaseLightmapFogOfWarState.effect->SetDword("TFcop", 2);
			g_RSBaseLightmapFogOfWarState.effect->SetDword("TFaop", 2);
		}
		else
		{
			g_RSBaseLightmapFogOfWarState.effect->SetDword("TFcop", useRGB ? 5 : 2);
			g_RSBaseLightmapFogOfWarState.effect->SetDword("TFaop", useAlpha ? 4 : 2);
			g_D3D.SetRenderState(D3DRS_TEXTUREFACTOR, color);
		}
		if (material->flagB9)
			result = g_D3D.SetRenderState(D3DRS_CULLMODE, 3);
		else
			result = g_D3D.SetRenderState(D3DRS_CULLMODE, 1);
		D3DXMATRIX map = g_D3D.identity;
		D3DXVECTOR3 minimum, maximum;
		g_GfxSystem->GetLevelAABB(&minimum.x, &maximum.x);
		const float width = maximum.x - minimum.x;
		const float height = maximum.z - minimum.z;
		D3DXMATRIX inverseView;
		Gfx_Camera* camera = g_GfxSystem->Camera();
		camera->GetInverseViewMatrix(inverseView);
		const float inverseWidth = 1.0f / width;
		map._22 = 0.0f;
		map._33 = 0.0f;
		map._11 = inverseWidth;
		map._41 = -(inverseWidth * minimum.x);
		const float inverseHeight = 1.0f / height;
		map._32 = -1.0f * inverseHeight;
		map._42 = inverseHeight * maximum.z;
		D3DXMATRIX product;
		D3DXMatrixMultiply(&product, &inverseView, &map);
		D3DXMATRIX fogMatrix = product;
		g_RSBaseLightmapFogOfWarState.effect->SetMatrix("FOWmatrix", &fogMatrix);
		const float seconds = g_GfxSystem->TimeSeconds();
		const DWORD flags = material->values140[0];
		if (flags)
			SetTextureTransform(0, static_cast<BYTE>(flags), &material->textureParameters[0], seconds);
		else g_D3D.SetTextureStageState(0, D3DTSS_TEXTURETRANSFORMFLAGS, 0);

		if (g_RSBaseLightmapFogOfWarState.techniqueChanged)
		{
			D3DXTECHNIQUE_DESC selected;
			result = g_RSBaseLightmapFogOfWarState.effect->FindNextValidTechnique(
				g_RSBaseLightmapFogOfWarState.technique, &selected);
			if (SUCCEEDED(result))
			{
				g_RSBaseLightmapFogOfWarState.passes = selected.Passes;
				g_RSBaseLightmapFogOfWarState.techniqueChanged = 0;
				g_RSBaseLightmapFogOfWarState.technique = selected.Index;
				//Original53F0A3..53F0D0 does not call SetTechnique here.
			}
			else
				JE::FatalError("Gfx_RS_B_LM_FOW::SetMaterial - Failed to find valid technique (%x)\n", result);
		}
	}
	return result;
}
