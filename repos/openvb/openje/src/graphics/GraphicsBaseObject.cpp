#include "GraphicsBaseObject.hpp"

void Gfx_BaseObject::SetTransform(const D3DXMATRIX& matrix)
{
	const D3DXMATRIX snapshot(matrix);
	transform = snapshot;
}

void Gfx_BaseObject::SetPosition(float x, float y, float z)
{
	transform._41 = x;
	transform._42 = y;
	transform._43 = z;
}

void Gfx_BaseObject::SetPosition(const D3DXVECTOR3& position)
{
	SetPosition(position.x, position.y, position.z);
}

void Gfx_BaseObject::SetEulerRotation(float yaw, float pitch, float roll)
{
	const D3DXVECTOR3 position(transform._41, transform._42, transform._43);
	D3DXMatrixRotationYawPitchRoll(&transform, yaw, pitch, roll);
	transform._41 = position.x;
	transform._42 = position.y;
	transform._43 = position.z;
}

void Gfx_BaseObject::SetQuaternionRotation(const D3DXQUATERNION& rotation)
{
	const D3DXVECTOR3 position(transform._41, transform._42, transform._43);
	D3DXVECTOR3 axis;
	float angle;
	D3DXQuaternionToAxisAngle(&rotation, &axis, &angle);
	D3DXMatrixRotationAxis(&transform, &axis, angle);
	transform._41 = position.x;
	transform._42 = position.y;
	transform._43 = position.z;
}

void Gfx_BaseObject::OnTransformChanged()
{
}
