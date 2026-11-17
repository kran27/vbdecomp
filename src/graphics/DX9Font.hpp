#pragma once

#include <tchar.h>
#include <d3dx9.h>

// Small D3DX9-backed replacement for the DX8 CD3DFont sample used only by
// the statistics overlay; avoids pulling in its obsolete compiled sample.
class CD3DFont
{
  public:
    CD3DFont(const TCHAR* name, DWORD height, DWORD flags);
    ~CD3DFont();

    HRESULT InitDeviceObjects(IDirect3DDevice9* device);
    HRESULT RestoreDeviceObjects();
    HRESULT InvalidateDeviceObjects();
    HRESULT DeleteDeviceObjects();
    HRESULT DrawText(FLOAT x, FLOAT y, DWORD color, const TCHAR* text, DWORD flags = 0);

  private:
    TCHAR m_name[80] = {};
    DWORD m_height = 0;
    DWORD m_flags = 0;
    ID3DXFont* m_font = nullptr;
};
