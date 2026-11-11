#pragma once

#include <windows.h>

//Original loader-zero handles. Their distinct consumers are still unresolved;
//the two identical declaration streams do not make these interchangeable owners.
extern DWORD g_VertexShader70D094;
extern DWORD g_VertexShader70D090;

HRESULT Gfx_CreateBuiltinVertexShaders(); //5CC3D0; first HRESULT ignored.
void Gfx_DeleteBuiltinVertexShaders(); //5CC410; unused EAX is not a return contract.
