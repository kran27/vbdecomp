#include "VfxEffects.hpp"
#include "VfxParticles.hpp"
#include "VfxCollision.hpp"
#include "VfxSound.hpp"
#include "VfxSprite.hpp"
#include "VfxModel.hpp"
#include "VfxLight.hpp"
#include "VfxTrail.hpp"
#include "VfxModelEffect.hpp"
#include "VfxMotionBlur.hpp"
#include "VfxWeather.hpp"
#include "VfxWater.hpp"
#include "VfxVeg.hpp"
#include "VfxFrame.hpp"
#include "VfxLightningMesh.hpp"
#include "OctreeCamera.hpp"
#include "Viewport.hpp"
#include "SystemInterface.hpp"
#include "ModelInstance.hpp"
#include "ModelData.hpp"
#include "AnimationInstance.hpp"
#include "ParticleSystem.hpp"
#include <mbstring.h>
#include <float.h>
#include <math.h>
#include <stdlib.h>
#include <stdexcept>

namespace
{
	Gfx_ModelInstance* ResolveModel(DWORD target, const JE::VFX_UpdateContext& context) // Compatibility context implementation.
	{
		if (target == 1) return context.source;
		if (target == 2) return context.target;
		if (static_cast<LONG>(target) >= 3 && context.group)
		{
			JE::VFX_EffectInstance* effect = context.group->GetEffect(LONG(target - 3));
			if (effect) return effect->GetTargetModel();
		}
		return NULL;
	}
	//Shared proprietary suffix: scaled offsets are captured before model callbacks.
	void FinishTransformOffset(D3DXMATRIX& output, const float* scaled, BYTE rotate)
	{
		if (rotate)
		{
			if (scaled)
			{
				D3DXMATRIX rotation = output;
				rotation._41 = rotation._42 = rotation._43 = 0;
				D3DXVECTOR3 value(scaled[0], scaled[1], scaled[2]);
				D3DXVec3TransformCoord(&value, &value, &rotation);
				output._41 = float(double(value.x) + output._41);
				output._42 = float(double(value.y) + output._42);
				output._43 = float(double(value.z) + output._43);
			}
		}
		else
		{
			D3DXMATRIX translation;
			D3DXMatrixIdentity(&translation);
			translation._41 = float((scaled ? double(scaled[0]) : 0.0) + output._41);
			translation._42 = float((scaled ? double(scaled[1]) : 0.0) + output._42);
			translation._43 = float((scaled ? double(scaled[2]) : 0.0) + output._43);
			output = translation;
		}
	}
	BOOL ValidSemanticBone(LONG bone) //51E920, signed range and zero exclusion.
	{
		return bone >= 0 && bone < 212 && bone != 0;
	}
}

namespace JE
{
	VFX_Effect::VFX_Effect()
		: data(NULL)
	{
	}

	void VFX_VEG::Cleanup()
	{
		//The actual supplied VC7 clear calls _Tidy: destroy values, free and zero storage.
		effects.clear();
	}

	VFX_Effect::VFX_Effect(const VFX_Effect& input)
		: data(NULL)
	{
		//4F9AF0 delegates actual assignment; fresh copies start unprepared.
		*this = input;
	}

	VFX_Effect& VFX_Effect::operator=(const VFX_Effect& input)
	{
		BYTE hadData = 0;
		if (data != NULL)
		{
			delete data;
			data = NULL;
			hadData = 1;
		}
		VFX_ParameterHolder::operator=(input);
		if (hadData) Prepare();
		return *this;
	}

	VFX_BaseData* VFX_Effect::Prepare()
	{
		Cleanup();
		data = BuildVfxEffectData(*this);
		return data;
	}

	Gfx_ModelInstance* __cdecl ResolveVfxModel(VFX_Target selectedTarget, Gfx_ModelInstance* source,
		Gfx_ModelInstance* target, VFX_VEGInstance* group) //4E0B20.
	{
		Gfx_ModelInstance* result = NULL;
		switch (selectedTarget.Value())
		{
		case 0: break;
		case 1: result = source; break;
		case 2: result = target; break;
		default:
			if (static_cast<LONG>(selectedTarget.Value()) >= 3 && group)
			{
				VFX_EffectInstance* effect = group->GetEffect(LONG(selectedTarget.Value() - 3));
				if (effect) result = effect->GetTargetModel();
			}
			break;
		}
		return result;
	}
	Gfx_ModelInstance* ResolveVfxModel(DWORD target, const VFX_UpdateContext& context)
	{
		return ResolveModel(target, context);
	}
	void GetVfxTransform(const D3DXMATRIX* source, D3DXMATRIX& output,
		const float* offset, BYTE rotate, float scale)
	{
		D3DXVECTOR3 scaled;
		if (offset)
		{
			scaled.x = scale * offset[0];
			scaled.y = scale * offset[1];
			scaled.z = scale * offset[2];
		}
		if (source)
		{
			output = *source;
			if (rotate)
			{
				if (offset)
				{
					// Native rereads input after publishing the initial output copy.
					D3DXMATRIX rotation = *source;
					rotation._41 = rotation._42 = rotation._43 = 0;
					D3DXVec3TransformCoord(&scaled, &scaled, &rotation);
					output._41 = float(double(scaled.x) + output._41);
					output._42 = float(double(scaled.y) + output._42);
					output._43 = float(double(scaled.z) + output._43);
				}
			}
			else
			{
				D3DXMATRIX translation;
				D3DXMatrixIdentity(&translation);
				translation._41 = (offset ? scaled.x : 0.0f) + output._41;
				translation._42 = (offset ? scaled.y : 0.0f) + output._42;
				translation._43 = (offset ? scaled.z : 0.0f) + output._43;
				output = translation;
			}
		}
		else
		{
			D3DXMatrixIdentity(&output);
			if (offset) { output._41 = scaled.x; output._42 = scaled.y; output._43 = scaled.z; }
		}
	}
	BYTE VFX_EffectInstance::PrepareTarget(Gfx_ModelInstance* source, Gfx_ModelInstance* targetModel,
		VFX_VEGInstance* group, VFX_Target selectedTarget, LONG socket, std::string snapBone,
		const D3DXMATRIX* sourceTransform, const D3DXMATRIX* targetTransform) const //4E1580, RET38h.
	{
		if (!baseData.hideIfNoTarget) return 1; //Live receiver flag, after the parameter copy.
		//The original tests sourceTransform for BOTH explicit target cases.
		if (selectedTarget.Value() == 1 && sourceTransform)
			return socket == 0 && snapBone.empty();
		if (selectedTarget.Value() == 2 && sourceTransform)
			return socket == 0 && snapBone.empty();
		Gfx_ModelInstance* model = ResolveVfxModel(selectedTarget, source, targetModel, group);
		if (!model) return 0;
		if (!snapBone.empty())
		{
			Gfx_ModelData526620* data = model->data;
			if (!data) return 0;
			const LONG count = data->GetBoneCount();
			for (LONG index = 0; index < count; ++index)
			{
				const Gfx_ModelBone* bone = &data->bones[index];
				if (bone && !_mbsicmp(reinterpret_cast<const unsigned char*>(bone->name.c_str()),
					reinterpret_cast<const unsigned char*>(snapBone.c_str()))) return 1;
			}
			return 0;
		}
		if (socket)
		{
			if (!ValidSemanticBone(socket)) return 0;
			Gfx_AnimationInstance* animation = model->animation;
			if (!animation || !animation->GetSemanticBoneIndex(socket)) return 0;
		}
		return 1;
	}
	void GetVfxModelTransform(Gfx_ModelInstance* model, LONG socket, std::string snapBone,
		D3DXMATRIX& output, const float* offset, BYTE rotate, float scale) //4E0F70, owns its argument.
	{
		float scaled[3];
		if (offset)
		{
			scaled[0] = scale * offset[0];
			scaled[1] = scale * offset[1];
			scaled[2] = scale * offset[2];
		}
		if (!snapBone.empty() && model)
		{
			D3DXMATRIX world;
			output = *model->GetWorldTransform(world, 0);
			const D3DXVECTOR3 modelScale = model->scale;
			Gfx_ModelData526620* data = model->data;
			if (data)
			{
				const LONG count = LONG(data->bones.size());
				for (LONG index = 0; index < count; ++index)
				{
					const Gfx_ModelBone* bone = &data->bones[index];
					if (bone && !_mbsicmp(reinterpret_cast<const unsigned char*>(bone->name.c_str()),
						reinterpret_cast<const unsigned char*>(snapBone.c_str())))
					{
						D3DXMATRIX local = bone->transform;
						local._41 *= modelScale.x; local._42 *= modelScale.y; local._43 *= modelScale.z;
						D3DXMATRIX combined;
						D3DXMatrixMultiply(&combined, &local, &output);
						output = combined;
						break;
					}
				}
			}
			FinishTransformOffset(output, offset ? scaled : NULL, rotate);
			return;
		}
		if (ValidSemanticBone(socket))
		{
			if (model)
			{
				Gfx_AnimationInstance* animation = model->animation;
				const D3DXMATRIX* bone = animation ? animation->GetBoneTransform(socket) : NULL;
				if (bone)
				{
					output = *bone; //Original publishes this pose before world-transform evaluation.
					D3DXMATRIX world;
					if (offset && rotate)
					{
						D3DXMATRIX worldOutput;
						world = *model->GetWorldTransform(worldOutput, 0);
					}
					else model->GetWorldTransform(world, 0);
					D3DXMatrixMultiply(&output, &output, &world);
					FinishTransformOffset(output, offset ? scaled : NULL, rotate);
					return;
				}
			}
		}
		if (model)
		{
			D3DXMATRIX world;
			output = *model->GetWorldTransform(world, 0);
			FinishTransformOffset(output, offset ? scaled : NULL, rotate);
		}
		else
		{
			D3DXMatrixIdentity(&output);
			if (offset) { output._41 = scaled[0]; output._42 = scaled[1]; output._43 = scaled[2]; }
		}
	}
	VFX_EffectInstance::VFX_EffectInstance()
		: scale(1.0f), active(0)
	{
		D3DXMatrixIdentity(&transform);
		flagC0 = 0;
		flagC1 = 1;
	}

	VFX_EffectInstance::~VFX_EffectInstance()
	{
	}

	void VFX_EffectInstance::Load(const VFX_BaseData& input, float value)
	{
		LoadBaseData(input, value);
		SetData(input);
	}
	void VFX_EffectInstance::SetData(const VFX_BaseData* input) { if (input) baseData = *input; } //4E1D00.
	VFX_EffectInstance& VFX_EffectInstance::CopyFrom(const VFX_EffectInstance& other)
	{
		baseData = other.baseData;
		scale = other.scale;
		effectIndex = other.effectIndex;
		return *this;
	}
	void VFX_EffectInstance::Draw(Gfx_Viewport*) {} //Original5A9320, not a noarg virtual bridge.
	BYTE VFX_EffectInstance::AddLight(Gfx_Light*) { return 0; }
	void VFX_EffectInstance::RemoveLight(Gfx_Light*) {}
	void VFX_EffectInstance::ClearLights() {}
	void VFX_EffectInstance::FinishLights() {}
	void VFX_EffectInstance::Stop() { flagC0 = 1; }
	void VFX_EffectInstance::OnStart() {}
	void VFX_EffectInstance::OnStop() {}
	Gfx_ModelInstance* VFX_EffectInstance::GetTargetModel() const { return NULL; }
	BYTE VFX_EffectInstance::AppliesToExtraSources() const { return 0; }
	void VFX_EffectInstance::SetEnabled(BYTE enabled) { flagC1 = enabled; }
	BYTE VFX_EffectInstance::GetEnabled() const { return flagC1; } //4E18C0, rawBYTE.
	void VFX_EffectInstance::Reset() { active = 0; } //4E1890.
	BYTE VFX_EffectInstance::GetFlag() const { return 0; } //549020.
	BYTE VFX_EffectInstance::SkipBounds() const { return 0; } //549020.

	void VFX_EffectInstance::SetTransform(const D3DXMATRIX& input) { transform = input; }
	void VFX_EffectInstance::GetTransform(D3DXMATRIX& output) const { output = transform; }
	void VFX_EffectInstance::GetBounds(D3DXVECTOR3& low, D3DXVECTOR3& high) const //4E0D70.
	{
		D3DXVECTOR3 point(transform._41, transform._42, transform._43);
		high = point;
		low = point;
	}

	BYTE VFX_EffectInstance::Prepare(Gfx_ModelInstance* source, Gfx_ModelInstance* target, VFX_VEGInstance* group,
		const D3DXMATRIX* sourceTransform, const D3DXMATRIX* targetTransform) const
	{
		return PrepareTarget(source, target, group, baseData.target, baseData.socket,
			baseData.snapBone, sourceTransform, targetTransform);
	}

	void VFX_EffectInstance::UpdateTransform(const VFX_UpdateContext& context)
	{
		VFX_EffectInstance::Update(0, context.source, context.target, context.group,
			context.sourceTransform, context.targetTransform);
	}
	void VFX_EffectInstance::Update(float, Gfx_ModelInstance* source, Gfx_ModelInstance* target, VFX_VEGInstance* group,
		const D3DXMATRIX* sourceTransform, const D3DXMATRIX* targetTransform)
	{
		VFX_UpdateContext context(source, target, group, sourceTransform, targetTransform);
		const float offset[3] = {baseData.offset.xy.values[0], baseData.offset.xy.values[1], baseData.offset.z};
		const LONG socket = baseData.socket;
		std::string snapBone(baseData.snapBone);
		const BYTE rotate = baseData.rotateWithTarget; //4E19D0..4E1A0E snapshots precede target resolution.
		D3DXMATRIX matrix;
		const D3DXMATRIX* explicitTransform = baseData.target.selector == 1 ? sourceTransform
			: baseData.target.selector == 2 ? targetTransform : NULL;
		if (explicitTransform) GetVfxTransform(explicitTransform, matrix, offset, rotate, scale);
		else
		{
			Gfx_ModelInstance* model = ResolveModel(baseData.target.selector, context);
			GetVfxModelTransform(model, socket, snapBone, matrix, offset, rotate, scale);
		}
		D3DXMATRIX rotation;
		D3DXMatrixRotationYawPitchRoll(&rotation, baseData.rotation.values[0] * 0.017453292f,
			baseData.rotation.values[1] * 0.017453292f, baseData.rotation.values[2] * 0.017453292f);
		D3DXMatrixMultiply(&matrix, &rotation, &matrix);
		SetTransform(matrix);
	}

	BYTE VFX_EffectInstance::IsVisible() const
	{
		OCTREE_Camera* camera = VfxFrame::GetCamera();
		if (!camera) return 0;
		D3DXVECTOR3 low, high;
		GetBounds(low, high);
		float bounds[6] = {low.x, low.y, low.z, high.x, high.y, high.z};
		return camera->ClassifyBox(bounds) != 0;
	}

	BYTE VFX_EffectInstance::IsFinished(float time) const
	{
		if (flagC0 || !flagC1) return 1;
		if (baseData.duration < 0.0f) return 0;
		if (baseData.duration == 0.0f) return 1;
		return baseData.delay + baseData.duration < time ? 1 : 0;
	}

	VFX_Lightning::VFX_Lightning()
	{
		dirty = 1;
		D3DXMatrixIdentity(&transform);
		fresh = 1;
		lastTime = 0;
		repeats = 0;
		meshes.resize(2);
		value268 = 0;
	}

	VFX_Lightning::~VFX_Lightning()
	{
		for (std::vector<std::list<VFX_LightningMesh*> >::iterator bucket = meshes.begin();
			bucket != meshes.end(); ++bucket)
		{
			for (std::list<VFX_LightningMesh*>::iterator i = bucket->begin(); i != bucket->end(); ++i)
			{
				VFX_LightningMesh* owned = *i;
				if (owned->vertices) delete[] owned->vertices;
				if (owned->indices) delete[] owned->indices;
				if (owned->instances)
				{
					delete[] owned->instances;
					owned->instances = NULL;
				}
				if (owned->packet)
				{
					delete owned->packet;
					owned->packet = NULL;
				}
				delete owned;
			}
		}
	}

	void VFX_Lightning::SetData(const VFX_BaseData* input)
	{
		VFX_EffectInstance::SetData(input);
		data = *static_cast<const VFX_LightningData*>(input);
		dirty = 1;
		segments.clear();
		lastTime = -1.0f;
		fresh = 1;
		repeats = 0;
	}

	void VFX_Lightning::Reset() { active = 0; fresh = 1; repeats = 0; }
	BYTE VFX_Lightning::GetFlag() const { return 0; }
	BYTE VFX_Lightning::SkipBounds() const { return 0; }
	void VFX_Lightning::SetTransform(const D3DXMATRIX& input) { transform = input; dirty = 1; }
	BYTE VFX_Lightning::Prepare(Gfx_ModelInstance* source, Gfx_ModelInstance* target, VFX_VEGInstance* group,
		const D3DXMATRIX* sourceTransform, const D3DXMATRIX* targetTransform) const
	{
		VFX_UpdateContext context(source, target, group, sourceTransform, targetTransform);
		if (!VFX_EffectInstance::Prepare(source, target, group, sourceTransform, targetTransform)) return 0;
		return PrepareTarget(source, target, group, data.endPointTarget, data.endPointSocket,
			data.endPointSnapBone, sourceTransform, targetTransform); //4E9170, one owned parameter.
	}

	void VFX_Lightning::GetBounds(D3DXVECTOR3& low, D3DXVECTOR3& high) const
	{
		low.x = FLT_MAX; low.y = FLT_MAX; low.z = FLT_MAX;
		high.x = -FLT_MAX; high.y = -FLT_MAX; high.z = -FLT_MAX;
		for (std::list<VFX_LightningSegment>::const_iterator i = segments.begin(); i != segments.end(); ++i)
		{
			const std::list<VFX_LightningPoint>& points = i->points;
			std::list<VFX_LightningPoint>::const_iterator point = points.begin();
			if (point != points.end()) do
			{
				low.x = point->position.x < low.x ? point->position.x : low.x;
				low.y = point->position.y < low.y ? point->position.y : low.y;
				low.z = point->position.z < low.z ? point->position.z : low.z;
				high.x = point->position.x > high.x ? point->position.x : high.x;
				high.y = point->position.y > high.y ? point->position.y : high.y;
				high.z = point->position.z > high.z ? point->position.z : high.z;
				++point;
			} while (point != points.end());
		}
	}

	float VFX_Lightning::ComputeWidth(LONG index, LONG count) const
	{
		float width = 1.0f;
		switch (data.widthStyle)
		{
		case 0: width = float(double(index) / count * (data.maxWidth - data.minWidth) + data.minWidth); break;
		case 1: width = float(double(index) / count * (data.minWidth - data.maxWidth) + data.maxWidth); break;
		case 2: width = float(VfxFrame::RandomRange(0, data.maxWidth - data.minWidth) + data.minWidth); break;
		}
		return width;
	}
	void VFX_Lightning::GenerateSegments(const D3DXVECTOR3& start, const D3DXVECTOR3& end,
		LONG level, LONG* branchCount)
	{
		if (level > data.maxBranchLevels) return;
		LONG localBranchCount = 0;
		if (!branchCount) branchCount = &localBranchCount;
		D3DXVECTOR3 delta = end - start;
		float length = D3DXVec3Length(&delta);
		float density = float(VfxFrame::RandomRange(data.minSegmentsPerFoot, data.maxSegmentsPerFoot));
		double desired = VfxFrame::RandomRange(data.minAdditionalSegments, data.maxAdditionalSegments)
			+ double(density) * length / (double(scale) * 0.1) + 1.0;
		LONG count = LONG(desired);
		if (count < desired) ++count;
		if (count <= 1) return;
		VFX_LightningSegment segment;
		segment.level = level;
		D3DXVECTOR3 previousPoint, previousBase;
		for (LONG index = 0; index < count; ++index)
		{
			double fraction = double(index) / float(count - 1);
			D3DXVECTOR3 base(float(fraction * delta.x + start.x), float(fraction * delta.y + start.y),
				float(fraction * delta.z + start.z));
			D3DXVECTOR3 position = base;
			if (data.alwaysStrikeTarget && index == count - 1) position = end;
			else if (index > 0)
			{
				D3DXVECTOR3 forward, right, up;
				D3DXVec3Normalize(&forward, &delta);
				if (delta.x == 0 && delta.z == 0)
				{
					D3DXVECTOR3 vertical(0,0,-1);
					D3DXVec3Cross(&right, &forward, &vertical);
					D3DXVec3Normalize(&right, &right);
					D3DXVec3Cross(&up, &right, &forward);
				}
				else
				{
					D3DXVECTOR3 vertical(0,-1,0);
					D3DXVec3Cross(&right, &forward, &vertical);
					D3DXVec3Normalize(&right, &right);
					D3DXVec3Cross(&up, &forward, &right);
				}
				float meander = float(VfxFrame::RandomRange(data.minMeander, data.maxMeander));
				position += (previousPoint - previousBase) * meander * 0.0099999998f;
				float jaggedness = float(VfxFrame::RandomRange(data.minJaggedness, data.maxJaggedness));
				LONG direction = VfxFrame::RandomRange(0,100) < 50 ? 1 : -1;
				position += right * (direction * jaggedness) * 0.001f * length;
				jaggedness = float(VfxFrame::RandomRange(data.minJaggedness, data.maxJaggedness));
				direction = VfxFrame::RandomRange(0,100) < 50 ? 1 : -1;
				position += up * (direction * jaggedness) * 0.001f * length;
				double chance = VfxFrame::RandomRange(0,100);
				if (index < count - 1 && *branchCount < data.maxBranches && chance < data.branchProbability)
				{
					float angle = float(VfxFrame::RandomRange(data.minBranchAngle, data.maxBranchAngle) * 0.017453292f);
					D3DXVECTOR3 remaining = end - position;
					float branchLength = float(VfxFrame::RandomRange(data.minBranchLengthScale, data.maxBranchLengthScale)
						* D3DXVec3Length(&remaining));
					D3DXVECTOR3 branch = forward * float(Gfx_ParticleCos(angle));
					D3DXVECTOR3 radial = right * float(Gfx_ParticleSin(angle));
					D3DXMATRIX rotation;
					D3DXMatrixRotationAxis(&rotation, &forward, float(VfxFrame::RandomRange(0,360) * 0.017453292f));
					D3DXVec3TransformCoord(&radial, &radial, &rotation);
					branch += radial;
					D3DXVec3Normalize(&branch, &branch);
					D3DXVECTOR3 branchEnd = position + branch * branchLength;
					++*branchCount;
					GenerateSegments(position, branchEnd, level + 1, branchCount);
				}
			}
			VFX_LightningPoint point;
			point.position = position;
			float width = ComputeWidth(index, count);
			double widthScale = VfxFrame::RandomRange(data.minBranchWidthScale, data.maxBranchWidthScale);
			point.width = float(pow(widthScale, level) * width);
			segment.points.push_back(point);
			previousPoint = position;
			previousBase = base;
		}
		segments.push_back(segment); //Original appends an owning copy after recursive branches.
	}
	void VFX_Lightning::Update(float time)
	{
		Update(time, VFX_UpdateContext());
	}
	void VFX_Lightning::Update(float time, Gfx_ModelInstance* source, Gfx_ModelInstance* target, VFX_VEGInstance* group,
		const D3DXMATRIX* sourceTransform, const D3DXMATRIX* targetTransform)
	{
		VFX_UpdateContext context(source, target, group, sourceTransform, targetTransform);
		UpdateTransform(context);
		if (lastTime == time) return;
		lastTime = time;
		dirty = 1;
		BYTE generate = 0;
		if (fresh)
		{
			generate = 1; fresh = 0; cycleStart = 0;
			currentLifetime = float(VfxFrame::RandomRange(data.minLifetime, data.maxLifetime));
			currentRepeatDelay = float(VfxFrame::RandomRange(data.minRepeatDelay, data.maxRepeatDelay));
		}
		if (data.minLifetime >= 0 && data.maxLifetime >= 0)
		{
			if (time < cycleStart)
			{
				--repeats; generate = 1;
				currentLifetime = float(VfxFrame::RandomRange(data.minLifetime, data.maxLifetime));
				currentRepeatDelay = float(VfxFrame::RandomRange(data.minRepeatDelay, data.maxRepeatDelay));
				cycleStart = time - currentLifetime - currentRepeatDelay;
				if (cycleStart < 0) cycleStart = 0;
				if (repeats < 0) repeats = 0;
				else if (data.repeatCount < 0 || repeats <= data.repeatCount) segments.clear();
				else generate = 0;
			}
			else if (double(currentRepeatDelay) + cycleStart + currentLifetime < time)
			{
				++repeats; generate = 1; cycleStart = time;
				currentLifetime = float(VfxFrame::RandomRange(data.minLifetime, data.maxLifetime));
				currentRepeatDelay = float(VfxFrame::RandomRange(data.minRepeatDelay, data.maxRepeatDelay));
				if (repeats < 0) repeats = 0;
				else if (data.repeatCount < 0 || repeats <= data.repeatCount) segments.clear();
				else generate = 0;
			}
			if (currentLifetime >= 0 && double(time) - cycleStart > currentLifetime)
			{
				segments.clear();
				if (!generate) return;
			}
			else if (!segments.empty() && !generate) return;
		}
		else if (!generate) return;
		if (data.repeatCount >= 0 && repeats >= data.repeatCount) return;
		segments.clear();
		D3DXMATRIX endpoint;
		const D3DXVECTOR3 start(transform._41, transform._42, transform._43);
		const LONG socket = data.endPointSocket;
		const float offset[3] = {data.endPointOffset.xy.values[0], data.endPointOffset.xy.values[1], data.endPointOffset.z};
		std::string snapBone(data.endPointSnapBone);
		const BYTE rotate = data.rotateWithTarget;
		//Original4ECD57/4ECD8F tests the start target for explicit matrices.
		if (data.target.selector == 1 && context.sourceTransform)
			GetVfxTransform(context.sourceTransform, endpoint, offset, rotate, scale);
		else if (data.target.selector == 2 && context.targetTransform)
			GetVfxTransform(context.targetTransform, endpoint, offset, rotate, scale);
		else
		{
			Gfx_ModelInstance* model = ResolveModel(data.endPointTarget.selector, context);
			GetVfxModelTransform(model, socket, snapBone, endpoint, offset, rotate, scale);
		}
		GenerateSegments(start, D3DXVECTOR3(endpoint._41, endpoint._42, endpoint._43), 0, NULL);
	}
	void VFX_Lightning::Draw()
	{
		Draw(NULL);
	}
	void VFX_Lightning::Draw(Gfx_Viewport* viewport) //4ECEB0.
	{
		if (viewport != lastViewport)
		{
			dirty = 1;
			++value268;
			if (static_cast<LONG>(value268) >= static_cast<LONG>(meshes.size())) value268 = 0;
		}
		if (dirty)
		{
			Gfx_Camera* camera = viewport ? viewport->activeCamera : g_GfxSystem->Camera();
			BuildGeometry(camera);
			dirty = 0;
		}
		lastViewport = viewport;
		if (!meshes[value268].empty())
		{
			for (std::list<VFX_LightningMesh*>::iterator i = meshes[value268].begin();
				i != meshes[value268].end(); ++i)
				g_GfxSystem->QueueMeshInstances((*i)->instances, viewport, (*i)->instanceCount);
		}
	}

	VFX_EffectInstance* NewVfxEffectInstance(const VFX_BaseData* input, float scale)
	{
		if (!input) return NULL;
		VFX_EffectInstance* result = NULL;
		if (input->effectType == 0)
		{
			VFX_ParticleSystem* particles = new VFX_ParticleSystem;
			particles->Load(input, scale);
			return particles;
		}
		else if (input->effectType == 1) result = new VFX_MotionBlur;
		else if (input->effectType == 2) result = new VFX_Sprite;
		else if (input->effectType == 3) result = new VFX_Model;
		else if (input->effectType == 4) result = new VFX_Trail;
		else if (input->effectType == 5) result = new VFX_Collision;
		else if (input->effectType == 6)
		{
			VFX_Water* water = new VFX_Water;
			water->Load(input, scale);
			return water;
		}
		else if (input->effectType == 7) result = new VFX_Lightning;
		else if (input->effectType == 8) result = new VFX_ModelEffect;
		else if (input->effectType == 9) result = new VFX_Light;
		else if (input->effectType == 10) result = new VFX_Rain;
		else if (input->effectType == 11) result = new VFX_Snow;
		else if (input->effectType == 12) result = new VFX_Wind;
		else if (input->effectType == 13) result = new VFX_Sound;
		else if (input->effectType >= 0 && input->effectType <= 13)
			throw std::logic_error("VFX_VEGInstance48CDA0: required non-Lightning effect owner unresolved");
		if (result) result->Load(*input, scale);
		return result;
	}
	VFX_EffectInstance* VFX_VEGInstance::CreateEffect(VFX_Effect* definition) //48CDA0.
	{
		if (!definition || !definition->data) return NULL;
		switch (definition->data->effectType)
		{
		case 0:
		{
			VFX_ParticleSystem* result = new VFX_ParticleSystem;
			result->Load(definition->data, scale);
			return result;
		}
		case 1:
		{
			VFX_MotionBlur* result = new VFX_MotionBlur;
			result->Load(*definition->data, scale);
			return result;
		}
		case 2:
		{
			VFX_Sprite* result = new VFX_Sprite;
			result->Load(*definition->data, scale);
			return result;
		}
		case 3:
		{
			VFX_Model* result = new VFX_Model;
			result->Load(*definition->data, scale);
			return result;
		}
		case 4:
		{
			VFX_Trail* result = new VFX_Trail;
			result->Load(*definition->data, scale);
			return result;
		}
		case 5:
		{
			VFX_Collision* result = new VFX_Collision;
			result->Load(*definition->data, scale);
			return result;
		}
		case 6:
		{
			VFX_Water* result = new VFX_Water;
			result->Load(definition->data, scale);
			return result;
		}
		case 7:
		{
			VFX_Lightning* result = new VFX_Lightning;
			result->Load(*definition->data, scale);
			return result;
		}
		case 8:
		{
			VFX_ModelEffect* result = new VFX_ModelEffect;
			result->Load(*definition->data, scale);
			return result;
		}
		case 9:
		{
			VFX_Light* result = new VFX_Light;
			result->Load(*definition->data, scale);
			return result;
		}
		case 10:
		{
			VFX_Rain* result = new VFX_Rain;
			result->Load(*definition->data, scale);
			return result;
		}
		case 11:
		{
			VFX_Snow* result = new VFX_Snow;
			result->Load(*definition->data, scale);
			return result;
		}
		case 12:
		{
			VFX_Wind* result = new VFX_Wind;
			result->Load(*definition->data, scale);
			return result;
		}
		case 13:
		{
			VFX_Sound* result = new VFX_Sound;
			result->Load(*definition->data, scale);
			return result;
		}
		default: return NULL;
		}
	}

	VFX_EffectInstance* __stdcall CloneVfxEffectInstance(VFX_Effect* definition,
		const VFX_EffectInstance* prototype) //48D2C0.
	{
		if (!definition || !definition->data) return NULL;
		switch (definition->data->effectType)
		{
		case 0:
		{
			VFX_ParticleSystem* result=new VFX_ParticleSystem;
			result->CopyFrom(*static_cast<const VFX_ParticleSystem*>(prototype));
			return result;
		}
		case 1:
		{
			VFX_MotionBlur* result=new VFX_MotionBlur;
			result->CopyFrom(*static_cast<const VFX_MotionBlur*>(prototype));
			return result;
		}
		case 2:
		{
			VFX_Sprite* result=new VFX_Sprite;
			result->CopyFrom(*static_cast<const VFX_Sprite*>(prototype));
			return result;
		}
		case 3:
		{
			VFX_Model* result=new VFX_Model;
			result->CopyFrom(*static_cast<const VFX_Model*>(prototype));
			return result;
		}
		case 4:
		{
			VFX_Trail* result=new VFX_Trail;
			result->CopyFrom(*static_cast<const VFX_Trail*>(prototype));
			return result;
		}
		case 5:
		{
			VFX_Collision* result=new VFX_Collision;
			result->CopyFrom(*static_cast<const VFX_Collision*>(prototype));
			return result;
		}
		case 6:
		{
			VFX_Water* result=new VFX_Water;
			result->CopyFrom(*static_cast<const VFX_Water*>(prototype));
			return result;
		}
		case 8:
		{
			VFX_ModelEffect* result=new VFX_ModelEffect;
			result->CopyFrom(*static_cast<const VFX_ModelEffect*>(prototype));
			return result;
		}
		case 9:
		{
			VFX_Light* result=new VFX_Light;
			result->CopyFrom(*static_cast<const VFX_Light*>(prototype));
			return result;
		}
		case 10:
		{
			VFX_Rain* result=new VFX_Rain;
			result->CopyFrom(*static_cast<const VFX_Rain*>(prototype));
			return result;
		}
		case 11:
		{
			VFX_Snow* result=new VFX_Snow;
			result->CopyFrom(*static_cast<const VFX_Snow*>(prototype));
			return result;
		}
		case 12:
		{
			VFX_Wind* result=new VFX_Wind;
			result->CopyFrom(*static_cast<const VFX_Wind*>(prototype));
			return result;
		}
		case 13:
		{
			VFX_Sound* result=new VFX_Sound;
			result->CopyFrom(*static_cast<const VFX_Sound*>(prototype));
			return result;
		}
		default:
			return NULL;
		}
	}
}
