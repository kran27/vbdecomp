#pragma once
#include <windows.h>
namespace JE { namespace EngineInterfaceServices
{
    //Explicit output boundary for actual new/CSound.Load/Play request.
    typedef void (*TransientSoundRequest)(const char*,BYTE,DWORD,DWORD,DWORD,void*);
    TransientSoundRequest SetTransientSoundRequest(TransientSoundRequest request,void* context);
    void RequestTransientSound(const char* name,BYTE loadFlag,DWORD a,DWORD b,DWORD c);
} }

