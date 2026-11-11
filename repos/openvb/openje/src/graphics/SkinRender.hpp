#pragma once

#include "RenderManager.hpp"
#include "SkinManager.hpp"
#include "ModelMesh.hpp"
#include <vector>
#include <cstddef>

class Gfx_Camera;
class Gfx_AnimationInstance;
class Gfx_ModelData526620;
struct Gfx_G3D_BoneWeight;

struct Gfx_SkinFace
{
	Gfx_SkinFace() {} // Unwritten bytes stay unwritten, including trailing padding.
	D3DXVECTOR3 normal;
	struct Edge { WORD vertices[2]; LONG adjacentFace; DWORD ownerFace; } edges[3];
	BYTE flagB9, flagBA;
};

struct Gfx_SkinEdge { WORD vertices[2]; LONG adjacentFace; DWORD ownerFace; };

//Actual raw owners throughA4 and the two observed scratch arrays. The last12B
//of the original130h allocation remain untyped; no filler or full extent is claimed.
class Gfx_SkinMesh : public Gfx_NodeAttribute
{
public:
	Gfx_SkinMesh(); // 0x508920.
	virtual ~Gfx_SkinMesh(); // 0x508A50.
	void SetMaterialCount(DWORD count); //508830, publish/allocate; deliberately leaks old array.
	void ResetMaterialExtraNames(); //524450, distinct outer-record cleanup/allocation.
	void SetMaterialExtraNameCount(DWORD index, DWORD count); //524270, owned string array.
	std::string GetMaterialExtraName(DWORD index, DWORD name) const; //508D70, unchecked owned copy.
	void SetMaterial(DWORD index, DWORD style, const std::string& materialName, DWORD blend,
		const D3DMATERIAL8& values, DWORD textures, const std::string* names,
		const DWORD* handles, BYTE flagB9, BYTE flagBA); // 0x508E90.
	HRESULT CreateSkinMeshFVF(DWORD fvf, DWORD vertices, const void* vertexData,
		DWORD groups, const std::string* names, const DWORD* indexCounts,
		const DWORD* primitiveCounts, const void* const* indices,
		DWORD boneCount, Gfx_ModelData526620* model,
		const Gfx_G3D_BoneWeight* const* weights, DWORD unused); //50A600,12 stack words.
	void RebuildGroups(const Gfx_Material* values, std::list<DWORD>* output); // 0x5093B0.
	DWORD GetPrimaryTexture(DWORD index); //5079E0; unsigned live bound, absent returns0.
	void Register(); // 0x507C40.
	void CopyBoneMatrices(Gfx_AnimationInstance* instance); // 0x507A10.
	void ProcessShadowVolumeRenderSilhouette(std::list<Gfx_RenderSubmission16>* submissions,
		BYTE projectedShadows, Gfx_Camera* camera); // 0x507C90.
	void Render(UINT pass, std::list<Gfx_RenderSubmission16>* submissions,
		BYTE projectedShadows, Gfx_HeightFog* fog, BYTE flag48); // 0x509740.
	struct BoneInfluences
	{
		DWORD bone, count;
		DWORD* vertices;
		float* weights;
	};
	struct ExtraNames
	{
		DWORD count;
		std::string* names;
	};
	DWORD value24, handle; //24/28.
	ID3DXSkinMesh* skin; //2C.
	ID3DXMesh* blended; //30.
	DWORD* adjacency; //34, owned raw array.
	BoneInfluences* boneInfluences; //38, owns records and their two raw arrays.
	DWORD boneCount; //3C.
	DWORD* semanticBones; //40.
	DWORD combinationCount; //44.
	ID3DXBuffer* combinationBuffer; //48.
	D3DXBONECOMBINATION* combinations; //4C, borrowed from combinationBuffer.
	DWORD* blendCounts; //50.
	DWORD maximumBlendMatrices; //54.
	DWORD materialCount; //58.
	Gfx_Material* materials; //5C.
	DWORD* faceMaterials; //60.
	std::string* materialNames; //64.
	std::list<DWORD>* groups; //68, owns six real list objects.
	ExtraNames* materialNamesExtra; //6C, owns raw records and their string arrays.
	std::list<DWORD> usedGroups; //70, the only embedded list.
	BYTE optimize, flag7D, softwareBlends;
	ID3DXMesh* shadowMesh; //80.
	DWORD shadowVertexStride; //84.
	DWORD shadowFaceCount; //88.
	Gfx_SkinFace* faces; //8C, raw52B records.
	DWORD edgeCount; //90.
	Gfx_SkinEdge* edges; //94, raw12B records.
	DWORD shadowAttributeCount; //98, actual SDK-written count.
	D3DXATTRIBUTERANGE* shadowAttributes; //9C.
	D3DXMATRIX* boneMatrices; //A0, real CRT aligned allocation,16-byte SDK boundary.
	//507C90's eight ranked light contributions. The constructor leaves these
	//scratch arrays unwritten; this is not a complete original-layout claim.
	float shadowLightWeights[8];
	D3DXVECTOR3 shadowLightDirections[8];

private:
	HRESULT BuildShadowGeometry(); //50A600 tail; returns the latest buffer-lock result.
	Gfx_SkinMesh(const Gfx_SkinMesh&);
	Gfx_SkinMesh& operator=(const Gfx_SkinMesh&);
};

#if defined(_MSC_VER) && _MSC_VER == 1300 && defined(_M_IX86)
typedef char GfxSkinBoneRecordSize[(sizeof(Gfx_SkinMesh::BoneInfluences) == 0x10) ? 1 : -1];
typedef char GfxSkinBoneVerticesOffset[(offsetof(Gfx_SkinMesh::BoneInfluences, vertices) == 8) ? 1 : -1];
typedef char GfxSkinBoneWeightsOffset[(offsetof(Gfx_SkinMesh::BoneInfluences, weights) == 12) ? 1 : -1];
typedef char GfxSkinExtraNamesSize[(sizeof(Gfx_SkinMesh::ExtraNames) == 8) ? 1 : -1];
typedef char GfxSkinExtraNamesPointerOffset[(offsetof(Gfx_SkinMesh::ExtraNames, names) == 4) ? 1 : -1];
typedef char GfxSkinFaceSize[(sizeof(Gfx_SkinFace) == 52) ? 1 : -1];
typedef char GfxSkinEdgeSize[(sizeof(Gfx_SkinEdge) == 12) ? 1 : -1];
#define GFX_SKIN_OWNER_OFFSET(member, offset) \
	typedef char GfxSkinOwnerOffset_##member[(offsetof(Gfx_SkinMesh, member) == offset) ? 1 : -1]
GFX_SKIN_OWNER_OFFSET(value24, 0x24);
GFX_SKIN_OWNER_OFFSET(handle, 0x28);
GFX_SKIN_OWNER_OFFSET(skin, 0x2C);
GFX_SKIN_OWNER_OFFSET(blended, 0x30);
GFX_SKIN_OWNER_OFFSET(adjacency, 0x34);
GFX_SKIN_OWNER_OFFSET(boneInfluences, 0x38);
GFX_SKIN_OWNER_OFFSET(boneCount, 0x3C);
GFX_SKIN_OWNER_OFFSET(semanticBones, 0x40);
GFX_SKIN_OWNER_OFFSET(combinationCount, 0x44);
GFX_SKIN_OWNER_OFFSET(combinationBuffer, 0x48);
GFX_SKIN_OWNER_OFFSET(combinations, 0x4C);
GFX_SKIN_OWNER_OFFSET(blendCounts, 0x50);
GFX_SKIN_OWNER_OFFSET(maximumBlendMatrices, 0x54);
GFX_SKIN_OWNER_OFFSET(materialCount, 0x58);
GFX_SKIN_OWNER_OFFSET(materials, 0x5C);
GFX_SKIN_OWNER_OFFSET(faceMaterials, 0x60);
GFX_SKIN_OWNER_OFFSET(materialNames, 0x64);
GFX_SKIN_OWNER_OFFSET(groups, 0x68);
GFX_SKIN_OWNER_OFFSET(materialNamesExtra, 0x6C);
GFX_SKIN_OWNER_OFFSET(usedGroups, 0x70);
GFX_SKIN_OWNER_OFFSET(optimize, 0x7C);
GFX_SKIN_OWNER_OFFSET(flag7D, 0x7D);
GFX_SKIN_OWNER_OFFSET(softwareBlends, 0x7E);
GFX_SKIN_OWNER_OFFSET(shadowMesh, 0x80);
GFX_SKIN_OWNER_OFFSET(shadowVertexStride, 0x84);
GFX_SKIN_OWNER_OFFSET(shadowFaceCount, 0x88);
GFX_SKIN_OWNER_OFFSET(faces, 0x8C);
GFX_SKIN_OWNER_OFFSET(edgeCount, 0x90);
GFX_SKIN_OWNER_OFFSET(edges, 0x94);
GFX_SKIN_OWNER_OFFSET(shadowAttributeCount, 0x98);
GFX_SKIN_OWNER_OFFSET(shadowAttributes, 0x9C);
GFX_SKIN_OWNER_OFFSET(boneMatrices, 0xA0);
GFX_SKIN_OWNER_OFFSET(shadowLightWeights, 0xA4);
GFX_SKIN_OWNER_OFFSET(shadowLightDirections, 0xC4);
#undef GFX_SKIN_OWNER_OFFSET
#endif

