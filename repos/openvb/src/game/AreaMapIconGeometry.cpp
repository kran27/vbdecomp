#include "game/AreaMapIcon.hpp"
#include "graphics/Material.hpp"
#include "graphics/Primitive.hpp"

void AreaMapIcon::RebuildGeometry() //4203C0.
{
	D3DXMatrixIdentity(&transform);
	ready=0;
	for (LONG i=0;i<vertexCount;++i)
	{
		if (flags&1) vertices[i].diffuse=0xFFFFFFFF;
		else if (flags&2) vertices[i].diffuse=0xFFFFFFFF;
		else vertices[i].diffuse=0xFF808080;
		vertices[i].rhw=1; vertices[i].z=depth;
	}
	vertices[0].x=float(x)-0.5f; vertices[0].y=float(y)-0.5f;
	vertices[1].x=(float(x)+width)-1.0f+0.5f; vertices[1].y=float(y)-0.5f;
	vertices[2].x=float(x)-0.5f; vertices[2].y=(float(height)+y)-1.0f+0.5f;
	vertices[3].x=(float(x)+width)-1.0f+0.5f; vertices[3].y=(float(height)+y)-1.0f+0.5f;
	vertices[0].u=u; vertices[0].v=v;
	vertices[1].u=float(double(sourceWidth)+u); vertices[1].v=v;
	vertices[2].u=u; vertices[2].v=float(double(sourceHeight)+v);
	vertices[3].u=float(double(sourceWidth)+u); vertices[3].v=float(double(sourceHeight)+v);
	mesh.value00=0; mesh.value04=0xFFFFFFFF; mesh.vertices=vertices;
	mesh.SetVertexDesc(0x144,D3DXGetFVFVertexSize(0x144),DWORD(vertexCapacity));
	Gfx_MaterialSet* set=&mesh.materialSet;
	set->count=0;
	set->ResetGroups();
	if (set->materials)
	{
		delete[] set->materials;
		set->materials=NULL;
	}
	Gfx_Primitive* primitive=mesh.SetPrimitiveCount(1);
	primitive->value00=0; primitive->type=D3DPT_TRIANGLESTRIP; primitive->value24=0;
	Gfx_Material material;
	material.textureHandles[0]=texture;
	material.style1=material.style2=20;
	material.textureHandles[1]=0; material.mode28=1;
	material.flagBC=0; //420693 physical stack offset from material base is+BC.
	mesh.materialSet.materials[0]=material;
	primitive->value28=0; primitive->value2C=0; primitive->value30=2;
	Gfx_PacketData* currentPacket=packet;
	currentPacket->transformEnabled=1; currentPacket->transform=&transform;
	mesh.materialSet.RebuildGroups();
	mesh.value04=0xFFFFFFFF;
	instances[0].SetMesh(&mesh);
	instances[0].value0C=DWORD(packet);
	ready=1;
}
