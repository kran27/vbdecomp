#include "ParticleSystem.hpp"
#include "ModelInstance.hpp"

namespace
{
	const float byteScale=0.0039215688593685626983642578125f; //Original float668C60.
}

void Gfx_ParticleSystem::SampleTrail(Gfx_ParticleInfo& particle,
	std::list<Gfx_ParticleInfo>& output) //5185D0.
{
	const float currentAge=particle.age;
	const Gfx_ParticleConfiguration& config=configuration;
	for (std::list<Gfx_AfterImageInfo>::iterator i=particle.afterImages.begin();
		i!=particle.afterImages.end();)
	{
		double elapsed=double(currentAge)+particle.birthTime-i->birthTime;
		float trailAge=float(elapsed);
		if (elapsed>config.trailLifetime)
		{
			ClearTrail(*i); //513F60 precedes unlink.
			i=particle.afterImages.erase(i);
			continue;
		}
		SystemValues system;
		system.fraction=0;
		system.scale=1;
		system.alpha=1;
		EvaluateSystem(i->birthTime, system.fraction, system.red, system.green, system.blue, system.scale, system.alpha, NULL, NULL, NULL, NULL, NULL, NULL);
		UpdateParticle(*i, float(double(i->sampleAge)+i->birthTime), system.fraction, system.red, system.green, system.blue, system.scale, system.alpha);
		double fraction=config.trailLifetime>0.0f?double(trailAge)/config.trailLifetime:0.0;
		double red=(config.endTrailColorX-double(config.beginTrailColorX))*fraction+config.beginTrailColorX;
		double green=(config.endTrailColorY-double(config.beginTrailColorY))*fraction+config.beginTrailColorY;
		double blue=(config.endTrailColorZ-double(config.beginTrailColorZ))*fraction+config.beginTrailColorZ;
		double alpha=(config.endTrailAlpha-double(config.beginTrailAlpha))*fraction+config.beginTrailAlpha;
		double size=(double(config.endTrailScale)-config.beginTrailScale)*fraction+config.beginTrailScale;
		i->red=LONG(double(i->red)*red*byteScale);
		i->green=LONG(double(i->green)*green*byteScale);
		i->blue=LONG(double(i->blue)*blue*byteScale);
		i->alpha=LONG(double(i->alpha)*alpha*byteScale);
		i->scale=float(size*i->scale);
		{
			Gfx_ParticleInfo sample;
			sample.Gfx_ParticleInfoBase::operator=(Gfx_ParticleInfoBase(*i));
			sample.modelInstance=NULL;
			sample.flagD0=1;
			output.push_back(sample);
		}
		++i;
	}
	if ((config.particleTrailType==2 || config.particleTrailType==3 || currentAge<=particle.lifetime)
		&& config.trailInterval>0.0f)
	{
		float lastTime=particle.afterImages.empty()?0.0f:particle.afterImages.back().birthTime;
		float sample=0;
		for (;;)
		{
			double limit=currentAge>=particle.lifetime?particle.lifetime:currentAge;
			if (sample>limit) break;
			sample=float(double(sample)+config.trailInterval); //Increment precedes sample admission.
			if (sample<=currentAge && double(currentAge)-sample<=config.trailLifetime)
			{
				float sampleTime=float(double(sample)+particle.birthTime);
				if (sampleTime>lastTime)
				{
					SystemValues system;
					system.fraction=0;
					system.scale=1;
					system.alpha=1;
					EvaluateSystem(sampleTime, system.fraction, system.red, system.green, system.blue, system.scale, system.alpha, NULL, NULL, NULL, NULL, NULL, NULL);
					Gfx_AfterImageInfo snapshot;
					snapshot.Gfx_ParticleInfoBase::operator=(Gfx_ParticleInfoBase(particle));
					snapshot.modelInstance=NULL; //518B33, after copying the base payload.
					UpdateParticle(snapshot, sampleTime, system.fraction, system.red, system.green, system.blue, system.scale, system.alpha);
					snapshot.birthTime=float(double(sample)+particle.birthTime);
					snapshot.sampleAge=sample;
					particle.afterImages.push_back(snapshot);
					{
						Gfx_ParticleInfo value;
						value.Gfx_ParticleInfoBase::operator=(Gfx_ParticleInfoBase(snapshot));
						value.modelInstance=NULL;
						value.flagD0=1;
						output.push_back(value);
					}
				}
			}
		}
	}
}

void Gfx_ParticleSystem::BuildRibbonGeometry(Gfx_ParticleInfo& particle,Gfx_Camera* camera) //518C60.
{
	(void)camera; //Original third argument is not read by this ribbon branch.
	std::list<Gfx_ParticleInfo> samples;
	SampleTrail(particle,samples);
	if (LONG(samples.size())-(particle.age>particle.lifetime?1:0)<=0) return;
	const Gfx_ParticleConfiguration& config=configuration;
	D3DXVECTOR3 previous;
	float previousFraction;
	LONG index=0;
	for (std::list<Gfx_ParticleInfo>::iterator i=samples.begin();; ++index)
	{
		D3DXVECTOR3 position;
		float fraction;
		if (i==samples.end())
		{
			if (particle.age>particle.lifetime) break;
			position=D3DXVECTOR3(particle.position.x,particle.position.y,particle.position.z);
			fraction=particle.age>particle.lifetime
				?float((double(particle.age)-particle.lifetime)/config.trailLifetime):0.0f;
		}
		else
		{
			position=D3DXVECTOR3(i->position.x,i->position.y,i->position.z);
			fraction=float((double(particle.age)-i->age)/config.trailLifetime);
		}
		if (index>0)
		{
			double beginRed=config.beginTrailColorX,beginGreen=config.beginTrailColorY,beginBlue=config.beginTrailColorZ;
			double beginAlpha=config.beginTrailAlpha*byteScale;
			double endRed=config.endTrailColorX,endGreen=config.endTrailColorY,endBlue=config.endTrailColorZ;
			double endAlpha=config.endTrailAlpha*byteScale;
			float deltaRed=float(endRed-beginRed);
			float deltaGreen=float(endGreen-beginGreen);
			float deltaBlue=float(endBlue-beginBlue);
			float deltaAlpha=float(endAlpha-beginAlpha);
			lines[currentBatchIndex].AddSegment(previous.x,previous.y,previous.z,position.x,position.y,position.z,
				LONG(double(deltaRed)*previousFraction+beginRed),
				LONG(double(deltaGreen)*previousFraction+beginGreen),
				LONG(double(deltaBlue)*previousFraction+beginBlue),
				LONG((double(deltaAlpha)*previousFraction+beginAlpha)*255.0f),
				LONG(double(deltaRed)*fraction+beginRed),
				LONG(double(deltaGreen)*fraction+beginGreen),
				LONG(double(deltaBlue)*fraction+beginBlue),
				LONG((double(deltaAlpha)*fraction+beginAlpha)*255.0f));
		}
		previousFraction=fraction;
		previous=position;
		if (i==samples.end()) break;
		++i;
	}
}
