#include "DynamicObject.hpp"
#include "graphics/Octree.hpp"
#include <stdexcept>
#include <cstring>

namespace JE
{
	DynamicObject::DynamicObject()
		: position(0,0,0), minimum(-0.5f,-0.5f,-0.5f), maximum(0.5f,0.5f,0.5f),
		octree(NULL), octreeHandle(DWORD(-1))
	{
		flag30 = 1; //5683E4, after the set and debug-line members construct.
	}

	DynamicObject::~DynamicObject()
	{
		RemoveFromOctree();
	}

	void DynamicObject::Draw()
	{
		// Original shared404B70 returns without any work.
	}

	void DynamicObject::GetBounds(D3DXVECTOR3& low, D3DXVECTOR3& high) const
	{
		low=minimum;
		high=maximum;
	}

	void DynamicObject::CopyFrom(DynamicObject* source) //568280.
	{
		DWORD word;
		memcpy(&word,&source->position.x,sizeof(word));
		memcpy(&position.x,&word,sizeof(word));
		memcpy(&word,&source->position.y,sizeof(word));
		memcpy(&position.y,&word,sizeof(word));
		memcpy(&word,&source->position.z,sizeof(word));
		memcpy(&position.z,&word,sizeof(word));
		flag30=source->flag30;
	}

	BYTE DynamicObject::IsVisible()
	{
		if (!octree || octreeHandle == DWORD(-1)) return 0;
		return octree->IsObjectVisible(octreeHandle) ? 1 : 0;
	}

	void DynamicObject::RemoveFromOctree()
	{
		if (octree && octreeHandle != DWORD(-1))
			octree->RemoveObject(octreeHandle);
		octree = NULL;
		octreeHandle = DWORD(-1);
	}

	void DynamicObject::GetSpatialCenter(D3DXVECTOR3& output) const
	{
		output.x = 0.0f;
		output.y = 0.0f;
		output.z = 0.0f;
	}

	int DynamicObject::AttachToOctree(OCTREE* value)
	{
		octree = value; // Original publishes before the existing-handle branch.
		float bounds[6];
		bounds[3] = (maximum.x - minimum.x) * 0.5f;
		bounds[4] = (maximum.y - minimum.y) * 0.5f;
		bounds[5] = (maximum.z - minimum.z) * 0.5f;
		bounds[0] = -bounds[3];
		bounds[1] = -bounds[4];
		bounds[2] = -bounds[5];
		if (octreeHandle != DWORD(-1)) return octree->ResizeObject(octreeHandle,bounds);
		D3DXVECTOR3 center;
		GetSpatialCenter(center);
		D3DXVECTOR3 translated = center + position;
		octree->AddObject(&translated.x,bounds,this,&octreeHandle);
		return octree->InsertObject(octreeHandle);
	}

	void DynamicObject::ChangePosition(const D3DXVECTOR3& value)
	{
		DWORD word;
		memcpy(&word,&value.x,sizeof(word));
		memcpy(&position.x,&word,sizeof(word));
		memcpy(&word,&value.y,sizeof(word));
		memcpy(&position.y,&word,sizeof(word));
		memcpy(&word,&value.z,sizeof(word));
		memcpy(&position.z,&word,sizeof(word));
		if (octree && octreeHandle != DWORD(-1))
		{
			D3DXVECTOR3 center;
			GetSpatialCenter(center);
			D3DXVECTOR3 translated = center + position;
			octree->MoveObject(octreeHandle,&translated.x);
		}
	}
}
