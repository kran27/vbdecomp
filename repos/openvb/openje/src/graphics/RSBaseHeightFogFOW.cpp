#include "RSBaseHeightFogFOW.hpp"
#include "FogOfWarTextureMatrix.hpp"
#include "HeightFog.hpp"
#include "D3DState.hpp"
#include "SystemInterface.hpp"
#include "TextureManager.hpp"
#include "Texture.hpp"
#include "Viewport.hpp"
#include "TextureTransform.hpp"
#include "diagnostics/DebugOutput.hpp"
#include <cstring>

Gfx_RS_BaseHeightFogFOWState g_RSBaseHeightFogFOWState = {0};

namespace
{
	//Exact original effect bytes; split for the supplied VC7 literal limit.

//Original3820-byte effect at6844D0, passed directly to the actual SDK.
#define GFX_RS_BASE_HFOG_FOW_SOURCE \
"DWORD\t\tBcop;\nDWORD\t\tBaop;\nDWORD\t\tTFcop;\nDWORD\t\tTFaop;\nDWORD\t\tFOWcop;\nDWORD\t\tFOWaop;\nDWORD\t\talphatest;\nDWORD\t\tbLighting;\nDWORD\t\tbFog;\nDWORD\t\tDiffuseSource;\nMATRIX\t\tfogmatrix;\nTexture\tHFog;\nMATRIX\t\tFOWmatrix;\nTexture\tFOW;\nDWORD\t\tbZEnable;\nTechnique ffp\n{\nPass p0\n{\nZEnable\t\t\t\t\t= < bZEnable >;\nAlphaTestEnable\t\t\t= < alphatest >;\nLighting\t\t\t\t\t= < bLighting >;\nFogEnable\t\t\t\t\t= < bFog >;\nDiffuseMaterialSource\t\t= < DiffuseSource >;\nAmbientMaterialSource\t\t= < DiffuseSource >;\nColorOp[ 0 ]\t\t\t\t= < Bcop >;\nColorArg1[ 0 ]\t\t\t\t= Texture;\nColorArg2[ 0 ]\t\t\t\t= Diffuse;\nAlphaOp[ 0 ]\t\t\t\t= < Baop >;\nAlphaArg1[ 0 ]\t\t\t\t= Texture;\nAlphaArg2[ 0 ]\t\t\t\t= Diffuse;\nColorOp[ 1 ]\t\t\t\t= < TFcop >;\nColorArg1[ 1 ]\t\t\t\t= Current;\nColorArg2[ 1 ]\t\t\t\t= Tfactor;\nAlphaOp[ 1 ]\t\t\t\t= < TFaop >;\nAlphaArg1[ 1 ]\t\t\t\t= Current;\nAlphaArg2[ 1 ]\t\t\t\t= Tfactor;\nTexture[ 2 ]\t\t\t\t= < HFog >;\nTextureTransform[ 2 ]\t\t= < fogmatrix >;\nTexCoordIndex[ 2 ]\t\t\t= CameraSpacePosition | 2;\nTextureTransformFl" \
"ags[ 2 ]\t= Count2;\nAddressU[ 2 ]\t\t\t\t= Clamp;\nAddressV[ 2 ]\t\t\t\t= Clamp;\nMipfilter[ 2 ]\t\t\t\t= None;\nColorOp[ 2 ]\t\t\t\t= BlendTextureAlpha;\nColorArg1[ 2 ]\t\t\t\t= Texture;\nColorArg2[ 2 ]\t\t\t\t= Current;\nAlphaOp[ 2 ]\t\t\t\t= SelectArg2;\nAlphaArg1[ 2 ]\t\t\t\t= Texture;\nAlphaArg2[ 2 ]\t\t\t\t= Current;\nTexture[ 3 ]\t\t\t\t= < FOW >;\nTextureTransform[ 3 ]\t\t= < FOWmatrix >;\nTexCoordIndex[ 3 ]\t\t\t= CameraSpacePosition | 3;\nTextureTransformFlags[ 3 ]\t= Count2;\nAddressU[ 3 ]\t\t\t\t= Clamp;\nAddressV[ 3 ]\t\t\t\t= Clamp;\nMipfilter[ 3 ]\t\t\t\t= None;\nColorOp[ 3 ]\t\t\t\t= < FOWcop >;\nColorArg1[ 3 ]\t\t\t\t= Current;\nColorArg2[ 3 ]\t\t\t\t= Texture;\nAlphaOp[ 3 ]\t\t\t\t= < FOWaop >;\nAlphaArg1[ 3 ]\t\t\t\t= Current;\nAlphaArg2[ 3 ]\t\t\t\t= Texture;\nColorOp[ 4 ]\t\t\t\t= Disable;\nAlphaOp[ 4 ]\t\t\t\t= Disable;\n}\n}\nTechnique\tffp_3pass\n{\nPass\tp0\n{\nZEnable\t\t\t\t\t= < bZEnable >;\nAlphaTestEnable\t\t\t= < alphatest >;\nLighting\t\t\t\t\t= < bLighting >;\nFogEnable\t\t\t\t\t= < bFog >;\nDiffuseMaterialSource\t\t= < DiffuseSource >;\nAmbientMate" \
"rialSource\t\t= < DiffuseSource >;\nColorOp[ 0 ]\t\t\t\t= < Bcop >;\nColorArg1[ 0 ]\t\t\t\t= Texture;\nColorArg2[ 0 ]\t\t\t\t= Diffuse;\nAlphaOp[ 0 ]\t\t\t\t= <Baop>;\nAlphaArg1[ 0 ]\t\t\t\t= Texture;\nAlphaArg2[ 0 ]\t\t\t\t= Diffuse;\nColorOp[ 1 ]\t\t\t\t= < TFcop >;\nColorArg1[ 1 ]\t\t\t\t= Current;\nColorArg2[ 1 ]\t\t\t\t= Tfactor;\nAlphaOp[ 1 ]\t\t\t\t= < TFaop >;\nAlphaArg1[ 1 ]\t\t\t\t= Current;\nAlphaArg2[ 1 ]\t\t\t\t= Tfactor;\nColorOp[ 2 ]\t\t\t\t= Disable;\nAlphaOp[ 2 ]\t\t\t\t= Disable;\n}\nPass\tp1\n{\nAlphaBlendEnable\t\t\t= TRUE;\nSrcBlend\t\t\t\t\t= SrcAlpha;\nDestBlend\t\t\t\t\t= InvSrcAlpha;\nBlendOp\t\t\t\t\t= Add;\nColorOp[ 0 ]\t\t\t\t= SelectArg2;\nColorArg1[ 0 ]\t\t\t\t= Texture;\nColorArg2[ 0 ]\t\t\t\t= Diffuse;\nAlphaOp[ 0 ]\t\t\t\t= < Baop >;\nAlphaArg1[ 0 ]\t\t\t\t= Texture;\nAlphaArg2[ 0 ]\t\t\t\t= Diffuse;\nTexture[ 1 ]\t\t\t\t= < HFog >;\nTextureTransform[ 1 ]\t\t= < fogmatrix >;\nTexCoordIndex[ 1 ]\t\t\t= CameraSpacePosition | 1;\nTextureTransformFlags[ 1 ]\t= Count2;\nAddressU[ 1 ]\t\t\t\t= Clamp;\nAddressV[ 1 ]\t\t\t\t= Clamp;\nMipfilter[ 1 ]\t\t\t\t= None;\nCo" \
"lorOp[ 1 ]\t\t\t\t= SelectArg1;\nColorArg1[ 1 ]\t\t\t\t= Texture;\nColorArg2[ 1 ]\t\t\t\t= Current;\nAlphaOp[ 1 ]\t\t\t\t= Modulate;\nAlphaArg1[ 1 ]\t\t\t\t= Texture;\nAlphaArg2[ 1 ]\t\t\t\t= Current;\nColorOp[ 2 ]\t\t\t\t= Disable;\nAlphaOp[ 2 ]\t\t\t\t= Disable;\n}\nPass\tp2\n{\nAlphaBlendEnable\t\t\t= TRUE;\nSrcBlend\t\t\t\t\t= DestColor;\nDestBlend\t\t\t\t\t= Zero;\nBlendOp\t\t\t\t\t= Add;\nColorOp[ 0 ]\t\t\t\t= SelectArg2;\nColorArg1[ 0 ]\t\t\t\t= Texture;\nColorArg2[ 0 ]\t\t\t\t= Diffuse;\nAlphaOp[ 0 ]\t\t\t\t= < Baop >;\nAlphaArg1[ 0 ]\t\t\t\t= Texture;\nAlphaArg2[ 0 ]\t\t\t\t= Diffuse;\nTexture[ 1 ]\t\t\t\t= < FOW >;\nTextureTransform[ 1 ]\t\t= < FOWmatrix >;\nTexCoordIndex[ 1 ]\t\t\t= CameraSpacePosition | 1;\nTextureTransformFlags[ 1 ]\t= Count2;\nAddressU[ 1 ]\t\t\t\t= Clamp;\nAddressV[ 1 ]\t\t\t\t= Clamp;\nMipfilter[ 1 ]\t\t\t\t= None;\nColorOp[ 1 ]\t\t\t\t= SelectArg2;\nColorArg1[ 1 ]\t\t\t\t= Current;\nColorArg2[ 1 ]\t\t\t\t= Texture;\nAlphaOp[ 1 ]\t\t\t\t= SelectArg1;\nAlphaArg1[ 1 ]\t\t\t\t= Current;\nAlphaArg2[ 1 ]\t\t\t\t= Texture;\nColorOp[ 2 ]\t\t\t\t= Disable;\nAlphaOp[ 2 " \
"]\t\t\t\t= Disable;\n}\n}\n"
typedef char Source3820[(sizeof(GFX_RS_BASE_HFOG_FOW_SOURCE) - 1 == 3820) ? 1 : -1];

	HRESULT UploadFogConstants(const D3DMATERIAL8& material,
		const D3DXVECTOR3& position, float thickness)
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
			float(1.0 / (double(viewport->fogEnd) - viewport->fogStart)), 0.0f, 0.0f };
		g_D3D.device->SetVertexShaderConstant(2, range, 1);
		g_D3D.device->SetVertexShaderConstant(3, &material.Diffuse, 1);
		g_D3D.device->SetVertexShaderConstant(4, &material.Emissive, 1);
		const float fogPosition[4] = { position.x, position.y, position.z, 0.0f };
		g_D3D.device->SetVertexShaderConstant(40, fogPosition, 1);
		const float scale[4] = { 1.0f, float(1.0 / double(thickness)),
			0.0066666668f, 0.0034482759f };
		return g_D3D.device->SetVertexShaderConstant(41, scale, 1);
	}
}

Gfx_RS_Base_HFog_FOW::Gfx_RS_Base_HFog_FOW(DWORD color, DWORD alpha, DWORD test)
: colorOperation(color), alphaOperation(alpha), alphaTest(test)
{
}

Gfx_RS_Base_HFog_FOW::~Gfx_RS_Base_HFog_FOW()
{
	Release();
}

HRESULT Gfx_RS_Base_HFog_FOW::Initialize()
{
	++g_RSBaseHeightFogFOWState.users;
	if (g_RSBaseHeightFogFOWState.effect) return S_OK;
	ID3DXBuffer* errors;
	HRESULT result;
	result = D3DXCreateEffect(g_D3D.device, GFX_RS_BASE_HFOG_FOW_SOURCE, sizeof(GFX_RS_BASE_HFOG_FOW_SOURCE) - 1,
		&g_RSBaseHeightFogFOWState.effect, &errors);
	if (SUCCEEDED(result))
	{
		g_RSBaseHeightFogFOWState.effect->GetDesc(&g_RSBaseHeightFogFOWState.description);
		g_RSBaseHeightFogFOWState.effect->SetTechnique(0);
		D3DXMATRIX identity = g_D3D.identity;
		g_RSBaseHeightFogFOWState.effect->SetDword("alphatest", 0);
		g_RSBaseHeightFogFOWState.effect->SetDword("bLighting", 0);
		g_RSBaseHeightFogFOWState.effect->SetDword("bFog", 0);
		g_RSBaseHeightFogFOWState.effect->SetDword("Bcop", colorOperation);
		g_RSBaseHeightFogFOWState.effect->SetDword("Baop", alphaOperation);
		g_RSBaseHeightFogFOWState.effect->SetDword("TFcop", 2);
		g_RSBaseHeightFogFOWState.effect->SetDword("TFaop", 2);
		g_RSBaseHeightFogFOWState.effect->SetDword("FOWcop", 4);
		g_RSBaseHeightFogFOWState.effect->SetDword("FOWaop", 2);
		g_RSBaseHeightFogFOWState.effect->SetDword("DiffuseSource", 1);
		g_RSBaseHeightFogFOWState.effect->SetMatrix("fogmatrix", &identity);
		g_RSBaseHeightFogFOWState.effect->SetTexture("HFog", NULL);
		g_RSBaseHeightFogFOWState.effect->SetMatrix("FOWmatrix", &identity);
		g_RSBaseHeightFogFOWState.effect->SetTexture("FOW", NULL);
		g_RSBaseHeightFogFOWState.effect->SetDword("bZEnable", 1);
		D3DXTECHNIQUE_DESC selected;
		result = g_RSBaseHeightFogFOWState.effect->FindNextValidTechnique(g_RSBaseHeightFogFOWState.technique, &selected);
		if (SUCCEEDED(result))
		{
			g_RSBaseHeightFogFOWState.technique = selected.Index;
			result = g_RSBaseHeightFogFOWState.effect->SetTechnique(selected.Index);
			if (selected.Name[0] == 's' && selected.Name[1] == 'h') g_RSBaseHeightFogFOWState.usesShader = 1;
			g_RSBaseHeightFogFOWState.passes = selected.Passes;
		}
		else
			JE::FatalError("Gfx_RS_Base_HFog_FOW::Create - Failed to find valid technique (%x)\n", result);
	}
	else
	{
		JE::LogDebugString("Gfx_RS_Base_HFog_FOW - Failed to compile\n");
		JE::FatalError(static_cast<const char*>(errors->GetBufferPointer()));
	}
	if (errors)
	{
		errors->Release();
	}
	return result;
}

void Gfx_RS_Base_HFog_FOW::Release()
{
	if (g_RSBaseHeightFogFOWState.users)
	{
		--g_RSBaseHeightFogFOWState.users;
		if (!g_RSBaseHeightFogFOWState.users)
		{
			if (g_RSBaseHeightFogFOWState.effect)
			{
				g_RSBaseHeightFogFOWState.effect->Release();
				g_RSBaseHeightFogFOWState.effect = NULL;
			}
		}
	}
}

HRESULT Gfx_RS_Base_HFog_FOW::OnLostDevice()
{
	HRESULT result = D3DERR_INVALIDCALL;
	if (g_RSBaseHeightFogFOWState.effect) result = g_RSBaseHeightFogFOWState.effect->OnLostDevice();
	return result;
}

HRESULT Gfx_RS_Base_HFog_FOW::OnResetDevice()
{
	HRESULT result = D3DERR_INVALIDCALL;
	if (g_RSBaseHeightFogFOWState.effect) result = g_RSBaseHeightFogFOWState.effect->OnResetDevice();
	return result;
}

DWORD Gfx_RS_Base_HFog_FOW::TechniqueCount() { return g_RSBaseHeightFogFOWState.description.Techniques; }
BYTE Gfx_RS_Base_HFog_FOW::UsesVertexShader() { return g_RSBaseHeightFogFOWState.usesShader; }
DWORD Gfx_RS_Base_HFog_FOW::PassCount() { return g_RSBaseHeightFogFOWState.passes; }

DWORD Gfx_RS_Base_HFog_FOW::SelectTechnique(DWORD index)
{
	if (index < g_RSBaseHeightFogFOWState.description.Techniques &&
		index != reinterpret_cast<DWORD>(g_RSBaseHeightFogFOWState.technique))
	{
		g_RSBaseHeightFogFOWState.techniqueChanged = 1;
		g_RSBaseHeightFogFOWState.technique = reinterpret_cast<LPCSTR>(index);
	}
	return index;
}

DWORD Gfx_RS_Base_HFog_FOW::NextTechnique()
{
	DWORD next = reinterpret_cast<DWORD>(g_RSBaseHeightFogFOWState.technique) + 1;
	if (next >= g_RSBaseHeightFogFOWState.description.Techniques) next = 0;
	return SelectTechnique(next);
}

HRESULT Gfx_RS_Base_HFog_FOW::Begin(UINT* passes) { return g_RSBaseHeightFogFOWState.effect->Begin(passes, 0); }
HRESULT Gfx_RS_Base_HFog_FOW::End() { return g_RSBaseHeightFogFOWState.effect->End(); }

HRESULT Gfx_RS_Base_HFog_FOW::Pass(UINT pass)
{
	g_RSBaseHeightFogFOWState.currentPass = pass;
	return g_RSBaseHeightFogFOWState.effect->Pass(pass);
}

HRESULT Gfx_RS_Base_HFog_FOW::SetMaterial(Gfx_Material* material, DWORD fogBits)
{
	if (!g_RSBaseHeightFogFOWState.effect)
	{
		JE::LogDebugString("Gfx_RS_Base_HFog_FOW::SetMaterial() failed - not created yet.");
		return D3DERR_INVALIDCALL;
	}
	Gfx_HeightFog* fog = reinterpret_cast<Gfx_HeightFog*>(fogBits);
	if (!fog)
	{
		JE::LogDebugString("Gfx_RS_Base_HFog_FOW: No HeightFog object specified!");
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
			const std::string name = Gfx_GetTextureDiagnosticName(texture);
			JE::LogDebugString("Gfx_RS_Base_HFog_FOW: Cannot find Base Texture %s", name.c_str());
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
			g_RSBaseHeightFogFOWState.effect->SetTexture("HFog", frame);
		}
		else
		{
			g_RSBaseHeightFogFOWState.effect->SetTexture("HFog", NULL);
			JE::LogDebugString("Gfx_RS_Base_HFog_FOW: Height Fog map not found!");
		}
	}
	else
	{
		g_RSBaseHeightFogFOWState.effect->SetTexture("HFog", NULL);
		JE::LogDebugString("Gfx_RS_Base_HFog_FOW: Height Fog map not created!");
	}
	IDirect3DTexture8* fowTexture = g_GfxSystem->FogOfWarRenderTexture();
	g_RSBaseHeightFogFOWState.effect->SetTexture("FOW", fowTexture);
	g_RSBaseHeightFogFOWState.effect->SetDword("FOWcop", fowTexture ? 4 : 1);
	g_RSBaseHeightFogFOWState.effect->SetDword("FOWaop", fowTexture ? 2 : 1);
	g_RSBaseHeightFogFOWState.effect->SetDword("bZEnable", material->flagBB);
	g_RSBaseHeightFogFOWState.effect->SetDword("alphatest", alphaTest);
	BOOL lighting = material->flagBC ? g_GfxSystem->Lighting() != 0 : FALSE;
	g_RSBaseHeightFogFOWState.effect->SetDword("bLighting", lighting);
	BOOL distanceFog = material->flagBD ? g_GfxSystem->CurrentViewport()->fogEnabled != 0 : FALSE;
	g_RSBaseHeightFogFOWState.effect->SetDword("bFog", distanceFog);
	BYTE useRGB = 0, useAlpha = 0;
	DWORD color;
	material->GetColor(useRGB, useAlpha, color);
	g_RSBaseHeightFogFOWState.effect->SetDword("DiffuseSource", material->flagB8 != 0);
	g_RSBaseHeightFogFOWState.effect->SetDword("Bcop", colorOperation);
	g_RSBaseHeightFogFOWState.effect->SetDword("Baop", alphaOperation);
	D3DMATERIAL8 values;
	std::memcpy(&values, &material->values, sizeof(values));
	g_D3D.device->SetMaterial(&values);
	if (useRGB || useAlpha)
	{
		g_RSBaseHeightFogFOWState.effect->SetDword("TFcop", useRGB ? 5 : 2);
		g_RSBaseHeightFogFOWState.effect->SetDword("TFaop", useAlpha ? 4 : 2);
		g_D3D.SetRenderState(D3DRS_TEXTUREFACTOR, color);
	}
	else
	{
		g_RSBaseHeightFogFOWState.effect->SetDword("TFcop", 2);
		g_RSBaseHeightFogFOWState.effect->SetDword("TFaop", 2);
	}
	HRESULT result = g_D3D.SetRenderState(D3DRS_CULLMODE, material->flagB9 ? 3 : 1);
	D3DXMATRIX fowMatrix;
	Gfx_BuildFogOfWarTextureMatrix(fowMatrix);
	g_RSBaseHeightFogFOWState.effect->SetMatrix("FOWmatrix", &fowMatrix);
	const float seconds = g_GfxSystem->TimeSeconds();
	float offset, thickness;
	fog->Evaluate(seconds, &offset, &thickness);
	const D3DXVECTOR3 position(fog->transform._41, fog->transform._42, fog->transform._43);
	if (g_RSBaseHeightFogFOWState.usesShader)
	{
		result = UploadFogConstants(values, position, thickness);
	}
	else
	{
		D3DXMATRIX map = g_D3D.identity;
		D3DXMATRIX inverseView;
		g_GfxSystem->Camera()->GetInverseViewMatrix(inverseView);
		const double inverse = 1.0 / double(thickness);
		map._11 = 0.0f;
		map._33 = 0.0f;
		map._22 = float(inverse);
		map._42 = float(-((double(position.y) + offset) * inverse));
		D3DXMATRIX product;
		D3DXMatrixMultiply(&product, &inverseView, &map);
		D3DXMATRIX fogMatrix = product;
		g_RSBaseHeightFogFOWState.effect->SetMatrix("fogmatrix", &fogMatrix);
	}
	const DWORD flags = material->values140[0];
	if (flags)
		Gfx_SetTextureTransform(0, static_cast<BYTE>(flags), &material->textureParameters[0], seconds);
	else g_D3D.SetTextureStageState(0, D3DTSS_TEXTURETRANSFORMFLAGS, 0);
	if (g_RSBaseHeightFogFOWState.techniqueChanged)
	{
		D3DXTECHNIQUE_DESC selected;
		result = g_RSBaseHeightFogFOWState.effect->FindNextValidTechnique(g_RSBaseHeightFogFOWState.technique, &selected);
		if (FAILED(result))
			JE::FatalError("Gfx_RS_Base_HFog_FOW::SetMaterial - Failed to find valid technique (%x)\n", result);
		g_RSBaseHeightFogFOWState.techniqueChanged = 0;
		g_RSBaseHeightFogFOWState.technique = selected.Index;
		g_RSBaseHeightFogFOWState.passes = selected.Passes;
	}
	return result;
}
