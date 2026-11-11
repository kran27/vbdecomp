#include "ParticleSystem.hpp"
#include <new>
#include <stdexcept>

namespace
{
	struct TrailVertex
	{
		float x,y,z;
		DWORD color;
		float u,v;
	};

	DWORD TrailColor(const Gfx_ParticleConfiguration& config,float fraction)
	{
		const double byteScale=0.0039215688593685626983642578125; //668C60.
		const double beginAlpha=config.beginTrailAlpha*byteScale;
		const float red=float(double(config.endTrailColorX)-config.beginTrailColorX);
		const float green=float(double(config.endTrailColorY)-config.beginTrailColorY);
		const float blue=float(double(config.endTrailColorZ)-config.beginTrailColorZ);
		const float alpha=float(config.endTrailAlpha*byteScale-beginAlpha);
		const DWORD a=DWORD(LONG((double(alpha)*fraction+beginAlpha)*255.0));
		const DWORD r=DWORD(LONG(double(red)*fraction+config.beginTrailColorX))&0xFF;
		const DWORD g=DWORD(LONG(double(green)*fraction+config.beginTrailColorY))&0xFF;
		const DWORD b=DWORD(LONG(double(blue)*fraction+config.beginTrailColorZ))&0xFF;
		return (((a<<8)|r)<<8|g)<<8|b;
	}

	void WriteTrailVertex(TrailVertex& vertex,const D3DXVECTOR3& point,DWORD color,
		DWORD texture,float u,float v)
	{
		vertex.x=point.x; vertex.y=point.y; vertex.z=point.z;
		vertex.color=color;
		if (texture) { vertex.u=u; vertex.v=v; } //Untextured UV bytes remain unwritten.
	}
}

void Gfx_ParticleSystem::SampleTrails(std::list<Gfx_ParticleInfo>& output) //5192C0; RET4.
{
	for (std::list<Gfx_ParticleInfo>::iterator i=particles.begin();i!=particles.end();++i)
		SampleTrail(*i,output);
}

void Gfx_ParticleSystem::RebuildTrails() //519300, sampled retired values for trail1.
{
	ClearRetired();
	for (std::list<Gfx_ParticleInfo>::const_iterator i=particles.begin();i!=particles.end();++i)
	{
		if (!(i->age<=i->lifetime) && configuration.particleTrailType!=2
			&& configuration.particleTrailType!=3) continue;
		Gfx_ParticleInfo copy(*i); //Base and nested value list; no source ownership transfer.
		copy.modelInstance=NULL;
		copy.flagD0=0;
		retiredParticles.push_back(copy);
	}
	if (configuration.particleTrailType==1) //After all current values are copied.
		SampleTrails(retiredParticles);
}

void Gfx_ParticleSystem::GetTrailEndpoints(Gfx_Camera* camera,const Gfx_ParticleInfoBase& particle,
	D3DXVECTOR3& first,D3DXVECTOR3& last,float time) const //513BF0; RET20.
{
	(void)camera;
	const Gfx_ParticleConfiguration& config=configuration;
	const float fraction=float((double(time)-particle.age)/config.trailLifetime);
	D3DXVECTOR3 a=particle.position;
	D3DXVECTOR3 b=a;
	D3DXVECTOR3 offset(config.ribbonOriginX,config.ribbonOriginY,config.ribbonOriginZ);
	offset*=scale;
	a+=offset;
	offset=D3DXVECTOR3(config.ribbonEndPointX,config.ribbonEndPointY,config.ribbonEndPointZ);
	offset*=scale;
	b+=offset;
	switch (config.ribbonAlignment)
	{
	case 0:
		{
			first=a;
			const D3DXVECTOR3 delta=b-first;
			const float factor=float((double(config.endTrailScale)-config.beginTrailScale)*fraction+config.beginTrailScale);
			last=first+delta*factor;
		}
		break;
	case 1:
		{
			last=b;
			first=a;
			const D3DXVECTOR3 delta=last-first;
			const float factor=float((double(config.endTrailScale)-config.beginTrailScale)*fraction+config.beginTrailScale);
			const D3DXVECTOR3 scaled=delta*factor;
			const D3DXVECTOR3 half=(delta-scaled)*0.5f;
			first=last-scaled-half;
			last=last-half;
		}
		break;
	case 2:
		{
			last=b;
			const D3DXVECTOR3 delta=last-a;
			const float factor=float((double(config.endTrailScale)-config.beginTrailScale)*fraction+config.beginTrailScale);
			first=last-delta*factor;
		}
		break;
	default: break; //Original leaves both outputs unwritten for invalid alignment.
	}
}

void Gfx_ParticleSystem::BuildAfterimageGeometry(Gfx_ParticleInfo& particle,
	Gfx_Camera* camera) //519490, independently owned textured triangle strip.
{
	(void)camera;
	std::list<Gfx_ParticleInfo> samples;
	SampleTrail(particle,samples);
	const LONG segments=LONG(samples.size())-(particle.age>particle.lifetime?1:0);
	if (segments<=0) return;
	Gfx_ParticleBatch* batch=new Gfx_ParticleBatch;
	batch->instanceCount=1;
	batch->instances=new Gfx_MeshInstance[1]; //Original defaults; no primary-batch flag overrides.
	batch->matrices=new D3DXMATRIX[1];
	D3DXMatrixIdentity(batch->matrices);
	batch->vertexCount=batch->vertexCapacity=segments>1?2*segments+2:4;
	batch->vertices=operator new(sizeof(TrailVertex)*batch->vertexCapacity);
	TrailVertex* vertices=static_cast<TrailVertex*>(batch->vertices);
	const Gfx_ParticleConfiguration& config=configuration;
	D3DXVECTOR3 previousFirst,previousLast;
	std::list<Gfx_ParticleInfo>::iterator i=samples.begin(); //Positive segment count guarantees a sample.
	GetTrailEndpoints(camera,*i,previousFirst,previousLast,particle.age);
	float previousFraction=float((double(particle.age)-i->age)/config.trailLifetime);
	++i;
	LONG point=1;
	DWORD written=0;
	for (;;)
	{
		D3DXVECTOR3 first,last;
		float fraction;
		if (i==samples.end())
		{
			if (particle.age>particle.lifetime) break;
			GetTrailEndpoints(camera,particle,first,last,float(double(particle.age)+0.0));
			fraction=0.0f;
		}
		else
		{
			GetTrailEndpoints(camera,*i,first,last,particle.age);
			fraction=float((double(particle.age)-i->age)/config.trailLifetime);
		}
		const float textureFraction=float(double(point)/segments);
		if (point)
		{
			if (point==1)
			{
				const DWORD color=TrailColor(config,previousFraction);
				WriteTrailVertex(vertices[written++],previousFirst,color,trailTexture,
					config.ribbonTextureUVsX,config.ribbonTextureUVsW);
				WriteTrailVertex(vertices[written++],previousLast,color,trailTexture,
					config.ribbonTextureUVsX,config.ribbonTextureUVsY);
			}
			const DWORD color=TrailColor(config,fraction);
			const float u=float((double(config.ribbonTextureUVsZ)-config.ribbonTextureUVsX)
				*textureFraction+config.ribbonTextureUVsX);
			WriteTrailVertex(vertices[written++],first,color,trailTexture,u,config.ribbonTextureUVsW);
			WriteTrailVertex(vertices[written++],last,color,trailTexture,u,config.ribbonTextureUVsY);
		}
		previousFirst=first; previousLast=last; previousFraction=fraction;
		++point;
		if (i==samples.end()) break;
		++i;
	}
	Gfx_MeshData& mesh=batch->mesh;
	mesh.value00=0;
	mesh.value04=DWORD(-1);
	mesh.vertices=batch->vertices;
	mesh.SetVertexDesc(0x142,D3DXGetFVFVertexSize(0x142),batch->vertexCount);
	batch->packet=new Gfx_PacketData;
	mesh.materialSet.count=0;
	mesh.materialSet.ResetGroups();
	delete[] mesh.materialSet.materials;
	mesh.materialSet.materials=NULL;
	Gfx_Primitive* primitive=mesh.SetPrimitiveCount(1);
	primitive->value00=0;
	primitive->type=D3DPT_TRIANGLESTRIP;
	primitive->value24=0;
	Gfx_Material material;
	material.mode24=material.mode28=config.trailRenderPhase;
	material.flagBC=0;
	material.flagB9=0;
	material.flagBD=1;
	material.style1=material.style2=19+(config.trailModulate2x!=0);
	material.textureHandles[0]=trailTexture;
	mesh.materialSet.materials[0]=material;
	primitive->value28=0;
	primitive->value30=2*segments;
	batch->packet->transform=batch->matrices;
	batch->packet->transformEnabled=1;
	mesh.materialSet.RebuildGroups();
	mesh.value04=DWORD(-1);
	batch->instances[0].SetMesh(&mesh);
	batch->instances[0].value0C=reinterpret_cast<DWORD>(batch->packet);
	pools[currentBatchIndex].push_back(batch); //Publish only after actual mesh/packet construction.
}
