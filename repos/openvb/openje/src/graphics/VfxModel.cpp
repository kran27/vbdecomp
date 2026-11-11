#include "VfxModel.hpp"
#include "StringFormat.hpp"
#include "AnimationInstance.hpp"
#include "G3DModel.hpp"
#include "ModelData.hpp"
#include "ModelInstance.hpp"
#include "ParticleSystem.hpp"
#include "SystemInterface.hpp"
#include "resources/ResourceSystem.hpp"
#include <mbstring.h>
#include <string.h>
#include <stdexcept>

namespace Granny21
{
	// F3's4F497E pushes a hidden return buffer for this sixteen-byte POD result.
	// The old export's @0 excludes that hidden argument. Reuse the shipped DLL.
	struct SystemClockSnapshot { DWORD words[4]; };
	extern "C" __declspec(dllimport) SystemClockSnapshot __stdcall GrannyGetSystemSeconds();
}

namespace
{
	bool DifferentName(const std::string& first, const std::string& second)
	{
		return _mbsicmp(reinterpret_cast<const unsigned char*>(first.c_str()),
			reinterpret_cast<const unsigned char*>(second.c_str())) != 0;
	}
	double ModelWave(LONG kind, BYTE overLifetime, float duration, float fraction,
		float time, float frequency, float phase)
	{
		double selected = overLifetime && duration > 0 ? fraction : time;
		float clock = float(selected * frequency + phase);
		return Gfx_ParticleWaveform(kind,0,1,clock);
	}
	DWORD PackedColor(BYTE red, BYTE green, BYTE blue, BYTE alpha)
	{
		return (DWORD(alpha)<<24) | (DWORD(red)<<16) | (DWORD(green)<<8) | blue;
	}
}

namespace JE
{
	VFX_Model::VFX_Model()
	{
		D3DXMatrixIdentity(&transform);
		currentTime=0;
		modelSource=NULL;
		modelHandle=0;
		model=NULL;
		animationObject=new Gfx_AnimationObject;
		ownsSource=1;
		ownsHandle=1;
		ownsAnimationObject=1;
		animation=NULL;
		loopCount=0;
	}
	VFX_Model::~VFX_Model()
	{
		if (model) { delete model; model=NULL; }
		if (modelHandle && ownsHandle) g_GfxSystem->ReleaseModelData(&modelHandle);
		if (animation) { delete animation; animation=NULL; }
		if (ownsAnimationObject && animationObject) { delete animationObject; animationObject=NULL; }
		if (modelSource && ownsSource) delete modelSource;
	}
	VFX_Model& VFX_Model::CopyFrom(const VFX_Model& other) //4F4000.
	{
		if (model) { delete model; model=NULL; }
		if (modelHandle && ownsHandle) g_GfxSystem->ReleaseModelData(&modelHandle);
		if (animation) { delete animation; animation=NULL; }
		if (ownsAnimationObject && animationObject)
		{
			delete animationObject;
			animationObject=NULL;
		}
		if (modelSource && ownsSource) delete modelSource;
		VFX_EffectInstance::CopyFrom(other);
		data=other.data;
		transform=other.transform;
		modelHandle=other.modelHandle;
		model=NULL;
		if (modelHandle)
		{
			model=g_GfxSystem->CreateModelInstance(modelHandle);
			if (model)
			{
				model->SetShadow(data.useShadowVolume);
				if (data.useRenderPhase) model->SetRenderPhase(data.renderPhase);
			}
		}
		animationObject=other.animationObject;
		animation=NULL;
		if (animationObject && other.animation)
		{
			animation=new Gfx_AnimationInstance(animationObject);
			if (model) model->SetAnimation(animation,0);
			animation->SelectAnimation(0,0,0,0,0,data.loopCount);
			loopCount=data.loopCount;
		}
		if (model) model->SetScale(data.scaleX,data.scaleY,data.scaleZ);
		ownsSource=0;
		ownsHandle=0;
		ownsAnimationObject=0;
		return *this;
	}
	void VFX_Model::Reset() { active=0; } //Shared4E1890; leave stop state unchanged.
	BYTE VFX_Model::GetFlag() const { return 1; } //Original slot1C, independent of model presence.
	BYTE VFX_Model::AddLight(Gfx_Light* light) //4F3720.
	{
		Gfx_ModelInstance* current = model;
		if (current) return current->AddLight(light);
		return 0;
	}
	void VFX_Model::RemoveLight(Gfx_Light* light) //4F3740.
	{
		Gfx_ModelInstance* current = model;
		if (current) current->RemoveLight(light);
	}
	void VFX_Model::ClearLights() //4F3760.
	{
		Gfx_ModelInstance* current = model;
		if (current) current->ClearLights();
	}
	void VFX_Model::FinishLights() //4F3770.
	{
		Gfx_ModelInstance* current = model;
		if (current) current->FinishLights();
	}
	BYTE VFX_Model::SkipBounds() const { return 0; }
	Gfx_ModelInstance* VFX_Model::GetTargetModel() const { return model; }
	void VFX_Model::SetTransform(const D3DXMATRIX& input)
	{
		transform=input;
		if (model) model->SetTransform(input);
	}
	void VFX_Model::GetTransform(D3DXMATRIX& output) const
	{
		if (model) output = model->transform;
		else output = transform;
	}
	void VFX_Model::GetBounds(D3DXVECTOR3& low,D3DXVECTOR3& high) const
	{
		if (!model)
		{
			VFX_EffectInstance::GetBounds(low,high);
			return;
		}
		D3DXMATRIX matrix;
		GetTransform(matrix);
		D3DXVECTOR3 position(matrix._41,matrix._42,matrix._43);
		model->GetBounds(low,high);
		low+=position;
		high+=position;
	}
	void VFX_Model::Draw() { Draw(NULL); }
	void VFX_Model::Draw(Gfx_Viewport* viewport)
	{
		if (model) model->Submit(viewport,0,0.05f,0xFFFF0000);
	}
	void VFX_Model::LoadModel(const std::string& name,float units)
	{
		if (model) { delete model; model=NULL; }
		if (modelHandle && ownsHandle && g_GfxSystem->ReleaseModelData(&modelHandle))
			Gfx_DeferModelDataRelease(modelHandle);
		ownsHandle=1;
		modelName=name;
		loadedShadow=data.useShadowVolume;
		if (IsStringBlank(name.c_str(), " \n\t\r")) return;
		modelHandle=Gfx_LoadModelData(name.c_str(),1,1,data.useShadowVolume,units,0x252);
		if (!modelHandle) return;
		model=g_GfxSystem->CreateModelInstance(modelHandle);
		if (!model) return;
		model->SetShadow(data.useShadowVolume);
		model->SetScale(data.scaleX,data.scaleY,data.scaleZ);
		if (data.useRenderPhase) model->SetRenderPhase(data.renderPhase);
	}
	BYTE VFX_Model::LoadSkeleton(const std::string& name,float units)
	{
		BYTE loaded=0;
		if (model) model->SetAnimation(NULL,0);
		if (animation) { delete animation; animation=NULL; }
		if (ownsAnimationObject && animationObject) { delete animationObject; animationObject=NULL; }
		ownsAnimationObject=1;
		animationObject=new Gfx_AnimationObject;
		skeletonName=name;
		if (name.empty()) return 0;
		Resources::ResourceMemoryStream* stream=Resources::g_pResourceSystem->OpenMemoryStream(name.c_str(),0);
		if (!stream) return 0;
		if (animationObject->LoadSkeletonFromBuffer(int(stream->end-stream->begin),stream->begin,units))
		{
			animation=new Gfx_AnimationInstance(animationObject);
			if (model) model->SetAnimation(animation,0);
			loaded=1;
		}
		Resources::g_pResourceSystem->CloseMemoryStream(stream);
		return loaded;
	}
	void VFX_Model::LoadAnimation(const std::string& name,float units)
	{
		if (animation && model && animationObject->file)
		{
			animationObject->ClearAnimationGroups();
			animationName=name;
			animation->CompleteCurrentControl(0,0);
			if (name.empty())
			{
				if (animation) { delete animation; animation=NULL; }
				animation=new Gfx_AnimationInstance(animationObject);
				// Original4F4495 does not rebind the model after this replacement.
			}
			else
			{
				Resources::ResourceMemoryStream* stream=Resources::g_pResourceSystem->OpenMemoryStream(name.c_str(),0);
				if (stream)
				{
					animationObject->LoadAnimationFileFromBuffer(0,int(stream->end-stream->begin),stream->begin,units);
					Resources::g_pResourceSystem->CloseMemoryStream(stream);
				}
				animation->SelectAnimation(0,0,0,0,0,data.loopCount);
				loopCount=data.loopCount;
			}
		}
		else
		{
			animationName="";
			if (model) model->SetAnimation(NULL,0);
		}
	}
	void VFX_Model::SetData(const VFX_BaseData* input)
	{
		VFX_EffectInstance::SetData(input);
		data=*static_cast<const VFX_ModelData*>(input);
		Granny21::SystemClockSnapshot snapshot=Granny21::GrannyGetSystemSeconds();
		memcpy(clockSnapshot,snapshot.words,sizeof(clockSnapshot));
		bool modelChanged=false, skeletonChanged=false;
		if ((data.useShadowVolume!=loadedShadow && !data.modelResource.name.empty())
			|| DifferentName(data.modelResource.name,modelName))
		{
			LoadModel(data.modelResource.name,scale);
			modelChanged=true;
		}
		if ((modelChanged && !data.skeletonResource.name.empty()) || DifferentName(data.skeletonResource.name,skeletonName))
		{
			LoadSkeleton(data.skeletonResource.name,scale);
			skeletonChanged=true;
		}
		if (((modelChanged || skeletonChanged) && !data.animationResource.name.empty())
			|| (loopCount!=data.loopCount && !data.animationResource.name.empty())
			|| DifferentName(data.animationResource.name,animationName)) LoadAnimation(data.animationResource.name,scale);
		if (!model) return;
		model->ResetMaterialValues();
		model->ResetMaterialStates();
		model->SetShadow(data.useShadowVolume);
		if (data.useRenderPhase) model->SetRenderPhase(data.renderPhase);
		if (data.useColor || data.useAlpha || data.ignoreLighting)
		{
			BYTE red=data.color[0],green=data.color[1],blue=data.color[2];
			if (data.ignoreLighting && !data.useColor) red=green=blue=0x80;
			model->SetMaterialColors(PackedColor(red,green,blue,data.alpha),
				data.useColor || data.ignoreLighting,data.useAlpha,NULL);
		}
		model->SetMaterialFlagBC(data.ignoreLighting==0);
		model->SetScale(data.scaleX,data.scaleY,data.scaleZ);
	}
	void VFX_Model::Update(float time)
	{
		VFX_UpdateContext context;
		Update(time,context);
	}
	void VFX_Model::Update(float time, Gfx_ModelInstance* source, Gfx_ModelInstance* target, VFX_VEGInstance* group,
		const D3DXMATRIX* sourceTransform, const D3DXMATRIX* targetTransform)
	{
		VFX_UpdateContext context(source, target, group, sourceTransform, targetTransform);
		UpdateTransform(context);
		float delta=0;
		if (currentTime!=time) { delta=time-currentTime; currentTime=time; }
		if (animation) animation->Update(time);
		if (!model) return;
		model->UpdateAfterimages(delta);
		float fraction=float(double(currentTime)/data.duration);
		if (fraction<0) fraction=0;
		else if (fraction>1) fraction=1;
		if (data.adjustScale)
		{
			double wave=ModelWave(data.scaleWaveform,data.adjustScaleOverLifetime,data.duration,fraction,
				currentTime,data.scaleFreq,data.scalePhase);
			model->SetScale(float((data.finalScaleX-data.scaleX)*wave+data.scaleX),
				float((data.finalScaleY-data.scaleY)*wave+data.scaleY),
				float((data.finalScaleZ-data.scaleZ)*wave+data.scaleZ));
		}
		if (!IsVisible()) return;
		BYTE red=data.color[0],green=data.color[1],blue=data.color[2],alpha=data.alpha;
		if (data.adjustColor)
		{
			double wave=ModelWave(data.colorWaveform,data.adjustColorOverLifetime,data.duration,fraction,
				currentTime,data.colorFreq,data.colorPhase);
			red=BYTE((int(data.finalColor[0])-data.color[0])*wave+data.color[0]);
			green=BYTE((int(data.finalColor[1])-data.color[1])*wave+data.color[1]);
			blue=BYTE((int(data.finalColor[2])-data.color[2])*wave+data.color[2]);
		}
		if (data.adjustAlpha)
		{
			double wave=ModelWave(data.alphaWaveform,data.adjustAlphaOverLifetime,data.duration,fraction,
				currentTime,data.alphaFreq,data.alphaPhase);
			alpha=BYTE((int(data.finalAlpha)-data.alpha)*wave+data.alpha);
		}
		DWORD emissive=0;
		const DWORD* emission=NULL;
		if (data.adjustEmissive)
		{
			double wave=ModelWave(data.emissiveWaveform,data.adjustEmissiveOverLifetime,data.duration,fraction,
				currentTime,data.emissiveFreq,data.emissivePhase);
			emissive=PackedColor(BYTE((int(data.finalEmissive[0])-data.emissive[0])*wave+data.emissive[0]),
				BYTE((int(data.finalEmissive[1])-data.emissive[1])*wave+data.emissive[1]),
				BYTE((int(data.finalEmissive[2])-data.emissive[2])*wave+data.emissive[2]),255);
			emission=&emissive;
		}
		else if (data.useEmissive)
		{
			emissive=PackedColor(data.emissive[0],data.emissive[1],data.emissive[2],255);
			emission=&emissive;
		}
		if (!data.useRenderPhase && !data.adjustColor && !data.adjustAlpha && !data.useColor
			&& !data.useAlpha && !data.ignoreLighting && !data.useEmissive)
		{
			model->ResetMaterialValues();
			model->ResetMaterialStates();
		}
		if (data.adjustColor || data.adjustAlpha || data.useColor || data.useAlpha
			|| data.adjustEmissive || data.useEmissive)
		{
			if (data.ignoreLighting && !data.useColor) red=green=blue=0x80;
			model->SetMaterialColors(PackedColor(red,green,blue,alpha),
				data.adjustColor || data.useColor || data.ignoreLighting,
				data.adjustAlpha || data.useAlpha,emission);
		}
		model->SetMaterialFlagBC(data.ignoreLighting==0);
		if (data.useRenderPhase) model->SetRenderPhase(data.renderPhase);
	}
}
