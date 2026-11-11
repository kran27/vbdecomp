#include "CameraController.hpp"
#include "Camera.hpp"
#include <math.h>
#include <string.h>

Gfx_CamCtrl::Gfx_CamCtrl() : dirty(1)
{
	D3DXMatrixIdentity(&transform);
	ExtractBasis();
}

Gfx_CamCtrl::~Gfx_CamCtrl()
{
	// Real STL owns the nodes/head. No camera payload is deleted or detached.
}

void Gfx_CamCtrl::ExtractBasis()
{
	right.x = transform._11;
	right.y = transform._12;
	right.z = transform._13;
	up.x = transform._21;
	up.y = transform._22;
	up.z = transform._23;
	look.x = transform._31;
	look.y = transform._32;
	look.z = transform._33;
	position.x = transform._41;
	position.y = transform._42;
	position.z = transform._43;
}

void Gfx_CamCtrl::RemoveCamera(Gfx_Camera* camera)
{
	if (camera)
	{
		std::map<Gfx_Camera*, BYTE>::iterator found = cameras.find(camera);
		if (found != cameras.end())
		{
			cameras.erase(found);
		}
	}
}

BYTE Gfx_CamCtrl::Update()
{
	if (dirty)
	{
		ComputeTransform();
		ExtractBasis();
		for (std::map<Gfx_Camera*, BYTE>::iterator current = cameras.begin();
			current != cameras.end(); ++current)
		{
			Gfx_Camera* camera = current->first;
			D3DXMATRIX snapshot;
			memcpy(&snapshot, &transform, sizeof(snapshot));
			memcpy(&camera->transform, &snapshot, sizeof(snapshot));
			camera->viewDirty = 1;
		}
		dirty = 0;
		return 1;
	}
	for (std::map<Gfx_Camera*, BYTE>::iterator current = cameras.begin();
		current != cameras.end(); ++current)
	{
		if (current->first->viewDirty || current->first->projectionDirty)
		{
			return 1;
		}
	}
	return 0;
}

Gfx_CamCtrl_Orbit::Gfx_CamCtrl_Orbit()
	: upAxis(0.0f, 1.0f, 0.0f), target(0.0f, 0.0f, 0.0f), azimuth(0.0f),
	elevation(-1.5620697736740112f), distance(1.0f),
	minimumElevation(-1.5620697736740112f), maximumElevation(1.5620697736740112f),
	minimumDistance(0.0f), maximumDistance(1000.0f)
{
}

void Gfx_CamCtrl_Orbit::SetOrbit(float newAzimuth, float newElevation, float newDistance)
{
	// Original double651B48 equals the float651B44 value, not mathematical2pi.
	const double twoPi = 6.2831854820251465;
	const float remainder = static_cast<float>(fmod(newAzimuth, twoPi));
	azimuth = static_cast<float>(remainder);
	if (remainder < 0.0f)
	{
		//424682/424695 use float zero and float twoPi operands. Their values
		//promote exactly, but their source widths also determine the x87 opcodes.
		azimuth = static_cast<float>(remainder + 6.2831855f);
	}
	elevation = newElevation;
	if (newElevation < minimumElevation)
	{
		elevation = minimumElevation;
	}
	if (elevation > maximumElevation)
	{
		elevation = maximumElevation;
	}
	distance = newDistance;
	if (newDistance < minimumDistance)
	{
		distance = minimumDistance;
	}
	dirty = 1;
	if (distance > maximumDistance)
	{
		distance = maximumDistance;
	}
}

void Gfx_CamCtrl_Orbit::Rotate(float azimuth, float elevation, float distance)
{
	const float newDistance = distance + this->distance;
	const float newElevation = elevation + this->elevation;
	const float newAzimuth = azimuth + this->azimuth;
	SetOrbit(newAzimuth, newElevation, newDistance);
}

void Gfx_CamCtrl_Orbit::ComputeTransform()
{
	// Original4C6430 constructs these rotation matrices around real CRT
	// sine/cosine expressions. SDK RotationX/Y has observed rounding differences.
	const double sineAzimuth = sin(azimuth);
	const double cosineAzimuth = cos(azimuth);
	const float sineElevation = static_cast<float>(sin(elevation));
	const float cosineElevation = static_cast<float>(cos(elevation));
	D3DXMATRIX rotationY(static_cast<float>(cosineAzimuth), 0.0f, static_cast<float>(-sineAzimuth), 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		static_cast<float>(sineAzimuth), 0.0f, static_cast<float>(cosineAzimuth), 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f);
	D3DXMATRIX rotationX(1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, cosineElevation, sineElevation, 0.0f,
		0.0f, -sineElevation, cosineElevation, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f);
	D3DXMATRIX rotation, targetTranslation, distanceTranslation;
	D3DXMatrixMultiply(&rotation, &rotationX, &rotationY);
	D3DXMatrixTranslation(&targetTranslation, target.x, target.y, target.z);
	D3DXMatrixTranslation(&distanceTranslation, 0.0f, 0.0f, -distance);
	D3DXMatrixMultiply(&rotation, &distanceTranslation, &rotation);
	D3DXMatrixMultiply(&transform, &rotation, &targetTranslation);
}

void Gfx_CamCtrl_Orbit::TranslateTarget(float x, float y, float z)
{
	D3DXVECTOR3 rightDirection, lookDirection;
	{
		D3DXVECTOR3 input(right.x, 0.0f, right.z);
		D3DXVec3Normalize(&rightDirection, &input);
	}
	{
		D3DXVECTOR3 input(look.x, 0.0f, look.z);
		D3DXVec3Normalize(&lookDirection, &input);
	}
	D3DXMATRIX basis(rightDirection.x, rightDirection.y, rightDirection.z, 0.0f,
		upAxis.x, upAxis.y, upAxis.z, 0.0f,
		lookDirection.x, lookDirection.y, lookDirection.z, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f);
	D3DXVECTOR3 movement(x, y, z);
	D3DXVec3TransformCoord(&movement, &movement, &basis);
	target += movement;
	dirty = 1;
}
