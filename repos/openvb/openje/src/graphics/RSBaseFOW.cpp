#include "RSBaseFOW.hpp"
#include "D3DState.hpp"
#include "SystemInterface.hpp"
#include "TextureManager.hpp"
#include "Texture.hpp"
#include "Viewport.hpp"
#include "TextureTransform.hpp"
#include "Camera.hpp"
#include "diagnostics/DebugOutput.hpp"
#include <cstring>

Gfx_RS_BaseFOWState g_RSBaseFOWState = {0};

namespace
{
#define GFX_RS_BASE_FOW_NORMAL_SOURCE \
"DWORD\t\tBcop;\n" \
"DWORD\t\tBaop;\n" \
"DWORD\t\tFOWcop;\n" \
"DWORD\t\tFOWaop;\n" \
"DWORD\t\tTFcop;\n" \
"DWORD\t\tTFaop;\n" \
"DWORD\t\talphatest;\n" \
"DWORD\t\tbLighting;\n" \
"DWORD\t\tbFog;\n" \
"DWORD\t\tDiffuseSource;\n" \
"MATRIX\t\tFOWmatrix;\n" \
"Texture\tFOW;\n" \
"DWORD\t\tbZEnable;\n" \
"Technique\tffp\n" \
"{\n" \
"Pass\tp0\n" \
"{\n" \
"ZEnable\t\t\t\t\t= < bZEnable >;\n" \
"AlphaTestEnable\t\t\t= < alphatest >;\n" \
"Lighting\t\t\t\t\t= < bLighting >;\n" \
"FogEnable\t\t\t\t\t= < bFog >;\n" \
"DiffuseMaterialSource\t\t= < DiffuseSource >;\n" \
"AmbientMaterialSource\t\t= < DiffuseSource >;\n" \
"ColorOp[ 0 ]\t\t\t\t= < Bcop >;\n" \
"ColorArg1[ 0 ]\t\t\t\t= Texture;\n" \
"ColorArg2[ 0 ]\t\t\t\t= Diffuse;\n" \
"AlphaOp[ 0 ]\t\t\t\t= < Baop >;\n" \
"AlphaArg1[ 0 ]\t\t\t\t= Texture;\n" \
"AlphaArg2[ 0 ]\t\t\t\t= Diffuse;\n" \
"Texture[ 1 ]\t\t\t\t= < FOW >;\n" \
"TextureTransform[ 1 ]\t\t= < FOWmatrix >;\n" \
"TexCoordIndex[ 1 ]\t\t\t= CameraSpacePosition | 1;\n" \
"TextureTransformFlags[ 1 ]\t= Count2;\n" \
"AddressU[ 1 ]\t\t\t\t= Clamp;\n" \
"AddressV[ 1 ]\t\t\t\t= Clamp;\n" \
"Mipfilter[ 1 ]\t\t\t\t= None;\n" \
"ColorOp[ 1 ]\t\t\t\t= < FOWcop >;\n" \
"ColorArg1[ 1 ]\t\t\t\t= Current;\n" \
"ColorArg2[ 1 ]\t\t\t\t= Texture;\n" \
"AlphaOp[ 1 ]\t\t\t\t= < FOWaop >;\n" \
"AlphaArg1[ 1 ]\t\t\t\t= Current;\n" \
"AlphaArg2[ 1 ]\t\t\t\t= Texture;\n" \
"ColorOp[ 2 ]\t\t\t\t= Disable;\n" \
"AlphaOp[ 2 ]\t\t\t\t= Disable;\n" \
"}\n" \
"}\n"
#define GFX_RS_BASE_FOW_FACTORED_SOURCE \
"DWORD\t\tBcop;\n" \
"DWORD\t\tBaop;\n" \
"DWORD\t\tFOWcop;\n" \
"DWORD\t\tFOWaop;\n" \
"DWORD\t\tTFcop;\n" \
"DWORD\t\tTFaop;\n" \
"DWORD\t\talphatest;\n" \
"DWORD\t\tbLighting;\n" \
"DWORD\t\tbFog;\n" \
"DWORD\t\tDiffuseSource;\n" \
"MATRIX\t\tFOWmatrix;\n" \
"Texture\tFOW;\n" \
"DWORD\t\tbZEnable;\n" \
"Technique\tffp\n" \
"{\n" \
"Pass\tp0\n" \
"{\n" \
"ZEnable\t\t\t\t\t= < bZEnable >;\n" \
"AlphaTestEnable\t\t\t= < alphatest >;\n" \
"Lighting\t\t\t\t\t= < bLighting >;\n" \
"FogEnable\t\t\t\t\t= < bFog >;\n" \
"DiffuseMaterialSource\t\t= < DiffuseSource >;\n" \
"AmbientMaterialSource\t\t= < DiffuseSource >;\n" \
"ColorOp[ 0 ]\t\t\t\t= < Bcop >;\n" \
"ColorArg1[ 0 ]\t\t\t\t= Texture;\n" \
"ColorArg2[ 0 ]\t\t\t\t= Diffuse;\n" \
"AlphaOp[ 0 ]\t\t\t\t= < Baop >;\n" \
"AlphaArg1[ 0 ]\t\t\t\t= Texture;\n" \
"AlphaArg2[ 0 ]\t\t\t\t= Diffuse;\n" \
"ColorOp[ 1 ]\t\t\t\t= < TFcop >;\n" \
"ColorArg1[ 1 ]\t\t\t\t= Current;\n" \
"ColorArg2[ 1 ]\t\t\t\t= Tfactor;\n" \
"AlphaOp[ 1 ]\t\t\t\t= < TFaop >;\n" \
"AlphaArg1[ 1 ]\t\t\t\t= Current;\n" \
"AlphaArg2[ 1 ]\t\t\t\t= Tfactor;\n" \
"Texture[ 2 ]\t\t\t\t= < FOW >;\n" \
"TextureTransform[ 2 ]\t\t= < FOWmatrix >;\n" \
"TexCoordIndex[ 2 ]\t\t\t= CameraSpacePosition | 2;\n" \
"TextureTransformFlags[ 2 ]\t= Count2;\n" \
"AddressU[ 2 ]\t\t\t\t= Clamp;\n" \
"AddressV[ 2 ]\t\t\t\t= Clamp;\n" \
"Mipfilter[ 2 ]\t\t\t\t= None;\n" \
"ColorOp[ 2 ]\t\t\t\t= < FOWcop >;\n" \
"ColorArg1[ 2 ]\t\t\t\t= Current;\n" \
"ColorArg2[ 2 ]\t\t\t\t= Texture;\n" \
"AlphaOp[ 2 ]\t\t\t\t= < FOWaop >;\n" \
"AlphaArg1[ 2 ]\t\t\t\t= Current;\n" \
"AlphaArg2[ 2 ]\t\t\t\t= Texture;\n" \
"ColorOp[ 3 ]\t\t\t= Disable;\n" \
"AlphaOp[ 3 ]\t\t\t= Disable;\n" \
"}\n" \
"}\n" \
"Technique\tffp_2pass\n" \
"{\n" \
"Pass\tp0\n" \
"{\n" \
"ZEnable\t\t\t\t\t= < bZEnable >;\n" \
"AlphaTestEnable\t\t\t= < alphatest >;\n" \
"Lighting\t\t\t\t\t= < bLighting >;\n" \
"FogEnable\t\t\t\t\t= < bFog >;\n" \
"DiffuseMaterialSource\t\t= < DiffuseSource >;\n" \
"AmbientMaterialSource\t\t= < DiffuseSource >;\n" \
"Texture[ 1 ]\t\t\t\t= < FOW >;\n" \
"ColorOp[ 0 ]\t\t\t\t= < Bcop >;\n" \
"ColorArg1[ 0 ]\t\t\t\t= Texture;\n" \
"ColorArg2[ 0 ]\t\t\t\t= Diffuse;\n" \
"AlphaOp[ 0 ]\t\t\t\t= < Baop >;\n" \
"AlphaArg1[ 0 ]\t\t\t\t= Texture;\n" \
"AlphaArg2[ 0 ]\t\t\t\t= Diffuse;\n" \
"ColorOp[ 1 ]\t\t\t\t= < TFcop >;\n" \
"ColorArg1[ 1 ]\t\t\t\t= Current;\n" \
"ColorArg2[ 1 ]\t\t\t\t= Tfactor;\n" \
"AlphaOp[ 1 ]\t\t\t\t= < TFaop >;\n" \
"AlphaArg1[ 1 ]\t\t\t\t= Current;\n" \
"AlphaArg2[ 1 ]\t\t\t\t= Tfactor;\n" \
"}\n" \
"Pass\tp1\n" \
"{\n" \
"AlphaBlendEnable\t\t= TRUE;\n" \
"SrcBlend\t\t\t\t= DestColor;\n" \
"DestBlend\t\t\t\t= Zero;\n" \
"BlendOp\t\t\t\t= Add;\n" \
"Texture[ 0 ]\t\t\t\t= < FOW >;\n" \
"TextureTransform[ 0 ]\t\t= < FOWmatrix >;\n" \
"TexCoordIndex[ 0 ]\t\t\t= CameraSpacePosition | 0;\n" \
"TextureTransformFlags[ 0 ]\t= Count2;\n" \
"AddressU[ 0 ]\t\t\t\t= Clamp;\n" \
"AddressV[ 0 ]\t\t\t\t= Clamp;\n" \
"Mipfilter[ 0 ]\t\t\t\t= None;\n" \
"ColorOp[ 0 ]\t\t\t\t= SelectArg2;\n" \
"ColorArg1[ 0 ]\t\t\t\t= Current;\n" \
"ColorArg2[ 0 ]\t\t\t\t= Texture;\n" \
"AlphaOp[ 0 ]\t\t\t\t= SelectArg2;\n" \
"AlphaArg1[ 0 ]\t\t\t\t= Current;\n" \
"AlphaArg2[ 0 ]\t\t\t\t= Texture;\n" \
"ColorOp[ 1 ]\t\t\t\t= Disable;\n" \
"AlphaOp[ 1 ]\t\t\t\t= Disable;\n" \
"}\n" \
"}\n"
	typedef char Source1073[(sizeof(GFX_RS_BASE_FOW_NORMAL_SOURCE)-1 == 1073) ? 1 : -1];
	typedef char Source2423[(sizeof(GFX_RS_BASE_FOW_FACTORED_SOURCE)-1 == 2423) ? 1 : -1];
}

Gfx_RS_Base_FOW::Gfx_RS_Base_FOW(DWORD color, DWORD alpha, DWORD test)
{
	colorOperation = color;
	effect = 0;
	alphaOperation = alpha;
	alphaTest = test;
}

Gfx_RS_Base_FOW::~Gfx_RS_Base_FOW() { Release(); }

HRESULT Gfx_RS_Base_FOW::Initialize()
{
	HRESULT result = D3DERR_INVALIDCALL;
	++g_RSBaseFOWState.users;
	if (!g_RSBaseFOWState.normal)
	{
		ID3DXBuffer* errors;
		result = D3DXCreateEffect(g_D3D.device, GFX_RS_BASE_FOW_NORMAL_SOURCE, sizeof(GFX_RS_BASE_FOW_NORMAL_SOURCE) - 1,
			&g_RSBaseFOWState.normal, &errors);
		if (SUCCEEDED(result))
		{
			HRESULT status = g_RSBaseFOWState.normal->GetDesc(&g_RSBaseFOWState.description);
			if (FAILED(status))
				JE::FatalError("Gfx_RS_Base_FOW::Gfx_RS_Base_FOW - Failed GetDesc (%x)\n", status);
			status = g_RSBaseFOWState.normal->SetTechnique(0);
			if (FAILED(status))
				JE::FatalError("Gfx_RS_Base_FOW::Gfx_RS_Base_FOW - Failed to initialize technique (%x)\n", status);
			D3DXMATRIX identity = g_D3D.identity;
			g_RSBaseFOWState.normal->SetDword("Bcop", colorOperation);
			g_RSBaseFOWState.normal->SetDword("Baop", alphaOperation);
			g_RSBaseFOWState.normal->SetDword("FOWcop", 2);
			g_RSBaseFOWState.normal->SetDword("FOWaop", 2);
			g_RSBaseFOWState.normal->SetDword("TFcop", 2);
			g_RSBaseFOWState.normal->SetDword("TFaop", 2);
			g_RSBaseFOWState.normal->SetDword("alphatest", 0);
			g_RSBaseFOWState.normal->SetDword("bLighting", 0);
			g_RSBaseFOWState.normal->SetDword("bFog", 0);
			g_RSBaseFOWState.normal->SetDword("DiffuseSource", 1);
			g_RSBaseFOWState.normal->SetMatrix("FOWmatrix", &identity);
			g_RSBaseFOWState.normal->SetTexture("FOW", NULL);
			g_RSBaseFOWState.normal->SetDword("bZEnable", 1);
			D3DXTECHNIQUE_DESC selected;
			result = g_RSBaseFOWState.normal->FindNextValidTechnique(g_RSBaseFOWState.technique, &selected);
			if (SUCCEEDED(result))
			{
				g_RSBaseFOWState.technique = selected.Index;
				result = g_RSBaseFOWState.normal->SetTechnique(selected.Index);
				if (selected.Name[0] == 's' && selected.Name[1] == 'h')
					g_RSBaseFOWState.usesShader = 1;
				g_RSBaseFOWState.passes = selected.Passes;
			}
			else
				JE::FatalError("Gfx_RS_Base_FOW::Create - Failed to find valid technique (normal) (%x)\n", result);
		}
		else
		{
			JE::LogDebugString("Gfx_RS_Base_FOW::Create - Failed to compile (normal)", result);
			JE::FatalError(static_cast<const char*>(errors->GetBufferPointer()));
		}
		if (errors) errors->Release();
	}
	if (!g_RSBaseFOWState.factored)
	{
		ID3DXBuffer* errors;
		result = D3DXCreateEffect(g_D3D.device, GFX_RS_BASE_FOW_FACTORED_SOURCE, sizeof(GFX_RS_BASE_FOW_FACTORED_SOURCE) - 1,
			&g_RSBaseFOWState.factored, &errors);
		if (SUCCEEDED(result))
		{
			HRESULT status = g_RSBaseFOWState.factored->GetDesc(&g_RSBaseFOWState.description);
			if (FAILED(status))
				JE::FatalError("Gfx_RS_Base_FOW::Gfx_RS_Base_FOW - Failed GetDesc (%x)\n", status);
			status = g_RSBaseFOWState.factored->SetTechnique(0);
			if (FAILED(status))
				JE::FatalError("Gfx_RS_Base_FOW::Gfx_RS_Base_FOW - Failed to initialize technique (%x)\n", status);
			D3DXMATRIX identity = g_D3D.identity;
			g_RSBaseFOWState.factored->SetDword("Bcop", colorOperation);
			g_RSBaseFOWState.factored->SetDword("Baop", alphaOperation);
			g_RSBaseFOWState.factored->SetDword("FOWcop", 2);
			g_RSBaseFOWState.factored->SetDword("FOWaop", 2);
			g_RSBaseFOWState.factored->SetDword("TFcop", 2);
			g_RSBaseFOWState.factored->SetDword("TFaop", 2);
			g_RSBaseFOWState.factored->SetDword("alphatest", 0);
			g_RSBaseFOWState.factored->SetDword("bLighting", 0);
			g_RSBaseFOWState.factored->SetDword("bFog", 0);
			g_RSBaseFOWState.factored->SetDword("DiffuseSource", 1);
			g_RSBaseFOWState.factored->SetMatrix("FOWmatrix", &identity);
			g_RSBaseFOWState.factored->SetTexture("FOW", NULL);
			g_RSBaseFOWState.factored->SetDword("bZEnable", 1);
			D3DXTECHNIQUE_DESC selected;
			result = g_RSBaseFOWState.factored->FindNextValidTechnique(g_RSBaseFOWState.technique, &selected);
			if (SUCCEEDED(result))
			{
				g_RSBaseFOWState.technique = selected.Index;
				result = g_RSBaseFOWState.factored->SetTechnique(selected.Index);
				if (selected.Name[0] == 's' && selected.Name[1] == 'h')
					g_RSBaseFOWState.usesShader = 1;
				if (g_RSBaseFOWState.passes < selected.Passes)
					g_RSBaseFOWState.passes = selected.Passes;
			}
			else
				JE::FatalError("Gfx_RS_Base_FOW::SetMaterial - Failed to find valid technique (override) (%x)\n", result);
		}
		else
		{
			JE::LogDebugString("Gfx_RS_Base_FOW::Gfx_RS_Base_FOW - Failed to compile (override)", result);
			JE::FatalError(static_cast<const char*>(errors->GetBufferPointer()));
		}
		if (errors) errors->Release();
	}
	return result;
}

void Gfx_RS_Base_FOW::Release()
{
	if (g_RSBaseFOWState.users)
	{
		--g_RSBaseFOWState.users;
		if (!g_RSBaseFOWState.users)
		{
			if (g_RSBaseFOWState.normal)
			{
				g_RSBaseFOWState.normal->Release();
				g_RSBaseFOWState.normal = 0;
			}
			if (g_RSBaseFOWState.factored)
			{
				g_RSBaseFOWState.factored->Release();
				g_RSBaseFOWState.factored = 0;
			}
		}
	}
}
HRESULT Gfx_RS_Base_FOW::OnLostDevice()
{
	HRESULT result = D3DERR_INVALIDCALL;
	if (g_RSBaseFOWState.normal) result = g_RSBaseFOWState.normal->OnLostDevice();
	if (g_RSBaseFOWState.factored) result = g_RSBaseFOWState.factored->OnLostDevice();
	return result;
}
HRESULT Gfx_RS_Base_FOW::OnResetDevice()
{
	HRESULT result = D3DERR_INVALIDCALL;
	if (g_RSBaseFOWState.normal) result = g_RSBaseFOWState.normal->OnResetDevice();
	if (g_RSBaseFOWState.factored) result = g_RSBaseFOWState.factored->OnResetDevice();
	return result;
}
DWORD Gfx_RS_Base_FOW::TechniqueCount() { return g_RSBaseFOWState.description.Techniques; }
BYTE Gfx_RS_Base_FOW::UsesVertexShader() { return g_RSBaseFOWState.usesShader; }
DWORD Gfx_RS_Base_FOW::PassCount() { return g_RSBaseFOWState.passes; }
DWORD Gfx_RS_Base_FOW::SelectTechnique(DWORD index)
{
	if (index < g_RSBaseFOWState.description.Techniques &&
		index != reinterpret_cast<DWORD>(g_RSBaseFOWState.technique))
	{
		g_RSBaseFOWState.techniqueChanged = 1;
		g_RSBaseFOWState.technique = reinterpret_cast<LPCSTR>(index);
	}
	return index;
}
DWORD Gfx_RS_Base_FOW::NextTechnique()
{
	DWORD next = reinterpret_cast<DWORD>(g_RSBaseFOWState.technique) + 1;
	if (next >= g_RSBaseFOWState.description.Techniques) next = 0;
	return SelectTechnique(next);
}
HRESULT Gfx_RS_Base_FOW::Begin(UINT* passes) { return effect->Begin(passes, 0); }
HRESULT Gfx_RS_Base_FOW::End() { return effect->End(); }
HRESULT Gfx_RS_Base_FOW::Pass(UINT pass)
{
	g_RSBaseFOWState.currentPass = pass;
	return effect->Pass(pass);
}

HRESULT Gfx_RS_Base_FOW::SetMaterial(Gfx_Material* material, DWORD)
{
	HRESULT result = D3DERR_INVALIDCALL;
	BYTE useRGB = 0, useAlpha = 0;
	DWORD color;
	material->GetColor(useRGB, useAlpha, color);
	if (!useRGB && !useAlpha) effect = g_RSBaseFOWState.normal;
	else effect = g_RSBaseFOWState.factored;
	if (effect)
	{
		const DWORD texture = material->textureHandles[0];
		if (texture)
		{
			Gfx_Texture* object = g_TextureManager->Lookup(texture);
			if (object) object->Bind(0, 0);
			else
			{
				g_D3D.SetTexture(0, 0);
				JE::LogDebugString("Gfx_RS_Base_FOW: Cannot find Base Texture\n");
			}
		}
		else g_D3D.SetTexture(0, 0);

		IDirect3DTexture8* fog = g_GfxSystem->FogOfWarRenderTexture();
		if (fog)
		{
			effect->SetTexture("FOW", fog);
			effect->SetDword("FOWcop", 4);
			effect->SetDword("FOWaop", 2);
		}
		else
		{
			effect->SetTexture("FOW", NULL);
			effect->SetDword("FOWcop", 2);
			effect->SetDword("FOWaop", 2);
		}
		effect->SetDword("bZEnable", material->flagBB);
		effect->SetDword("alphatest", alphaTest);
		BOOL lighting = FALSE;
		if (material->flagBC) lighting = g_GfxSystem->Lighting() != 0;
		effect->SetDword("bLighting", lighting);
		BOOL fogEnabled = FALSE;
		if (material->flagBD) fogEnabled = g_GfxSystem->CurrentViewport()->fogEnabled != 0;
		effect->SetDword("bFog", fogEnabled);
		effect->SetDword("DiffuseSource", material->flagB8 != 0);
		effect->SetDword("Bcop", colorOperation);
		effect->SetDword("Baop", alphaOperation);
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
			effect->SetDword("TFcop", useRGB ? 5 : 2);
			effect->SetDword("TFaop", useAlpha ? 4 : 2);
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
		effect->SetMatrix("FOWmatrix", &fogMatrix);
		DWORD flags = material->values140[0];
		if (flags)
		{
			const float seconds = g_GfxSystem->TimeSeconds();
			SetTextureTransform(0, static_cast<BYTE>(flags), &material->textureParameters[0], seconds);
		}
		else g_D3D.SetTextureStageState(0, D3DTSS_TEXTURETRANSFORMFLAGS, 0);
		if (g_RSBaseFOWState.techniqueChanged)
		{
			D3DXTECHNIQUE_DESC selected;
			result = effect->FindNextValidTechnique(g_RSBaseFOWState.technique, &selected);
			if (SUCCEEDED(result))
			{
				g_RSBaseFOWState.techniqueChanged = 0;
				g_RSBaseFOWState.technique = selected.Index;
				g_RSBaseFOWState.passes = selected.Passes;
			}
			else
				JE::FatalError("Gfx_RS_Base_FOW::SetMaterial - Failed to find valid technique (%x)\n", result);
		}
	}
	return result;
}

