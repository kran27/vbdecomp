#include "VfxWeather.hpp"
#include "VfxWeatherMesh.hpp"
#include "Camera.hpp"
#include "Viewport.hpp"
#include "SystemInterface.hpp"
#include <new>
#include <stdlib.h>

namespace JE
{
	VFX_WeatherMesh::VFX_WeatherMesh() {}
	Gfx_Camera* WeatherCamera(Gfx_Viewport* viewport)
	{ return viewport ? viewport->activeCamera : g_GfxSystem->Camera(); }

	void FinishWeatherMesh(VFX_WeatherMesh& output,DWORD fvf,DWORD stride,
		D3DPRIMITIVETYPE type,DWORD primitiveCount,LONG phase,DWORD style,DWORD texture,
		const D3DXMATRIX& camera,const VFX_Vector& offset,float scale)
	{
		output.mesh.value00=0;
		output.mesh.value04=DWORD(-1);
		output.mesh.vertices=output.vertices;
		output.mesh.value10=reinterpret_cast<DWORD>(output.indices);
		output.mesh.SetVertexDesc(fvf,stride,output.vertexCount);
		output.mesh.SetIndexDesc(2,output.indexCount);
		output.mesh.materialSet.count=0;
		output.mesh.materialSet.ResetGroups();
		if (output.mesh.materialSet.materials)
		{ delete[] output.mesh.materialSet.materials; output.mesh.materialSet.materials=NULL; }
		Gfx_Primitive* primitive=output.mesh.SetPrimitiveCount(1);
		primitive->value00=0;
		primitive->type=type;
		primitive->value24=1;
		Gfx_Material material;
		material.style1=material.style2=style;
		material.mode28=phase;
		material.flagBC=material.flagBD=0;
		material.textureHandles[0]=texture;
		material.textureHandles[1]=0;
		output.mesh.materialSet.materials[0]=material;
		primitive->value28=primitive->value2C=0;
		primitive->value30=primitiveCount;
		D3DXMatrixIdentity(&output.transform);
		//512D90 extracts the three camera rows. The original adds the offset
		//in those axes, with scale applied to each contribution before addition.
		for (int axis=0;axis<3;++axis)
		{
			float x=float(double(camera.m[0][axis])*offset.xy.values[0]);
			float y=float(double(camera.m[1][axis])*offset.xy.values[1]);
			float z=float(double(camera.m[2][axis])*offset.z);
			float first=float(double(x)*scale+camera.m[3][axis]);
			float second=float(double(y)*scale+first);
			output.transform.m[3][axis]=float(double(z)*scale+second);
		}
		output.packet->transformEnabled=1;
		output.packet->transform=&output.transform;
		output.mesh.materialSet.RebuildGroups();
		output.mesh.value04=DWORD(-1);
		output.instances[0].SetMesh(&output.mesh);
		output.instances[0].value0C=reinterpret_cast<DWORD>(output.packet);
		output.ready=1;
	}

	void VFX_Rain::BuildGeometry(Gfx_Viewport* viewport) //4E6B20.
	{
		Gfx_Camera* camera=WeatherCamera(viewport);
		if (!viewport) (void)g_GfxSystem->DefaultViewport(); //Original getter call4E6B86.
		if (data.maxDrops<=0) { if (mesh) mesh->ready=0; return; }
		if (!mesh)
		{
			mesh=new VFX_WeatherMesh;
			mesh->instanceCount=1;
			mesh->instances=new Gfx_MeshInstance[mesh->instanceCount];
			for (LONG i=0;i<LONG(mesh->instanceCount);++i)
			{
				mesh->instances[i].flag1D=0;
				mesh->instances[i].flag1E=0;
			}
			mesh->vertexCount=2u*DWORD(data.maxDrops);
			mesh->vertices=::operator new[](16u*mesh->vertexCount);
			mesh->indexCount=2u*DWORD(data.maxDrops);
			mesh->indices=static_cast<WORD*>(::operator new[](sizeof(WORD)*mesh->indexCount));
			mesh->packet=new Gfx_PacketData;
			mesh->ready=0;
		}
		D3DXMATRIX cameraTransform=camera->transform;
		LONG count=LONG(drops.size());
		if (count>=data.maxDrops) count=data.maxDrops;
		mesh->vertexCount=mesh->indexCount=2*count;
		struct RainVertex { float x,y,z; DWORD color; };
		RainVertex* output=static_cast<RainVertex*>(mesh->vertices);
		DWORD index=0;
		for (std::list<VFX_RainDrop>::const_iterator drop=drops.begin();drop!=drops.end();++drop)
		{
			output[index].x=drop->position.x;
			output[index].y=drop->position.y;
			output[index].z=drop->position.z;
			output[index].color=DWORD(data.color[2])|(DWORD(data.color[1])<<8)
				|(DWORD(data.color[0])<<16)|(DWORD(LONG(drop->beginAlpha))<<24);
			mesh->indices[index]=WORD(index);
			if (++index>=DWORD(2*data.maxDrops)) break;
			D3DXVECTOR3 wind(0,0,0);
			if (data.affectedByWind) wind=drop->wind*data.windScale;
			double length=-double(drop->length);
			output[index].x=float((double(wind.x)+drop->velocity.x)*length+drop->position.x);
			output[index].y=float((double(wind.y)+drop->velocity.y)*length+drop->position.y);
			output[index].z=float((double(wind.z)+drop->velocity.z)*length+drop->position.z);
			output[index].color=DWORD(data.color[2])|(DWORD(data.color[1])<<8)
				|(DWORD(data.color[0])<<16)|(DWORD(LONG(drop->endAlpha))<<24);
			mesh->indices[index]=WORD(index);
			if (++index>=DWORD(2*data.maxDrops)) break;
		}
		FinishWeatherMesh(*mesh,0x42,D3DXGetFVFVertexSize(0x42),D3DPT_LINELIST,
			DWORD(drops.size()),data.renderPhase,15+(data.modulate2x!=0),0,
			cameraTransform,data.offsetFromCamera,scale);
	}
}
