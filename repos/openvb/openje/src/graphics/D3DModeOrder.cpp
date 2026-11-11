// Adapted from DirectX8.1 SDK Common/src/d3dapp.cpp SortModesCallback.
// Copyright (c) 1998-2001 Microsoft Corporation. All rights reserved.
// Baseline pinned by config/directx81-samples.lock.json; SDK EULA retained.
// Only the refresh-rate tie-break is an evidenced game modification.
#include "D3DModeOrder.hpp"

int __cdecl CompareD3DDisplayModes( const VOID* arg1, const VOID* arg2 )
{
    D3DDISPLAYMODE* p1 = (D3DDISPLAYMODE*)arg1;
    D3DDISPLAYMODE* p2 = (D3DDISPLAYMODE*)arg2;

    if( p1->Format > p2->Format )   return -1;
    if( p1->Format < p2->Format )   return +1;
    if( p1->Width  < p2->Width )    return -1;
    if( p1->Width  > p2->Width )    return +1;
    if( p1->Height < p2->Height )   return -1;
    if( p1->Height > p2->Height )   return +1;

    // Original F3.exe0x502510 adds refresh-rate ordering to the SDK baseline.
    if( p1->RefreshRate < p2->RefreshRate ) return -1;
    if( p1->RefreshRate > p2->RefreshRate ) return +1;
    return 0;
}
