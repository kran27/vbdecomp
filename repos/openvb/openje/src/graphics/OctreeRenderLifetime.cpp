#include "OctreeRender.hpp"
#include "SystemInterface.hpp"

OctreeFloorSegment::OctreeFloorSegment()
	: visible(0), nodeOrdinal(0), triangleCount(0), indexCount(0), indices(0)
{
}

OctreeFloorSegment::~OctreeFloorSegment()
{
	if (indices)
	{
		delete[] indices;
		indices = NULL;
	}
}

OctreeFloorGroup::OctreeFloorGroup()
	: visible(0), count(0), segments(0)
{
}

OctreeFloorGroup::~OctreeFloorGroup()
{
	if (segments)
	{
		delete[] segments;
		segments = NULL;
	}
}

Octree2Gfx::Octree2Gfx()
	: floorEnabled(0), resourceTextures(1), sceneMode(1),
	octree(0), camera(0), minimum(0, 0, 0), maximum(0, 0, 0), nodeIDCapacity(0),
	nodeIDs(0), materialCount(0), textureCount(0), textureHandles(0), value4C(13),
	lightCount(0), lights(0), meshCount(0), nodeCount(0), vertices(0), indices(0),
	floorGroups(0), meshes(0), instances(0), packets(0), nodeMinimum(0), nodeMaximum(0),
	value80(0), showBounds(0), boundsVertices(0), boundsVertexCount(0), boundsIndices(0),
	boundsMesh(0), boundsPrimitive(0), boundsInstance(0)
{
	ambient.x = 0;
	ambient.y = 0;
	ambient.z = 0;
	ambient.w = 1;
}

Octree2Gfx::~Octree2Gfx()
{
	Cleanup();
	// Normal member destruction retains the original reverse container order.
}

OCTREE* Octree2Gfx::Cleanup()
{
	OCTREE* result = octree;
	for (DWORD index = 0; index < meshCount; ++index)
	{
		g_GfxSystem->ReleaseGeometryData(meshes + index);
		if (packets && packets[index])
		{
			delete packets[index];
			packets[index] = NULL;
		}
		if (vertices && vertices[index]) { delete[] vertices[index]; vertices[index] = NULL; }
		if (indices && indices[index]) { delete[] indices[index]; indices[index] = NULL; }
		if (nodeMinimum && nodeMinimum[index])
		{
			delete[] nodeMinimum[index]; nodeMinimum[index] = NULL;
		}
		if (nodeMaximum && nodeMaximum[index])
		{
			delete[] nodeMaximum[index]; nodeMaximum[index] = NULL;
		}
	}
	meshCount = 0;
	nodeIDCapacity = 0;
	if (nodeIDs) { delete[] nodeIDs; nodeIDs = NULL; }
	if (textureHandles)
	{
		for (DWORD index = 0; index < textureCount; ++index)
		{
			if (textureHandles[index]) g_GfxSystem->DeleteTexture(textureHandles[index]);
		}
	}
	if (textureHandles)
	{
		delete[] textureHandles;
		textureHandles = NULL;
	}
	lightCount = 0;
	if (lights) { delete[] lights; lights = NULL; }
	if (vertices) { delete[] vertices; vertices = NULL; }
	if (indices) { delete[] indices; indices = NULL; }
	if (floorGroups) { delete[] floorGroups; floorGroups = NULL; }
	if (meshes) { delete[] meshes; meshes = NULL; }
	if (instances) { delete[] instances; instances = NULL; }
	if (packets) { delete[] packets; packets = NULL; }
	if (nodeMinimum) { delete[] nodeMinimum; nodeMinimum = NULL; }
	if (nodeMaximum) { delete[] nodeMaximum; nodeMaximum = NULL; }
	if (boundsVertices) { delete[] boundsVertices; boundsVertices = NULL; }
	if (boundsIndices) { delete[] boundsIndices; boundsIndices = NULL; }
	if (boundsMesh) { delete boundsMesh; boundsMesh = NULL; }
	if (boundsInstance) { delete boundsInstance; boundsInstance = NULL; }
	if (camera) { camera->Release(); camera = NULL; }
	octree = NULL;
	return result;
}

void Octree2Gfx::SetAmbient(DWORD color)
{
	D3DXVECTOR4& value = ambient;
	const float scale = 0.0039215689f;
	value.x = ((color >> 16) & 0xFF) * scale;
	value.y = ((color >> 8) & 0xFF) * scale;
	value.z = (color & 0xFF) * scale;
	value.w = 1;
	for (DWORD index = 0; index < meshCount; ++index) packets[index]->extraLight = value;
}

void Octree2Gfx::SetInstanceFlag(BYTE value)
{
	for (DWORD index = 0; index < meshCount; ++index) instances[index].flag1E = value;
}
