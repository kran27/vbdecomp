#include "ParticleSystem.hpp"
#include "ParticleSystemOrdering.hpp"
#include "ModelNode.hpp"
#include "PacketData.hpp"
#include <d3dx8mesh.h>
#include <new>
#include <stdlib.h>

namespace
{
	struct PointParticleVertex
	{
		float x, y, z;
		DWORD color;
		float u, v;
	};
}

void Gfx_ParticleSystem::BuildPointGeometry(Gfx_Camera* camera) //51AE20.
{
	(void)camera; //The point builder does not read its camera argument.
	Gfx_ParticleBatch& batch=*batches[currentBatchIndex];
	batch.flag94=0;
	if (particles.size()>0)
	{
		D3DXMatrixInverse(&GfxParticleDraw::particleCamera,NULL,&GfxParticleDraw::particleCamera);
		if (owner)
			GfxParticleDraw::particleTransform=static_cast<Gfx_ModelNode*>(owner)->localTransform;
		else
			D3DXMatrixIdentity(&GfxParticleDraw::particleTransform);
		D3DXMATRIX rotation;
		D3DXMatrixRotationYawPitchRoll(&rotation,configuration.rotationX,
			configuration.rotationY,configuration.rotationZ);
		D3DXMatrixMultiply(&GfxParticleDraw::particleTransform,&GfxParticleDraw::particleTransform,&rotation);
		D3DXMatrixMultiply(&GfxParticleDraw::particleTransform,&GfxParticleDraw::particleTransform,&secondTransform);
		particles.sort(); //51ABF0, actual default SDK list sort and private value ordering.
		const LONG count=static_cast<LONG>(particles.size());
		if (count>0)
		{
			if (count>static_cast<LONG>(batch.vertexCapacity))
			{
				if (batch.vertices) { free(batch.vertices); batch.vertices=NULL; }
				LONG capacity=static_cast<LONG>(batch.vertexCapacity+20);
				if (count>capacity) capacity=count;
				batch.vertexCapacity=capacity;
				if (capacity>0) batch.vertices=operator new(sizeof(PointParticleVertex)*capacity);
			}
			batch.vertexCount=count;
		}
		else batch.vertexCount=0;
		DWORD index=0;
		for (std::list<Gfx_ParticleInfo>::const_iterator i=particles.begin();i!=particles.end();++i,++index)
		{
			static_cast<PointParticleVertex*>(batch.vertices)[index].x=i->position.x;
			static_cast<PointParticleVertex*>(batch.vertices)[index].y=i->position.y;
			static_cast<PointParticleVertex*>(batch.vertices)[index].z=i->position.z;
			static_cast<PointParticleVertex*>(batch.vertices)[index].color=(static_cast<DWORD>(i->alpha)<<24)
				| ((static_cast<DWORD>(i->red)&255)<<16)
				| ((static_cast<DWORD>(i->green)&255)<<8)
				| (static_cast<DWORD>(i->blue)&255);
			//51B007..51B04E writes onlyXYZ/ARGB. The advertised FVF142 has
			//two texture coordinates, but their existing allocation bytes survive.
		}
		Gfx_MeshData& mesh=batch.mesh;
		mesh.vertices=batch.vertices;
		mesh.value00=0;
		mesh.value04=0xFFFFFFFF;
		mesh.SetVertexDesc(0x142,D3DXGetFVFVertexSize(0x142),batch.vertexCount);
		mesh.materialSet.count=0;
		mesh.materialSet.ResetGroups();
		delete[] mesh.materialSet.materials;
		mesh.materialSet.materials=NULL;
		Gfx_Primitive* primitive=mesh.SetPrimitiveCount(1);
		primitive->value00=0;
		primitive->type=D3DPT_POINTLIST;
		primitive->value24=0;
		Gfx_Material material;
		material.style1=material.style2=19+(configuration.modulate2x!=0);
		material.mode24=material.mode28=configuration.renderPhase;
		material.flagBC=0;
		material.flagBD=1;
		material.textureHandles[0]=texture;
		material.textureHandles[1]=0;
		mesh.materialSet.materials[0]=material;
		primitive->value28=0;
		primitive->value30=static_cast<DWORD>(particles.size());
		batch.packet->transform=batch.matrices;
		batch.packet->transformEnabled=1;
		mesh.materialSet.RebuildGroups();
		mesh.value04=0xFFFFFFFF;
		batch.instances[0].SetMesh(&mesh);
		batch.instances[0].value0C=reinterpret_cast<DWORD>(batch.packet);
		batch.flag94=1;
		//Original point geometry neither allocates indices nor calls SetIndexDesc.
	}
}
