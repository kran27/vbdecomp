#pragma once

#include <d3dx9.h>
#include <cstdint>
#include <new>

// D3DX8 exposed the technique index as an LPCSTR-like value.  The existing
// renderer stores and increments that value as a zero-based integer.
// Keep that convention at this boundary; never expose a D3DX9 technique handle
// to those render-style classes.
struct Gfx_DX9TechniqueDesc
{
    D3DXHANDLE Index = nullptr;
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

    // The existing callers pass D3DX8-style numeric indices encoded as LPCSTR.
    HRESULT SetTechnique(D3DXHANDLE index) { return SetTechniqueIndex(reinterpret_cast<std::uintptr_t>(index)); }

    HRESULT GetParameterDesc(D3DXHANDLE parameter, D3DXPARAMETER_DESC* desc)
    {
        D3DXEFFECT_DESC effectDesc = {};
        const HRESULT result = m_effect->GetDesc(&effectDesc);
        if (FAILED(result))
        {
            return result;
        }

        const std::uintptr_t value = reinterpret_cast<std::uintptr_t>(parameter);
        if (value < effectDesc.Parameters)
        {
            parameter = m_effect->GetParameter(nullptr, static_cast<UINT>(value));
        }
        else if (value < 65536)
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

    HRESULT FindNextValidTechnique(D3DXHANDLE from, Gfx_DX9TechniqueDesc* desc)
    {
        return FindValidTechniqueFrom(reinterpret_cast<std::uintptr_t>(from), desc);
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

            output->Index = reinterpret_cast<D3DXHANDLE>(index);
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
