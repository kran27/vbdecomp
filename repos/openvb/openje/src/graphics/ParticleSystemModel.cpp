#include "ParticleSystem.hpp"
#include "ModelInstance.hpp"
#include "SystemInterface.hpp"
#include <math.h>

namespace
{
	DWORD ParticleModelColor(const Gfx_ParticleInfo& particle)
	{
		return (static_cast<DWORD>(particle.alpha)<<24)
			| ((static_cast<DWORD>(particle.red)&255)<<16)
			| ((static_cast<DWORD>(particle.green)&255)<<8)
			| (static_cast<DWORD>(particle.blue)&255);
	}
}

void Gfx_ParticleSystem::ClearModelInstances() //514AD0.
{
	modelInstances.clear();
}

void Gfx_ParticleSystem::BuildModelGeometry(Gfx_Camera* camera) //519E70.
{
	ClearModelInstances(); //Borrowed list nodes, never the owned particle models.
	std::list<Gfx_ParticleInfo>& selected=configuration.particleTrailType==1
		? retiredParticles:particles;
	if (selected.empty() || !model) return;
	for (std::list<Gfx_ParticleInfo>::iterator i=selected.begin();i!=selected.end();++i)
	{
		Gfx_ParticleInfo& particle=*i;
		if (configuration.particleTrailType==2) BuildAfterimageGeometry(particle,camera);
		if (configuration.particleTrailType==3) BuildRibbonGeometry(particle,camera);
		if (particle.age>particle.lifetime) continue;
		if (!particle.modelInstance)
			particle.modelInstance=g_GfxSystem->CreateModelInstance(model); //519F42, actual slotBC.
		Gfx_ModelInstance* instance=particle.modelInstance;
		const bool systemRGB=(configuration.flags&0x200)!=0;
		const bool systemAlpha=(configuration.flags&0x180)!=0;
		if (!configuration.model_UseRenderPhase && !systemRGB && !systemAlpha
			&& !configuration.model_UseColor && !configuration.model_UseAlpha
			&& !configuration.model_IgnoreLighting)
			instance->ResetMaterialStates(); //519FA2 ->49CDD0.
		if (configuration.model_UseColor || configuration.model_UseAlpha || systemRGB || systemAlpha)
		{
			const BYTE useAlpha=systemAlpha || configuration.model_UseAlpha;
			const BYTE useRGB=systemRGB || configuration.model_UseColor || configuration.model_IgnoreLighting;
			instance->SetMaterialColors(ParticleModelColor(particle),useRGB,useAlpha,NULL);
		}
		if (configuration.model_UseRenderPhase)
			instance->SetRenderPhase(particle.flagD0
				? configuration.trailRenderPhase:configuration.renderPhase);
		if (particle.rotatable)
		{
			if (configuration.faceVelocityVectorUsed)
			{
				float resistance=particle.resistance;
				if (fabs(resistance)<0.000007999999979801942)
					resistance=(resistance>=0.0f ? 1:-1)*0.000008f;
				const D3DXVECTOR3 acceleration=Acceleration();
				const double mass=double(scale)*particle.mass;
				const float roundedMass=static_cast<float>(mass);
				const float forceX=static_cast<float>(mass*acceleration.x);
				const double decay=exp(-(double(resistance)*particle.age));
				const double inverseResistance=1.0/resistance;
				const double termX=forceX*inverseResistance;
				const double termY=double(roundedMass)*acceleration.y*inverseResistance;
				const double termZ=double(roundedMass)*acceleration.z*inverseResistance;
				const float velocityX=static_cast<float>(termX+(particle.initialVelocity.x-termX)*decay);
				const float velocityY=static_cast<float>(termY+(particle.initialVelocity.y-termY)*decay);
				const double velocityZ=termZ+(particle.initialVelocity.z-termZ)*decay;
				D3DXVECTOR3 forward(-velocityX,-velocityY,static_cast<float>(-velocityZ));
				D3DXVec3Normalize(&forward,&forward); //471047 is real stdcall/out,in/RET8.
				D3DXVECTOR3 right,up;
				if (velocityX==0.0f && velocityZ==0.0)
				{
					const D3DXVECTOR3 vertical(0,0,1);
					D3DXVec3Cross(&right,&vertical,&forward);
					D3DXVec3Normalize(&right,&right);
					D3DXVec3Cross(&up,&right,&forward); //51A241..51A324 opposite cross order.
				}
				else
				{
					const D3DXVECTOR3 vertical(0,1,0);
					D3DXVec3Cross(&right,&vertical,&forward);
					D3DXVec3Normalize(&right,&right);
					D3DXVec3Cross(&up,&forward,&right);
				}
				D3DXMATRIX basis(right.x,right.y,right.z,0,up.x,up.y,up.z,0,
					forward.x,forward.y,forward.z,0,0,0,0,1);
				instance->SetEulerRotation(particle.rotation.x,particle.rotation.y,particle.rotation.z);
				D3DXMATRIX rotation;
				D3DXMatrixMultiply(&rotation,&instance->transform,&basis);
				instance->SetTransform(rotation);
			}
			else
			{
				instance->SetEulerRotation(particle.rotation.x,particle.rotation.y,particle.rotation.z);
				D3DXMATRIX rotation=instance->transform;
				D3DXVECTOR3 angles=particle.rotation;
				if (particle.spin.x!=0.0f) angles.x=particle.spin.x*particle.age;
				if (particle.spin.y!=0.0f) angles.y=particle.spin.y*particle.age;
				if (particle.spin.z!=0.0f) angles.z=particle.spin.z*particle.age;
				if (particle.spin.x!=0.0f || particle.spin.y!=0.0f || particle.spin.z!=0.0f)
				{
					D3DXMATRIX spin;
					D3DXMatrixRotationYawPitchRoll(&spin,angles.x,angles.y,angles.z);
					D3DXMatrixMultiply(&rotation,&rotation,&spin);
				}
				instance->SetTransform(rotation);
			}
		}
		instance->SetPosition(particle.position);
		const float size=static_cast<float>(double(scale)*particle.scale*particle.size);
		instance->SetScale(size,size,size);
		modelInstances.push_back(instance); //51A65D..51A68F; particle retains ownership.
	}
}
