#include "RSBase.hpp"
#include "D3DState.hpp"
#include "SystemInterface.hpp"
#include "TextureManager.hpp"
#include "Texture.hpp"
#include "Viewport.hpp"
#include "diagnostics/DebugOutput.hpp"
#include <cstring>

namespace
{
	// Exact original753-byte effect description; the actual SDK compiles it.
	const char source[] = "DWORD\t\tBcop;\nDWORD\t\tBaop;\nDWORD\t\tTFcop;\nDWORD\t\tTFaop;\nDWORD\t\talphatest;\nDWORD\t\tbLighting;\nDWORD\t\tbFog;\nDWORD\t\tDiffuseSource;\nDWORD\t\tbZEnable;\nTechnique\tffp\n{\nPass\tp0\n{\nZEnable\t\t\t\t= < bZEnable >;\nAlphaTestEnable\t\t= <alphatest>;\nLighting\t\t\t\t= <bLighting>;\nFogEnable\t\t\t\t= <bFog>;\nDiffuseMaterialSource\t= < DiffuseSource >;\nAmbientMaterialSource\t= < DiffuseSource >;\nColorOp[ 0 ]\t\t\t= < Bcop >;\nColorArg1[ 0 ]\t\t\t= Texture;\nColorArg2[ 0 ]\t\t\t= Diffuse;\nAlphaOp[ 0 ]\t\t\t= < Baop >;\nAlphaArg1[ 0 ]\t\t\t= Texture;\nAlphaArg2[ 0 ]\t\t\t= Diffuse;\nColorOp[ 1 ]\t\t\t= < TFcop >;\nColorArg1[ 1 ]\t\t\t= Current;\nColorArg2[ 1 ]\t\t\t= Tfactor;\nAlphaOp[ 1 ]\t\t\t= < TFaop >;\nAlphaArg1[ 1 ]\t\t\t= Current;\nAlphaArg2[ 1 ]\t\t\t= Tfactor;\nColorOp[ 2 ]\t\t\t= Disable;\nAlphaOp[ 2 ]\t\t\t= Disable;\n}\n}\n";
	typedef char Source753[(sizeof(source) - 1 == 753) ? 1 : -1];
}


Gfx_RS_Base::Gfx_RS_Base(DWORD color, DWORD alpha, DWORD test)
{
	colorOperation = color;
	alphaOperation = alpha;
	alphaTest = test != 0;
}

Gfx_RS_Base::~Gfx_RS_Base()
{
	Release();
}

HRESULT Gfx_RS_Base::Initialize()
{
	HRESULT result = D3DERR_INVALIDCALL;
	++g_RSBaseState.users;
	if (!g_RSBaseState.effect)
	{
		ID3DXBuffer* errors;
		result = D3DXCreateEffect(g_D3D.device, source, sizeof(source) - 1,
			&g_RSBaseState.effect, &errors);
		if (SUCCEEDED(result))
		{
			HRESULT queried = g_RSBaseState.effect->GetDesc(&g_RSBaseState.description);
			if (FAILED(queried)) JE::FatalError("Gfx_RS_Base::Gfx_RS_Base - Failed GetDesc (%x)\n", queried);
			queried = g_RSBaseState.effect->SetTechnique(0);
			if (FAILED(queried)) JE::FatalError("Gfx_RS_Base::Gfx_RS_Base - Failed to initialize technique (%x)\n", queried);
			g_RSBaseState.effect->SetDword("Bcop", colorOperation);
			g_RSBaseState.effect->SetDword("Baop", alphaOperation);
			g_RSBaseState.effect->SetDword("TFcop", 2);
			g_RSBaseState.effect->SetDword("TFaop", 2);
			g_RSBaseState.effect->SetDword("alphatest", 0);
			g_RSBaseState.effect->SetDword("bLighting", 0);
			g_RSBaseState.effect->SetDword("bFog", 0);
			g_RSBaseState.effect->SetDword("DiffuseSource", 1);
			g_RSBaseState.effect->SetDword("bZEnable", 1);
			D3DXTECHNIQUE_DESC selected;
			result = g_RSBaseState.effect->FindNextValidTechnique(g_RSBaseState.technique, &selected);
			if (SUCCEEDED(result))
			{
				g_RSBaseState.technique = selected.Index;
				result = g_RSBaseState.effect->SetTechnique(selected.Index);
				if (selected.Name[0] == 's' && selected.Name[1] == 'h') g_RSBaseState.usesShader = 1;
				g_RSBaseState.passes = selected.Passes;
			}
			else JE::FatalError("Gfx_RS_Base::SetMaterial - Failed to find valid technique (%x)\n", result);
		}
		else
		{
			JE::LogDebugString("Gfx_RS_Base::Gfx_RS_Base - Failed to compile", result);
			JE::FatalError(static_cast<const char*>(errors->GetBufferPointer()));
		}
		if (errors) errors->Release();
	}
	return result;
}

void Gfx_RS_Base::Release()
{
	if (g_RSBaseState.users)
	{
		--g_RSBaseState.users;
		if (!g_RSBaseState.users && g_RSBaseState.effect)
		{
			g_RSBaseState.effect->Release();
			g_RSBaseState.effect = 0;
		}
	}
}

HRESULT Gfx_RS_Base::OnLostDevice()
{
	HRESULT result = D3DERR_INVALIDCALL;
	if (g_RSBaseState.effect) result = g_RSBaseState.effect->OnLostDevice();
	return result;
}

HRESULT Gfx_RS_Base::OnResetDevice()
{
	HRESULT result = D3DERR_INVALIDCALL;
	if (g_RSBaseState.effect) result = g_RSBaseState.effect->OnResetDevice();
	return result;
}

DWORD Gfx_RS_Base::TechniqueCount() { return g_RSBaseState.description.Techniques; }
BYTE Gfx_RS_Base::UsesVertexShader() { return g_RSBaseState.usesShader; }
DWORD Gfx_RS_Base::PassCount() { return g_RSBaseState.passes; }

DWORD Gfx_RS_Base::SelectTechnique(DWORD index)
{
	if (index < g_RSBaseState.description.Techniques && index != reinterpret_cast<DWORD>(g_RSBaseState.technique))
	{
		g_RSBaseState.techniqueChanged = 1;
		g_RSBaseState.technique = reinterpret_cast<LPCSTR>(index);
	}
	return index;
}

DWORD Gfx_RS_Base::NextTechnique()
{
	DWORD next = reinterpret_cast<DWORD>(g_RSBaseState.technique) + 1;
	if (next >= g_RSBaseState.description.Techniques) next = 0;
	return SelectTechnique(next);
}

HRESULT Gfx_RS_Base::Begin(UINT* passes) { return g_RSBaseState.effect->Begin(passes, 0); }
HRESULT Gfx_RS_Base::End() { return g_RSBaseState.effect->End(); }
HRESULT Gfx_RS_Base::Pass(UINT pass)
{
	g_RSBaseState.currentPass = pass;
	return g_RSBaseState.effect->Pass(pass);
}

HRESULT Gfx_RS_Base::SetMaterial(Gfx_Material* material, DWORD)
{
	HRESULT result = D3DERR_INVALIDCALL;
	if (g_RSBaseState.effect)
	{
		DWORD texture = material->textureHandles[0];
		if (texture)
		{
			Gfx_Texture* object = g_TextureManager->Lookup(texture);
			if (object) object->Bind(0, 0);
			else
			{
				g_D3D.SetTexture(0, 0);
				JE::LogDebugString("Gfx_RS_Base: Cannot find Base Texture %x\n", texture);
			}
		}
		else g_D3D.SetTexture(0, 0);
		g_RSBaseState.effect->SetDword("bZEnable", material->flagBB);
		g_RSBaseState.effect->SetDword("alphatest", alphaTest);
		DWORD lighting = 0;
		if (material->flagBC) lighting = g_GfxSystem->Lighting() != 0;
		g_RSBaseState.effect->SetDword("bLighting", lighting);
		DWORD fog = 0;
		if (material->flagBD) fog = g_GfxSystem->CurrentViewport()->fogEnabled != 0;
		g_RSBaseState.effect->SetDword("bFog", fog);
		BYTE useRGB = 0, useAlpha = 0;
		DWORD color;
		material->GetColor(useRGB, useAlpha, color);
		g_RSBaseState.effect->SetDword("DiffuseSource", material->flagB8 != 0);
		g_RSBaseState.effect->SetDword("Bcop", colorOperation);
		g_RSBaseState.effect->SetDword("Baop", alphaOperation);
		D3DMATERIAL8 values;
		std::memcpy(&values, &material->values, sizeof(values));
		g_D3D.device->SetMaterial(&values);
		if (useRGB || useAlpha)
		{
			g_RSBaseState.effect->SetDword("TFcop", useRGB ? 5 : 2);
			g_RSBaseState.effect->SetDword("TFaop", useAlpha ? 4 : 2);
			g_D3D.SetRenderState(D3DRS_TEXTUREFACTOR, color);
		}
		else
		{
			g_RSBaseState.effect->SetDword("TFcop", 1);
			g_RSBaseState.effect->SetDword("TFaop", 1);
		}
		if (material->flagB9) result = g_D3D.SetRenderState(D3DRS_CULLMODE, 3);
		else result = g_D3D.SetRenderState(D3DRS_CULLMODE, 1);
		DWORD flags = material->values140[0];
		if (flags)
		{
			float seconds = g_GfxSystem->TimeSeconds();
			SetTextureTransform(0, static_cast<BYTE>(flags), &material->textureParameters[0], seconds);
		}
		else g_D3D.SetTextureStageState(0, D3DTSS_TEXTURETRANSFORMFLAGS, 0);
		if (g_RSBaseState.techniqueChanged)
		{
			D3DXTECHNIQUE_DESC selected;
			result = g_RSBaseState.effect->FindNextValidTechnique(g_RSBaseState.technique, &selected);
			if (SUCCEEDED(result))
			{
				g_RSBaseState.technique = selected.Index;
				g_RSBaseState.techniqueChanged = 0;
			}
			else JE::FatalError("Gfx_RS_Base::SetMaterial - Failed to find valid technique (%x)\n", result);
			g_RSBaseState.passes = selected.Passes;
		}
	}
	return result;
}
