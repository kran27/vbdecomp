#include "DX9Font.hpp"
#include <tchar.h>

CD3DFont::CD3DFont(const TCHAR* name, DWORD height, DWORD flags) : m_height(height), m_flags(flags)
{
    _tcsncpy_s(m_name, name, _TRUNCATE);
}

CD3DFont::~CD3DFont()
{
    DeleteDeviceObjects();
}

HRESULT CD3DFont::InitDeviceObjects(IDirect3DDevice9* device)
{
    DeleteDeviceObjects();
    if (!device)
    {
        return E_POINTER;
    }
    return D3DXCreateFont(device, static_cast<INT>(m_height), 0, (m_flags & 1) ? FW_BOLD : FW_NORMAL, 1,
                          (m_flags & 2) != 0, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, ANTIALIASED_QUALITY,
                          DEFAULT_PITCH | FF_DONTCARE, m_name, &m_font);
}

HRESULT CD3DFont::RestoreDeviceObjects()
{
    return m_font ? m_font->OnResetDevice() : S_OK;
}

HRESULT CD3DFont::InvalidateDeviceObjects()
{
    return m_font ? m_font->OnLostDevice() : S_OK;
}

HRESULT CD3DFont::DeleteDeviceObjects()
{
    if (m_font)
    {
        m_font->Release();
        m_font = nullptr;
    }
    return S_OK;
}

HRESULT CD3DFont::DrawText(FLOAT x, FLOAT y, DWORD color, const TCHAR* text, DWORD flags)
{
    if (!m_font || !text)
    {
        return D3DERR_INVALIDCALL;
    }
    RECT bounds = { static_cast<LONG>(x), static_cast<LONG>(y), static_cast<LONG>(x) + 4096,
                    static_cast<LONG>(y) + 4096 };
    const DWORD format = (flags & 1) ? DT_CENTER | DT_NOCLIP : DT_LEFT | DT_TOP | DT_NOCLIP;
    return m_font->DrawText(nullptr, text, -1, &bounds, format, color) != 0 ? S_OK : E_FAIL;
}
