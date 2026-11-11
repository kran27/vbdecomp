#include "OctreeRender.hpp"
#include "SystemInterface.hpp"
#include "Viewport.hpp"

int Octree2Gfx::UpdateCamera(Gfx_Viewport* viewport)
{
	int result = reinterpret_cast<int>(octree);
	if (result)
	{
		if (!viewport)
		{
			viewport = g_GfxSystem->DefaultViewport();
		}
		Gfx_Camera* active = viewport->activeCamera;
		D3DXMATRIX transform = active->transform;
		D3DVIEWPORT8 descriptor = viewport->viewport;
		D3DXMATRIX projection;
		active->GetProjectionMatrix(projection);
		float degrees = active->fieldOfViewY * 57.295776f;
		camera->SetFrustum(degrees, active->aspect, active->nearPlane, active->farPlane);
		camera->SetScreen(static_cast<float>(descriptor.Width),
			static_cast<float>(descriptor.Height), &projection);
		camera->SetTransform(&transform);
		result = octree->CullVisible(camera, 0);
	}
	return result;
}

int Octree2Gfx::RestoreVisibility()
{
	int result = reinterpret_cast<int>(octree);
	if (result)
	{
		result = octree->RestoreVisibility(camera);
	}
	return result;
}

void Octree2Gfx::SetHeightFogFlag(BYTE value)
{
	for (DWORD i = 0; i < meshCount; ++i)
	{
		instances[i].flag1D = value;
	}
}

void Octree2Gfx::ClearHiddenMaterials()
{
	valueC0.clear();
}

void Octree2Gfx::SetHiddenMaterials(const std::set<int>& hidden)
{
	valueC0 = hidden;
}
