#include "VfxFrame.hpp"
#include "VfxParameters.hpp"
#include <string.h>

namespace JE
{
	//0x4B8130: NULL output counts bytes; reload the live offset after copying.
	DWORD WriteVfxBuffer(const void* input, DWORD size, DWORD count, BYTE* buffer, DWORD& offset)
	{
		if (buffer) memcpy(buffer + offset, input, size * count);
		offset += size * count;
		return size * count;
	}

	namespace VfxFrame
	{
		namespace
		{
			OCTREE_Camera* currentCamera; //Original707F0C, loader zero.
			float windX, windY, windZ; //707F10/14/18.
			float pendingX, pendingY, pendingZ; //707F2C/30/34.
		}
		void SetCamera(OCTREE_Camera* camera) { currentCamera = camera; }
		OCTREE_Camera* GetCamera() { return currentCamera; }
		void SetWind(float x,float y,float z)
		{ windX = x; windY = y; windZ = z; }
		void GetWind(float& x,float& y,float& z)
		{ x = windX; y = windY; z = windZ; }
		void AddWind(float x,float y,float z)
		{ pendingX += x; pendingY += y; pendingZ += z; }
		void SetWind(const D3DXVECTOR3& wind)
		{ SetWind(wind.x,wind.y,wind.z); }
		void GetWind(D3DXVECTOR3& wind)
		{ GetWind(wind.x,wind.y,wind.z); }
		void AddWind(const D3DXVECTOR3& wind)
		{ AddWind(wind.x,wind.y,wind.z); }
		void AdvanceWind(float elapsedSeconds)
		{
			windX = pendingX; windY = pendingY; windZ = pendingZ;
			pendingX = pendingY = pendingZ = 0;
		}
	}
}
