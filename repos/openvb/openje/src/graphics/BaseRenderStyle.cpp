#include "BaseRenderStyle.hpp"
#include "diagnostics/DebugOutput.hpp"

#include <d3dx8effect.h>
#include <cstring>
#include <stdexcept>

namespace JE
{
namespace Gfx
{
	namespace
	{
		const char source[] = "DWORD\t\tBcop;\nDWORD\t\tBaop;\nDWORD\t\tTFcop;\nDWORD\t\tTFaop;\nDWORD\t\talphatest;\nDWORD\t\tbLighting;\nDWORD\t\tbFog;\nDWORD\t\tDiffuseSource;\nDWORD\t\tbZEnable;\nTechnique\tffp\n{\nPass\tp0\n{\nZEnable\t\t\t\t= < bZEnable >;\nAlphaTestEnable\t\t= <alphatest>;\nLighting\t\t\t\t= <bLighting>;\nFogEnable\t\t\t\t= <bFog>;\nDiffuseMaterialSource\t= < DiffuseSource >;\nAmbientMaterialSource\t= < DiffuseSource >;\nColorOp[ 0 ]\t\t\t= < Bcop >;\nColorArg1[ 0 ]\t\t\t= Texture;\nColorArg2[ 0 ]\t\t\t= Diffuse;\nAlphaOp[ 0 ]\t\t\t= < Baop >;\nAlphaArg1[ 0 ]\t\t\t= Texture;\nAlphaArg2[ 0 ]\t\t\t= Diffuse;\nColorOp[ 1 ]\t\t\t= < TFcop >;\nColorArg1[ 1 ]\t\t\t= Current;\nColorArg2[ 1 ]\t\t\t= Tfactor;\nAlphaOp[ 1 ]\t\t\t= < TFaop >;\nAlphaArg1[ 1 ]\t\t\t= Current;\nAlphaArg2[ 1 ]\t\t\t= Tfactor;\nColorOp[ 2 ]\t\t\t= Disable;\nAlphaOp[ 2 ]\t\t\t= Disable;\n}\n}\n";
		typedef char OriginalEffectSourceSize[(sizeof(source) - 1 == 753) ? 1 : -1];
	}

	BaseRenderStyles::~BaseRenderStyles()
	{
		Release();
	}

	void BaseRenderStyles::Initialize(DWORD colorOperation, DWORD alphaOperation)
	{
		if (effect)
			return;
		ID3DXBuffer* errors;
		const HRESULT compiled = StyleServices::Compile(context.device, source, sizeof(source) - 1,
			&effect, &errors);
		if (FAILED(compiled))
		{
			JE::LogDebugString("Gfx_RS_Base::Gfx_RS_Base - Failed to compile", compiled);
			JE::FatalError(static_cast<const char*>(StyleServices::ErrorText(errors)));
		}
		D3DXEFFECT_DESC description;
		HRESULT result = StyleServices::Description(effect, &description);
		if (FAILED(result))
			JE::FatalError("Gfx_RS_Base::Gfx_RS_Base - Failed GetDesc (%x)\n", result);
		result = StyleServices::Technique(effect, NULL);
		if (FAILED(result))
			JE::FatalError("Gfx_RS_Base::Gfx_RS_Base - Failed to initialize technique (%x)\n", result);
		StyleServices::Value(effect, "Bcop", colorOperation);
		StyleServices::Value(effect, "Baop", alphaOperation);
		StyleServices::Value(effect, "TFcop", 2);
		StyleServices::Value(effect, "TFaop", 2);
		StyleServices::Value(effect, "alphatest", 0);
		StyleServices::Value(effect, "bLighting", 0);
		StyleServices::Value(effect, "bFog", 0);
		StyleServices::Value(effect, "DiffuseSource", 1);
		StyleServices::Value(effect, "bZEnable", 1);
		D3DXTECHNIQUE_DESC selected;
		result = StyleServices::Next(effect, technique, &selected);
		if (FAILED(result))
			JE::FatalError("Gfx_RS_Base::SetMaterial - Failed to find valid technique (%x)\n", result);
		technique = selected.Index;
		StyleServices::Technique(effect, technique);
		if (selected.Name[0] == 's' && selected.Name[1] == 'h')
			shaderTechnique = 1;
		techniquePasses = selected.Passes;
		if (errors)
			StyleServices::Release(errors);
	}

	void BaseRenderStyles::Release()
	{
		if (effect)
		{
			StyleServices::Release(effect);
			effect = NULL;
		}
	}

	HRESULT BaseRenderStyles::SetMaterial(const UIMaterial& material, BYTE lighting, BYTE fog)
	{
		if (!effect)
			return D3DERR_INVALIDCALL;
		if (material.style != 9 && material.style != 20)
			throw std::logic_error("Unrecovered non-UI render style");
		context.SetTexture(0, material.texture);
		StyleServices::Value(effect, "bZEnable", material.depthTest);
		StyleServices::Value(effect, "alphatest", 1);
		StyleServices::Value(effect, "bLighting", material.lighting ? lighting != 0 : 0);
		StyleServices::Value(effect, "bFog", material.fog ? fog != 0 : 0);
		const DWORD factor = MaterialFactor(material);
		StyleServices::Value(effect, "DiffuseSource", material.vertexColor != 0);
		StyleServices::Value(effect, "Bcop", 5);
		StyleServices::Value(effect, "Baop", material.style == 9 ? 2 : 4);
		StyleServices::Material(context, &material.values);
		if (material.textureFactorColor || material.textureFactorAlpha)
		{
			StyleServices::Value(effect, "TFcop", material.textureFactorColor ? 5 : 2);
			StyleServices::Value(effect, "TFaop", material.textureFactorAlpha ? 4 : 2);
			context.SetRenderState(D3DRS_TEXTUREFACTOR, factor);
		}
		else
		{
			StyleServices::Value(effect, "TFcop", 1);
			StyleServices::Value(effect, "TFaop", 1);
		}
		const HRESULT result = context.SetRenderState(D3DRS_CULLMODE, material.cull ? 3 : 1);
		// UI materials have no texture-transform owner (original material+320).
		context.SetTextureStageState(0, D3DTSS_TEXTURETRANSFORMFLAGS, 0);
		if (techniqueChanged)
		{
			D3DXTECHNIQUE_DESC selected;
			const HRESULT found = StyleServices::Next(effect, technique, &selected);
			if (FAILED(found))
				JE::FatalError("Gfx_RS_Base::SetMaterial - Failed to find valid technique (%x)\n", found);
			technique = selected.Index;
			techniqueChanged = 0;
			techniquePasses = selected.Passes;
			return found;
		}
		return result;
	}

	HRESULT BaseRenderStyles::Begin(UINT* passes) { return StyleServices::Begin(effect, passes, 0); }
	HRESULT BaseRenderStyles::Pass(UINT pass)
	{
		currentPass = pass;
		return StyleServices::Pass(effect, pass);
	}
	HRESULT BaseRenderStyles::End() { return StyleServices::End(effect); }
	namespace StyleServices
	{
		HRESULT Compile(IDirect3DDevice8* device, const char* data, UINT size,
			ID3DXEffect** effect, ID3DXBuffer** errors)
		{ return D3DXCreateEffect(device, data, size, effect, errors); }
		HRESULT Description(ID3DXEffect* effect, D3DXEFFECT_DESC* out)
		{ return effect->GetDesc(out); }
		HRESULT Technique(ID3DXEffect* effect, const char* name)
		{ return effect->SetTechnique(name); }
		HRESULT Value(ID3DXEffect* effect, const char* name, DWORD value)
		{ return effect->SetDword(name, value); }
		HRESULT Next(ID3DXEffect* effect, const char* from, D3DXTECHNIQUE_DESC* out)
		{ return effect->FindNextValidTechnique(from, out); }
		ULONG Release(ID3DXEffect* effect) { return effect->Release(); }
		ULONG Release(ID3DXBuffer* errors) { return errors->Release(); }
		const char* ErrorText(ID3DXBuffer* errors)
		{ return static_cast<const char*>(errors->GetBufferPointer()); }
		HRESULT Material(D3DContext& context, const D3DMATERIAL8* values)
		{ return context.device->SetMaterial(values); }
		HRESULT Begin(ID3DXEffect* effect, UINT* count, DWORD flags)
		{ return effect->Begin(count, flags); }
		HRESULT Pass(ID3DXEffect* effect, UINT index) { return effect->Pass(index); }
		HRESULT End(ID3DXEffect* effect) { return effect->End(); }
	}

}
}

