#include "VfxWeather.hpp"
#include "VfxWeatherMesh.hpp"
#include "VfxFrame.hpp"
#include "GraphicsScene.hpp"
#include "Octree.hpp"
#include "ParticleSystem.hpp"
#include "Camera.hpp"
#include "Viewport.hpp"
#include "SystemInterface.hpp"
#include "TextureResource.hpp"
#include "TextureManager.hpp"
#include "resources/ResourceMemoryStream.hpp"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdexcept>

namespace
{
	double RandomRange(float first,float last) //4B8310.
	{ float difference=last-first; return double(rand())*difference*0.000030518509f+first; }
	LONG RandomInteger(LONG first,LONG last) //4B82E0.
	{ return first+(last-first)*rand()/32767; }
	void WrapPosition(D3DXVECTOR3& position,D3DXVECTOR3& origin,float width,float bottom,float top)
	{
		while (-0.5*width>position.x) { position.x+=width; origin.x+=width; }
		while (0.5*width<position.x) { position.x-=width; origin.x-=width; }
		while (bottom>position.y) { position.y+=top-bottom; origin.y+=top-bottom; }
		while (top<position.y) { position.y-=top-bottom; origin.y-=top-bottom; }
		while (-0.5*width>position.z) { position.z+=width; origin.z+=width; }
		while (0.5*width<position.z) { position.z-=width; origin.z-=width; }
	}
}

namespace JE
{
	VFX_Rain::VFX_Rain()
	{
		time=0;
		delta=0;
		simulationDirty=1;
		emissionRemainder=0;
		D3DXMatrixIdentity(&transform);
		mesh=NULL;
		geometryDirty=1;
		emitting=1;
	}
	VFX_Rain::~VFX_Rain()
	{
		if (mesh)
		{
			if (mesh->vertices) ::operator delete[](mesh->vertices);
			if (mesh->indices) ::operator delete[](mesh->indices);
			if (mesh->instances)
			{
				delete[] mesh->instances;
				mesh->instances=NULL;
			}
			if (mesh->packet)
			{
				delete mesh->packet;
				mesh->packet=NULL;
			}
			delete mesh;
		}
	}
	VFX_Rain& VFX_Rain::CopyFrom(const VFX_Rain& other) //0x4E6460.
	{
		VFX_EffectInstance::CopyFrom(other);
		data = other.data;
		time = other.time;
		return *this;
	}
	void VFX_Rain::SetData(const VFX_BaseData* input)
	{
		VFX_EffectInstance::SetData(input);
		data=*static_cast<const VFX_RainData*>(input);
		if (mesh)
		{
			if (mesh->vertices) ::operator delete[](mesh->vertices);
			if (mesh->indices) ::operator delete[](mesh->indices);
			if (mesh->instances)
			{
				delete[] mesh->instances;
				mesh->instances=NULL;
			}
			if (mesh->packet)
			{
				delete mesh->packet;
				mesh->packet=NULL;
			}
			delete mesh;
			mesh=NULL;
		} //4E6A40 retains drops and clocks.
	}
	void VFX_Rain::Reset() { active=0; } //4E1890.
	BYTE VFX_Rain::GetFlag() const { return 0; }
	BYTE VFX_Rain::SkipBounds() const { return 0; }
	BYTE VFX_Rain::Prepare(Gfx_ModelInstance* source, Gfx_ModelInstance* target, VFX_VEGInstance* group,
		const D3DXMATRIX* sourceTransform, const D3DXMATRIX* targetTransform) const
	{ return 1; } //4E5EC0.
	void VFX_Rain::GetBounds(D3DXVECTOR3& low,D3DXVECTOR3& high) const
	{
		OCTREE* tree = GetSceneOctree();
		if (!tree) return;
		float bounds[6];
		tree->GetLevelBounds(bounds);
		low.x = bounds[0] + 1.0f;
		low.y = bounds[1] + 1.0f;
		low.z = bounds[2] + 1.0f;
		high.x = bounds[3] - 1.0f;
		high.y = bounds[4] - 1.0f;
		high.z = bounds[5] - 1.0f;
	}
	void VFX_Rain::Update(float value) { Update(value,VFX_UpdateContext()); }
	void VFX_Rain::Update(float value, Gfx_ModelInstance* source, Gfx_ModelInstance* target, VFX_VEGInstance* group,
		const D3DXMATRIX* sourceTransform, const D3DXMATRIX* targetTransform)
	{
		VFX_EffectInstance::Update(value,source,target,group,sourceTransform,targetTransform);
		if (time!=value) { double elapsed=double(value)-time; time=value; simulationDirty=1; delta=float(elapsed); }
	}
	void VFX_Rain::Simulate(const D3DXVECTOR3& cameraDelta) //4E64A0.
	{
		for (std::list<VFX_RainDrop>::iterator i=drops.begin();i!=drops.end();)
		{
			float age=time-i->born;
			VfxFrame::GetWind(i->wind);
			if (data.affectedByWind) i->origin.y=float(data.windScale*i->wind.y*delta+i->origin.y);
			float y=float(age*i->velocity.y+i->origin.y);
			i->position.y=y;
			if (age<0 || scale*data.groundYOffset>y) { i=drops.erase(i); continue; }
			if (data.affectedByWind)
			{
				i->origin.x=float(data.windScale*i->wind.x*delta+i->origin.x);
				i->origin.z=float(delta*data.windScale*i->wind.z+i->origin.z);
			}
			i->position.x=float(age*i->velocity.x+i->origin.x+cameraDelta.x);
			i->position.y=y+cameraDelta.y;
			i->position.z=float(age*i->velocity.z+i->origin.z+cameraDelta.z);
			i->origin+=cameraDelta;
			WrapPosition(i->position,i->origin,scale*data.emitterSize,scale*data.groundYOffset,scale*data.emitterYOffset);
			++i;
		}
		if (emitting)
		{
			double amount=double(delta)*data.dropsPerSecond+emissionRemainder;
			LONG count=LONG(amount);
			emissionRemainder=float(amount-count);
			if (count>data.maxDrops) count=data.maxDrops;
			if (LONG(drops.size())<data.maxDrops)
			{
				LONG index=0;
				do //Original emits count+1 and also enters for count0; preserve that policy.
				{
					VFX_RainDrop drop;
					drop.origin.x=float((RandomRange(0,data.emitterSize)-data.emitterSize*0.5)*scale);
					drop.origin.y=scale*data.emitterYOffset;
					drop.origin.z=float((RandomRange(0,data.emitterSize)-data.emitterSize*0.5)*scale);
					drop.velocity.x=float(RandomRange(data.minVelocity.xy.values[0],data.maxVelocity.xy.values[0])*scale);
					drop.velocity.y=float(RandomRange(data.minVelocity.xy.values[1],data.maxVelocity.xy.values[1])*scale);
					drop.velocity.z=float(RandomRange(data.minVelocity.z,data.maxVelocity.z)*scale);
					VfxFrame::GetWind(drop.wind);
					drop.position=drop.origin;
					drop.born=float(double(time)-double(index)*delta/float(count));
					drop.length=float(RandomRange(data.minLength,data.maxLength)*scale);
					drop.beginAlpha=float(RandomInteger(data.minBeginAlpha,data.maxBeginAlpha));
					drop.endAlpha=float(RandomInteger(data.minEndAlpha,data.maxEndAlpha));
					drops.push_back(drop);
					++index;
				} while (index<=count && LONG(drops.size())<data.maxDrops);
			}
		}
		geometryDirty=1;
	}
	void VFX_Rain::Draw() { Draw(NULL); }
	void VFX_Rain::Draw(Gfx_Viewport* viewport) //4E7330.
	{
		D3DXMATRIX camera=WeatherCamera(viewport)->transform;
		if (memcmp(&camera,&lastCamera,sizeof camera)) simulationDirty=1;
		if (simulationDirty)
		{
			Simulate(D3DXVECTOR3(lastCamera._41-camera._41,lastCamera._42-camera._42,lastCamera._43-camera._43));
			simulationDirty=0; geometryDirty=1;
		}
		if (geometryDirty) { BuildGeometry(viewport); geometryDirty=0; }
		lastCamera=camera;
		if (mesh && mesh->ready) g_GfxSystem->QueueMeshInstances(mesh->instances,viewport,mesh->instanceCount);
	}

	VFX_Snow::VFX_Snow()
	{
		time=0;
		delta=0;
		simulationDirty=1;
		emissionRemainder=0;
		D3DXMatrixIdentity(&transform);
		mesh=NULL;
		texture=0;
		geometryDirty=1;
		emitting=1;
		ownsTexture=1;
		lastFieldOfView=-1;
	}
	VFX_Snow::~VFX_Snow()
	{
		if (mesh)
		{
			if (mesh->vertices) ::operator delete[](mesh->vertices);
			if (mesh->indices) ::operator delete[](mesh->indices);
			if (mesh->instances)
			{
				delete[] mesh->instances;
				mesh->instances=NULL;
			}
			if (mesh->packet)
			{
				delete mesh->packet;
				mesh->packet=NULL;
			}
			delete mesh;
		}
		if (texture && ownsTexture) g_TextureManager->Release(texture);
	}
	VFX_Snow& VFX_Snow::CopyFrom(const VFX_Snow& other) //0x4E4680.
	{
		if (texture && ownsTexture) g_TextureManager->Release(texture);
		VFX_EffectInstance::CopyFrom(other);
		data = other.data;
		time = other.time;
		delta = other.delta;
		texture = other.texture;
		ownsTexture = 0;
		return *this;
	}
	void VFX_Snow::SetData(const VFX_BaseData* input) //4E4DD0.
	{
		VFX_EffectInstance::SetData(input);
		data=*static_cast<const VFX_SnowData*>(input);
		if (mesh)
		{
			if (mesh->vertices) ::operator delete[](mesh->vertices);
			if (mesh->indices) ::operator delete[](mesh->indices);
			if (mesh->instances)
			{
				delete[] mesh->instances;
				mesh->instances=NULL;
			}
			if (mesh->packet)
			{
				delete mesh->packet;
				mesh->packet=NULL;
			}
			delete mesh;
			mesh=NULL;
		}
		if (texture && ownsTexture) g_TextureManager->Release(texture);
		texture=Gfx_RequestTextureResource(data.texture.name.c_str(), UINT(-1), 3, 5, 0); ownsTexture=1;
	}
	void VFX_Snow::Reset() { active=0; }
	BYTE VFX_Snow::GetFlag() const { return 0; }
	BYTE VFX_Snow::SkipBounds() const { return 0; }
	BYTE VFX_Snow::Prepare(Gfx_ModelInstance* source, Gfx_ModelInstance* target, VFX_VEGInstance* group,
		const D3DXMATRIX* sourceTransform, const D3DXMATRIX* targetTransform) const
	{ return 1; }
	void VFX_Snow::GetBounds(D3DXVECTOR3& low,D3DXVECTOR3& high) const
	{
		OCTREE* tree = GetSceneOctree();
		if (!tree) return;
		float bounds[6];
		tree->GetLevelBounds(bounds);
		low.x = bounds[0] + 1.0f;
		low.y = bounds[1] + 1.0f;
		low.z = bounds[2] + 1.0f;
		high.x = bounds[3] - 1.0f;
		high.y = bounds[4] - 1.0f;
		high.z = bounds[5] - 1.0f;
	}
	void VFX_Snow::Update(float value) { Update(value,VFX_UpdateContext()); }
	void VFX_Snow::Update(float value, Gfx_ModelInstance* source, Gfx_ModelInstance* target, VFX_VEGInstance* group,
		const D3DXMATRIX* sourceTransform, const D3DXMATRIX* targetTransform)
	{
		VFX_EffectInstance::Update(value,source,target,group,sourceTransform,targetTransform);
		if (time!=value) { double elapsed=double(value)-time; time=value; simulationDirty=1; delta=float(elapsed); }
	}
	void VFX_Snow::Simulate(const D3DXVECTOR3& cameraDelta) //4E4700.
	{
		for (std::list<VFX_SnowFlake>::iterator i=flakes.begin();i!=flakes.end();)
		{
			float age=time-i->born;
			D3DXVECTOR3 wind; VfxFrame::GetWind(wind);
			if (data.affectedByWind) i->origin.y=float(wind.y*data.windScale*delta+i->origin.y);
			double y=double(age)*i->velocity.y+i->origin.y;
			i->position.y=float(y);
			if (age<0 || y<scale*data.groundYOffset) { i=flakes.erase(i); continue; }
			if (data.affectedByWind)
			{
				i->origin.x=float(wind.x*delta*data.windScale+i->origin.x);
				i->origin.z=float(wind.z*delta*data.windScale+i->origin.z);
			}
			i->position.x=float(age*i->velocity.x+i->origin.x);
			i->position.z=float(age*i->velocity.z+i->origin.z);
			if (i->xAmplitude!=0) i->position.x=float(Gfx_ParticleSin(float(age*i->xFrequency+i->xPhase))*i->xAmplitude+i->position.x);
			if (i->zAmplitude!=0) i->position.z=float(Gfx_ParticleSin(float(age*i->zFrequency+i->zPhase))*i->zAmplitude+i->position.z);
			//The original emits an empty printf here when the camera moves.
			if (cameraDelta.x!=0 || cameraDelta.y!=0 || cameraDelta.z!=0) std::printf("");
			D3DXVECTOR3 shift(cameraDelta.x/scale,cameraDelta.y/scale,cameraDelta.z/scale);
			i->position+=shift; i->origin+=shift;
			WrapPosition(i->position,i->origin,data.emitterSize*scale,scale*data.groundYOffset,data.emitterYOffset*scale);
			++i;
		}
		if (emitting)
		{
			double amount=double(delta)*data.flakesPerSecond+emissionRemainder;
			LONG count=LONG(amount);
			emissionRemainder=float(amount-count);
			if (count>data.maxFlakes) count=data.maxFlakes;
			if (LONG(flakes.size())<data.maxFlakes)
			{
				LONG index=0;
				do
				{
					VFX_SnowFlake flake;
					flake.origin.x=float((RandomRange(0,data.emitterSize)-data.emitterSize*0.5)*scale);
					flake.origin.y=scale*data.emitterYOffset;
					flake.origin.z=float((RandomRange(0,data.emitterSize)-data.emitterSize*0.5)*scale);
					flake.velocity.x=float(RandomRange(data.minVelocity.xy.values[0],data.maxVelocity.xy.values[0])*scale);
					flake.velocity.y=float(RandomRange(data.minVelocity.xy.values[1],data.maxVelocity.xy.values[1])*scale);
					flake.velocity.z=float(RandomRange(data.minVelocity.z,data.maxVelocity.z)*scale);
					flake.position=flake.origin;
					flake.born=float(double(time)-double(index)*delta/float(count));
					flake.size=float(RandomRange(data.minSize,data.maxSize)*scale);
					flake.alpha=float(RandomInteger(data.minAlpha,data.maxAlpha));
					flake.xAmplitude=float(RandomRange(data.minXSinAmplitude,data.maxXSinAmplitude)*scale);
					flake.xFrequency=float(RandomRange(data.minXSinFrequency,data.maxXSinFrequency)*scale);
					flake.xPhase=float(RandomRange(data.minXSinPhase,data.maxXSinPhase)*scale);
					flake.zAmplitude=float(RandomRange(data.minZSinAmplitude,data.maxZSinAmplitude)*scale);
					flake.zFrequency=float(RandomRange(data.minZSinFrequency,data.maxZSinFrequency)*scale);
					flake.zPhase=float(RandomRange(data.minZSinPhase,data.maxZSinPhase)*scale);
					flakes.push_back(flake);
					++index;
				} while (index<=count && LONG(flakes.size())<data.maxFlakes);
			}
		}
		geometryDirty=1;
	}
	void VFX_Snow::Draw() { Draw(NULL); }
	void VFX_Snow::Draw(Gfx_Viewport* viewport) //4E5D20.
	{
		Gfx_Camera* input=WeatherCamera(viewport);
		D3DXMATRIX camera=input->transform;
		float fov=input->fieldOfViewY;
		if (memcmp(&camera,&lastCamera,sizeof camera) || fov!=lastFieldOfView) simulationDirty=1;
		if (simulationDirty)
		{
			Simulate(D3DXVECTOR3(lastCamera._41-camera._41,lastCamera._42-camera._42,lastCamera._43-camera._43));
			simulationDirty=0; geometryDirty=1;
		}
		if (geometryDirty) { BuildGeometry(viewport); geometryDirty=0; }
		lastCamera=camera; lastFieldOfView=fov;
		if (mesh && mesh->ready) g_GfxSystem->QueueMeshInstances(mesh->instances,viewport,mesh->instanceCount);
	}
}
