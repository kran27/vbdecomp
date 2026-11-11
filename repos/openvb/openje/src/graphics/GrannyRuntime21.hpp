#pragma once

#include <cstddef>

// Declarations for the shipped x86 Granny2.1.0.5 DLL, not the newer2.8 SDK ABI.
// Opaque handles are owned by the real runtime. The selected public record
// layouts are established by its exported type schemas and F3's callers.
namespace Granny21
{
	struct File;
	struct FileStringTable;
	struct ArtToolInfo;
	struct ExporterInfo;
	struct Texture;
	struct Material;
	struct VertexData;
	struct TriTopology;
	struct Mesh;
	struct MorphTarget;
	struct MaterialBinding;
	struct ModelInstance;
	struct LocalPose;
	struct WorldPose;
	struct MeshBinding;
	struct TrackMask;
	struct TrackGroup;
	struct Animation;
	struct Control;
	struct ControlledAnimationBuilder;
	struct DataTypeDefinition;

	struct Variant
	{
		DataTypeDefinition* type;
		void* object;
	};

	struct Transform
	{
		unsigned int flags;
		float position[3];
		float orientation[4];
		float scaleShear[9];
	};

	struct Bone
	{
		const char* name;
		int parentIndex;
		Transform transform;
		float inverseWorldTransform[16];
		void* lightInfo;
		void* cameraInfo;
		Variant extendedData;
	};

	struct Skeleton
	{
		const char* name;
		int boneCount;
		Bone* bones;
	};

	// Public mesh/OBB records shared with the SDK. F3's 4A4CE0 establishes
	// Mesh bone count+1C, bindings+20 and the 36-byte binding stride in2.1.
	struct BoneBinding
	{
		const char* boneName;
		float minimum[3], maximum[3];
		int triangleCount;
		int* triangleIndices;
	};

	struct Mesh
	{
		const char* name;
		VertexData* primaryVertexData;
		int morphTargetCount;
		MorphTarget* morphTargets;
		TriTopology* primaryTopology;
		int materialBindingCount;
		MaterialBinding* materialBindings;
		int boneBindingCount;
		BoneBinding* boneBindings;
		Variant extendedData;
	};

	struct BoxIntersection
	{
		float minimumT, minimumNormal[3];
		float maximumT, maximumNormal[3];
	};

	struct ModelMeshBinding
	{
		Mesh* mesh;
	};

	struct Model
	{
		const char* name;
		Skeleton* skeleton;
		Transform initialPlacement;
		int meshBindingCount;
		ModelMeshBinding* meshBindings;
	};

	struct FileInfo
	{
		FileStringTable* stringTable; // Removed from the later2.8 public layout.
		ArtToolInfo* artToolInfo;
		ExporterInfo* exporterInfo;
		const char* fromFileName;
		int textureCount;
		Texture** textures;
		int materialCount;
		Material** materials;
		int skeletonCount;
		Skeleton** skeletons;
		int vertexDataCount;
		VertexData** vertexDatas;
		int triTopologyCount;
		TriTopology** triTopologies;
		int meshCount;
		Mesh** meshes;
		int modelCount;
		Model** models;
		int trackGroupCount;
		TrackGroup** trackGroups;
		int animationCount;
		Animation** animations;
	};

#if defined(_M_IX86)
	typedef char TransformSize[(sizeof(Transform) == 68) ? 1 : -1];
	typedef char BoneSize[(sizeof(Bone) == 156) ? 1 : -1];
	typedef char SkeletonSize[(sizeof(Skeleton) == 12) ? 1 : -1];
	typedef char BoneBindingSize[(sizeof(BoneBinding) == 36) ? 1 : -1];
	typedef char MeshBoneBindingCount[(offsetof(Mesh, boneBindingCount) == 0x1C) ? 1 : -1];
	typedef char MeshBoneBindings[(offsetof(Mesh, boneBindings) == 0x20) ? 1 : -1];
	typedef char BoxIntersectionSize[(sizeof(BoxIntersection) == 32) ? 1 : -1];
	typedef char ModelSize[(sizeof(Model) == 84) ? 1 : -1];
	typedef char FileInfoSize[(sizeof(FileInfo) == 88) ? 1 : -1];
	typedef char FileInfoSkeletons[(offsetof(FileInfo, skeletons) == 0x24) ? 1 : -1];
	typedef char FileInfoModels[(offsetof(FileInfo, models) == 0x44) ? 1 : -1];
#endif

	// These declarations reuse the actual DLL; no file decoder, pose sampler or
	// animation runtime implementation is supplied here. The import library must
	// preserve both the C stdcall symbol and DLL's leading-underscore export name.
	extern "C"
	{
		__declspec(dllimport) File* __stdcall GrannyReadEntireFileFromMemory(int size, const void* memory);
		__declspec(dllimport) FileInfo* __stdcall GrannyGetFileInfo(File* file);
		__declspec(dllimport) void __stdcall GrannyFreeFile(File* file);
		__declspec(dllimport) bool __stdcall GrannyComputeBasisConversion(const FileInfo* file,
			float unitsPerMeter, const float* origin, const float* right, const float* up,
			const float* back, float* affine, float* linear, float* inverseLinear);
		// 2.1 takes FOUR arguments; the2.8 SDK's seven-argument declaration is incompatible.
		__declspec(dllimport) void __stdcall GrannyTransformFile(FileInfo* file,
			const float* affine, const float* linear, const float* inverseLinear);
		__declspec(dllimport) ModelInstance* __stdcall GrannyInstantiateModel(const Model* model);
		__declspec(dllimport) void __stdcall GrannyFreeModelInstance(ModelInstance* instance);
		__declspec(dllimport) Skeleton* __stdcall GrannyGetSourceSkeleton(const ModelInstance* instance);
		__declspec(dllimport) LocalPose* __stdcall GrannyNewLocalPose(int boneCount);
		__declspec(dllimport) void __stdcall GrannyFreeLocalPose(LocalPose* pose);
		__declspec(dllimport) WorldPose* __stdcall GrannyNewWorldPose(int boneCount);
		__declspec(dllimport) void __stdcall GrannyFreeWorldPose(WorldPose* pose);
		__declspec(dllimport) void __stdcall GrannySampleModelAnimations(const ModelInstance* instance,
			int firstBone, int boneCount, LocalPose* result);
		__declspec(dllimport) void __stdcall GrannyBuildWorldPose(const Skeleton* skeleton,
			int firstBone, int boneCount, const LocalPose* localPose, const float* offset, WorldPose* result);
		__declspec(dllimport) float* __stdcall GrannyGetWorldPose4x4Array(const WorldPose* pose);
		__declspec(dllimport) float* __stdcall GrannyGetWorldPoseComposite4x4Array(const WorldPose* pose);
		__declspec(dllimport) MeshBinding* __stdcall GrannyNewMeshBinding(const Mesh* mesh,
			const Skeleton* fromSkeleton, const Skeleton* toSkeleton);
		__declspec(dllimport) void __stdcall GrannyFreeMeshBinding(MeshBinding* binding);
		__declspec(dllimport) const int* __stdcall GrannyGetMeshBindingToBoneIndices(const MeshBinding* binding);
		__declspec(dllimport) int __stdcall GrannyRayIntersectsBoxAt(const float* transform,
			const float* minimum, const float* maximum, const float* origin,
			const float* direction, BoxIntersection* intersection);
		__declspec(dllimport) bool __stdcall GrannyFindBoneByName(const Skeleton* skeleton,
			const char* name, int* index);
		__declspec(dllimport) TrackMask* __stdcall GrannyNewTrackMask(float defaultWeight, int boneCount);
		__declspec(dllimport) void __stdcall GrannyFreeTrackMask(TrackMask* mask);
		__declspec(dllimport) ControlledAnimationBuilder* __stdcall GrannyBeginControlledAnimation(
			float startTime, const Animation* animation);
		__declspec(dllimport) void __stdcall GrannySetTrackGroupTarget(ControlledAnimationBuilder* builder,
			int trackGroupIndex, ModelInstance* instance);
		__declspec(dllimport) void __stdcall GrannySetTrackGroupModelMask(ControlledAnimationBuilder* builder,
			int trackGroupIndex, const TrackMask* mask);
		__declspec(dllimport) Control* __stdcall GrannyEndControlledAnimation(ControlledAnimationBuilder* builder);
		__declspec(dllimport) void __stdcall GrannySetControlClock(Control* control, float time);
		__declspec(dllimport) void __stdcall GrannyFreeControl(Control* control);
	}
}

// Proprietary integration policy4A1E20. The original passes FileInfo in ESI;
// this ordinary source boundary makes that receiver explicit. No SDK internals.
void Gfx_TransformGrannyFileToLHS(Granny21::FileInfo* file, float unitsPerMeter);
