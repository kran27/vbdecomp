#include "AnimationObject.hpp"
#include "AnimationData.hpp"
#include "AnimationInstance.hpp"
#include "../diagnostics/DebugOutput.hpp"
#include <string.h>
#include <string>
#include <stdexcept>

// Semantic table data is transcribed from F3's6FC0C0 pointer table, independently
// of B3D file bone order. No Granny implementation is supplied here.
const char* const g_GfxSemanticBoneNames[212] =
{
	"-- none --", // 0
	"Root", // 1
	"Pelvis", // 2
	"R Hip", // 3
	"R Thigh", // 4
	"R Calf", // 5
	"R Foot", // 6
	"R Toes", // 7
	"L Hip", // 8
	"L Thigh", // 9
	"L Calf", // 10
	"L Foot", // 11
	"L Toes", // 12
	"Spine_1", // 13
	"Spine_2", // 14
	"Neck", // 15
	"Head", // 16
	"R Clavicle", // 17
	"R Clavicle_1", // 18
	"R Clavicle_2", // 19
	"R UpperArm", // 20
	"R Forearm", // 21
	"R Wrist", // 22
	"R Hand", // 23
	"R Fingers", // 24
	"R Fingers_1", // 25
	"R Fingers_2", // 26
	"R Finger Index", // 27
	"R Finger Point", // 28
	"R Finger Pinky", // 29
	"R Handle_1", // 30
	"R Handle_2", // 31
	"L Clavicle", // 32
	"L Clavicle_1", // 33
	"L Clavicle_2", // 34
	"L UpperArm", // 35
	"L Forearm", // 36
	"L Wrist", // 37
	"L Hand", // 38
	"L Fingers", // 39
	"L Fingers_1", // 40
	"L Fingers_2", // 41
	"L Finger Index", // 42
	"L Finger Point", // 43
	"L Finger Pinky", // 44
	"L Handle_1", // 45
	"L Handle_2", // 46
	"Ponytail_1", // 47
	"Ponytail_2", // 48
	"Ponytail_3", // 49
	"Cloak_1", // 50
	"Cloak_2", // 51
	"Cloak_3", // 52
	"Cloak_4", // 53
	"Jaw", // 54
	"R ForeHip", // 55
	"R ForeThigh", // 56
	"R ForeCalf", // 57
	"R ForeFoot", // 58
	"R ForeToes", // 59
	"L ForeHip", // 60
	"L ForeThigh", // 61
	"L ForeCalf", // 62
	"L ForeFoot", // 63
	"L ForeToes", // 64
	"R HindHip", // 65
	"R HindThigh", // 66
	"R HindCalf", // 67
	"R HindFoot", // 68
	"R HindToes", // 69
	"L HindHip", // 70
	"L HindThigh", // 71
	"L HindCalf", // 72
	"L HindFoot", // 73
	"L HindToes", // 74
	"1 Hip", // 75
	"2 Hip", // 76
	"3 Hip", // 77
	"1 Thigh", // 78
	"2 Thigh", // 79
	"3 Thigh", // 80
	"1 Calf", // 81
	"2 Calf", // 82
	"3 Calf", // 83
	"1 Foot", // 84
	"2 Foot", // 85
	"3 Foot", // 86
	"1 Toes", // 87
	"2 Toes", // 88
	"3 Toes", // 89
	"Spine_3", // 90
	"1 Clavicle", // 91
	"2 Clavicle", // 92
	"3 Clavicle", // 93
	"1 UpperArm", // 94
	"2 UpperArm", // 95
	"3 UpperArm", // 96
	"1 Forearm", // 97
	"2 Forearm", // 98
	"3 Forearm", // 99
	"1 Hand", // 100
	"2 Hand", // 101
	"3 Hand", // 102
	"1 Fingers", // 103
	"2 Fingers", // 104
	"3 Fingers", // 105
	"1 Mouth_0", // 106
	"1 Mouth_1", // 107
	"2 Mouth_0", // 108
	"2 Mouth_1", // 109
	"3 Mouth_0", // 110
	"3 Mouth_1", // 111
	"1 Neck", // 112
	"2 Neck", // 113
	"3 Neck", // 114
	"jaw_1", // 115
	"jaw_2", // 116
	"L HindHip 2", // 117
	"L HindThigh 2", // 118
	"L HindCalf 2", // 119
	"L HindFoot 2", // 120
	"L HindToes 2", // 121
	"R HindHip 2", // 122
	"R HindThigh 2", // 123
	"R HindCalf 2", // 124
	"R HindFoot 2", // 125
	"R HindToes 2", // 126
	"L HindHip 3", // 127
	"L HindThigh 3", // 128
	"L HindCalf 3", // 129
	"L HindFoot 3", // 130
	"L HindToes 3", // 131
	"R HindHip 3", // 132
	"R HindThigh 3", // 133
	"R HindCalf 3", // 134
	"R HindFoot 3", // 135
	"R HindToes 3", // 136
	"L Mandible_1", // 137
	"L Mandible_2", // 138
	"L Mandible_3", // 139
	"R Mandible_1", // 140
	"R Mandible_2", // 141
	"R Mandible_3", // 142
	"Tail_1", // 143
	"Tail_2", // 144
	"Tail_3", // 145
	"R Brow", // 146
	"L Brow", // 147
	"L Thumb_1", // 148
	"L Thumb_2", // 149
	"R Thumb_1", // 150
	"R Thumb_2", // 151
	"L Wing_1", // 152
	"L Wing_2", // 153
	"L Wing_3", // 154
	"R Wing_1", // 155
	"R Wing_2", // 156
	"R Wing_3", // 157
	"L Tentcl_1", // 158
	"R Tentcl_1", // 159
	"L Tentcl_2", // 160
	"R Tentcl_2", // 161
	"L Tentcl_3", // 162
	"R Tentcl_3", // 163
	"L Tentcl_4", // 164
	"R Tentcl_4", // 165
	"L Tentcl_5", // 166
	"R Tentcl_5", // 167
	"L Tentcl_6", // 168
	"R Tentcl_6", // 169
	"L Brain", // 170
	"R Brain", // 171
	"Weapon_1", // 172
	"Weapon_2", // 173
	"Weapon_3", // 174
	"Weapon_4", // 175
	"FX_Muzzle", // 176
	"FX_Brass", // 177
	"Tree_01", // 178
	"Tree_02", // 179
	"Tree_03", // 180
	"Tree_04", // 181
	"Bone01", // 182
	"Bone02", // 183
	"Bone03", // 184
	"Bone04", // 185
	"Bone05", // 186
	"Bone06", // 187
	"Bone07", // 188
	"Bone08", // 189
	"Bone09", // 190
	"Bone10", // 191
	"Bone11", // 192
	"Bone12", // 193
	"Bone13", // 194
	"Bone14", // 195
	"Bone15", // 196
	"Bone16", // 197
	"Bone17", // 198
	"Bone18", // 199
	"Bone19", // 200
	"Bone20", // 201
	"Bone21", // 202
	"Bone22", // 203
	"Bone23", // 204
	"Bone24", // 205
	"Bone25", // 206
	"Bone26", // 207
	"Bone27", // 208
	"Bone28", // 209
	"Bone29", // 210
	"Bone30" // 211
};

std::string g_GfxReservedAnimationNames[4] = { "Light", "Camera", "Null", "NULL" };

namespace
{
	bool Reserved(const char* name)
	{
		for (DWORD index = 0; index < 4; ++index)
			if (g_GfxReservedAnimationNames[index] == name) return true;
		return false;
	}

	const char* MeshName(const Granny21::Mesh* mesh)
	{
		// Public Granny2.1 Mesh prefix: Name is its first pointer. Keep the rest
		// opaque; this is a borrowed DLL record, never a fabricated mesh owner.
		const char* name;
		memcpy(&name, mesh, sizeof(name));
		return name;
	}
}

DWORD Gfx_FindSemanticBone(const char* name)
{
	for (DWORD index = 0; index < 212; ++index)
	{
		if (!strcmp(name, g_GfxSemanticBoneNames[index])) return index;
	}
	return 212;
}

Gfx_AnimationObject::Gfx_AnimationObject()
	: file(NULL), fileInfo(NULL), model(NULL), skeleton(NULL), collisionMesh(NULL),
	meshBinding(NULL), scale(1.0f), localPose(NULL)
{
	memset(semanticBoneIndices, 0, sizeof(semanticBoneIndices));
}

Gfx_AnimationObject::~Gfx_AnimationObject()
{
	ClearAnimationGroups();
	Unload();
}

void Gfx_AnimationObject::DestroyAnimationGroup(DWORD group)
{
	std::map<DWORD, std::vector<Gfx_AnimationData*> >::iterator found = animationGroups.find(group);
	if (found != animationGroups.end())
	{
		std::vector<Gfx_AnimationData*>& entries = found->second;
		std::vector<Gfx_AnimationData*>::iterator item = entries.begin();
		while (item != entries.end())
		{
			Gfx_AnimationData* data = *item;
			if (data) delete data;
			item = entries.erase(item);
		}
	}
	else
	{
		JE::LogDebugString("Gfx_AnimationObject::DestroyAnimationGroup - Unknown animation group: %d\n", group);
	}
}

BYTE Gfx_AnimationObject::LoadAnimationFileFromBuffer(DWORD group, int size,
	const void* buffer, float value)
{
	if (!size)
	{
		JE::LogDebugString("Gfx_AnimationObject::LoadAnimationFileFromBuffer - 0 buffersize!\n");
		return 0;
	}
	if (!buffer)
	{
		JE::LogDebugString("Gfx_AnimationObject::LoadAnimationFileFromBuffer - NULL buffer!\n");
		return 0;
	}
	if (value <= 0.0f)
	{
		JE::LogDebugString("Gfx_AnimationObject::LoadAnimationFileFromBuffer - invalid scale: %f\n", value);
		return 0;
	}
	BYTE result = 0;
	Gfx_AnimationData* data = new Gfx_AnimationData;
	if (data->LoadAnimationDataFromBuffer(size, buffer, value))
	{
		std::vector<Gfx_AnimationData*>& entries = animationGroups[group];
		entries.push_back(data);
		result = 1;
	}
	// Original4A4B58..4A4B89 does not destroy/delete the failed new owner.
	return result;
}

BYTE Gfx_AnimationObject::HasAnimation(DWORD group, DWORD index, DWORD subAnimation) const
{
	BYTE result = 0;
	std::map<DWORD, std::vector<Gfx_AnimationData*> >::const_iterator found = animationGroups.find(group);
	if (found != animationGroups.end())
	{
		const std::vector<Gfx_AnimationData*>& entries = found->second;
		if (index < entries.size() && subAnimation < static_cast<DWORD>(entries[index]->fileInfo->animationCount))
			result = 1;
	}
	return result;
}

float Gfx_AnimationObject::GetDuration(DWORD group, DWORD index, DWORD subAnimation)
{
	float result = 0.0f;
	std::map<DWORD, std::vector<Gfx_AnimationData*> >::iterator found = animationGroups.find(group);
	if (found != animationGroups.end())
	{
		const std::vector<Gfx_AnimationData*>& entries = found->second;
		if (index < entries.size())
			result = *reinterpret_cast<const float*>(reinterpret_cast<const BYTE*>(entries[index]->fileInfo->animations[subAnimation]) + 4);
		else
			JE::LogDebugString("Gfx_AnimationObject::GetDuration - %s Unknown animation index: %d\n", skeleton->name, index);
	}
	return result;
}

Granny21::Control* Gfx_AnimationObject::ApplyAnimation(Granny21::ModelInstance* instance,
	Granny21::TrackMask* mask, DWORD group, DWORD index, DWORD subAnimation, float startTime)
{
	if (!instance)
	{
		JE::LogDebugString("Gfx_AnimationObject::ApplyAnimation - pModelInstance == NULL\n");
		return NULL;
	}
	if (!mask)
	{
		JE::LogDebugString("Gfx_AnimationObject::ApplyAnimation - pModelTrackMask == NULL\n");
		return NULL;
	}
	std::map<DWORD, std::vector<Gfx_AnimationData*> >::iterator found = animationGroups.find(group);
	if (found == animationGroups.end())
	{
		JE::LogDebugString("Gfx_AnimationObject::ApplyAnimation - %s Unknown animation group: %d\n", skeleton->name, group);
		return NULL;
	}
	if (index >= found->second.size())
	{
		JE::LogDebugString("Gfx_AnimationObject::ApplyAnimation - %s anim_index out of bounds: %d\n", skeleton->name, index);
		return NULL;
	}
	Gfx_AnimationData* data = found->second[index];
	Granny21::ControlledAnimationBuilder* builder = Granny21::GrannyBeginControlledAnimation(startTime,
		data->fileInfo->animations[subAnimation]); // No original subAnimation bound check here.
	Granny21::GrannySetTrackGroupTarget(builder, data->trackGroupIndex, instance);
	Granny21::GrannySetTrackGroupModelMask(builder, data->trackGroupIndex, mask);
	return Granny21::GrannyEndControlledAnimation(builder);
}

void Gfx_AnimationObject::ClearAnimationCues(DWORD group, DWORD index)
{
	std::map<DWORD, std::vector<Gfx_AnimationData*> >::iterator found;
	found = animationGroups.find(group);
	if (found == animationGroups.end()) return;
	const std::vector<Gfx_AnimationData*>& entries = found->second;
	if (index >= entries.size())
	{
		JE::LogDebugString("Gfx_AnimationObject::ClearAnimationCues - %s anim_index out of bounds: %d\n",
			skeleton->name, index);
		return;
	}
	entries[index]->ClearAnimationCues();
}

void Gfx_AnimationObject::AddAnimationCue(DWORD group, DWORD index, float time, const char* text)
{
	std::map<DWORD, std::vector<Gfx_AnimationData*> >::iterator found;
	found = animationGroups.find(group);
	if (found != animationGroups.end())
	{
		const std::vector<Gfx_AnimationData*>& entries = found->second;
		if (index >= entries.size())
		{
			JE::LogDebugString("Gfx_AnimationObject::AddAnimationCue - %s anim_index out of bounds: %d\n",
				skeleton->name, index);
			return;
		}
		entries[index]->AddAnimationCue(time, text);
	}
	else
	{
		JE::LogDebugString("Gfx_AnimationObject::AddAnimationCue - %s Unknown animation group: %d\n",
			skeleton->name, group);
	}
}

BYTE Gfx_AnimationObject::ProcessAnimationCues(DWORD group, DWORD index, float seconds,
	BYTE reverse, float** position, std::list<Gfx_AnimationCue>& output)
{
	std::map<DWORD, std::vector<Gfx_AnimationData*> >::iterator found = animationGroups.find(group);
	if (found == animationGroups.end()) return 0;
	if (index >= found->second.size())
	{
		JE::LogDebugString("Gfx_AnimationObject::ProcessAnimationCues - %s anim_index out of bounds: %d\n", skeleton->name, index);
		return 0;
	}
	return found->second[index]->ProcessAnimationCues(group,index,seconds,reverse,position,output);
}

void Gfx_AnimationObject::ClearAnimationGroups()
{
	for (std::map<DWORD, std::vector<Gfx_AnimationData*> >::iterator entry = animationGroups.begin();
		entry != animationGroups.end(); ++entry)
	{
		DestroyAnimationGroup(entry->first);
	}
	animationGroups.clear();
}

void Gfx_AnimationObject::Unload()
{
	if (localPose)
	{
		Granny21::GrannyFreeLocalPose(localPose);
		localPose = NULL;
	}
	if (meshBinding)
	{
		Granny21::GrannyFreeMeshBinding(meshBinding);
		meshBinding = NULL;
	}
	model = NULL;
	skeleton = NULL;
	collisionMesh = NULL;
	if (file)
	{
		Granny21::GrannyFreeFile(file);
		file = NULL;
		fileInfo = NULL;
		scale = 1.0f;
	}
}

BYTE Gfx_AnimationObject::LoadSkeletonFromBuffer(int size, const void* buffer, float value)
{
	if (file)
	{
		JE::LogDebugString("Gfx_AnimationObject::LoadSkeletonFromBuffer - Skeleton already loaded!  Unload it first!\n");
		return 0;
	}
	file = Granny21::GrannyReadEntireFileFromMemory(size, buffer);
	if (!file)
	{
		JE::LogDebugString("Gfx_AnimationObject::LoadSkeletonFromBuffer - Failed!");
		return 0;
	}
	fileInfo = Granny21::GrannyGetFileInfo(file);
	Gfx_TransformGrannyFileToLHS(fileInfo, value);
	scale = value;
	if (!fileInfo->modelCount)
	{
		JE::LogDebugString("Gfx_AnimationObject::LoadSkeletonFromBuffer - NO MODEL (SKELETON) DATA IN .GR2! (ModelCount = 0)");
		return 0;
	}
	// The native negative-model path reads the low byte of the incoming scale
	// representation after publishing scale; later negative counts select index0.
	if (fileInfo->modelCount < 0 && reinterpret_cast<const BYTE*>(&value)[0])
	{
		JE::LogDebugString("Gfx_AnimationObject::LoadSkeletonFromBuffer - NO VALID MODEL (SKELETON) DATA IN .GR2!");
		return 0;
	}
	int index = 0;
	while (index < fileInfo->modelCount && Reserved(fileInfo->models[index]->name)) ++index;
	if (index == fileInfo->modelCount)
	{
		JE::LogDebugString("Gfx_AnimationObject::LoadSkeletonFromBuffer - NO VALID MODEL (SKELETON) DATA IN .GR2!");
		return 0;
	}
	model = fileInfo->models[index];
	if (!fileInfo->skeletonCount)
	{
		JE::LogDebugString("Gfx_AnimationObject::LoadSkeletonFromBuffer - NO SKELETON DATA IN .GR2! (SkeletonCount = 0)");
		return 0;
	}
	index = 0;
	while (index < fileInfo->skeletonCount && Reserved(fileInfo->skeletons[index]->name)) ++index;
	if (index == fileInfo->skeletonCount)
	{
		JE::LogDebugString("Gfx_AnimationObject::LoadSkeletonFromBuffer - NO VALID SKELETON DATA IN .GR2!");
		return 0;
	}
	skeleton = fileInfo->skeletons[index];
	if (fileInfo->meshCount)
	{
		index = 0;
		while (index < fileInfo->meshCount && Reserved(MeshName(fileInfo->meshes[index]))) ++index;
		if (index != fileInfo->meshCount)
		{
			collisionMesh = fileInfo->meshes[index];
			meshBinding = Granny21::GrannyNewMeshBinding(collisionMesh, skeleton, skeleton);
			if (!meshBinding)
			{
				JE::LogDebugString("Gfx_AnimationObject::LoadSkeletonFromBuffer - %s Failed to create Collision Mesh Bindings!\n", skeleton->name);
				collisionMesh = NULL;
			}
		}
		else
		{
			JE::LogDebugString("Gfx_AnimationObject::LoadSkeletonFromBuffer - %s NO VALID COLLISION MESH DATA IN .GR2!\n", skeleton->name);
		}
	}
	else
	{
		JE::LogDebugString("Gfx_AnimationObject::LoadSkeletonFromBuffer - %s NO COLLISION MESH DATA IN .GR2!\n", skeleton->name);
	}
	localPose = Granny21::GrannyNewLocalPose(skeleton->boneCount);
	ResolveGrannyBoneIndices();
	return 1;
}

void Gfx_AnimationObject::ResolveGrannyBoneIndices()
{
	int count = skeleton->boneCount;
	BYTE* resolved = static_cast<BYTE*>(::operator new[](count));
	memset(resolved, 0, count);
	for (DWORD index = 0; index < 212; ++index)
	{
		if (Granny21::GrannyFindBoneByName(skeleton, g_GfxSemanticBoneNames[index], semanticBoneIndices + index))
		{
			resolved[semanticBoneIndices[index]] = 1;
		}
	}
	for (int bone = 0; bone < count; ++bone)
	{
		if (!resolved[bone] && _stricmp(skeleton->name, skeleton->bones[bone].name))
		{
			JE::LogDebugString("Gfx_AnimationObject::ResolveGrannyBoneIndex - %s Unresolved Bone: %s\n",
				skeleton->name, skeleton->bones[bone].name);
		}
	}
	if (resolved) ::operator delete[](resolved);
}

Granny21::ModelInstance* Gfx_AnimationObject::CreateModelInstance()
{
	if (model) return Granny21::GrannyInstantiateModel(model);
	JE::LogDebugString("Gfx_AnimationObject::CreateModelInstance - NO MODEL LOADED!\n");
	return NULL;
}

void Gfx_AnimationObject::UpdateWorldPose(Granny21::ModelInstance* instance,
	Granny21::WorldPose* result, const float* offset)
{
	if (!skeleton)
	{
		JE::LogDebugString("Gfx_AnimationObject::UpdateWorldPose - No skeleton loaded!\n");
	}
	else if (!instance)
	{
		JE::LogDebugString("Gfx_AnimationObject::UpdateWorldPose - NULL ModelInstance\n");
	}
	else
	{
		Granny21::GrannySampleModelAnimations(instance, 0, skeleton->boneCount, localPose);
		Granny21::GrannyBuildWorldPose(skeleton, 0, skeleton->boneCount, localPose, offset, result);
	}
}

#if defined(_MSC_VER) && _MSC_VER == 1300
typedef char AnimationObjectSize[(sizeof(Gfx_AnimationObject) == 892) ? 1 : -1];
#endif
