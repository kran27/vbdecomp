#include "RSExpand.hpp"
#include "D3DState.hpp"
#include "SystemInterface.hpp"
#include "TextureManager.hpp"
#include "Texture.hpp"
#include "diagnostics/DebugOutput.hpp"

namespace
{
	//Exact original effect text; compiled by the real DirectX8.1 SDK.
#define GFX_RS_EXPAND_SOURCE \
"technique sh_expand\n" \
"{\n" \
"Pass p0\n" \
"{\n" \
"AddressU[ 0 ]\t\t= Clamp;\n" \
"ColorOp[ 0 ]\t\t= SelectArg2;\n" \
"ColorArg1[ 0 ]\t\t= Texture;\n" \
"ColorArg2[ 0 ]\t\t= Diffuse;\n" \
"AlphaOp[ 0 ]\t\t= SelectArg1;\n" \
"AlphaArg1[ 0 ]\t\t= Texture;\n" \
"AlphaArg2[ 0 ]\t\t= Diffuse;\n" \
"ColorOp[ 1 ]\t\t= Disable;\n" \
"AlphaOp[ 1 ]\t\t= Disable;\n" \
"VertexShader =\n" \
"decl\n" \
"{\n" \
"stream 0;\n" \
"float\t\tv0[ 3 ];\n" \
"float\t\tv3[ 3 ];\n" \
"D3DCOLOR\tv5;\n" \
"float\t\tv7[ 2 ];\n" \
"float\t\tv8[ 2 ];\n" \
"}\n" \
"asm\n" \
"{\n" \
"vs.1.0\n" \
"mul\tr0, v3, c[ 13 ].xxxy\n" \
"add\tr0, v0, r0\n" \
"m4x4\toPos, r0, c[ 0 ]\n" \
"m3x3\tr1, v3, c[ 4 ]\n" \
"dp3\tr1.w, r1, r1\n" \
"rsq\tr1.w, r1.w\n" \
"mul\tr1, r1, r1.w\n" \
"m4x3\tr2, r0, c[ 8 ]\n" \
"add\tr3.xyz, c[ 14 ], -r2\n" \
"dp3\tr3.w, r3, r3\n" \
"rsq\tr3.w, r3.w\n" \
"mul\tr3, r3, r3.w\n" \
"mov\toD0, c[ 12 ]\n" \
"dp3\tr3.w, r3, r1\n" \
"mad\toT0.x, r3.w, c[ 13 ].z, c[ 13 ].z\n" \
"};\n" \
"}\n" \
"}\n" \
"technique sh_expand_skin\n" \
"{\n" \
"Pass p0\n" \
"{\n" \
"AddressU[ 0 ]\t\t= Clamp;\n" \
"ColorOp[ 0 ]\t\t= SelectArg2;\n" \
"ColorArg1[ 0 ]\t\t= Texture;\n" \
"ColorArg2[ 0 ]\t\t= Diffuse;\n" \
"AlphaOp[ 0 ]\t\t= SelectArg1;\n" \
"AlphaArg1[ 0 ]\t\t= Texture;\n" \
"AlphaArg2[ 0 ]\t\t= Diffuse;\n" \
"ColorOp[ 1 ]\t\t= Disable;\n" \
"AlphaOp[ 1 ]\t\t= Disable;\n" \
"VertexShader =\n" \
"decl\n" \
"{\n" \
"stream 0;\n" \
"float\t\tv0[ 3 ];\n" \
"float\t\tv3[ 3 ];\n" \
"D3DCOLOR\tv5;\n" \
"float\t\tv7[ 2 ];\n" \
"}\n" \
"asm\n" \
"{\n" \
"vs.1.0\n" \
"mul\tr0, v3, c[ 13 ].xxxy\n" \
"add\tr0, v0, r0\n" \
"m4x4\toPos, r0, c[ 0 ]\n" \
"m3x3\tr1, v3, c[ 4 ]\n" \
"dp3\tr1.w, r1, r1\n" \
"rsq\tr1.w, r1.w\n" \
"mul\tr1, r1, r1.w\n" \
"m4x3\tr2, r0, c[ 8 ]\n" \
"add\tr3.xyz, c[ 14 ], -r2\n" \
"dp3\tr3.w, r3, r3\n" \
"rsq\tr3.w, r3.w\n" \
"mul\tr3, r3, r3.w\n" \
"mov\toD0, c[ 12 ]\n" \
"dp3\tr3.w, r3, r1\n" \
"mad\toT0.x, r3.w, c[ 13 ].z, c[ 13 ].z\n" \
"};\n" \
"}\n" \
"}\n"
	typedef char ExactEffectSize[(sizeof(GFX_RS_EXPAND_SOURCE) - 1 == 1385) ? 1 : -1];
}

Gfx_RS_ExpandState g_RSExpandState;

Gfx_RS_Expand::Gfx_RS_Expand()
{
}

Gfx_RS_Expand::~Gfx_RS_Expand()
{
	Release();
}

HRESULT Gfx_RS_Expand::Initialize()
{
	++g_RSExpandState.users;
	if (g_RSExpandState.effect)
		return S_OK;
	ID3DXBuffer* errors;
	HRESULT result = D3DXCreateEffect(g_D3D.device, GFX_RS_EXPAND_SOURCE, sizeof(GFX_RS_EXPAND_SOURCE) - 1,
		&g_RSExpandState.effect, &errors);
	if (SUCCEEDED(result))
	{
		result = g_RSExpandState.effect->GetDesc(&g_RSExpandState.description);
		if (FAILED(result))
			JE::FatalError("Gfx_RS_Expand::Gfx_RS_Expand - Failed GetDesc (%x)\n", result);
		for (DWORD index = 0; index < g_RSExpandState.description.Parameters; ++index)
		{
			D3DXPARAMETER_DESC parameter;
			g_RSExpandState.effect->GetParameterDesc(reinterpret_cast<LPCSTR>(index), &parameter);
		}
		result = g_RSExpandState.effect->SetTechnique(NULL);
		if (FAILED(result))
			JE::FatalError("Gfx_RS_Expand::Gfx_RS_Expand - Failed to initialize technique (%x)\n", result);
		if (!g_D3D.currentDevice->vertexShader)
			g_D3D.SetRenderState(D3DRS_SOFTWAREVERTEXPROCESSING, 1);
		D3DXTECHNIQUE_DESC selected;
		result = g_RSExpandState.effect->FindNextValidTechnique(g_RSExpandState.technique, &selected);
		if (SUCCEEDED(result))
		{
			g_RSExpandState.technique = selected.Index;
			result = g_RSExpandState.effect->SetTechnique(selected.Index);
			if (selected.Name[0] == 's' && selected.Name[1] == 'h')
				g_RSExpandState.usesShader = 1;
			g_RSExpandState.passes = selected.Passes;
		}
		else
			JE::FatalError("Gfx_RS_Expand::Gfx_RS_Expand - Failed to find valid technique (%x)\n", result);
		if (!g_D3D.currentDevice->vertexShader)
			g_D3D.SetRenderState(D3DRS_SOFTWAREVERTEXPROCESSING, 0);
	}
	else
	{
		JE::LogDebugString("Gfx_RS_Expand::Gfx_RS_Expand - Failed to compile", result);
		JE::FatalError(static_cast<const char*>(errors->GetBufferPointer()));
	}
	if (errors)
		errors->Release();
	return result;
}

HRESULT Gfx_RS_Expand::OnLostDevice()
{
	HRESULT result = D3DERR_INVALIDCALL;
	if (g_RSExpandState.effect)
		result = g_RSExpandState.effect->OnLostDevice();
	return result;
}

HRESULT Gfx_RS_Expand::OnResetDevice()
{
	HRESULT result = D3DERR_INVALIDCALL;
	if (g_RSExpandState.effect)
		result = g_RSExpandState.effect->OnResetDevice();
	return result;
}

DWORD Gfx_RS_Expand::TechniqueCount() { return g_RSExpandState.description.Techniques; }
BYTE Gfx_RS_Expand::UsesVertexShader() { return g_RSExpandState.usesShader; }
DWORD Gfx_RS_Expand::PassCount() { return g_RSExpandState.passes; }

DWORD Gfx_RS_Expand::SelectTechnique(DWORD index)
{
	if (index < g_RSExpandState.description.Techniques &&
		index != reinterpret_cast<DWORD>(g_RSExpandState.technique))
	{
		g_RSExpandState.techniqueChanged = 1;
		g_RSExpandState.technique = reinterpret_cast<LPCSTR>(index);
	}
	return index;
}

DWORD Gfx_RS_Expand::NextTechnique()
{
	DWORD next = reinterpret_cast<DWORD>(g_RSExpandState.technique) + 1;
	if (next >= g_RSExpandState.description.Techniques)
		next = 0;
	return SelectTechnique(next);
}

HRESULT Gfx_RS_Expand::SetMaterial(Gfx_Material*, DWORD)
{
	HRESULT result = D3DERR_INVALIDCALL;
	if (!g_RSExpandState.effect)
	{
		JE::LogDebugString("Gfx_RS_Expand::SetMaterial() failed - not created yet.");
		return result;
	}
	Gfx_TextureManager* manager = g_TextureManager;
	const DWORD handle = g_GfxSystem->SilhouetteEdgeTexture();
	Gfx_Texture* texture = manager->Lookup(handle);
	if (texture)
		texture->Bind(0, 0);
	else
	{
		g_D3D.SetTexture(0, NULL);
		JE::LogDebugString("Gfx_RS_Expand::SetMaterial - Cannot find Silhouette Edge Texture!\n");
	}
	if (g_RSExpandState.techniqueChanged)
	{
		D3DXTECHNIQUE_DESC selected;
		result = g_RSExpandState.effect->FindNextValidTechnique(g_RSExpandState.technique, &selected);
		if (SUCCEEDED(result))
		{
			g_RSExpandState.technique = selected.Index;
			g_RSExpandState.techniqueChanged = 0;
			g_RSExpandState.passes = selected.Passes;
		}
		else
			JE::FatalError("Gfx_RS_Expand::SetMaterial - Failed to find valid technique (%x)\n", result);
	}
	return result;
}

HRESULT Gfx_RS_Expand::Begin(UINT* passes) { return g_RSExpandState.effect->Begin(passes, 0); }
HRESULT Gfx_RS_Expand::End() { return g_RSExpandState.effect->End(); }
HRESULT Gfx_RS_Expand::Pass(UINT pass) { return g_RSExpandState.effect->Pass(pass); }
