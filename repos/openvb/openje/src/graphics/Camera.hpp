#pragma once

#include "GraphicsBaseObject.hpp"

// Actual shared read-only degree conversion constant at0x649C50. Keeping its
// definition in a separate translation unit preserves the observed live multiply.
extern const float g_GfxDegreesToRadians;

// Actual Gfx_Camera RTTI identity and observed300-byte layout.
class Gfx_Camera : public Gfx_BaseObject
{
public:
	Gfx_Camera(); // 0x495EE0.
	virtual ~Gfx_Camera() {} // Ordinary policy folds to base0x4245B0; no separate address.
	virtual void SetPosition(float x, float y, float z); // +0C ->0x496090.
	void SetViewTransform(const D3DXMATRIX& matrix); //529EC0; snapshot and view-dirty flag.
	HRESULT Prepare(); // 0x496200; projection then live view state, returns final SDK result.
	void GetViewMatrix(D3DXMATRIX& output); // 0x496250; void source-return hypothesis.
	void GetInverseViewMatrix(D3DXMATRIX& output); //0x496280; uses current view without refreshing.
	void GetProjectionMatrix(D3DXMATRIX& output); // 0x4962C0; void return hypothesis.
	void UpdateViewMatrix(); // 0x4960D0; partial matrix writes and dirty policy.
	void UpdateProjectionMatrix(); // 0x4961C0; real D3DXMatrixPerspectiveFovLH.

	DWORD value44; // +44; role not recovered here.
	BYTE viewDirty, inverseViewDirty, projectionDirty; // +48/+49/+4A; raw flags.
	BYTE unknown4B;
	D3DXMATRIX viewMatrix; // +4C.
	D3DXMATRIX inverseViewMatrix; // +8C; lazy real D3DX inverse of viewMatrix.
	D3DXMATRIX projectionMatrix; // +CC.
	float aspect, fieldOfViewY, nearPlane, farPlane; // +10C/+110/+114/+118.
	DWORD unknown11C[4]; // +11C..12B; constructor zeros all four words, roles unresolved.
};
