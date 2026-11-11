#include "FogOfWarTextureMatrix.hpp"
#include "D3DState.hpp"
#include "SystemInterface.hpp"
#include "Camera.hpp"

void Gfx_BuildFogOfWarTextureMatrix(D3DXMATRIX& output)
{
	D3DXMATRIX map = g_D3D.identity;
	D3DXVECTOR3 minimum, maximum;
	g_GfxSystem->GetLevelAABB(&minimum.x, &maximum.x);
	const float width = maximum.x - minimum.x;
	const float height = maximum.z - minimum.z;
	D3DXMATRIX inverseView;
	g_GfxSystem->Camera()->GetInverseViewMatrix(inverseView);
	const double inverseWidth = 1.0 / double(width);
	map._22 = 0.0f;
	map._33 = 0.0f;
	map._11 = float(inverseWidth);
	map._41 = float(-(inverseWidth * minimum.x));
	const double inverseHeight = 1.0 / double(height);
	map._32 = float(-1.0 * inverseHeight);
	map._42 = float(inverseHeight * maximum.z);
	D3DXMATRIX product;
	D3DXMatrixMultiply(&product, &inverseView, &map);
	output = product; //Actual owned copies; no half-texel term or dimension query.
}

