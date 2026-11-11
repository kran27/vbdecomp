#pragma once
#include <windows.h>
#include <d3dx8math.h>
#include "../startup/FrameLookupCaches.hpp"

namespace F3
{
	//Shared original6FEEA0 allocator/cache publication for55A030 and559D90.
	//Consumes the vector; empty results delete it and publish handle0.
	extern DWORD nextObjectList; //Original6FEEA0, initial100.
	inline DWORD PublishScriptObjectList(DWORD* handle,FrameLookupCaches::ObjectList* results)
	{
		if (results->empty())
		{
			*handle=0;
			delete results;
			return 0;
		}
		*handle=nextObjectList ? nextObjectList : 1;
		nextObjectList=*handle+1;
		g_FrameObjectResults[*handle]=results; //Original wrap overwrites without deleting old payload.
		return DWORD(results->size());
	}
	DWORD FindScriptObjects(DWORD* handle, DWORD mapID, const D3DXVECTOR3& position,
		float radius, DWORD exclude); //559D90, shared actual object-list producer.
}
