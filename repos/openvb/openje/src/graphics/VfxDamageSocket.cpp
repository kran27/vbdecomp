#include "VfxVeg.hpp"
#include "VfxTrail.hpp"
#include "VfxParticles.hpp"

namespace JE
{
	void VFX_EffectInstance::SetSocket(DWORD socket) //4E1B80.
	{
		if (!baseData.targetBodyPart) return;
		baseData.socket=LONG(socket);
		std::string empty;
		baseData.snapBone=std::string(empty); //Original clears the named bone after numeric socket.
	}
	void VFX_Lightning::SetSocket(DWORD socket) //4E9530.
	{
		VFX_EffectInstance::SetSocket(socket);
		if (!data.endPointTargetBodyPart) return;
		data.endPointSocket = LONG(socket);
		std::string empty("");
		data.endPointSnapBone = std::string(empty);
	}
	void VFX_Trail::SetSocket(DWORD socket) //4F19F0.
	{
		VFX_EffectInstance::SetSocket(socket);
		if (!data.endPointTargetBodyPart) return;
		data.endPointSocket = LONG(socket);
		std::string empty("");
		data.endPointSnapBone = std::string(empty);
	}
	void VFX_ParticleSystem::SetSocket(DWORD socket) //4F74E0.
	{
		VFX_EffectInstance::SetSocket(socket);
		if (!data.initialVelocityTargetBodyPart) return;
		data.initialVelocitySocket = LONG(socket);
		std::string empty("");
		data.initialVelocitySnapBone = std::string(empty);
	}
	void VFX_VEGInstance::SetSocket(DWORD socket) //48DC90.
	{
		for (std::vector<VFX_EffectInstance*>::iterator i=effects.begin();i!=effects.end();++i)
			if (*i) (*i)->SetSocket(socket);
	}
}
