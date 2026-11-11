#include "Camera.hpp"

Gfx_Camera::Gfx_Camera() : value44(0)
{
	for (UINT i = 0; i < 4; ++i)
	{
		unknown11C[i] = 0;
	}
	D3DXMatrixIdentity(&viewMatrix);
	D3DXMatrixIdentity(&inverseViewMatrix);
	D3DXMatrixIdentity(&projectionMatrix);
	aspect = 4.0f / 3.0f;
	farPlane = 20000.0f;
	fieldOfViewY = 15.0f * g_GfxDegreesToRadians;
	nearPlane = 10.0f;
	viewDirty = 1;
	inverseViewDirty = 1;
	projectionDirty = 1;
	// Byte4B is deliberately unwritten. The original establishes no role for it.
}

void Gfx_Camera::SetPosition(float x, float y, float z)
{
	transform._41 = x;
	transform._42 = y;
	transform._43 = z;
	viewDirty = 1;
}
