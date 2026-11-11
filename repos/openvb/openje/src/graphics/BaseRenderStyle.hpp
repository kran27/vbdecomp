#pragma once

#include "RenderQueue.hpp"
#include <d3dx8effect.h>

namespace JE
{
namespace Gfx
{
	// Game-owned material policy; the actual DirectX8.1 SDK owns the effect runtime.
	class BaseRenderStyles
	{
	public:
		explicit BaseRenderStyles(D3DContext& context) : context(context), effect(NULL),
			technique(NULL), currentPass(0), techniqueChanged(0), shaderTechnique(0) {}
		~BaseRenderStyles();
		void Initialize(DWORD colorOperation = 5, DWORD alphaOperation = 2);
		void Release();
		HRESULT SetMaterial(const UIMaterial& material, BYTE lighting, BYTE fog);
		HRESULT Begin(UINT* passes);
		HRESULT Pass(UINT pass);
		HRESULT End();
		D3DContext& context;
		ID3DXEffect* effect;
		const char* technique; // Borrowed from actual SDK effect.
		UINT techniquePasses;
		UINT currentPass;
		BYTE techniqueChanged, shaderTechnique;
	private:
		BaseRenderStyles(const BaseRenderStyles&);
		BaseRenderStyles& operator=(const BaseRenderStyles&);
	};

	namespace StyleServices
	{
		HRESULT Compile(IDirect3DDevice8*, const char*, UINT, ID3DXEffect**, ID3DXBuffer**);
		HRESULT Description(ID3DXEffect*, D3DXEFFECT_DESC*);
		HRESULT Technique(ID3DXEffect*, const char*);
		HRESULT Value(ID3DXEffect*, const char*, DWORD);
		HRESULT Next(ID3DXEffect*, const char*, D3DXTECHNIQUE_DESC*);
		ULONG Release(ID3DXEffect*);
		ULONG Release(ID3DXBuffer*);
		const char* ErrorText(ID3DXBuffer*);
		HRESULT Material(D3DContext&, const D3DMATERIAL8*);
		HRESULT Begin(ID3DXEffect*, UINT*, DWORD);
		HRESULT Pass(ID3DXEffect*, UINT);
		HRESULT End(ID3DXEffect*);
	}
}
}
