#include "OctreeRender.hpp"
#include "SystemInterface.hpp"
#include "Viewport.hpp"

BYTE Octree2Gfx::Initialize(OCTREE* map, DWORD capacity, DWORD color, BYTE bounds, BYTE textures)
{
	resourceTextures = textures;
	materialStates.clear();
	fadeStates.clear();
	valueCC.clear();
	if (capacity && nodeIDCapacity != capacity)
	{
		if (nodeIDs) { delete[] nodeIDs; nodeIDs = NULL; }
		nodeIDCapacity = capacity;
		nodeIDs = new DWORD[capacity];
	}
	// Prefix effects above happen even when the owner already has a map.
	if (octree) return 0;
	showBounds = bounds;
	octree = map;
	float level[6];
	map->GetLevelBounds(level);
	minimum.x = static_cast<float>((level[3] - level[0]) * 0.5f);
	minimum.y = static_cast<float>((level[4] - level[1]) * 0.5f);
	minimum.z = static_cast<float>((level[5] - level[2]) * 0.5f);
	maximum = minimum;
	ConstructRenderData();
	SetAmbient(~color);
	Gfx_Camera* active = g_GfxSystem->DefaultViewport()->activeCamera;
	float degrees = active->fieldOfViewY * 57.295776f;
	// Original deliberately ignores status and dereferences the published camera.
	octree->CreateCamera(degrees, active->aspect, active->nearPlane, active->farPlane, &camera);
	D3DXMATRIX projection;
	active->GetProjectionMatrix(projection);
	RECT rectangle;
	GetClientRect(g_GfxSystem->Window(), &rectangle);
	D3DXMATRIX screenProjection(
		projection._11, projection._12, projection._13, projection._14,
		projection._21, projection._22, projection._23, projection._24,
		projection._31, projection._32, projection._33, projection._34,
		projection._41, projection._42, projection._43, projection._44);
	camera->SetScreen(static_cast<float>(rectangle.right - rectangle.left),
		static_cast<float>(rectangle.bottom - rectangle.top), &screenProjection);
	camera->SetMode(sceneMode);
	D3DXMATRIX identity(
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f);
	camera->SetTransform(&identity);
	return 1;
}
