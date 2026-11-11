#pragma once

#include "MeshData.hpp"
#include "PacketData.hpp"
#include <d3dx8.h>

class Gfx_Viewport;
class Gfx_Camera;
namespace JE
{
	struct VFX_Vector;
	//Actual shared204-byte aggregate4E6190, allocated lazily by Rain/Snow.
	//The buffers have16-byte Rain or24-byte Snow vertices; no STL/ABI overlay.
	struct VFX_WeatherMesh
	{
		VFX_WeatherMesh();
		Gfx_MeshData mesh;
		Gfx_MeshInstance* instances;
		Gfx_PacketData* packet;
		D3DXMATRIX transform;
		DWORD instanceCount;
		void* vertices;
		DWORD vertexCount;
		WORD* indices;
		DWORD indexCount;
		BYTE ready;
	private:
		VFX_WeatherMesh(const VFX_WeatherMesh&);
		VFX_WeatherMesh& operator=(const VFX_WeatherMesh&);
	};
	Gfx_Camera* WeatherCamera(Gfx_Viewport* viewport);
	void FinishWeatherMesh(VFX_WeatherMesh& mesh,DWORD fvf,DWORD stride,
		D3DPRIMITIVETYPE primitiveType,DWORD primitiveCount,LONG renderPhase,
		DWORD materialStyle,DWORD texture,const D3DXMATRIX& camera,
		const VFX_Vector& offset,float scale); //Common4E6FA3/4E5976 upload/material suffix.
}
