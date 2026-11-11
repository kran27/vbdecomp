#include "RSBaseEmissive.hpp"
#include "D3DState.hpp"
#include "SystemInterface.hpp"
#include "TextureManager.hpp"
#include "Texture.hpp"
#include "diagnostics/DebugOutput.hpp"
#include "Material.hpp"
#include "Camera.hpp"
#include "Viewport.hpp"
#include "TextureTransform.hpp"
#include <cstring>

namespace
{
	//Exact original effect text; compiled by the real DirectX8.1 SDK.
	const char source[] =
		"DWORD\t\tcop0;\n"
		"DWORD\t\taop0;\n"
		"matrix\t\tTexMat;\n"
		"DWORD\t\talphatest;\n"
		"DWORD\t\tbLighting;\n"
		"DWORD\t\tbFog;\n"
		"DWORD\t\tDiffuseSource;\n"
		"Technique\tffp_CubeEM\n"
		"{\n"
		"Pass\tp0\n"
		"{\n"
		"AlphaTestEnable\t\t\t= <alphatest>;\n"
		"Lighting\t\t\t\t\t= <bLighting>;\n"
		"FogEnable\t\t\t\t\t= <bFog>;\n"
		"DiffuseMaterialSource\t\t= < DiffuseSource >;\n"
		"AmbientMaterialSource\t\t= < DiffuseSource >;\n"
		"ColorOp[ 0 ]\t\t\t\t= <cop0>;\n"
		"ColorArg1[ 0 ]\t\t\t\t= Texture;\n"
		"ColorArg2[ 0 ]\t\t\t\t= Diffuse;\n"
		"AlphaOp[ 0 ]\t\t\t\t= <aop0>;\n"
		"AlphaArg1[ 0 ]\t\t\t\t= Texture;\n"
		"AlphaArg2[ 0 ]\t\t\t\t= Diffuse;\n"
		"TextureTransform[ 1 ]\t\t= < TexMat >;\n"
		"AddressU[ 1 ]\t\t\t\t= Clamp;\n"
		"AddressV[ 1 ]\t\t\t\t= Clamp;\n"
		"AddressW[ 1 ]\t\t\t\t= Clamp;\n"
		"ColorOp[ 1 ]\t\t\t\t= AddSmooth;\n"
		"ColorArg1[ 1 ]\t\t\t\t= Texture;\n"
		"ColorArg2[ 1 ]\t\t\t\t= Current;\n"
		"TexCoordIndex[ 1 ]\t\t\t= CameraSpaceReflectionVector | 1;\n"
		"TextureTransformFlags[ 1 ]\t= Count3;\n"
		"AlphaOp[ 1 ]\t\t\t\t= SelectArg2;\n"
		"AlphaArg2[ 1 ]\t\t\t\t= Current;\n"
		"ColorOp[ 2 ]\t\t\t\t= Disable;\n"
		"AlphaOp[ 2 ]\t\t\t\t= Disable;\n"
		"}\n"
		"}\n"
		"Technique\tffp_Base\n"
		"{\n"
		"Pass\tp0\n"
		"{\n"
		"AlphaTestEnable\t\t\t= <alphatest>;\n"
		"Lighting\t\t\t\t\t= <bLighting>;\n"
		"FogEnable\t\t\t\t\t= <bFog>;\n"
		"ColorOp[ 0 ]\t\t\t\t= <cop0>;\n"
		"ColorArg1[ 0 ]\t\t\t\t= Texture;\n"
		"ColorArg2[ 0 ]\t\t\t\t= Diffuse;\n"
		"AlphaOp[ 0 ]\t\t\t\t= <aop0>;\n"
		"AlphaArg1[ 0 ]\t\t\t\t= Texture;\n"
		"AlphaArg2[ 0 ]\t\t\t\t= Diffuse;\n"
		"ColorOp[ 1 ]\t\t\t\t= Disable;\n"
		"AlphaOp[ 1 ]\t\t\t\t= Disable;\n"
		"}\n"
		"}\n";
	typedef char ExactEffectSize[(sizeof(source) - 1 == 1241) ? 1 : -1];
}

Gfx_RS_B_EMState g_RSBaseEmissiveState;

Gfx_RS_B_EM::Gfx_RS_B_EM(DWORD color, DWORD alpha, DWORD test)
{
	colorOperation = color;
	alphaOperation = alpha;
	alphaTest = test != 0;
}

Gfx_RS_B_EM::~Gfx_RS_B_EM()
{
	Release();
}

HRESULT Gfx_RS_B_EM::Initialize()
{
	HRESULT result = D3DERR_INVALIDCALL;
	++g_RSBaseEmissiveState.users;
	if (!g_RSBaseEmissiveState.effect)
	{
		ID3DXBuffer* errors;
		result = D3DXCreateEffect(g_D3D.device, source, sizeof(source) - 1,
			&g_RSBaseEmissiveState.effect, &errors);
		if (SUCCEEDED(result))
		{
			HRESULT queried = g_RSBaseEmissiveState.effect->GetDesc(&g_RSBaseEmissiveState.description);
			if (FAILED(queried))
				JE::FatalError("Gfx_RS_B_EM::Gfx_RS_B_EM - Failed GetDesc (%x)\n", queried);
			queried = g_RSBaseEmissiveState.effect->SetTechnique(NULL);
			if (FAILED(queried))
				JE::FatalError("Gfx_RS_B_EM::Gfx_RS_B_EM - Failed to initialize technique (%x)\n", queried);
			D3DXMATRIX matrix = g_D3D.identity;
			g_RSBaseEmissiveState.effect->SetDword("cop0", colorOperation);
			g_RSBaseEmissiveState.effect->SetDword("aop0", alphaOperation);
			g_RSBaseEmissiveState.effect->SetMatrix("TexMat", &matrix);
			g_RSBaseEmissiveState.effect->SetDword("alphatest", 0);
			g_RSBaseEmissiveState.effect->SetDword("bLighting", 0);
			g_RSBaseEmissiveState.effect->SetDword("bFog", 0);
			g_RSBaseEmissiveState.effect->SetDword("DiffuseSource", 1);
			D3DXTECHNIQUE_DESC selected;
			result = g_RSBaseEmissiveState.effect->FindNextValidTechnique(g_RSBaseEmissiveState.technique, &selected);
			if (SUCCEEDED(result))
			{
				g_RSBaseEmissiveState.technique = selected.Index;
				result = g_RSBaseEmissiveState.effect->SetTechnique(selected.Index);
				if (selected.Name[0] == 's' && selected.Name[1] == 'h')
					g_RSBaseEmissiveState.usesShader = 1;
				g_RSBaseEmissiveState.passes = selected.Passes;
			}
			else
				JE::FatalError("Gfx_RS_B_EM::Create - Failed to find valid technique (%x)\n", result);
		}
		else
		{
			JE::LogDebugString("Gfx_RS_B_EM::Gfx_RS_B_EM - Failed to compile (%x)\n", result);
			JE::FatalError(static_cast<const char*>(errors->GetBufferPointer()));
		}
		if (errors) errors->Release();
	}
	return result;
}

void Gfx_RS_B_EM::Release()
{
	if (g_RSBaseEmissiveState.users)
	{
		--g_RSBaseEmissiveState.users;
		if (!g_RSBaseEmissiveState.users && g_RSBaseEmissiveState.effect)
		{
			g_RSBaseEmissiveState.effect->Release();
			g_RSBaseEmissiveState.effect = NULL;
		}
	}
}

HRESULT Gfx_RS_B_EM::OnLostDevice()
{
	HRESULT result = D3DERR_INVALIDCALL;
	if (g_RSBaseEmissiveState.effect) result = g_RSBaseEmissiveState.effect->OnLostDevice();
	return result;
}

HRESULT Gfx_RS_B_EM::OnResetDevice()
{
	HRESULT result = D3DERR_INVALIDCALL;
	if (g_RSBaseEmissiveState.effect) result = g_RSBaseEmissiveState.effect->OnResetDevice();
	return result;
}

DWORD Gfx_RS_B_EM::TechniqueCount() { return g_RSBaseEmissiveState.description.Techniques; }
BYTE Gfx_RS_B_EM::UsesVertexShader() { return g_RSBaseEmissiveState.usesShader; }
DWORD Gfx_RS_B_EM::PassCount() { return g_RSBaseEmissiveState.passes; }

DWORD Gfx_RS_B_EM::SelectTechnique(DWORD index)
{
	if (index < g_RSBaseEmissiveState.description.Techniques &&
		index != reinterpret_cast<DWORD>(g_RSBaseEmissiveState.technique))
	{
		g_RSBaseEmissiveState.techniqueChanged = 1;
		g_RSBaseEmissiveState.technique = reinterpret_cast<LPCSTR>(index);
	}
	return index;
}

DWORD Gfx_RS_B_EM::NextTechnique()
{
	DWORD next = reinterpret_cast<DWORD>(g_RSBaseEmissiveState.technique) + 1;
	if (next >= g_RSBaseEmissiveState.description.Techniques)
		next = 0;
	return SelectTechnique(next);
}

HRESULT Gfx_RS_B_EM::Begin(UINT* passes) { return g_RSBaseEmissiveState.effect->Begin(passes, 0); }
HRESULT Gfx_RS_B_EM::End() { return g_RSBaseEmissiveState.effect->End(); }
HRESULT Gfx_RS_B_EM::Pass(UINT pass)
{
	g_RSBaseEmissiveState.currentPass = pass;
	return g_RSBaseEmissiveState.effect->Pass(pass);
}

HRESULT Gfx_RS_B_EM::SetMaterial(Gfx_Material* material, DWORD)
{
	HRESULT result = D3DERR_INVALIDCALL;
	if (g_RSBaseEmissiveState.effect)
	{
		DWORD handle = material->textureHandles[0];
		if (handle)
		{
			Gfx_Texture* texture = g_TextureManager->Lookup(handle);
			if (texture)
				texture->Bind(0, 0);
			else
			{
				g_D3D.SetTexture(0, NULL);
				//Original53C87B passes NULL into the unguarded diagnostic-name copy.
				JE::LogDebugString("Gfx_RS_B_EM: Cannot find Base Texture %s\n", texture->GetDiagnosticName().c_str());
			}
		}
		else
		{
			g_D3D.SetTexture(0, NULL);
			JE::LogDebugString("Gfx_RS_B_EM: NULL Base Texture handle\n");
		}
		handle = material->textureHandles[2];
		if (handle)
		{
			Gfx_Texture* texture = g_TextureManager->Lookup(handle);
			if (texture)
				texture->Bind(1, 0);
			else
			{
				g_D3D.SetTexture(1, NULL);
				JE::LogDebugString("Gfx_RS_B_EM: Cannot find Cubic Texture %s\n", texture->GetDiagnosticName().c_str());
			}
		}
		else
			g_D3D.SetTexture(1, NULL);
		g_RSBaseEmissiveState.effect->SetDword("alphatest", alphaTest);
		BOOL lighting = FALSE;
		if (material->flagBC) lighting = g_GfxSystem->Lighting() != 0;
		g_RSBaseEmissiveState.effect->SetDword("bLighting", lighting);
		BOOL fog = FALSE;
		if (material->flagBD) fog = g_GfxSystem->CurrentViewport()->fogEnabled != 0;
		g_RSBaseEmissiveState.effect->SetDword("bFog", fog);
		BYTE useRGB = 0, useAlpha = 0;
		DWORD factor;
		material->GetColor(useRGB, useAlpha, factor);
		g_RSBaseEmissiveState.effect->SetDword("cop0", colorOperation);
		if (!useRGB) g_RSBaseEmissiveState.effect->SetDword("c0a2", 0);
		else g_RSBaseEmissiveState.effect->SetDword("c0a2", 3);
		g_RSBaseEmissiveState.effect->SetDword("aop0", alphaOperation);
		if (!useAlpha) g_RSBaseEmissiveState.effect->SetDword("a0a2", 0);
		else g_RSBaseEmissiveState.effect->SetDword("a0a2", 3);
		D3DXMATRIX view, inverse;
		Gfx_Camera* camera = g_GfxSystem->Camera();
		camera->GetViewMatrix(view);
		view._41 = view._42 = view._43 = 0;
		D3DXMatrixInverse(&inverse, NULL, &view);
		g_RSBaseEmissiveState.effect->SetMatrix("TexMat", &inverse);
		D3DMATERIAL8 values = material->values;
		g_D3D.device->SetMaterial(&values);
		if (useRGB || useAlpha)
			g_D3D.SetRenderState(D3DRS_TEXTUREFACTOR, factor);
		if (material->flagB9) result = g_D3D.SetRenderState(D3DRS_CULLMODE, 3);
		else result = g_D3D.SetRenderState(D3DRS_CULLMODE, 1);
		DWORD flags = material->values140[0];
		if (flags)
		{
			float seconds = g_GfxSystem->TimeSeconds();
			SetTextureTransform(0, static_cast<BYTE>(flags), &material->textureParameters[0], seconds);
		}
		else
			g_D3D.SetTextureStageState(0, D3DTSS_TEXTURETRANSFORMFLAGS, 0);
		if (g_RSBaseEmissiveState.techniqueChanged)
		{
			D3DXTECHNIQUE_DESC selected;
			HRESULT selectedResult;
			for (;;)
			{
				selectedResult = g_RSBaseEmissiveState.effect->FindNextValidTechnique(
					g_RSBaseEmissiveState.technique, &selected);
				if (::_stricmp(selected.Name, "CubeEM") == 0)
				{
					if (g_D3D.currentDevice->caps.TextureCaps & D3DPTEXTURECAPS_CUBEMAP)
					{
						g_RSBaseEmissiveState.technique = selected.Index;
						g_RSBaseEmissiveState.passes = selected.Passes;
						g_RSBaseEmissiveState.techniqueChanged = 0;
					}
					else JE::LogDebugString("Gfx_RS_B_EM: Device does not support Cube Textures\n");
				}
				else
				{
					g_RSBaseEmissiveState.technique = selected.Index;
					g_RSBaseEmissiveState.passes = selected.Passes;
					g_RSBaseEmissiveState.techniqueChanged = 0;
				}
				if (!g_RSBaseEmissiveState.techniqueChanged)
				{
					selectedResult = g_RSBaseEmissiveState.effect->SetTechnique(selected.Index);
					if (SUCCEEDED(selectedResult)) break;
					JE::FatalError("Gfx_RS_B_EM::SetMaterial - Failed to set technique (%x)\n", selectedResult);
				}
				else if (SUCCEEDED(selectedResult)) continue;
				JE::FatalError("Gfx_RS_B_EM: Cannot find a valid technique for this device! (%x)\n", selectedResult);
				break;
			}
			result = selectedResult;
		}
	}
	return result;
}
