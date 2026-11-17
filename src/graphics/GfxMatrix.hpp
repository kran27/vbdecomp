#pragma once

#include <d3dx9math.h>

// 412AD0: 64-byte matrix value; original type name and provider remain unresolved.
struct Gfx_Matrix4x4
{
    Gfx_Matrix4x4();
    D3DXMATRIX matrix;
};

typedef char GfxMatrix4x4Size[(sizeof(Gfx_Matrix4x4) == 64) ? 1 : -1];

// 48F6E0..48FD50: observed float-valued helpers; original provider is unresolved.
struct Gfx_Matrix3x3
{
    void SetZero(); // 48F6E0; three independent twelve-byte rows.
    void SetDiagonal(float diagonal); // 48F780; preserves the supplied word.
    void SetIdentity(float diagonal); // 48FD30; zero, then diagonal.
    float value[9];
};

struct Gfx_AffineMatrix3x4
{
    void SetIdentity(float diagonal); // 48FD50; linear reset, then final three words.
    Gfx_Matrix3x3 linear;
    float translation[3];
};

typedef char GfxMatrix3x3Size[(sizeof(Gfx_Matrix3x3) == 36) ? 1 : -1];
typedef char GfxAffineMatrix3x4Size[(sizeof(Gfx_AffineMatrix3x4) == 48) ? 1 : -1];
