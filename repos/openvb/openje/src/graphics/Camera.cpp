#include "Camera.hpp"

void Gfx_Camera::GetViewMatrix(D3DXMATRIX& output)
{
	if (viewDirty) UpdateViewMatrix();
	output = viewMatrix;
}

void Gfx_Camera::GetProjectionMatrix(D3DXMATRIX& output)
{
	if (projectionDirty) UpdateProjectionMatrix();
	output = projectionMatrix;
}

void Gfx_Camera::GetInverseViewMatrix(D3DXMATRIX& output)
{
	if (inverseViewDirty)
	{
		D3DXMatrixInverse(&inverseViewMatrix, NULL, &viewMatrix);
		inverseViewDirty = 0;
	}
	output = inverseViewMatrix;
}

void Gfx_Camera::UpdateViewMatrix()
{
	for (UINT row = 0; row < 3; ++row)
	{
		viewMatrix.m[row][0] = transform.m[0][row];
		viewMatrix.m[row][1] = transform.m[1][row];
		viewMatrix.m[row][2] = transform.m[2][row];
	}
	const D3DXVECTOR3 position(transform._41, transform._42, transform._43);
	const D3DXVECTOR3 xAxis(transform._11, transform._12, transform._13);
	const D3DXVECTOR3 yAxis(transform._21, transform._22, transform._23);
	const D3DXVECTOR3 zAxis(transform._31, transform._32, transform._33);
	viewMatrix._41 = -D3DXVec3Dot(&position, &xAxis);
	viewMatrix._42 = -D3DXVec3Dot(&position, &yAxis);
	viewMatrix._43 = -D3DXVec3Dot(&position, &zAxis);
	// The fourth column is deliberately untouched, including _44.
	inverseViewDirty = 1;
	viewDirty = 0;
}

void Gfx_Camera::UpdateProjectionMatrix()
{
	D3DXMatrixPerspectiveFovLH(&projectionMatrix, fieldOfViewY, aspect, nearPlane, farPlane);
	projectionDirty = 0;
}
