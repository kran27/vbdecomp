#include "Camera.hpp"
#include "D3DState.hpp"

HRESULT Gfx_Camera::Prepare()
{
	if (projectionDirty) UpdateProjectionMatrix();
	g_D3D.SetTransform(D3DTS_PROJECTION, &projectionMatrix);
	// The projection SDK callback may change this flag or the view matrix.
	if (viewDirty) UpdateViewMatrix();
	return g_D3D.SetTransform(D3DTS_VIEW, &viewMatrix);
}
