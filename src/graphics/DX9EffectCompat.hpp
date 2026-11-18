#pragma once

#include <d3dx9.h>
#include <cstdint>
#include <new>

// The renderer works with zero-based technique indices. The D3DX9 handles
// are obtained privately from ID3DXEffect and must not be used as indices.
struct Gfx_DX9TechniqueDesc
{
    DWORD Index = 0;
    LPCSTR Name = nullptr;
    UINT Passes = 0;
};

class Gfx_DX9Effect
{
  public:
    explicit Gfx_DX9Effect(ID3DXEffect* effect) : m_effect(effect) {}

    Gfx_DX9Effect(const Gfx_DX9Effect&) = delete;
    Gfx_DX9Effect& operator=(const Gfx_DX9Effect&) = delete;

    ULONG Release()
    {
        const ULONG references = m_effect->Release();
        delete this;
        return references;
    }

    HRESULT GetDesc(D3DXEFFECT_DESC* desc) { return m_effect->GetDesc(desc); }

    HRESULT OnLostDevice() { return m_effect->OnLostDevice(); }

    HRESULT OnResetDevice() { return m_effect->OnResetDevice(); }

    HRESULT Begin(UINT* passes, DWORD flags)
    {
        const HRESULT result = m_effect->Begin(passes, flags);
        if (SUCCEEDED(result))
        {
            m_passActive = false;
        }
        return result;
    }

    // D3DX8 Pass() applied the chosen pass directly. D3DX9 requires every
    // BeginPass() to be paired with EndPass().
    HRESULT Pass(UINT pass)
    {
        if (m_passActive)
        {
            const HRESULT result = m_effect->EndPass();
            if (FAILED(result))
            {
                return result;
            }
            m_passActive = false;
        }

        const HRESULT result = m_effect->BeginPass(pass);
        if (SUCCEEDED(result))
        {
            m_passActive = true;
        }
        return result;
    }

    HRESULT End()
    {
        if (m_passActive)
        {
            const HRESULT result = m_effect->EndPass();
            if (FAILED(result))
            {
                return result;
            }
            m_passActive = false;
        }
        return m_effect->End();
    }

    HRESULT SetDword(D3DXHANDLE name, DWORD value) { return m_effect->SetInt(name, static_cast<INT>(value)); }

    HRESULT SetMatrix(D3DXHANDLE name, const D3DXMATRIX* matrix) { return m_effect->SetMatrix(name, matrix); }

    HRESULT SetTexture(D3DXHANDLE name, IDirect3DBaseTexture9* texture) { return m_effect->SetTexture(name, texture); }

    HRESULT SetTechnique(int index)
    {
        if (index < 0)
        {
            return D3DERR_INVALIDCALL;
        }
        return SetTechniqueIndex(static_cast<UINT>(index));
    }

    HRESULT SetTechnique(DWORD index) { return SetTechniqueIndex(index); }

HRESULT GetParameterDesc(DWORD index, D3DXPARAMETER_DESC* desc)
    {
        if (!desc)
        {
            return E_POINTER;
        }

        D3DXEFFECT_DESC effectDesc{};
        const HRESULT result = m_effect->GetDesc(&effectDesc);
        if (FAILED(result))
        {
            return result;
        }

        if (index >= effectDesc.Parameters)
        {
            return D3DERR_INVALIDCALL;
        }

        const D3DXHANDLE parameter = m_effect->GetParameter(nullptr, index);
        if (!parameter)
        {
            return D3DERR_INVALIDCALL;
        }

        return m_effect->GetParameterDesc(parameter, desc);
    }

    HRESULT FindNextValidTechnique(int from, Gfx_DX9TechniqueDesc* desc)
    {
        if (from < 0)
        {
            return D3DERR_INVALIDCALL;
        }
        return FindValidTechniqueFrom(static_cast<UINT>(from), desc);
    }

    HRESULT FindNextValidTechnique(DWORD from, Gfx_DX9TechniqueDesc* desc)
    {
        return FindValidTechniqueFrom(from, desc);
    }


  private:
    HRESULT SetTechniqueIndex(std::uintptr_t index)
    {
        D3DXEFFECT_DESC desc = {};
        const HRESULT result = m_effect->GetDesc(&desc);
        if (FAILED(result))
        {
            return result;
        }
        if (index >= desc.Techniques)
        {
            return D3DERR_INVALIDCALL;
        }

        const D3DXHANDLE handle = m_effect->GetTechnique(static_cast<UINT>(index));
        if (!handle)
        {
            return D3DERR_INVALIDCALL;
        }
        return m_effect->SetTechnique(handle);
    }

    // Unlike D3DX9 FindNextValidTechnique(), the D3DX8 code expects an
    // inclusive, zero-based search. ValidateTechnique() preserves that usage.
    HRESULT FindValidTechniqueFrom(std::uintptr_t from, Gfx_DX9TechniqueDesc* output)
    {
        if (!output)
        {
            return E_POINTER;
        }

        *output = {};
        D3DXEFFECT_DESC effectDesc = {};
        const HRESULT result = m_effect->GetDesc(&effectDesc);
        if (FAILED(result))
        {
            return result;
        }

        for (std::uintptr_t index = from; index < effectDesc.Techniques; ++index)
        {
            const D3DXHANDLE handle = m_effect->GetTechnique(static_cast<UINT>(index));
            if (!handle || FAILED(m_effect->ValidateTechnique(handle)))
            {
                continue;
            }

            D3DXTECHNIQUE_DESC details = {};
            const HRESULT describe = m_effect->GetTechniqueDesc(handle, &details);
            if (FAILED(describe))
            {
                return describe;
            }

            output->Index = static_cast<DWORD>(index);
            output->Name = details.Name;
            output->Passes = details.Passes;
            return S_OK;
        }

        return D3DERR_INVALIDCALL;
    }

    ID3DXEffect* m_effect = nullptr;
    bool m_passActive = false;
};

inline HRESULT Gfx_CreateDX9Effect(IDirect3DDevice9* device, const void* source, UINT length, Gfx_DX9Effect** output,
                                   ID3DXBuffer** errors)
{
    if (!output)
    {
        return E_POINTER;
    }

    *output = nullptr;
    if (errors)
    {
        *errors = nullptr;
    }

    ID3DXEffect* raw = nullptr;
    const HRESULT result = D3DXCreateEffect(device, source, length, nullptr, nullptr, 0, nullptr, &raw, errors);

    if (FAILED(result))
    {
        if (raw)
        {
            raw->Release();
        }
        return result;
    }

    Gfx_DX9Effect* wrapper = new (std::nothrow) Gfx_DX9Effect(raw);
    if (!wrapper)
    {
        raw->Release();
        return E_OUTOFMEMORY;
    }

    *output = wrapper;
    return result;
}
