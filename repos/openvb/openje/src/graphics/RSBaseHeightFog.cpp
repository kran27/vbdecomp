#include "RSBaseHeightFog.hpp"
#include "HeightFog.hpp"
#include "D3DState.hpp"
#include "SystemInterface.hpp"
#include "TextureManager.hpp"
#include "Texture.hpp"
#include "Viewport.hpp"
#include "TextureTransform.hpp"
#include "diagnostics/DebugOutput.hpp"
#include <cstring>

Gfx_RS_BaseHeightFogState g_RSBaseHeightFogState = {0};

namespace
{
	//Exact original effect bytes; split for the supplied VC7 literal limit.
#define GFX_RS_BASE_HEIGHTFOG_SOURCE \
	"DWORD\t\tBcop;\nDWORD\t\tBaop;\nDWORD\t\tTFcop;\nDWORD\t\tTFaop;\nDWORD\t\talphatest;\nDWORD\t\tbLighting;\nDWORD\t\tbFog;\nDWORD\t\tDiffuseSource;\nMATRIX\t\tfogmatrix;\nTexture\tHFog;\nDWORD\t\tbZEnable;\nTechnique ffp\n{\nPass p0\n{\nZEnable\t\t\t\t\t= < bZEnable >;\nAlphaTestEnable\t\t\t= < alphatest >;\nLighting\t\t\t\t\t= < bLighting >;\nFogEnable\t\t\t\t\t= < bFog >;\nDiffuseMaterialSource\t\t= < DiffuseSource >;\nAmbientMaterialSource\t\t= < DiffuseSource >;\nColorOp[ 0 ]\t\t\t\t= < Bcop >;\nColorArg1[ 0 ]\t\t\t\t= Texture;\nColorArg2[ 0 ]\t\t\t\t= Diffuse;\nAlphaOp[ 0 ]\t\t\t\t= < Baop >;\nAlphaArg1[ 0 ]\t\t\t\t= Texture;\nAlphaArg2[ 0 ]\t\t\t\t= Diffuse;\nColorOp[ 1 ]\t\t\t\t= < TFcop >;\nColorArg1[ 1 ]\t\t\t\t= Current;\nColorArg2[ 1 ]\t\t\t\t= Tfactor;\nAlphaOp[ 1 ]\t\t\t\t= < TFaop >;\nAlphaArg1[ 1 ]\t\t\t\t= Current;\nAlphaArg2[ 1 ]\t\t\t\t= Tfactor;\nTexture[ 2 ]\t\t\t\t= < HFog >;\nTextureTransform[ 2 ]\t\t= < fogmatrix >;\nTexCoordIndex[ 2 ]\t\t\t= CameraSpacePosition | 2;\nTextureTransformFlags[ 2 ]\t= Count2;\nAddressU[ 2 ]\t\t\t\t= Clamp;\nAddressV[ 2 ]\t\t\t\t" \
	"= Clamp;\nMipfilter[ 2 ]\t\t\t\t= None;\nColorOp[ 2 ]\t\t\t\t= BlendTextureAlpha;\nColorArg1[ 2 ]\t\t\t\t= Texture;\nColorArg2[ 2 ]\t\t\t\t= Current;\nAlphaOp[ 2 ]\t\t\t\t= SelectArg2;\nAlphaArg1[ 2 ]\t\t\t\t= Texture;\nAlphaArg2[ 2 ]\t\t\t\t= Current;\nColorOp[ 3 ]\t\t\t\t= Disable;\nAlphaOp[ 3 ]\t\t\t\t= Disable;\n}\n}\nTechnique ffp_2pass\n{\nPass p0\n{\nZEnable\t\t\t\t\t= < bZEnable >;\nAlphaTestEnable\t\t\t= < alphatest >;\nLighting\t\t\t\t\t= < bLighting >;\nFogEnable\t\t\t\t\t= < bFog >;\nDiffuseMaterialSource\t\t= < DiffuseSource >;\nAmbientMaterialSource\t\t= < DiffuseSource >;\nColorOp[ 0 ]\t\t\t\t= < Bcop >;\nColorArg1[ 0 ]\t\t\t\t= Texture;\nColorArg2[ 0 ]\t\t\t\t= Diffuse;\nAlphaOp[ 0 ]\t\t\t\t= < Baop >;\nAlphaArg1[ 0 ]\t\t\t\t= Texture;\nAlphaArg2[ 0 ]\t\t\t\t= Diffuse;\nColorOp[ 1 ]\t\t\t\t= < TFcop >;\nColorArg1[ 1 ]\t\t\t\t= Current;\nColorArg2[ 1 ]\t\t\t\t= Tfactor;\nAlphaOp[ 1 ]\t\t\t\t= < TFaop >;\nAlphaArg1[ 1 ]\t\t\t\t= Current;\nAlphaArg2[ 1 ]\t\t\t\t= Tfactor;\nColorOp[ 2 ]\t\t\t\t= Disable;\nAlphaOp[ 2 ]\t\t\t\t= Disable;\n}\nPass p1\n{\nAlphaBlendEnable\t\t\t= TR" \
	"UE;\nSrcBlend\t\t\t\t\t= SrcAlpha;\nDestBlend\t\t\t\t\t= InvSrcAlpha;\nBlendOp\t\t\t\t\t= Add;\nColorOp[ 0 ]\t\t\t\t= SelectArg2;\nColorArg1[ 0 ]\t\t\t\t= Texture;\nColorArg2[ 0 ]\t\t\t\t= Diffuse;\nAlphaOp[ 0 ]\t\t\t\t= < Baop >;\nAlphaArg1[ 0 ]\t\t\t\t= Texture;\nAlphaArg2[ 0 ]\t\t\t\t= Diffuse;\nTexture[ 1 ]\t\t\t\t= < HFog >;\nTextureTransform[ 1 ]\t\t= < fogmatrix >;\nTexCoordIndex[ 1 ]\t\t\t= CameraSpacePosition | 1;\nTextureTransformFlags[ 1 ]\t= Count2;\nAddressU[ 1 ]\t\t\t\t= Clamp;\nAddressV[ 1 ]\t\t\t\t= Clamp;\nMipfilter[ 1 ]\t\t\t\t= None;\nColorOp[ 1 ]\t\t\t\t= SelectArg1;\nColorArg1[ 1 ]\t\t\t\t= Texture;\nColorArg2[ 1 ]\t\t\t\t= Current;\nAlphaOp[ 1 ]\t\t\t\t= Modulate;\nAlphaArg1[ 1 ]\t\t\t\t= Texture;\nAlphaArg2[ 1 ]\t\t\t\t= Current;\nColorOp[ 2 ]\t\t\t\t= Disable;\nAlphaOp[ 2 ]\t\t\t\t= Disable;\n}\n}\n"


}

Gfx_RS_Base_HeightFog::Gfx_RS_Base_HeightFog(DWORD color, DWORD alpha, DWORD test)
: colorOperation(color), alphaOperation(alpha), alphaTest(test)
{
}

Gfx_RS_Base_HeightFog::~Gfx_RS_Base_HeightFog()
{
	Release();
}

HRESULT Gfx_RS_Base_HeightFog::Initialize()
{
	++g_RSBaseHeightFogState.users;
	if (g_RSBaseHeightFogState.effect) return S_OK;
	ID3DXBuffer* errors;
	HRESULT result = D3DXCreateEffect(g_D3D.device, GFX_RS_BASE_HEIGHTFOG_SOURCE, sizeof(GFX_RS_BASE_HEIGHTFOG_SOURCE) - 1,
		&g_RSBaseHeightFogState.effect, &errors);
	if (SUCCEEDED(result))
	{
		g_RSBaseHeightFogState.effect->GetDesc(&g_RSBaseHeightFogState.description);
		g_RSBaseHeightFogState.effect->SetTechnique(0);
		D3DXMATRIX identity = g_D3D.identity;
		g_RSBaseHeightFogState.effect->SetDword("alphatest", 0);
		g_RSBaseHeightFogState.effect->SetDword("bLighting", 0);
		g_RSBaseHeightFogState.effect->SetDword("bFog", 0);
		g_RSBaseHeightFogState.effect->SetDword("Bcop", colorOperation);
		g_RSBaseHeightFogState.effect->SetDword("Baop", alphaOperation);
		g_RSBaseHeightFogState.effect->SetDword("TFcop", 2);
		g_RSBaseHeightFogState.effect->SetDword("TFaop", 2);
		g_RSBaseHeightFogState.effect->SetDword("DiffuseSource", 1);
		g_RSBaseHeightFogState.effect->SetMatrix("fogmatrix", &identity);
		g_RSBaseHeightFogState.effect->SetTexture("HFog", NULL);
		g_RSBaseHeightFogState.effect->SetDword("bZEnable", 1);
		D3DXTECHNIQUE_DESC selected;
		result = g_RSBaseHeightFogState.effect->FindNextValidTechnique(g_RSBaseHeightFogState.technique, &selected);
		if (SUCCEEDED(result))
		{
			g_RSBaseHeightFogState.technique = selected.Index;
			result = g_RSBaseHeightFogState.effect->SetTechnique(selected.Index);
			if (selected.Name[0] == 's' && selected.Name[1] == 'h') g_RSBaseHeightFogState.usesShader = 1;
			g_RSBaseHeightFogState.passes = selected.Passes;
		}
		else JE::FatalError("Gfx_RS_Base_HeightFog::Create - Failed to find valid technique (%x)\n", result);
	}
	else
	{
		JE::LogDebugString("Gfx_RS_Base_HeightFog - Failed to compile\n");
		JE::FatalError(static_cast<const char*>(errors->GetBufferPointer()));
	}
	if (errors) errors->Release();
	return result;
}

#undef GFX_RS_BASE_HEIGHTFOG_SOURCE

void Gfx_RS_Base_HeightFog::Release()
{
	if (g_RSBaseHeightFogState.users)
	{
		--g_RSBaseHeightFogState.users;
		if (!g_RSBaseHeightFogState.users)
		{
			if (g_RSBaseHeightFogState.effect)
			{
				g_RSBaseHeightFogState.effect->Release();
				g_RSBaseHeightFogState.effect = NULL;
			}
		}
	}
}

HRESULT Gfx_RS_Base_HeightFog::OnLostDevice()
{
	HRESULT result = D3DERR_INVALIDCALL;
	if (g_RSBaseHeightFogState.effect) result = g_RSBaseHeightFogState.effect->OnLostDevice();
	return result;
}

HRESULT Gfx_RS_Base_HeightFog::OnResetDevice()
{
	HRESULT result = D3DERR_INVALIDCALL;
	if (g_RSBaseHeightFogState.effect) result = g_RSBaseHeightFogState.effect->OnResetDevice();
	return result;
}

DWORD Gfx_RS_Base_HeightFog::TechniqueCount() { return g_RSBaseHeightFogState.description.Techniques; }
BYTE Gfx_RS_Base_HeightFog::UsesVertexShader() { return g_RSBaseHeightFogState.usesShader; }
DWORD Gfx_RS_Base_HeightFog::PassCount() { return g_RSBaseHeightFogState.passes; }

DWORD Gfx_RS_Base_HeightFog::SelectTechnique(DWORD index)
{
	if (index < g_RSBaseHeightFogState.description.Techniques &&
		index != reinterpret_cast<DWORD>(g_RSBaseHeightFogState.technique))
	{
		g_RSBaseHeightFogState.techniqueChanged = 1;
		g_RSBaseHeightFogState.technique = reinterpret_cast<LPCSTR>(index);
	}
	return index;
}

DWORD Gfx_RS_Base_HeightFog::NextTechnique()
{
	DWORD next = reinterpret_cast<DWORD>(g_RSBaseHeightFogState.technique) + 1;
	if (next >= g_RSBaseHeightFogState.description.Techniques) next = 0;
	return SelectTechnique(next);
}

HRESULT Gfx_RS_Base_HeightFog::Begin(UINT* passes) { return g_RSBaseHeightFogState.effect->Begin(passes, 0); }
HRESULT Gfx_RS_Base_HeightFog::End() { return g_RSBaseHeightFogState.effect->End(); }

HRESULT Gfx_RS_Base_HeightFog::Pass(UINT pass)
{
	g_RSBaseHeightFogState.currentPass = pass;
	return g_RSBaseHeightFogState.effect->Pass(pass);
}

HRESULT Gfx_RS_Base_HeightFog::SetMaterial(Gfx_Material* material, DWORD fogBits)
{
	if (!g_RSBaseHeightFogState.effect)
	{
		JE::LogDebugString("Gfx_RS_Base_HeightFog::SetMaterial() failed - not created yet.");
		return D3DERR_INVALIDCALL;
	}
	Gfx_HeightFog* fog = reinterpret_cast<Gfx_HeightFog*>(fogBits);
	if (!fog)
	{
		JE::LogDebugString("Gfx_RS_Base_HeightFog: No HeightFog object specified!");
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
			JE::LogDebugString("Gfx_RS_Base_HeightFog: Cannot find Base Texture %s",
				texture->GetDiagnosticName().c_str());
		}
	}
	else
	{
		g_D3D.SetTexture(0, NULL);
	}
	if (fog->value44)
	{
		Gfx_Texture* texture = g_TextureManager->Lookup(fog->value44);
		if (texture)
		{
			IDirect3DTexture8* frame = texture->frameCount > 0 ? texture->frames[0].texture : NULL;
			g_RSBaseHeightFogState.effect->SetTexture("HFog", frame);
		}
		else
		{
			g_RSBaseHeightFogState.effect->SetTexture("HFog", NULL);
			JE::LogDebugString("Gfx_RS_Base_HeightFog: Height Fog map not found!");
		}
	}
	else
	{
		g_RSBaseHeightFogState.effect->SetTexture("HFog", NULL);
		JE::LogDebugString("Gfx_RS_Base_HeightFog: Height Fog map not created!");
	}
	g_RSBaseHeightFogState.effect->SetDword("bZEnable", material->flagBB);
	g_RSBaseHeightFogState.effect->SetDword("alphatest", alphaTest);
	BOOL lighting = material->flagBC ? g_GfxSystem->Lighting() != 0 : FALSE;
	g_RSBaseHeightFogState.effect->SetDword("bLighting", lighting);
	BOOL distanceFog = material->flagBD ? g_GfxSystem->CurrentViewport()->fogEnabled != 0 : FALSE;
	g_RSBaseHeightFogState.effect->SetDword("bFog", distanceFog);
	BYTE useRGB = 0, useAlpha = 0;
	DWORD color;
	material->GetColor(useRGB, useAlpha, color);
	g_RSBaseHeightFogState.effect->SetDword("DiffuseSource", material->flagB8 != 0);
	g_RSBaseHeightFogState.effect->SetDword("Bcop", colorOperation);
	g_RSBaseHeightFogState.effect->SetDword("Baop", alphaOperation);
	D3DMATERIAL8 values;
	std::memcpy(&values, &material->values, sizeof(values));
	g_D3D.device->SetMaterial(&values);
	if (!useRGB && !useAlpha)
	{
		g_RSBaseHeightFogState.effect->SetDword("TFcop", 2);
		g_RSBaseHeightFogState.effect->SetDword("TFaop", 2);
	}
	else
	{
		g_RSBaseHeightFogState.effect->SetDword("TFcop", useRGB ? 5 : 2);
		g_RSBaseHeightFogState.effect->SetDword("TFaop", useAlpha ? 4 : 2);
		g_D3D.SetRenderState(D3DRS_TEXTUREFACTOR, color);
	}
	HRESULT result;
	if (material->flagB9) result = g_D3D.SetRenderState(D3DRS_CULLMODE, 3);
	else result = g_D3D.SetRenderState(D3DRS_CULLMODE, 1);
	const float seconds = g_GfxSystem->TimeSeconds();
	float offset, thickness;
	fog->Evaluate(seconds, &offset, &thickness);
	D3DXVECTOR3 position;
	position.x = fog->transform._41;
	position.y = fog->transform._42;
	position.z = fog->transform._43;
	if (g_RSBaseHeightFogState.usesShader)
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
		Gfx_Camera* camera = g_GfxSystem->Camera();
		camera->GetInverseViewMatrix(inverseView);
		const float inverse = 1.0f / thickness;
		map._11 = 0.0f;
		map._33 = 0.0f;
		map._22 = inverse;
		map._42 = -((position.y + offset) * inverse);
		D3DXMATRIX product;
		D3DXMatrixMultiply(&product, &inverseView, &map);
		D3DXMATRIX fogMatrix = product;
		g_RSBaseHeightFogState.effect->SetMatrix("fogmatrix", &fogMatrix);
	}
	const DWORD flags = material->values140[0];
	if (flags)
		SetTextureTransform(0, static_cast<BYTE>(flags), &material->textureParameters[0], seconds);
	else g_D3D.SetTextureStageState(0, D3DTSS_TEXTURETRANSFORMFLAGS, 0);
	if (g_RSBaseHeightFogState.techniqueChanged)
	{
		D3DXTECHNIQUE_DESC selected;
		result = g_RSBaseHeightFogState.effect->FindNextValidTechnique(g_RSBaseHeightFogState.technique, &selected);
		if (SUCCEEDED(result))
		{
			g_RSBaseHeightFogState.techniqueChanged = 0;
			g_RSBaseHeightFogState.technique = selected.Index;
			g_RSBaseHeightFogState.passes = selected.Passes;
		}
		else JE::FatalError("Gfx_RS_Base_HeightFog::SetMaterial - Failed to find valid technique (%x)\n", result);
	}
	return result;
}
