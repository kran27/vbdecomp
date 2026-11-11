#include "Fragment.hpp"
#include <d3dx8.h>

namespace GUI
{
	Fragment::Fragment()
		: uvDirty(1), tileMode(0), frameWidth(2), frameHeight(2), frameRate(0),
		currentFrame(0), color(0xFF808080)
	{
		// Base, mesh and instance use their actual recovered constructors.
		// Frame count/time, padding, unknown mesh words and vertex X/Y stay unwritten.
		mesh.value00 = 0;
		mesh.value04 = 0xFFFFFFFF;
		mesh.vertices = vertices;
		mesh.SetVertexDesc(0x144, D3DXGetFVFVertexSize(0x144), 4);
		primitive = mesh.SetPrimitiveCount(1);
		primitive->type = D3DPT_TRIANGLESTRIP;
		primitive->value28 = 0;
		primitive->value2C = 0;
		primitive->value30 = 2;
		material = mesh.materialSet.materials;
		Gfx_Material* styleMaterial = material;
		styleMaterial->style1 = 20;
		styleMaterial->style2 = 20;
		Gfx_Material* modeMaterial = material;
		modeMaterial->mode28 = 1;
		modeMaterial->mode24 = 1;
		material->textureHandles[0] = 0;
		instance.SetMesh(&mesh);
		mesh.materialSet.RebuildGroups();
		for (int i = 0; i < 4; ++i)
		{
			vertices[i].diffuse = 0xFF808080;
			vertices[i].rhw = 1;
			vertices[i].z = 0;
			if (i & 1) vertices[i].u = 1;
			else vertices[i].u = 0;
			if (i < 2) vertices[i].v = 0;
			else vertices[i].v = 1;
		}
		sourceWidth = -1;
		sourceHeight = -1;
		sourceX = 0;
		sourceY = 0;
	}

	Fragment::~Fragment()
	{
		mesh.Free();
		// Normal C++ then destroys instance, mesh (Free again + material set), Base.
		// The borrowed primitive/material/vertex pointers are not cleared here.
	}
}
