#include "VfxWeather.hpp"
#include "VfxFrame.hpp"
#include "ParticleSystem.hpp"
#include <stdlib.h>

namespace JE
{
	namespace VfxFrame
	{
		double RandomRange(float first,float last) //4B8310, consumes rand even for equal bounds.
		{ float difference=last-first; return rand()*difference*0.000030518509f+first; }
	}
	VFX_Wind::VFX_Wind() //4E37D0.
	{
		stopped = 0;
		gustStart = 0;
		gustDelay = 0;
	}
	VFX_Wind::~VFX_Wind() {}
	VFX_Wind& VFX_Wind::CopyFrom(const VFX_Wind& other) //0x4E3E10.
	{
		VFX_EffectInstance::CopyFrom(other);
		data = other.data;
		stopped = other.stopped;
		gustStart = other.gustStart;
		gustDelay = other.gustDelay;
		return *this;
	}
	void VFX_Wind::SetData(const VFX_BaseData* input) //4E3E60.
	{
		VFX_EffectInstance::SetData(input);
		data=*static_cast<const VFX_WindData*>(input);
		gustStart=0; gustDelay=0;
		gusts.clear();
	}
	void VFX_Wind::Reset() { active=0; gustStart=0; gustDelay=0; gusts.clear(); }
	BYTE VFX_Wind::GetFlag() const { return 0; } //549020.
	BYTE VFX_Wind::SkipBounds() const { return 0; } //549020.
	BYTE VFX_Wind::Prepare(Gfx_ModelInstance* source, Gfx_ModelInstance* target, VFX_VEGInstance* group,
		const D3DXMATRIX* sourceTransform, const D3DXMATRIX* targetTransform) const
	{ return 1; } //4E5EC0.
	void VFX_Wind::GetBounds(D3DXVECTOR3& low,D3DXVECTOR3& high) const //4E0D70.
	{ low=high=D3DXVECTOR3(transform._41,transform._42,transform._43); }
	void VFX_Wind::Draw() {} //5A9320, actual empty draw.
	void VFX_Wind::Update(float time) { Update(time,VFX_UpdateContext()); }
	void VFX_Wind::Update(float time, Gfx_ModelInstance* source, Gfx_ModelInstance* target, VFX_VEGInstance* group,
		const D3DXMATRIX* sourceTransform, const D3DXMATRIX* targetTransform)
	{
		VFX_EffectInstance::Update(time, source, target, group, sourceTransform, targetTransform);
		if (stopped) return;
		if (data.windType==1)
		{
			if (gustStart==0 || gustDelay+gustStart<=time)
			{
				Gust gust;
				gust.born=time;
				gust.duration=float(VfxFrame::RandomRange(data.minGustDuration,data.maxGustDuration));
				gust.attack=float(VfxFrame::RandomRange(data.minGustAttack,data.maxGustAttack));
				gust.decay=float(VfxFrame::RandomRange(data.minGustDecay,data.maxGustDecay));
				gust.velocity=float(VfxFrame::RandomRange(data.minGustVelocity,data.maxGustVelocity));
				gusts.push_back(gust);
				gustStart=time;
				//4E3A8C passes an already subtracted range as the second argument.
				gustDelay=float(VfxFrame::RandomRange(data.minGustDelay,data.maxGustDelay-data.minGustDelay));
			}
			for (std::list<Gust>::iterator i=gusts.begin();i!=gusts.end();)
			{
				if (time<i->born || i->born+i->duration<=time) { i=gusts.erase(i); continue; }
				float phase=float((double(time)-i->born)/i->duration);
				double velocity;
				if (i->attack>0 && phase<i->attack) velocity=double(phase)/i->attack*i->velocity;
				else if (i->decay>0 && phase>=1.0-i->decay)
					velocity=(1.0-(phase-(1.0-i->decay))/i->decay)*i->velocity;
				else velocity=i->velocity;
				D3DXVECTOR3 wind(data.direction.xy.values[0], data.direction.xy.values[1], data.direction.z);
				wind.x = float(wind.x * velocity);
				wind.y = float(wind.y * velocity);
				wind.z = float(wind.z * velocity);
				VfxFrame::AddWind(wind.x, wind.y, wind.z);
				++i;
			}
		}
		else if (!data.windType)
		{
			double phase=0;
			if (data.duration>0) { phase=double(time)/data.duration; if (phase<0) phase=0; else if (phase>1) phase=1; }
			double velocity=data.velocity;
			if (data.adjustVelocity)
			{
				if (!data.adjustVelocityOverLifetime || !(data.duration>0)) phase=time;
				float argument=float(phase*data.velocityFreq+data.velocityPhase);
				velocity=Gfx_ParticleWaveform(data.velocityWaveform,0,1,argument)
					*(double(data.finalVelocity)-data.velocity)+data.velocity;
			}
			D3DXVECTOR3 wind(data.direction.xy.values[0], data.direction.xy.values[1], data.direction.z);
			wind.x = float(wind.x * velocity);
			wind.y = float(wind.y * velocity);
			wind.z = float(wind.z * velocity);
			VfxFrame::AddWind(wind.x, wind.y, wind.z);
		}
	}
}
