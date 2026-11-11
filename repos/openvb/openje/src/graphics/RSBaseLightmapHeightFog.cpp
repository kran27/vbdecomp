#include "RSBaseLightmapHeightFog.hpp"
#include "HeightFog.hpp"
#include "D3DState.hpp"
#include "SystemInterface.hpp"
#include "TextureManager.hpp"
#include "Texture.hpp"
#include "Viewport.hpp"
#include "TextureTransform.hpp"
#include "diagnostics/DebugOutput.hpp"
#include <cstring>

Gfx_RS_BaseLightmapHeightFogState g_RSBaseLightmapHeightFogState = {0};

namespace
{
	//Exact original effect bytes; split for the supplied VC7 literal limit.
#define GFX_RS_B_LM_HEIGHT_FOG_NORMAL_SOURCE \
"DWORD\tBcop;\nDWORD\tBaop;\nDWORD\tLMcop;\nDWORD\tLMaop;\nDWORD\tTFcop;\nDWORD\tTFaop;\nDWORD\talphatest;\nDWORD\tbLighting;\nDWORD\tbFog;\nDWORD\tDiffuseSource;\nMATRIX\tfogmatrix;\nTexture\tHFog;\nTechnique ffp_hfog\n{\nPass p0\n{\nAlphaTestEnable\t\t\t= < alphatest >;\nLighting\t\t\t\t\t= < bLighting >;\nFogEnable\t\t\t\t\t= < bFog >;\nDiffuseMaterialSource\t\t= < DiffuseSource >;\nAmbientMaterialSource\t\t= < DiffuseSource >;\nColorOp[ 0 ]\t\t\t\t= < Bcop >;\nColorArg1[ 0 ]\t\t\t\t= Texture;\nColorArg2[ 0 ]\t\t\t\t= Diffuse;\nAlphaOp[ 0 ]\t\t\t\t= < Baop >;\nAlphaArg1[ 0 ]\t\t\t\t= Texture;\nAlphaArg2[ 0 ]\t\t\t\t= Diffuse;\nColorOp[ 1 ]\t\t\t\t= < LMcop >;\nColorArg1[ 1 ]\t\t\t\t= Texture;\nColorArg2[ 1 ]\t\t\t\t= Current;\nAlphaOp[ 1 ]\t\t\t\t= < LMaop >;\nAlphaArg1[ 1 ]\t\t\t\t= Texture;\nAlphaArg2[ 1 ]\t\t\t\t= Current;\nTexture[ 2 ]\t\t\t\t= < HFog >;\nTextureTransform[ 2 ]\t\t= < fogmatrix >;\nTexCoordIndex[ 2 ]\t\t\t= CameraSpacePosition | 2;\nTextureTransformFlags[ 2 ]\t= Count2;\nAddressU[ 2 ]\t\t\t\t= Clamp;\nAddressV[ 2 ]\t\t\t\t= Clamp;\nMipfilter[ 2 ]" \
"\t\t\t\t= None;\nColorOp[ 2 ]\t\t\t\t= BlendTextureAlpha;\nColorArg1[ 2 ]\t\t\t\t= Texture;\nColorArg2[ 2 ]\t\t\t\t= Current;\nAlphaOp[ 2 ]\t\t\t\t= SelectArg2;\nAlphaArg1[ 2 ]\t\t\t\t= Texture;\nAlphaArg2[ 2 ]\t\t\t\t= Current;\nColorOp[ 3 ]\t\t\t\t= Disable;\nAlphaOp[ 3 ]\t\t\t\t= Disable;\n}\n}\nTechnique ffp_hfog_2pass\n{\nPass p0\n{\nAlphaTestEnable\t\t\t= < alphatest >;\nLighting\t\t\t\t\t= < bLighting >;\nFogEnable\t\t\t\t\t= < bFog >;\nDiffuseMaterialSource\t\t= < DiffuseSource >;\nAmbientMaterialSource\t\t= < DiffuseSource >;\nColorOp[ 0 ]\t\t\t\t= < Bcop >;\nColorArg1[ 0 ]\t\t\t\t= Texture;\nColorArg2[ 0 ]\t\t\t\t= Diffuse;\nAlphaOp[ 0 ]\t\t\t\t= < Baop >;\nAlphaArg1[ 0 ]\t\t\t\t= Texture;\nAlphaArg2[ 0 ]\t\t\t\t= Diffuse;\nColorOp[ 1 ]\t\t\t\t= < LMcop >;\nColorArg1[ 1 ]\t\t\t\t= Texture;\nColorArg2[ 1 ]\t\t\t\t= Current;\nAlphaOp[ 1 ]\t\t\t\t= < LMaop >;\nAlphaArg1[ 1 ]\t\t\t\t= Texture;\nAlphaArg2[ 1 ]\t\t\t\t= Current;\nColorOp[ 2 ]\t\t\t\t= Disable;\nAlphaOp[ 2 ]\t\t\t\t= Disable;\n}\nPass p1\n{\nAlphaBlendEnable\t\t\t= TRUE;\nSrcBlend\t\t\t\t\t= SrcAlpha;\nDestBlend\t\t\t\t\t= I" \
"nvSrcAlpha;\nBlendOp\t\t\t\t\t= Add;\nColorOp[ 0 ]\t\t\t\t= SelectArg2;\nColorArg1[ 0 ]\t\t\t\t= Texture;\nColorArg2[ 0 ]\t\t\t\t= Diffuse;\nAlphaOp[ 0 ]\t\t\t\t= < Baop >;\nAlphaArg1[ 0 ]\t\t\t\t= Texture;\nAlphaArg2[ 0 ]\t\t\t\t= Diffuse;\nTexture[ 1 ]\t\t\t\t= < HFog >;\nTextureTransform[ 1 ]\t\t= < fogmatrix >;\nTexCoordIndex[ 1 ]\t\t\t= CameraSpacePosition | 1;\nTextureTransformFlags[ 1 ]\t= Count2;\nAddressU[ 1 ]\t\t\t\t= Clamp;\nAddressV[ 1 ]\t\t\t\t= Clamp;\nMipfilter[ 1 ]\t\t\t\t= None;\nColorOp[ 1 ]\t\t\t\t= SelectArg1;\nColorArg1[ 1 ]\t\t\t\t= Texture;\nColorArg2[ 1 ]\t\t\t\t= Current;\nAlphaOp[ 1 ]\t\t\t\t= Modulate;\nAlphaArg1[ 1 ]\t\t\t\t= Texture;\nAlphaArg2[ 1 ]\t\t\t\t= Current;\nColorOp[ 2 ]\t\t\t\t= Disable;\nAlphaOp[ 2 ]\t\t\t\t= Disable;\n}\n}\n"
	typedef char NormalSourceSize[(sizeof(GFX_RS_B_LM_HEIGHT_FOG_NORMAL_SOURCE) - 1 == 2568) ? 1 : -1];
#define GFX_RS_B_LM_HEIGHT_FOG_FACTORED_SOURCE \
"DWORD\tBcop;\nDWORD\tBaop;\nDWORD\tLMcop;\nDWORD\tLMaop;\nDWORD\tTFcop;\nDWORD\tTFaop;\nDWORD\talphatest;\nDWORD\tbLighting;\nDWORD\tbFog;\nDWORD\tDiffuseSource;\nMATRIX\tfogmatrix;\nTexture\tHFog;\nTechnique ffp_hfog\n{\nPass p0\n{\nAlphaTestEnable\t\t\t= < alphatest >;\nLighting\t\t\t\t\t= < bLighting >;\nFogEnable\t\t\t\t\t= < bFog >;\nDiffuseMaterialSource\t\t= < DiffuseSource >;\nAmbientMaterialSource\t\t= < DiffuseSource >;\nColorOp[ 0 ]\t\t\t\t= < Bcop >;\nColorArg1[ 0 ]\t\t\t\t= Texture;\nColorArg2[ 0 ]\t\t\t\t= Diffuse;\nAlphaOp[ 0 ]\t\t\t\t= < Baop >;\nAlphaArg1[ 0 ]\t\t\t\t= Texture;\nAlphaArg2[ 0 ]\t\t\t\t= Diffuse;\nColorOp[ 1 ]\t\t\t\t= < LMcop >;\nColorArg1[ 1 ]\t\t\t\t= Texture;\nColorArg2[ 1 ]\t\t\t\t= Current;\nAlphaOp[ 1 ]\t\t\t\t= < LMaop >;\nAlphaArg1[ 1 ]\t\t\t\t= Texture;\nAlphaArg2[ 1 ]\t\t\t\t= Current;\nColorOp[ 2 ]\t\t\t\t= < TFcop >;\nColorArg1[ 2 ]\t\t\t\t= Current;\nColorArg2[ 2 ]\t\t\t\t= Tfactor;\nAlphaOp[ 2 ]\t\t\t\t= < TFaop >;\nAlphaArg1[ 2 ]\t\t\t\t= Current;\nAlphaArg2[ 2 ]\t\t\t\t= Tfactor;\nTexture[ 3 ]\t\t\t\t= < HFog >;\nTextureTransform[" \
" 3 ]\t\t= < fogmatrix >;\nTexCoordIndex[ 3 ]\t\t\t= CameraSpacePosition | 3;\nTextureTransformFlags[ 3 ]\t= Count2;\nAddressU[ 3 ]\t\t\t\t= Clamp;\nAddressV[ 3 ]\t\t\t\t= Clamp;\nMipfilter[ 3 ]\t\t\t\t= None;\nColorOp[ 3 ]\t\t\t\t= BlendTextureAlpha;\nColorArg1[ 3 ]\t\t\t\t= Texture;\nColorArg2[ 3 ]\t\t\t\t= Current;\nAlphaOp[ 3 ]\t\t\t\t= SelectArg2;\nAlphaArg1[ 3 ]\t\t\t\t= Texture;\nAlphaArg2[ 3 ]\t\t\t\t= Current;\nColorOp[ 4 ]\t\t\t\t= Disable;\nAlphaOp[ 4 ]\t\t\t\t= Disable;\n}\n}\nTechnique ffp_hfog_2pass\n{\nPass p0\n{\nAlphaTestEnable\t\t\t= < alphatest >;\nLighting\t\t\t\t\t= < bLighting >;\nFogEnable\t\t\t\t\t= < bFog >;\nDiffuseMaterialSource\t\t= < DiffuseSource >;\nAmbientMaterialSource\t\t= < DiffuseSource >;\nColorOp[ 0 ]\t\t\t\t= < Bcop >;\nColorArg1[ 0 ]\t\t\t\t= Texture;\nColorArg2[ 0 ]\t\t\t\t= Diffuse;\nAlphaOp[ 0 ]\t\t\t\t= < Baop >;\nAlphaArg1[ 0 ]\t\t\t\t= Texture;\nAlphaArg2[ 0 ]\t\t\t\t= Diffuse;\nColorOp[ 1 ]\t\t\t\t= < LMcop >;\nColorArg1[ 1 ]\t\t\t\t= Texture;\nColorArg2[ 1 ]\t\t\t\t= Current;\nAlphaOp[ 1 ]\t\t\t\t= < LMaop >;\nAlphaArg1[ 1 ]\t\t\t\t=" \
" Texture;\nAlphaArg2[ 1 ]\t\t\t\t= Current;\nColorOp[ 2 ]\t\t\t\t= < TFcop >;\nColorArg1[ 2 ]\t\t\t\t= Current;\nColorArg2[ 2 ]\t\t\t\t= Tfactor;\nAlphaOp[ 2 ]\t\t\t\t= < TFaop >;\nAlphaArg1[ 2 ]\t\t\t\t= Current;\nAlphaArg2[ 2 ]\t\t\t\t= Tfactor;\nColorOp[ 3 ]\t\t\t\t= Disable;\nAlphaOp[ 3 ]\t\t\t\t= Disable;\n}\nPass p1\n{\nAlphaBlendEnable\t\t\t= TRUE;\nSrcBlend\t\t\t\t\t= SrcAlpha;\nDestBlend\t\t\t\t\t= InvSrcAlpha;\nBlendOp\t\t\t\t\t= Add;\nColorOp[ 0 ]\t\t\t\t= SelectArg2;\nColorArg1[ 0 ]\t\t\t\t= Texture;\nColorArg2[ 0 ]\t\t\t\t= Diffuse;\nAlphaOp[ 0 ]\t\t\t\t= < Baop >;\nAlphaArg1[ 0 ]\t\t\t\t= Texture;\nAlphaArg2[ 0 ]\t\t\t\t= Diffuse;\nTexture[ 1 ]\t\t\t\t= < HFog >;\nTextureTransform[ 1 ]\t\t= < fogmatrix >;\nTexCoordIndex[ 1 ]\t\t\t= CameraSpacePosition | 1;\nTextureTransformFlags[ 1 ]\t= Count2;\nAddressU[ 1 ]\t\t\t\t= Clamp;\nAddressV[ 1 ]\t\t\t\t= Clamp;\nMipfilter[ 1 ]\t\t\t\t= None;\nColorOp[ 1 ]\t\t\t\t= SelectArg1;\nColorArg1[ 1 ]\t\t\t\t= Texture;\nColorArg2[ 1 ]\t\t\t\t= Current;\nAlphaOp[ 1 ]\t\t\t\t= Modulate;\nAlphaArg1[ 1 ]\t\t\t\t= Texture;\nAlphaArg2[ 1 ]\t\t\t\t= C" \
"urrent;\nColorOp[ 2 ]\t\t\t\t= Disable;\nAlphaOp[ 2 ]\t\t\t\t= Disable;\n}\n}\nTechnique ffp_hfog_3pass\n{\nPass p0\n{\nAlphaTestEnable\t\t\t= < alphatest >;\nLighting\t\t\t\t\t= < bLighting >;\nFogEnable\t\t\t\t\t= < bFog >;\nDiffuseMaterialSource\t\t= < DiffuseSource >;\nAmbientMaterialSource\t\t= < DiffuseSource >;\nColorOp[ 0 ]\t\t\t\t= < Bcop >;\nColorArg1[ 0 ]\t\t\t\t= Texture;\nColorArg2[ 0 ]\t\t\t\t= Diffuse;\nAlphaOp[ 0 ]\t\t\t\t= < Baop >;\nAlphaArg1[ 0 ]\t\t\t\t= Texture;\nAlphaArg2[ 0 ]\t\t\t\t= Diffuse;\nColorOp[ 1 ]\t\t\t\t= < LMcop >;\nColorArg1[ 1 ]\t\t\t\t= Texture;\nColorArg2[ 1 ]\t\t\t\t= Current;\nAlphaOp[ 1 ]\t\t\t\t= < LMaop >;\nAlphaArg1[ 1 ]\t\t\t\t= Texture;\nAlphaArg2[ 1 ]\t\t\t\t= Current;\nColorOp[ 3 ]\t\t\t\t= Disable;\nAlphaOp[ 3 ]\t\t\t\t= Disable;\n}\nPass p1\n{\nAlphaBlendEnable\t\t\t= TRUE;\nSrcBlend\t\t\t\t\t= DestColor;\nDestBlend\t\t\t\t\t= SrcColor;\nBlendOp\t\t\t\t\t= Add;\nColorOp[ 0 ]\t\t\t\t= SelectArg2;\nColorArg1[ 0 ]\t\t\t\t= Current;\nColorArg2[ 0 ]\t\t\t\t= Tfactor;\nAlphaOp[ 0 ]\t\t\t\t= < TFaop >;\nAlphaArg1[ 0 ]\t\t\t\t= Current;\nAlphaArg2[ 0" \
" ]\t\t\t\t= Tfactor;\nColorOp[ 1 ]\t\t\t\t= Disable;\nAlphaOp[ 1 ]\t\t\t\t= Disable;\n}\nPass p2\n{\nAlphaBlendEnable\t\t\t= TRUE;\nSrcBlend\t\t\t\t\t= SrcAlpha;\nDestBlend\t\t\t\t\t= InvSrcAlpha;\nBlendOp\t\t\t\t\t= Add;\nColorOp[ 0 ]\t\t\t\t= SelectArg2;\nColorArg1[ 0 ]\t\t\t\t= Texture;\nColorArg2[ 0 ]\t\t\t\t= Diffuse;\nAlphaOp[ 0 ]\t\t\t\t= < Baop >;\nAlphaArg1[ 0 ]\t\t\t\t= Texture;\nAlphaArg2[ 0 ]\t\t\t\t= Diffuse;\nTexture[ 1 ]\t\t\t\t= < HFog >;\nTextureTransform[ 1 ]\t\t= < fogmatrix >;\nTexCoordIndex[ 1 ]\t\t\t= CameraSpacePosition | 1;\nTextureTransformFlags[ 1 ]\t= Count2;\nAddressU[ 1 ]\t\t\t\t= Clamp;\nAddressV[ 1 ]\t\t\t\t= Clamp;\nMipfilter[ 1 ]\t\t\t\t= None;\nColorOp[ 1 ]\t\t\t\t= SelectArg1;\nColorArg1[ 1 ]\t\t\t\t= Texture;\nColorArg2[ 1 ]\t\t\t\t= Current;\nAlphaOp[ 1 ]\t\t\t\t= Modulate;\nAlphaArg1[ 1 ]\t\t\t\t= Texture;\nAlphaArg2[ 1 ]\t\t\t\t= Current;\nColorOp[ 2 ]\t\t\t\t= Disable;\nAlphaOp[ 2 ]\t\t\t\t= Disable;\n}\n}\n"
	typedef char FactoredSourceSize[(sizeof(GFX_RS_B_LM_HEIGHT_FOG_FACTORED_SOURCE) - 1 == 4620) ? 1 : -1];


}

Gfx_RS_B_LM_HeightFog::Gfx_RS_B_LM_HeightFog(DWORD color, DWORD alpha,
	DWORD lightmapColor, DWORD lightmapAlpha, DWORD test)
: effect(NULL), colorOperation(color), alphaOperation(alpha),
	lightmapColorOperation(lightmapColor), lightmapAlphaOperation(lightmapAlpha),
	alphaTest(test)
{
}

Gfx_RS_B_LM_HeightFog::~Gfx_RS_B_LM_HeightFog()
{
	Release();
}

HRESULT Gfx_RS_B_LM_HeightFog::Initialize()
{
	HRESULT result = D3DERR_INVALIDCALL;
	++g_RSBaseLightmapHeightFogState.users;
	ID3DXBuffer* errors;
	if (!g_RSBaseLightmapHeightFogState.normal)
	{
		result = D3DXCreateEffect(g_D3D.device, GFX_RS_B_LM_HEIGHT_FOG_NORMAL_SOURCE, sizeof(GFX_RS_B_LM_HEIGHT_FOG_NORMAL_SOURCE) - 1,
			&g_RSBaseLightmapHeightFogState.normal, &errors);
		if (SUCCEEDED(result))
		{
			g_RSBaseLightmapHeightFogState.normal->GetDesc(&g_RSBaseLightmapHeightFogState.description);
			g_RSBaseLightmapHeightFogState.normal->SetTechnique(0);
			D3DXMATRIX identity = g_D3D.identity;
			g_RSBaseLightmapHeightFogState.normal->SetDword("alphatest", 0);
			g_RSBaseLightmapHeightFogState.normal->SetDword("bLighting", 0);
			g_RSBaseLightmapHeightFogState.normal->SetDword("bFog", 0);
			g_RSBaseLightmapHeightFogState.normal->SetDword("Bcop", colorOperation);
			g_RSBaseLightmapHeightFogState.normal->SetDword("Baop", alphaOperation);
			g_RSBaseLightmapHeightFogState.normal->SetDword("LMcop", lightmapColorOperation);
			g_RSBaseLightmapHeightFogState.normal->SetDword("LMaop", lightmapAlphaOperation);
			g_RSBaseLightmapHeightFogState.normal->SetDword("TFcop", 2);
			g_RSBaseLightmapHeightFogState.normal->SetDword("TFaop", 2);
			g_RSBaseLightmapHeightFogState.normal->SetDword("DiffuseSource", 1);
			g_RSBaseLightmapHeightFogState.normal->SetMatrix("fogmatrix", &identity);
			g_RSBaseLightmapHeightFogState.normal->SetTexture("HFog", NULL);
			D3DXTECHNIQUE_DESC selected;
			result = g_RSBaseLightmapHeightFogState.normal->FindNextValidTechnique(0, &selected);
			if (SUCCEEDED(result))
			{
				g_RSBaseLightmapHeightFogState.technique = selected.Index;
				result = g_RSBaseLightmapHeightFogState.normal->SetTechnique(selected.Index);
				if (selected.Name[0] == 's' && selected.Name[1] == 'h') g_RSBaseLightmapHeightFogState.usesShader = 1;
				g_RSBaseLightmapHeightFogState.passes = selected.Passes;
			}
			else
				JE::FatalError("Gfx_RS_B_LM_HeightFog::Create - Failed to find valid technique (%x)\n", result);
		}
		else
		{
			JE::LogDebugString("Gfx_RS_B_LM_HeightFog - Failed to compile\n");
			JE::FatalError(static_cast<const char*>(errors->GetBufferPointer()));
		}
		if (errors)
		{
			errors->Release();
			errors = NULL;
		}
	}
	if (!g_RSBaseLightmapHeightFogState.factored)
	{
		result = D3DXCreateEffect(g_D3D.device, GFX_RS_B_LM_HEIGHT_FOG_FACTORED_SOURCE, sizeof(GFX_RS_B_LM_HEIGHT_FOG_FACTORED_SOURCE) - 1,
			&g_RSBaseLightmapHeightFogState.factored, &errors);
		if (SUCCEEDED(result))
		{
			g_RSBaseLightmapHeightFogState.factored->GetDesc(&g_RSBaseLightmapHeightFogState.description);
			g_RSBaseLightmapHeightFogState.factored->SetTechnique(0);
			D3DXMATRIX identity = g_D3D.identity;
			g_RSBaseLightmapHeightFogState.factored->SetDword("alphatest", 0);
			g_RSBaseLightmapHeightFogState.factored->SetDword("bLighting", 0);
			g_RSBaseLightmapHeightFogState.factored->SetDword("bFog", 0);
			g_RSBaseLightmapHeightFogState.factored->SetDword("Bcop", colorOperation);
			g_RSBaseLightmapHeightFogState.factored->SetDword("Baop", alphaOperation);
			g_RSBaseLightmapHeightFogState.factored->SetDword("LMcop", lightmapColorOperation);
			g_RSBaseLightmapHeightFogState.factored->SetDword("LMaop", lightmapAlphaOperation);
			g_RSBaseLightmapHeightFogState.factored->SetDword("TFcop", 2);
			g_RSBaseLightmapHeightFogState.factored->SetDword("TFaop", 2);
			g_RSBaseLightmapHeightFogState.factored->SetDword("DiffuseSource", 1);
			g_RSBaseLightmapHeightFogState.factored->SetMatrix("fogmatrix", &identity);
			g_RSBaseLightmapHeightFogState.factored->SetTexture("HFog", NULL);
			D3DXTECHNIQUE_DESC selected;
			result = g_RSBaseLightmapHeightFogState.factored->FindNextValidTechnique(0, &selected);
			if (SUCCEEDED(result))
			{
				g_RSBaseLightmapHeightFogState.technique = selected.Index;
				result = g_RSBaseLightmapHeightFogState.factored->SetTechnique(selected.Index);
				if (selected.Name[0] == 's' && selected.Name[1] == 'h') g_RSBaseLightmapHeightFogState.usesShader = 1;
				if (g_RSBaseLightmapHeightFogState.passes < selected.Passes) g_RSBaseLightmapHeightFogState.passes = selected.Passes;
			}
			else
				JE::FatalError("Gfx_RS_B_LM_HeightFog::Create - Failed to find valid technique (%x)\n", result);
		}
		else
		{
			JE::LogDebugString("Gfx_RS_B_LM_HeightFog - Failed to compile\n");
			JE::FatalError(static_cast<const char*>(errors->GetBufferPointer()));
		}
		if (errors)
		{
			errors->Release();
		}
	}
	return result;
}

void Gfx_RS_B_LM_HeightFog::Release()
{
	if (g_RSBaseLightmapHeightFogState.users)
	{
		--g_RSBaseLightmapHeightFogState.users;
		if (!g_RSBaseLightmapHeightFogState.users)
		{
			if (g_RSBaseLightmapHeightFogState.normal)
			{
				g_RSBaseLightmapHeightFogState.normal->Release();
				g_RSBaseLightmapHeightFogState.normal = NULL;
			}
			if (g_RSBaseLightmapHeightFogState.factored)
			{
				g_RSBaseLightmapHeightFogState.factored->Release();
				g_RSBaseLightmapHeightFogState.factored = NULL;
			}
		}
	}
}

HRESULT Gfx_RS_B_LM_HeightFog::OnLostDevice()
{
	HRESULT result = D3DERR_INVALIDCALL;
	if (g_RSBaseLightmapHeightFogState.normal) result = g_RSBaseLightmapHeightFogState.normal->OnLostDevice();
	if (g_RSBaseLightmapHeightFogState.factored) result = g_RSBaseLightmapHeightFogState.factored->OnLostDevice();
	return result;
}

HRESULT Gfx_RS_B_LM_HeightFog::OnResetDevice()
{
	HRESULT result = D3DERR_INVALIDCALL;
	if (g_RSBaseLightmapHeightFogState.normal) result = g_RSBaseLightmapHeightFogState.normal->OnResetDevice();
	if (g_RSBaseLightmapHeightFogState.factored) result = g_RSBaseLightmapHeightFogState.factored->OnResetDevice();
	return result;
}

DWORD Gfx_RS_B_LM_HeightFog::TechniqueCount() { return g_RSBaseLightmapHeightFogState.description.Techniques; }
BYTE Gfx_RS_B_LM_HeightFog::UsesVertexShader() { return g_RSBaseLightmapHeightFogState.usesShader; }
DWORD Gfx_RS_B_LM_HeightFog::PassCount() { return g_RSBaseLightmapHeightFogState.passes; }

DWORD Gfx_RS_B_LM_HeightFog::SelectTechnique(DWORD index)
{
	if (index < g_RSBaseLightmapHeightFogState.description.Techniques &&
		index != reinterpret_cast<DWORD>(g_RSBaseLightmapHeightFogState.technique))
	{
		g_RSBaseLightmapHeightFogState.techniqueChanged = 1;
		g_RSBaseLightmapHeightFogState.technique = reinterpret_cast<LPCSTR>(index);
	}
	return index;
}

DWORD Gfx_RS_B_LM_HeightFog::NextTechnique()
{
	DWORD next = reinterpret_cast<DWORD>(g_RSBaseLightmapHeightFogState.technique) + 1;
	if (next >= g_RSBaseLightmapHeightFogState.description.Techniques) next = 0;
	return SelectTechnique(next);
}

HRESULT Gfx_RS_B_LM_HeightFog::Begin(UINT* passes) { return effect->Begin(passes, 0); }
HRESULT Gfx_RS_B_LM_HeightFog::End() { return effect->End(); }

HRESULT Gfx_RS_B_LM_HeightFog::Pass(UINT pass)
{
	g_RSBaseLightmapHeightFogState.currentPass = pass;
	return effect->Pass(pass);
}

HRESULT Gfx_RS_B_LM_HeightFog::SetMaterial(Gfx_Material* material, DWORD fogBits)
{
	BYTE useRGB = 0, useAlpha = 0;
	DWORD color;
	material->GetColor(useRGB, useAlpha, color);
	if (!useRGB && !useAlpha) effect = g_RSBaseLightmapHeightFogState.normal;
	else effect = g_RSBaseLightmapHeightFogState.factored;
	if (!effect)
	{
		JE::LogDebugString("Gfx_RS_B_LM_HeightFog::SetMaterial() failed - not created yet.");
		return D3DERR_INVALIDCALL;
	}
	Gfx_HeightFog* fog = reinterpret_cast<Gfx_HeightFog*>(fogBits);
	if (!fog)
	{
		JE::LogDebugString("Gfx_RS_B_LM_HeightFog: No HeightFog object specified!");
		return D3DERR_INVALIDCALL;
	}
	const DWORD baseHandle = material->textureHandles[0];
	if (baseHandle)
	{
		Gfx_Texture* texture = g_TextureManager->Lookup(baseHandle);
		if (texture) texture->Bind(0, 0);
		else
		{
			g_D3D.SetTexture(0, NULL);
			JE::LogDebugString("Gfx_RS_B_LM_HeightFog: Cannot find Base Texture %s",
				texture->GetDiagnosticName().c_str());
		}
	}
	else
	{
		g_D3D.SetTexture(0, NULL);
		JE::LogDebugString("Gfx_RS_B_LM_HeightFog: NULL Base Texture handle\n");
	}
	const DWORD lightmapHandle = material->textureHandles[1];
	if (lightmapHandle)
	{
		Gfx_Texture* texture = g_TextureManager->Lookup(lightmapHandle);
		if (texture) texture->Bind(1, 0);
		else
		{
			g_D3D.SetTexture(1, NULL);
			JE::LogDebugString("Gfx_RS_B_LM_HeightFog: Cannot find Lightmap %s",
				texture->GetDiagnosticName().c_str());
		}
	}
	else g_D3D.SetTexture(1, NULL);
	if (fog->value44)
	{
		Gfx_Texture* texture = g_TextureManager->Lookup(fog->value44);
		if (texture)
		{
			if (texture->frameCount <= 0) effect->SetTexture("HFog", NULL);
			else effect->SetTexture("HFog", texture->frames[0].texture);
		}
		else
		{
			g_D3D.SetTexture(2, NULL);
			JE::LogDebugString("Gfx_RS_B_LM_HeightFog: Height Fog map not found!");
		}
	}
	else
	{
		g_D3D.SetTexture(2, NULL);
		JE::LogDebugString("Gfx_RS_B_LM_HeightFog: Height Fog map not created!");
	}
	effect->SetDword("alphatest", alphaTest);
	BOOL lighting = FALSE;
	if (material->flagBC) lighting = g_GfxSystem->Lighting() != 0;
	effect->SetDword("bLighting", lighting);
	BOOL distanceFog = FALSE;
	if (material->flagBD) distanceFog = g_GfxSystem->CurrentViewport()->fogEnabled != 0;
	effect->SetDword("bFog", distanceFog);
	effect->SetDword("DiffuseSource", material->flagB8 != 0);
	effect->SetDword("Bcop", colorOperation);
	effect->SetDword("Baop", alphaOperation);
	effect->SetDword("LMcop", lightmapColorOperation);
	effect->SetDword("LMaop", lightmapAlphaOperation);
	D3DMATERIAL8 values;
	std::memcpy(&values, &material->values, sizeof(values));
	g_D3D.device->SetMaterial(&values);
	if (useRGB || useAlpha)
	{
		effect->SetDword("TFcop", useRGB ? 5 : 2);
		effect->SetDword("TFaop", useAlpha ? 4 : 2);
		if (!useRGB) color = (color & 0xFF808080) | 0x00808080;
		g_D3D.SetRenderState(D3DRS_TEXTUREFACTOR, color);
	}
	else
	{
		effect->SetDword("TFcop", 2);
		effect->SetDword("TFaop", 2);
	}
	HRESULT result;
	if (material->flagB9) result = g_D3D.SetRenderState(D3DRS_CULLMODE, 3);
	else result = g_D3D.SetRenderState(D3DRS_CULLMODE, 1);
	const float seconds = g_GfxSystem->TimeSeconds();
	float offset, thickness;
	fog->Evaluate(seconds, &offset, &thickness);
	D3DXVECTOR3 position;
	std::memcpy(&position, &fog->transform._41, sizeof(position));
	if (g_RSBaseLightmapHeightFogState.usesShader)
	{
		const float constants[4] = { 0.0f, 0.5f, 1.0f, 2.0f };
		g_D3D.device->SetVertexShaderConstant(0, constants, 1);
		BYTE red, green, blue;
		g_GfxSystem->GetAmbientRGB(&red, &green, &blue);
		const float ambient[4] = { red * 0.0039215689f,
			green * 0.0039215689f, blue * 0.0039215689f, 1.0f };
		g_D3D.device->SetVertexShaderConstant(1, ambient, 1);
		g_GfxSystem->CurrentViewport();
		Gfx_Viewport* viewport = g_GfxSystem->CurrentViewport();
		const float range[4] = { viewport->fogEnd,
			1.0f / (viewport->fogEnd - viewport->fogStart), 0.0f, 0.0f };
		g_D3D.device->SetVertexShaderConstant(2, range, 1);
		g_D3D.device->SetVertexShaderConstant(3, &values.Diffuse, 1);
		g_D3D.device->SetVertexShaderConstant(4, &values.Emissive, 1);
		const float fogPosition[4] = { position.x, position.y, position.z, 0.0f };
		g_D3D.device->SetVertexShaderConstant(40, fogPosition, 1);
		const float scale[4] = { 1.0f, 1.0f / thickness,
			0.0066666668f, 0.0034482759f };
		result = g_D3D.device->SetVertexShaderConstant(41, scale, 1);
	}
	else
	{
		D3DXMATRIX map = g_D3D.identity;
		D3DXMATRIX inverseView;
		g_GfxSystem->Camera()->GetInverseViewMatrix(inverseView);
		const float inverse = 1.0f / thickness;
		map._11 = 0.0f;
		map._33 = 0.0f;
		map._22 = inverse;
		map._42 = -((position.y + offset) * inverse);
		D3DXMATRIX product;
		D3DXMatrixMultiply(&product, &inverseView, &map);
		D3DXMATRIX fogMatrix = product;
		effect->SetMatrix("fogmatrix", &fogMatrix);
	}
	const DWORD flags = material->values140[0];
	if (flags)
		SetTextureTransform(0, static_cast<BYTE>(flags), &material->textureParameters[0], seconds);
	else g_D3D.SetTextureStageState(0, D3DTSS_TEXTURETRANSFORMFLAGS, 0);
	if (g_RSBaseLightmapHeightFogState.techniqueChanged)
	{
		D3DXTECHNIQUE_DESC selected;
		result = effect->FindNextValidTechnique(0, &selected);
		if (SUCCEEDED(result))
		{
			g_RSBaseLightmapHeightFogState.techniqueChanged = 0;
			g_RSBaseLightmapHeightFogState.technique = selected.Index;
			g_RSBaseLightmapHeightFogState.passes = selected.Passes;
			result = effect->SetTechnique(selected.Index);
		}
		else JE::FatalError("Gfx_RS_B_LM_HeightFog::SetMaterial - Failed to find valid technique (%x)\n", result);
	}
	return result;
}
