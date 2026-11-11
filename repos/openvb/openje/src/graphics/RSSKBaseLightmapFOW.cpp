#include "RSSKBaseLightmapFOW.hpp"
#include "D3DState.hpp"
#include "SystemInterface.hpp"
#include "TextureManager.hpp"
#include "Texture.hpp"
#include "Viewport.hpp"
#include "TextureTransform.hpp"
#include "Camera.hpp"
#include "diagnostics/DebugOutput.hpp"
#include <cstring>

Gfx_RS_SK_B_LMFOWState g_RSSKBaseLightmapFOWState = {0};

//Original2963-byte effect at68C308, passed directly to the real SDK.
#define GFX_RS_SK_B_LM_FOW_SOURCE \
"DWORD\t\tBcop;\n" \
"DWORD\t\tBaop;\n" \
"DWORD\t\tLMcop;\n" \
"DWORD\t\tLMaop;\n" \
"DWORD\t\tTFcop;\n" \
"DWORD\t\tTFaop;\n" \
"DWORD\t\tFOWcop;\n" \
"DWORD\t\tFOWaop;\n" \
"DWORD\t\talphatest;\n" \
"DWORD\t\tbLighting;\n" \
"DWORD\t\tbFog;\n" \
"DWORD\t\tDiffuseSource;\n" \
"MATRIX\t\tFOWmatrix;\n" \
"Texture\tFOW;\n" \
"Texture\tBase;\n" \
"Texture\tLM;\n" \
"Technique\tffp\n" \
"{\n" \
"Pass\tp0\n" \
"{\n" \
"AlphaTestEnable\t\t\t= < alphatest >;\n" \
"Lighting\t\t\t\t\t= < bLighting >;\n" \
"FogEnable\t\t\t\t\t= < bFog >;\n" \
"DiffuseMaterialSource\t\t= < DiffuseSource >;\n" \
"AmbientMaterialSource\t\t= < DiffuseSource >;\n" \
"Ambient\t\t\t\t\t= 0x00;\n" \
"ColorOp[ 0 ]\t\t\t\t= < TFcop >;\n" \
"ColorArg1[ 0 ]\t\t\t\t= Diffuse;\n" \
"ColorArg2[ 0 ]\t\t\t\t= Tfactor;\n" \
"AlphaOp[ 0 ]\t\t\t\t= < TFaop >;\n" \
"AlphaArg1[ 0 ]\t\t\t\t= Diffuse;\n" \
"AlphaArg2[ 0 ]\t\t\t\t= Tfactor;\n" \
"Texture[ 1 ]\t\t\t\t= < LM >;\n" \
"ColorOp[ 1 ]\t\t\t\t= < LMcop >;\n" \
"ColorArg1[ 1 ]\t\t\t\t= Texture;\n" \
"ColorArg2[ 1 ]\t\t\t\t= Current;\n" \
"AlphaOp[ 1 ]\t\t\t\t= < LMaop >;\n" \
"AlphaArg1[ 1 ]\t\t\t\t= Texture;\n" \
"AlphaArg2[ 1 ]\t\t\t\t= Current;\n" \
"Texture[ 2 ]\t\t\t\t= < Base >;\n" \
"TexCoordIndex[ 2 ]\t\t\t= 0;\n" \
"ColorOp[ 2 ]\t\t\t\t= < Bcop >;\n" \
"ColorArg1[ 2 ]\t\t\t\t= Texture;\n" \
"ColorArg2[ 2 ]\t\t\t\t= Current;\n" \
"AlphaOp[ 2 ]\t\t\t\t= < Baop >;\n" \
"AlphaArg1[ 2 ]\t\t\t\t= Texture;\n" \
"AlphaArg2[ 2 ]\t\t\t\t= Current;\n" \
"Texture[ 3 ]\t\t\t\t= < FOW >;\n" \
"TextureTransform[ 3 ]\t\t= < FOWmatrix >;\n" \
"TexCoordIndex[ 3 ]\t\t\t= CameraSpacePosition | 3;\n" \
"TextureTransformFlags[ 3 ]\t= Count2;\n" \
"AddressU[ 3 ]\t\t\t\t= Clamp;\n" \
"AddressV[ 3 ]\t\t\t\t= Clamp;\n" \
"Mipfilter[ 3 ]\t\t\t\t= None;\n" \
"ColorOp[ 3 ]\t\t\t\t= < FOWcop >;\n" \
"ColorArg1[ 3 ]\t\t\t\t= Current;\n" \
"ColorArg2[ 3 ]\t\t\t\t= Texture;\n" \
"AlphaOp[ 3 ]\t\t\t\t= < FOWaop >;\n" \
"AlphaArg1[ 3 ]\t\t\t\t= Current;\n" \
"AlphaArg2[ 3 ]\t\t\t\t= Texture;\n" \
"ColorOp[ 4 ]\t\t\t\t= Disable;\n" \
"AlphaOp[ 4 ]\t\t\t\t= Disable;\n" \
"}\n" \
"}\n" \
"Technique\tffp_2pass\n" \
"{\n" \
"Pass\tp0\n" \
"{\n" \
"AlphaTestEnable\t\t\t= < alphatest >;\n" \
"Lighting\t\t\t\t\t= < bLighting >;\n" \
"FogEnable\t\t\t\t\t= < bFog >;\n" \
"DiffuseMaterialSource\t\t= < DiffuseSource >;\n" \
"AmbientMaterialSource\t\t= < DiffuseSource >;\n" \
"Ambient\t\t\t\t\t= 0x00;\n" \
"ColorOp[ 0 ]\t\t\t\t= < TFcop >;\n" \
"ColorArg1[ 0 ]\t\t\t\t= Diffuse;\n" \
"ColorArg2[ 0 ]\t\t\t\t= Tfactor;\n" \
"AlphaOp[ 0 ]\t\t\t\t= < TFaop >;\n" \
"AlphaArg1[ 0 ]\t\t\t\t= Diffuse;\n" \
"AlphaArg2[ 0 ]\t\t\t\t= Tfactor;\n" \
"Texture[ 1 ]\t\t\t\t= < LM >;\n" \
"ColorOp[ 1 ]\t\t\t\t= < LMcop >;\n" \
"ColorArg1[ 1 ]\t\t\t\t= Texture;\n" \
"ColorArg2[ 1 ]\t\t\t\t= Current;\n" \
"AlphaOp[ 1 ]\t\t\t\t= < LMaop >;\n" \
"AlphaArg1[ 1 ]\t\t\t\t= Texture;\n" \
"AlphaArg2[ 1 ]\t\t\t\t= Current;\n" \
"ColorOp[ 2 ]\t\t\t\t= Disable;\n" \
"AlphaOp[ 2 ]\t\t\t\t= Disable;\n" \
"}\n" \
"Pass\tp1\n" \
"{\n" \
"AlphaBlendEnable\t\t\t= TRUE;\n" \
"SrcBlend\t\t\t\t\t= DestColor;\n" \
"DestBlend\t\t\t\t\t= SrcColor;\n" \
"BlendOp\t\t\t\t\t= Add;\n" \
"Texture[ 1 ]\t\t\t\t= < FOW >;\n" \
"Texture[ 0 ]\t\t\t\t= < Base >;\n" \
"ColorOp[ 0 ]\t\t\t\t= SelectArg2;\n" \
"ColorArg1[ 0 ]\t\t\t\t= Texture;\n" \
"ColorArg2[ 0 ]\t\t\t\t= Diffuse;\n" \
"AlphaOp[ 0 ]\t\t\t\t= <Baop>;\n" \
"AlphaArg1[ 0 ]\t\t\t\t= Texture;\n" \
"AlphaArg2[ 0 ]\t\t\t\t= Diffuse;\n" \
"TextureTransform[ 1 ]\t\t= < FOWmatrix >;\n" \
"TexCoordIndex[ 1 ]\t\t\t= CameraSpacePosition | 1;\n" \
"TextureTransformFlags[ 1 ]\t= Count2;\n" \
"AddressU[ 1 ]\t\t\t\t= Clamp;\n" \
"AddressV[ 1 ]\t\t\t\t= Clamp;\n" \
"Mipfilter[ 1 ]\t\t\t\t= None;\n" \
"ColorOp[ 1 ]\t\t\t\t= SelectArg2;\n" \
"ColorArg1[ 1 ]\t\t\t\t= Current;\n" \
"ColorArg2[ 1 ]\t\t\t\t= Texture;\n" \
"AlphaOp[ 1 ]\t\t\t\t= < FOWaop >;\n" \
"AlphaArg1[ 1 ]\t\t\t\t= Current;\n" \
"AlphaArg2[ 1 ]\t\t\t\t= Texture;\n" \
"ColorOp[ 2 ]\t\t\t\t= Disable;\n" \
"AlphaOp[ 2 ]\t\t\t\t= Disable;\n" \
"}\n" \
"}\n"
typedef char Source2963[(sizeof(GFX_RS_SK_B_LM_FOW_SOURCE) - 1 == 2963) ? 1 : -1];

Gfx_RS_SK_B_LM_FOW::Gfx_RS_SK_B_LM_FOW(DWORD color, DWORD alpha,
	DWORD lightmapColor, DWORD lightmapAlpha, DWORD test)
{
	colorOperation = color;
	alphaOperation = alpha;
	lightmapColorOperation = lightmapColor;
	lightmapAlphaOperation = lightmapAlpha;
	alphaTest = test;
}
Gfx_RS_SK_B_LM_FOW::~Gfx_RS_SK_B_LM_FOW() { Release(); }
HRESULT Gfx_RS_SK_B_LM_FOW::Initialize()
{
	HRESULT result = D3DERR_INVALIDCALL;
	++g_RSSKBaseLightmapFOWState.users;
	if (!g_RSSKBaseLightmapFOWState.effect)
	{
		ID3DXBuffer* errors;
		result = D3DXCreateEffect(g_D3D.device, GFX_RS_SK_B_LM_FOW_SOURCE, sizeof(GFX_RS_SK_B_LM_FOW_SOURCE)-1,
			&g_RSSKBaseLightmapFOWState.effect, &errors);
		if (SUCCEEDED(result))
		{
			result = g_RSSKBaseLightmapFOWState.effect->GetDesc(&g_RSSKBaseLightmapFOWState.description);
			if (FAILED(result))
				JE::FatalError("Gfx_RS_SK_B_LM_FOW::Gfx_RS_SK_B_LM_FOW - Failed GetDesc (%x)\n", result);
			result = g_RSSKBaseLightmapFOWState.effect->SetTechnique(0);
			if (FAILED(result))
				JE::FatalError("Gfx_RS_SK_B_LM_FOW::Gfx_RS_SK_B_LM_FOW - Failed to initialize technique (%x)\n", result);
			D3DXMATRIX identity = g_D3D.identity;
			g_RSSKBaseLightmapFOWState.effect->SetDword("Bcop", colorOperation);
			g_RSSKBaseLightmapFOWState.effect->SetDword("Baop", alphaOperation);
			g_RSSKBaseLightmapFOWState.effect->SetDword("LMcop", lightmapColorOperation);
			g_RSSKBaseLightmapFOWState.effect->SetDword("LMaop", lightmapAlphaOperation);
			g_RSSKBaseLightmapFOWState.effect->SetDword("TFcop", 2);
			g_RSSKBaseLightmapFOWState.effect->SetDword("TFaop", 2);
			g_RSSKBaseLightmapFOWState.effect->SetDword("FOWcop", 2);
			g_RSSKBaseLightmapFOWState.effect->SetDword("FOWaop", 2);
			g_RSSKBaseLightmapFOWState.effect->SetDword("alphatest", 0);
			g_RSSKBaseLightmapFOWState.effect->SetDword("bLighting", 0);
			g_RSSKBaseLightmapFOWState.effect->SetDword("bFog", 0);
			g_RSSKBaseLightmapFOWState.effect->SetDword("DiffuseSource", 1);
			g_RSSKBaseLightmapFOWState.effect->SetMatrix("FOWmatrix", &identity);
			g_RSSKBaseLightmapFOWState.effect->SetTexture("FOW", NULL);
			g_RSSKBaseLightmapFOWState.effect->SetTexture("Base", NULL);
			g_RSSKBaseLightmapFOWState.effect->SetTexture("LM", NULL);
			D3DXTECHNIQUE_DESC selected;
			result = g_RSSKBaseLightmapFOWState.effect->FindNextValidTechnique(g_RSSKBaseLightmapFOWState.technique, &selected);
			if (SUCCEEDED(result))
			{
				g_RSSKBaseLightmapFOWState.technique = selected.Index;
				result = g_RSSKBaseLightmapFOWState.effect->SetTechnique(selected.Index);
				if (selected.Name[0] == 's' && selected.Name[1] == 'h')
					g_RSSKBaseLightmapFOWState.usesShader = 1;
				g_RSSKBaseLightmapFOWState.passes = selected.Passes;
			}
			else
				JE::FatalError("Gfx_RS_SK_B_LM_FOW::SetMaterial - Failed to find valid technique (%x)\n", result);
		}
		else
		{
			JE::LogDebugString("Gfx_RS_SK_B_LM_FOW::Gfx_RS_SK_B_LM_FOW - Failed to compile (%x)\n", result);
			JE::FatalError(static_cast<const char*>(errors->GetBufferPointer()));
		}
		if (errors) errors->Release();
	}
	return result;
}
void Gfx_RS_SK_B_LM_FOW::Release()
{
	if (g_RSSKBaseLightmapFOWState.users)
	{
		--g_RSSKBaseLightmapFOWState.users;
		if (!g_RSSKBaseLightmapFOWState.users && g_RSSKBaseLightmapFOWState.effect)
		{
			g_RSSKBaseLightmapFOWState.effect->Release();
			g_RSSKBaseLightmapFOWState.effect = 0;
		}
	}
}
HRESULT Gfx_RS_SK_B_LM_FOW::OnLostDevice()
{
	HRESULT result = D3DERR_INVALIDCALL;
	if (g_RSSKBaseLightmapFOWState.effect)
		result = g_RSSKBaseLightmapFOWState.effect->OnLostDevice();
	return result;
}
HRESULT Gfx_RS_SK_B_LM_FOW::OnResetDevice()
{
	HRESULT result = D3DERR_INVALIDCALL;
	if (g_RSSKBaseLightmapFOWState.effect)
		result = g_RSSKBaseLightmapFOWState.effect->OnResetDevice();
	return result;
}
DWORD Gfx_RS_SK_B_LM_FOW::TechniqueCount() { return g_RSSKBaseLightmapFOWState.description.Techniques; }
BYTE Gfx_RS_SK_B_LM_FOW::UsesVertexShader() { return g_RSSKBaseLightmapFOWState.usesShader; }
DWORD Gfx_RS_SK_B_LM_FOW::PassCount() { return g_RSSKBaseLightmapFOWState.passes; }
DWORD Gfx_RS_SK_B_LM_FOW::SelectTechnique(DWORD index)
{
	if (index < g_RSSKBaseLightmapFOWState.description.Techniques &&
		index != reinterpret_cast<DWORD>(g_RSSKBaseLightmapFOWState.technique))
	{
		g_RSSKBaseLightmapFOWState.techniqueChanged = 1;
		g_RSSKBaseLightmapFOWState.technique = reinterpret_cast<LPCSTR>(index);
	}
	return index;
}
DWORD Gfx_RS_SK_B_LM_FOW::NextTechnique()
{
	DWORD next = reinterpret_cast<DWORD>(g_RSSKBaseLightmapFOWState.technique) + 1;
	if (next >= g_RSSKBaseLightmapFOWState.description.Techniques) next = 0;
	return SelectTechnique(next);
}
HRESULT Gfx_RS_SK_B_LM_FOW::Begin(UINT* passes) { return g_RSSKBaseLightmapFOWState.effect->Begin(passes, 0); }
HRESULT Gfx_RS_SK_B_LM_FOW::End() { return g_RSSKBaseLightmapFOWState.effect->End(); }
HRESULT Gfx_RS_SK_B_LM_FOW::Pass(UINT pass)
{
	g_RSSKBaseLightmapFOWState.currentPass = pass;
	return g_RSSKBaseLightmapFOWState.effect->Pass(pass);
}
HRESULT Gfx_RS_SK_B_LM_FOW::SetMaterial(Gfx_Material* material, DWORD)
{
	HRESULT result = D3DERR_INVALIDCALL;
	if (g_RSSKBaseLightmapFOWState.effect)
	{
		const DWORD lightmapHandle = material->textureHandles[1];
		const DWORD baseHandle = material->textureHandles[0];
		if (lightmapHandle)
		{
			Gfx_Texture* object = g_TextureManager->Lookup(lightmapHandle);
			if (object)
				g_RSSKBaseLightmapFOWState.effect->SetTexture("LM", object->frameCount > 0 ? object->frames[0].texture : NULL);
			else g_RSSKBaseLightmapFOWState.effect->SetTexture("LM", NULL);
		}
		else g_RSSKBaseLightmapFOWState.effect->SetTexture("LM", NULL);
		if (baseHandle)
		{
			Gfx_Texture* object = g_TextureManager->Lookup(baseHandle);
			if (object)
			{
				object->Bind(0, 0);
				g_RSSKBaseLightmapFOWState.effect->SetTexture("Base", object->frameCount > 0 ? object->frames[0].texture : NULL);
			}
			else
			{
				g_RSSKBaseLightmapFOWState.effect->SetTexture("Base", NULL);
				JE::LogDebugString("Gfx_RS_SK_B_LM_FOW: Cannot find Base Texture %i\n", baseHandle);
			}
		}
		else
		{
			g_RSSKBaseLightmapFOWState.effect->SetTexture("Base", NULL);
			JE::LogDebugString("Gfx_RS_SK_B_LM_FOW: NULL Base Texture handle\n");
		}
		IDirect3DTexture8* fog = g_GfxSystem->FogOfWarRenderTexture();
		if (fog)
		{
			g_RSSKBaseLightmapFOWState.effect->SetTexture("FOW", fog);
			g_RSSKBaseLightmapFOWState.effect->SetDword("FOWcop", 4);
			g_RSSKBaseLightmapFOWState.effect->SetDword("FOWaop", 2);
		}
		else
		{
			g_RSSKBaseLightmapFOWState.effect->SetTexture("FOW", NULL);
			g_RSSKBaseLightmapFOWState.effect->SetDword("FOWcop", 1);
			g_RSSKBaseLightmapFOWState.effect->SetDword("FOWaop", 1);
		}
		g_RSSKBaseLightmapFOWState.effect->SetDword("alphatest", alphaTest);
		DWORD lighting = 0;
		if (material->flagBC) lighting = g_GfxSystem->Lighting() != 0;
		g_RSSKBaseLightmapFOWState.effect->SetDword("bLighting", lighting);
		DWORD fogEnabled = 0;
		if (material->flagBD) fogEnabled = g_GfxSystem->CurrentViewport()->fogEnabled != 0;
		g_RSSKBaseLightmapFOWState.effect->SetDword("bFog", fogEnabled);
		BYTE useRGB = 0, useAlpha = 0;
		DWORD color;
		material->GetColor(useRGB, useAlpha, color);
		g_RSSKBaseLightmapFOWState.effect->SetDword("DiffuseSource", material->flagB8 != 0);
		g_RSSKBaseLightmapFOWState.effect->SetDword("Bcop", colorOperation);
		g_RSSKBaseLightmapFOWState.effect->SetDword("Baop", alphaOperation);
		g_RSSKBaseLightmapFOWState.effect->SetDword("LMcop", lightmapColorOperation);
		g_RSSKBaseLightmapFOWState.effect->SetDword("LMaop", lightmapAlphaOperation);
		D3DMATERIAL8 values;
		std::memcpy(&values, &material->values, sizeof(values));
		g_D3D.device->SetMaterial(&values);
		if (!useRGB && !useAlpha)
		{
			g_RSSKBaseLightmapFOWState.effect->SetDword("TFcop", 2);
			g_RSSKBaseLightmapFOWState.effect->SetDword("TFaop", 2);
		}
		else
		{
			g_RSSKBaseLightmapFOWState.effect->SetDword("TFcop", useRGB ? 5 : 2);
			g_RSSKBaseLightmapFOWState.effect->SetDword("TFaop", useAlpha ? 4 : 2);
			g_D3D.SetRenderState(D3DRS_TEXTUREFACTOR, color);
		}
		result = g_D3D.SetRenderState(D3DRS_CULLMODE, material->flagB9 ? 3 : 1);
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
		g_RSSKBaseLightmapFOWState.effect->SetMatrix("FOWmatrix", &fogMatrix);
		const float seconds = g_GfxSystem->TimeSeconds(); //Unconditional original getter.
		DWORD flags = material->values140[0];
		if (flags) SetTextureTransform(2, static_cast<BYTE>(flags),
			&material->textureParameters[0], seconds);
		else g_D3D.SetTextureStageState(0, D3DTSS_TEXTURETRANSFORMFLAGS, 0); //Actual stage0 quirk.
		if (g_RSSKBaseLightmapFOWState.techniqueChanged)
		{
			D3DXTECHNIQUE_DESC selected;
			result = g_RSSKBaseLightmapFOWState.effect->FindNextValidTechnique(g_RSSKBaseLightmapFOWState.technique, &selected);
			if (SUCCEEDED(result))
			{
				g_RSSKBaseLightmapFOWState.passes = selected.Passes;
				g_RSSKBaseLightmapFOWState.techniqueChanged = 0;
				g_RSSKBaseLightmapFOWState.technique = selected.Index;
			}
			else JE::FatalError("Gfx_RS_SK_B_LM_FOW::SetMaterial - Failed to find valid technique (%x)\n", result);
		}
	}
	return result;
}
