#include "RSSKBaseLightmap.hpp"
#include "D3DState.hpp"
#include "SystemInterface.hpp"
#include "TextureManager.hpp"
#include "Texture.hpp"
#include "Viewport.hpp"
#include "TextureTransform.hpp"
#include "diagnostics/DebugOutput.hpp"
#include <cstring>

namespace
{
	// Exact original effect descriptions at6893B8/6896E8. The real SDK compiles
	// them; no replacement shader or SDK implementation is supplied here.
	const char normalSource[] = "DWORD\t\tBcop;\nDWORD\t\tBaop;\nDWORD\t\tLMcop;\nDWORD\t\tLMaop;\nDWORD\t\tTFcop;\nDWORD\t\tTFaop;\nDWORD\t\talphatest;\nDWORD\t\tbLighting;\nDWORD\t\tbFog;\nDWORD\t\tDiffuseSource;\nTechnique\tffp\n{\nPass\tp0\n{\nAlphaTestEnable\t\t= < alphatest >;\nLighting\t\t\t\t= < bLighting >;\nFogEnable\t\t\t\t= < bFog >;\nDiffuseMaterialSource\t= < DiffuseSource >;\nAmbientMaterialSource\t= < DiffuseSource >;\nAmbient\t\t\t\t= 0x00;\nTexCoordIndex[ 0 ]\t\t= 1;\nColorOp[ 0 ]\t\t\t= < LMcop >;\nColorArg1[ 0 ]\t\t\t= Texture;\nColorArg2[ 0 ]\t\t\t= Diffuse;\nAlphaOp[ 0 ]\t\t\t= < LMaop >;\nAlphaArg1[ 0 ]\t\t\t= Texture;\nAlphaArg2[ 0 ]\t\t\t= Diffuse;\nTexCoordIndex[ 1 ]\t\t= 0;\nColorOp[ 1 ]\t\t\t= < Bcop >;\nColorArg1[ 1 ]\t\t\t= Texture;\nColorArg2[ 1 ]\t\t\t= Current;\nAlphaOp[ 1 ]\t\t\t= < Baop >;\nAlphaArg1[ 1 ]\t\t\t= Texture;\nAlphaArg2[ 1 ]\t\t\t= Current;\nColorOp[ 2 ]\t\t\t= Disable;\nAlphaOp[ 2 ]\t\t\t= Disable;\n}\n}\n";
	typedef char NormalSource812[(sizeof(normalSource) - 1 == 812) ? 1 : -1];
	const char factoredSource[] = "DWORD\t\tBcop;\nDWORD\t\tBaop;\nDWORD\t\tLMcop;\nDWORD\t\tLMaop;\nDWORD\t\tTFcop;\nDWORD\t\tTFaop;\nDWORD\t\talphatest;\nDWORD\t\tbLighting;\nDWORD\t\tbFog;\nDWORD\t\tDiffuseSource;\nTechnique\tffp\n{\nPass\tp0\n{\nAlphaTestEnable\t\t= < alphatest >;\nLighting\t\t\t\t= < bLighting >;\nFogEnable\t\t\t\t= < bFog >;\nDiffuseMaterialSource\t= < DiffuseSource >;\nAmbientMaterialSource\t= < DiffuseSource >;\nAmbient\t\t\t\t= 0x00;\nColorOp[ 0 ]\t\t\t= < TFcop >;\nColorArg1[ 0 ]\t\t\t= Diffuse;\nColorArg2[ 0 ]\t\t\t= Tfactor;\nAlphaOp[ 0 ]\t\t\t= < TFaop >;\nAlphaArg1[ 0 ]\t\t\t= Diffuse;\nAlphaArg2[ 0 ]\t\t\t= Tfactor;\nColorOp[ 1 ]\t\t\t= < LMcop >;\nColorArg1[ 1 ]\t\t\t= Texture;\nColorArg2[ 1 ]\t\t\t= Current;\nAlphaOp[ 1 ]\t\t\t= < LMaop >;\nAlphaArg1[ 1 ]\t\t\t= Texture;\nAlphaArg2[ 1 ]\t\t\t= Current;\nTexCoordIndex[ 2 ]\t\t= 0;\nColorOp[ 2 ]\t\t\t= < Bcop >;\nColorArg1[ 2 ]\t\t\t= Texture;\nColorArg2[ 2 ]\t\t\t= Current;\nAlphaOp[ 2 ]\t\t\t= < Baop >;\nAlphaArg1[ 2 ]\t\t\t= Texture;\nAlphaArg2[ 2 ]\t\t\t= Current;\nColorOp[ 3 ]\t\t\t= Disable;\nAlphaOp[ 3 ]\t\t\t= Disable;\n}\n}\nTechnique\tffp_2pass\n{\nPass\tp0\n{\nAlphaTestEnable\t\t= < alphatest >;\nLighting\t\t\t\t= < bLighting >;\nFogEnable\t\t\t\t= < bFog >;\nDiffuseMaterialSource\t= < DiffuseSource >;\nAmbientMaterialSource\t= < DiffuseSource >;\nAmbient\t\t\t\t= 0x00;\nColorOp[ 0 ]\t\t\t= < TFcop >;\nColorArg1[ 0 ]\t\t\t= Diffuse;\nColorArg2[ 0 ]\t\t\t= Tfactor;\nAlphaOp[ 0 ]\t\t\t= < TFaop >;\nAlphaArg1[ 0 ]\t\t\t= Diffuse;\nAlphaArg2[ 0 ]\t\t\t= Tfactor;\nColorOp[ 1 ]\t\t\t= < LMcop >;\nColorArg1[ 1 ]\t\t\t= Texture;\nColorArg2[ 1 ]\t\t\t= Current;\nAlphaOp[ 1 ]\t\t\t= < LMaop >;\nAlphaArg1[ 1 ]\t\t\t= Texture;\nAlphaArg2[ 1 ]\t\t\t= Current;\nColorOp[ 2 ]\t\t\t= Disable;\nAlphaOp[ 2 ]\t\t\t= Disable;\n}\nPass\tp1\n{\nAlphaBlendEnable\t\t= TRUE;\nSrcBlend\t\t\t\t= DestColor;\nDestBlend\t\t\t\t= SrcColor;\nBlendOp\t\t\t\t= Add;\nColorOp[ 0 ]\t\t\t= < Bcop >;\nColorArg1[ 0 ]\t\t\t= Texture;\nColorArg2[ 0 ]\t\t\t= Diffuse;\nAlphaOp[ 0 ]\t\t\t= < Baop >;\nAlphaArg1[ 0 ]\t\t\t= Texture;\nAlphaArg2[ 0 ]\t\t\t= Diffuse;\nColorOp[ 1 ]\t\t\t= Disable;\nAlphaOp[ 1 ]\t\t\t= Disable;\n}\n}\n";
	typedef char FactoredSource1896[(sizeof(factoredSource) - 1 == 1896) ? 1 : -1];
}

Gfx_RS_SK_B_LM::Gfx_RS_SK_B_LM(DWORD color, DWORD alpha, DWORD lightmapColor,
	DWORD lightmapAlpha, DWORD test)
{
	colorOperation = color;
	alphaOperation = alpha;
	lightmapColorOperation = lightmapColor;
	lightmapAlphaOperation = lightmapAlpha;
	effect = 0;
	alphaTest = test != 0;
}

void Gfx_RS_SK_B_LM::Release()
{
	if (g_RSSKBaseLightmapState.users)
	{
		--g_RSSKBaseLightmapState.users;
		if (!g_RSSKBaseLightmapState.users)
		{
			if (g_RSSKBaseLightmapState.normal)
			{
				g_RSSKBaseLightmapState.normal->Release();
				g_RSSKBaseLightmapState.normal = 0;
			}
			if (g_RSSKBaseLightmapState.factored)
			{
				g_RSSKBaseLightmapState.factored->Release();
				g_RSSKBaseLightmapState.factored = 0;
			}
		}
	}
}

Gfx_RS_SK_B_LM::~Gfx_RS_SK_B_LM()
{
	Release();
}

HRESULT Gfx_RS_SK_B_LM::Initialize()
{
	HRESULT result = D3DERR_INVALIDCALL;
	++g_RSSKBaseLightmapState.users;
	ID3DXBuffer* errors;
	D3DXTECHNIQUE_DESC selected;
	if (!g_RSSKBaseLightmapState.normal)
	{
		result = D3DXCreateEffect(g_D3D.device, normalSource, sizeof(normalSource) - 1,
			&g_RSSKBaseLightmapState.normal, &errors);
		if (SUCCEEDED(result))
		{
			g_RSSKBaseLightmapState.normal->SetDword("Bcop", colorOperation);
			g_RSSKBaseLightmapState.normal->SetDword("Baop", alphaOperation);
			g_RSSKBaseLightmapState.normal->SetDword("LMcop", lightmapColorOperation);
			g_RSSKBaseLightmapState.normal->SetDword("LMaop", lightmapAlphaOperation);
			g_RSSKBaseLightmapState.normal->SetDword("TFcop", 2);
			g_RSSKBaseLightmapState.normal->SetDword("TFaop", 2);
			g_RSSKBaseLightmapState.normal->SetDword("alphatest", 0);
			g_RSSKBaseLightmapState.normal->SetDword("bLighting", 0);
			g_RSSKBaseLightmapState.normal->SetDword("bFog", 0);
			g_RSSKBaseLightmapState.normal->SetDword("DiffuseSource", 1);
			HRESULT queried = g_RSSKBaseLightmapState.normal->FindNextValidTechnique(0, &selected);
			if (FAILED(queried))
				JE::FatalError("Gfx_RS_SK_B_LM::Create - Failed to find valid technique for B_LM normal (%x)\n", queried);
			g_RSSKBaseLightmapState.technique = reinterpret_cast<DWORD>(selected.Index);
			result = g_RSSKBaseLightmapState.normal->SetTechnique(selected.Index);
			if (selected.Name[0] == 's' && selected.Name[1] == 'h')
				g_RSSKBaseLightmapState.usesShader = 1;
			g_RSSKBaseLightmapState.passes = selected.Passes;
		}
		else
		{
			JE::LogDebugString("Gfx_RS_SK_B_LM::Create - Failed to compile B_LM normal (%x)\n", result);
			JE::FatalError(static_cast<const char*>(errors->GetBufferPointer()));
		}
		if (errors)
		{
			errors->Release();
			errors = 0;
		}
	}
	if (!g_RSSKBaseLightmapState.factored)
	{
		result = D3DXCreateEffect(g_D3D.device, factoredSource, sizeof(factoredSource) - 1,
			&g_RSSKBaseLightmapState.factored, &errors);
		if (SUCCEEDED(result))
		{
			HRESULT queried = g_RSSKBaseLightmapState.factored->SetTechnique(0);
			if (FAILED(queried))
				JE::FatalError("Gfx_RS_SK_B_LM::Gfx_RS_SK_B_LM - Failed to initialize technique (%x)\n", queried);
			g_RSSKBaseLightmapState.factored->SetDword("Bcop", colorOperation);
			g_RSSKBaseLightmapState.factored->SetDword("Baop", alphaOperation);
			g_RSSKBaseLightmapState.factored->SetDword("LMcop", lightmapColorOperation);
			g_RSSKBaseLightmapState.factored->SetDword("LMaop", lightmapAlphaOperation);
			g_RSSKBaseLightmapState.factored->SetDword("TFcop", 2);
			g_RSSKBaseLightmapState.factored->SetDword("TFaop", 2);
			g_RSSKBaseLightmapState.factored->SetDword("alphatest", 0);
			g_RSSKBaseLightmapState.factored->SetDword("bLighting", 0);
			g_RSSKBaseLightmapState.factored->SetDword("bFog", 0);
			g_RSSKBaseLightmapState.factored->SetDword("DiffuseSource", 1);
			result = g_RSSKBaseLightmapState.factored->FindNextValidTechnique(0, &selected);
			if (SUCCEEDED(result))
			{
				g_RSSKBaseLightmapState.technique = reinterpret_cast<DWORD>(selected.Index);
				result = g_RSSKBaseLightmapState.factored->SetTechnique(selected.Index);
				if (selected.Name[0] == 's' && selected.Name[1] == 'h')
					g_RSSKBaseLightmapState.usesShader = 1;
				if (g_RSSKBaseLightmapState.passes < selected.Passes)
					g_RSSKBaseLightmapState.passes = selected.Passes;
			}
			else JE::FatalError("Gfx_RS_SK_B_LM::SetMaterial - Failed to find valid technique (%x)\n", result);
		}
		else
		{
			JE::LogDebugString("Gfx_RS_SK_B_LM::Gfx_RS_SK_B_LM - Failed to compile (%x)\n", result);
			JE::FatalError(static_cast<const char*>(errors->GetBufferPointer()));
		}
		if (errors) errors->Release();
	}
	return result;
}

HRESULT Gfx_RS_SK_B_LM::SetMaterial(Gfx_Material* material, DWORD)
{
	HRESULT result = D3DERR_INVALIDCALL;
	BYTE useRGB = 0, useAlpha = 0;
	DWORD color;
	material->GetColor(useRGB, useAlpha, color);
	if (!useRGB && !useAlpha) effect = g_RSSKBaseLightmapState.normal;
	else effect = g_RSSKBaseLightmapState.factored;
	if (effect)
	{
		DWORD baseTexture = material->textureHandles[0];
		DWORD lightmapTexture = material->textureHandles[1];
		if (lightmapTexture)
		{
			Gfx_Texture* object = g_TextureManager->Lookup(lightmapTexture);
			if (object)
			{
				if (!useRGB && !useAlpha) object->Bind(0, 0);
				else object->Bind(1, 0);
			}
			else g_D3D.SetTexture(0, 0);
		}
		else g_D3D.SetTexture(0, 0);
		if (baseTexture)
		{
			Gfx_Texture* object = g_TextureManager->Lookup(baseTexture);
			if (object)
			{
				if (!useRGB && !useAlpha) object->Bind(1, 0);
				else object->Bind(2, 0);
			}
			else
			{
				g_D3D.SetTexture(1, 0);
				JE::LogDebugString("Gfx_RS_SK_B_LM: Cannot find Base Texture %i\n", baseTexture);
			}
		}
		else
		{
			g_D3D.SetTexture(1, 0);
			JE::LogDebugString("Gfx_RS_SK_B_LM: NULL Base Texture handle\n");
		}
		effect->SetDword("alphatest", alphaTest);
		DWORD lighting = 0;
		if (material->flagBC) lighting = g_GfxSystem->Lighting() != 0;
		effect->SetDword("bLighting", lighting);
		DWORD fog = 0;
		if (material->flagBD) fog = g_GfxSystem->CurrentViewport()->fogEnabled != 0;
		effect->SetDword("bFog", fog);
		effect->SetDword("DiffuseSource", material->flagB8 != 0);
		effect->SetDword("Bcop", colorOperation);
		effect->SetDword("Baop", alphaOperation);
		effect->SetDword("LMcop", lightmapColorOperation);
		effect->SetDword("LMaop", lightmapAlphaOperation);
		D3DMATERIAL8 values;
		std::memcpy(&values, &material->values, sizeof(values));
		g_D3D.device->SetMaterial(&values);
		if (!useRGB && !useAlpha)
		{
			effect->SetDword("TFcop", 2);
			effect->SetDword("TFaop", 2);
		}
		else
		{
			if (useRGB) effect->SetDword("TFcop", 4);
			else effect->SetDword("TFcop", 2);
			if (useAlpha) effect->SetDword("TFaop", 4);
			else effect->SetDword("TFaop", 2);
			if (!useRGB) { color &= 0xFF808080; color |= 0x00808080; }
			g_D3D.SetRenderState(D3DRS_TEXTUREFACTOR, color);
		}
		if (material->flagB9) result = g_D3D.SetRenderState(D3DRS_CULLMODE, 3);
		else result = g_D3D.SetRenderState(D3DRS_CULLMODE, 1);
		float seconds = g_GfxSystem->TimeSeconds();
		DWORD flags = material->values140[0];
		if (flags) SetTextureTransform(1, static_cast<BYTE>(flags), &material->textureParameters[0], seconds);
		else g_D3D.SetTextureStageState(0, D3DTSS_TEXTURETRANSFORMFLAGS, 0);
		if (g_RSSKBaseLightmapState.techniqueChanged)
		{
			D3DXTECHNIQUE_DESC selected;
			result = effect->FindNextValidTechnique(0, &selected);
			if (SUCCEEDED(result))
			{
				g_RSSKBaseLightmapState.techniqueChanged = 0;
				g_RSSKBaseLightmapState.technique = reinterpret_cast<DWORD>(selected.Index);
				g_RSSKBaseLightmapState.passes = selected.Passes;
				result = effect->SetTechnique(selected.Index);
				if (FAILED(result))
					JE::FatalError("Gfx_RS_SK_B_LM::SetMaterial - Failed to set technique (%x)\n", result);
			}
			else JE::FatalError("Gfx_RS_SK_B_LM::SetMaterial - Failed to find valid technique (%x)\n", result);
		}
	}
	return result;
}
