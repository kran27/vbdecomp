#include "VfxSound.hpp"
#include "audio/SoundInstance.hpp"
#include "audio/SoundSystem.hpp"
#include "audio/SoundInstanceManager.hpp"
#include <cstdlib>
#include <stdexcept>
namespace JE
{
	namespace VfxServices
	{
		BYTE g_AudioEnabled=1; //Original initialized6FAFEC, no writers recovered.
		BYTE AudioEnabled() { return g_AudioEnabled; } //4B82D0.
	}
	VFX_SoundData::VFX_SoundData() {}
	void VFX_SoundData::Read(const VFX_ParameterHolder& parameters)
	{
		const VFX_Parameter* input;
		if((input=parameters.Find("PresetFile"))!=NULL)
		{ preset.type=input->GetResourceType(); preset.name=input->GetResourceName(); }
		if((input=parameters.Find("WAVFile"))!=NULL)
		{ wav.type=input->GetResourceType(); wav.name=input->GetResourceName(); }
		if((input=parameters.Find("Looping"))!=NULL) looping=input->GetBool();
		if((input=parameters.Find("FireAndForget"))!=NULL) fireAndForget=input->GetBool();
	}
	VFX_Sound::VFX_Sound() : value1A8(0),sound(NULL),paused(0),fired(0),finishedTime(NULL) {}
	VFX_Sound::~VFX_Sound()
	{
		if(sound) delete sound;
		if(finishedTime) delete finishedTime;
	}
	VFX_Sound& VFX_Sound::CopyFrom(const VFX_Sound& other) //0x4E34B0.
	{
		if (sound) { delete sound; sound = NULL; }
		VFX_EffectInstance::CopyFrom(other);
		data = other.data;
		transform = other.transform;
		value1A8 = other.value1A8;
		paused = other.paused;
		return *this;
	}
	void VFX_Sound::SetData(const VFX_BaseData* input)
	{
		VFX_EffectInstance::SetData(input);
		data=*static_cast<const VFX_SoundData*>(input);
		if(sound) { delete sound; sound=NULL; }
		std::string resource;
		if (!data.preset.name.empty()) resource=data.preset.name;
		else if (!data.wav.name.empty()) resource=data.wav.name;
		// This is deliberately NOT gated by AudioEnabled or flagC1: original
		// SetData constructs/registers52E2E0 then loads52D820 even for disabled VFX.
		if(!resource.empty())
		{
			sound=new VfxSoundInstance;
			sound->Load(resource.c_str(),0,NULL);
			sound->spatial=1;
			D3DXMATRIX matrix; GetTransform(matrix);
			sound->SetPosition(D3DXVECTOR3(matrix._41,matrix._42,matrix._43));
		}
	}
	void VFX_Sound::Reset()
	{
		active=0;
		if(VfxServices::AudioEnabled() && (data.looping || !data.fireAndForget) && sound && sound->IsPlaying())
			sound->Stop(0);
		fired=0;
		if(VfxServices::AudioEnabled() && data.fireAndForget && !sound)
		{
			std::string resource;
			if (!data.preset.name.empty()) resource = data.preset.name;
			else if (!data.wav.name.empty()) resource = data.wav.name;
			if(!resource.empty())
			{
				sound=new VfxSoundInstance;
				sound->Load(resource.c_str(),0,NULL);
				sound->spatial=1;
				D3DXMATRIX matrix; GetTransform(matrix);
				sound->SetPosition(D3DXVECTOR3(matrix._41,matrix._42,matrix._43));
			}
		}
	}
	BYTE VFX_Sound::GetFlag() const { return 0; }
	BYTE VFX_Sound::SkipBounds() const { return 0; }
	void VFX_Sound::GetBounds(D3DXVECTOR3& low,D3DXVECTOR3& high) const
	{ low=high=D3DXVECTOR3(transform._41,transform._42,transform._43); }
	void VFX_Sound::Update(float)
	{
		VFX_UpdateContext context;
		Update(0,context);
	}
	void VFX_Sound::Update(float time, Gfx_ModelInstance* source, Gfx_ModelInstance* target, VFX_VEGInstance* group,
		const D3DXMATRIX* sourceTransform, const D3DXMATRIX* targetTransform)
	{
		VFX_EffectInstance::Update(time, source, target, group, sourceTransform, targetTransform);
		if(paused || !VfxServices::AudioEnabled() || !sound) return;
		if(!data.looping && data.fireAndForget) return;
		D3DXMATRIX matrix; GetTransform(matrix);
		sound->SetPosition(D3DXVECTOR3(matrix._41,matrix._42,matrix._43));
		if(data.looping)
		{
			if(sound->IsPlaying())
			{
				if(sound->Priority()<=0) sound->Stop(0);
			}
			else
			{
				if (!AudioBackend::g_System) return;
				int value=AudioBackend::g_System->instanceManager->RequestPriority(*sound);
				if(value>0) { sound->SetPriority(value); sound->Play(data.looping!=0,0); }
			}
		}
	}
	void VFX_Sound::OnStart() //4E2E40, original activation hook.
	{
		if(paused || !VfxServices::AudioEnabled() || data.looping || fired || !sound) return;
		if(!sound->IsPlaying())
		{
			D3DXMATRIX matrix; GetTransform(matrix);
			sound->SetPosition(D3DXVECTOR3(matrix._41,matrix._42,matrix._43));
			sound->Play(data.fireAndForget ? 2 : 0,0);
			fired=1;
			if(data.fireAndForget) sound=NULL; //Original transfers lifetime to the registered instance owner.
		}
	}
	void VFX_Sound::OnStop()
	{
		if(VfxServices::AudioEnabled() && (data.looping || !data.fireAndForget) && sound && sound->IsPlaying())
			sound->Stop(0);
	}
	BYTE VFX_Sound::IsFinished(float time) const
	{
		if(data.fireAndForget && fired) return 1;
		if(data.duration<0.0f && !data.looping && sound
			&& ((fired && !sound->IsPlaying()) || (finishedTime && time>=*finishedTime)))
		{
			if(!finishedTime) finishedTime=new float;
			*finishedTime=time;
			return 1;
		}
		return VFX_EffectInstance::IsFinished(time);
	}
}
