#include "SkinRender.hpp"
#include "G3DMeshExport.hpp"
#include "ModelData.hpp"
#include "AnimationObject.hpp"
#include "D3DState.hpp"
#include "SystemInterface.hpp"
#include "../diagnostics/DebugOutput.hpp"
#include <malloc.h>
#include <map>
#include <math.h>
#include <stdio.h>
#include <string.h>

Gfx_SkinMesh::Gfx_SkinMesh()
	: value24(1), handle(0), skin(NULL), blended(NULL), adjacency(NULL),
	boneInfluences(NULL), boneCount(0), semanticBones(NULL), combinationCount(0),
	combinationBuffer(NULL), combinations(NULL), blendCounts(NULL), maximumBlendMatrices(0),
	materialCount(0), materials(NULL), faceMaterials(NULL), materialNames(NULL),
	materialNamesExtra(NULL), softwareBlends(0), shadowMesh(NULL), shadowVertexStride(0),
	shadowFaceCount(0), faces(NULL), edgeCount(0), edges(NULL), shadowAttributeCount(0),
	shadowAttributes(NULL), boneMatrices(NULL)
{
	flag7D = 1;
	optimize = 0;
	groups = new std::list<DWORD>[6]; //5089EE; no earlier pointer store or owning wrapper.
}

Gfx_SkinMesh::~Gfx_SkinMesh()
{
	g_SkinMeshManager->Remove(handle);
	if (shadowMesh) { shadowMesh->Release(); shadowMesh = NULL; }
	if (shadowAttributes) { delete[] shadowAttributes; shadowAttributes = NULL; }
	if (boneMatrices)
	{
		BYTE* aligned = reinterpret_cast<BYTE*>(boneMatrices);
		::operator delete(aligned - aligned[-1]);
		boneMatrices = NULL;
	}
	if (faces) { delete[] faces; faces = NULL; }
	if (edges) { delete[] edges; edges = NULL; }
	if (groups) { delete[] groups; groups = NULL; }
	if (semanticBones) { delete[] semanticBones; semanticBones = NULL; }
	if (materialNames) { delete[] materialNames; materialNames = NULL; }
	if (faceMaterials) { delete[] faceMaterials; faceMaterials = NULL; }
	for (DWORD index = 0; index < materialCount; ++index)
	{
		for (DWORD texture = 0; texture < 4; ++texture)
		{
			const DWORD handle = materials[index].textureHandles[texture];
			if (handle) g_GfxSystem->DeleteTexture(handle);
		}
		if (materialNamesExtra && materialNamesExtra[index].names)
		{
			delete[] materialNamesExtra[index].names;
			materialNamesExtra[index].names = NULL;
		}
	}
	if (materialNamesExtra) { delete[] materialNamesExtra; materialNamesExtra = NULL; }
	if (materials) { delete[] materials; materials = NULL; }
	if (blendCounts) { delete[] blendCounts; blendCounts = NULL; }
	for (DWORD bone = 0; bone < boneCount; ++bone)
	{
		if (boneInfluences[bone].vertices)
		{
			delete[] boneInfluences[bone].vertices;
			boneInfluences[bone].vertices = NULL;
		}
		if (boneInfluences[bone].weights)
		{
			delete[] boneInfluences[bone].weights;
			boneInfluences[bone].weights = NULL;
		}
	}
	if (boneInfluences) { delete[] boneInfluences; boneInfluences = NULL; }
	if (adjacency) { delete[] adjacency; adjacency = NULL; }
	if (combinationBuffer) { combinationBuffer->Release(); combinationBuffer = NULL; }
	if (blended) { blended->Release(); blended = NULL; }
	if (skin) { skin->Release(); skin = NULL; }
	//The real usedGroups member dies next, then the NodeAttribute base. Counts
	//and the borrowed combinations pointer retain the original stale values.
}

void Gfx_SkinMesh::SetMaterialCount(DWORD count)
{
	materialCount = count;
	materials = new Gfx_Material[count]; //508830 intentionally does not free old storage.
}

void Gfx_SkinMesh::ResetMaterialExtraNames()
{
	for (DWORD material = 0; material < materialCount; ++material)
	{
		if (materialNamesExtra && materialNamesExtra[material].names)
		{
			delete[] materialNamesExtra[material].names;
			materialNamesExtra[material].names = NULL;
		}
	}
	if (materialNamesExtra) { delete[] materialNamesExtra; materialNamesExtra = NULL; }
	materialNamesExtra = new ExtraNames[materialCount];
	for (DWORD material = 0; material < materialCount; ++material)
	{
		materialNamesExtra[material].count = 0;
		materialNamesExtra[material].names = NULL;
	}
}

void Gfx_SkinMesh::SetMaterialExtraNameCount(DWORD index, DWORD count)
{
	if (materialNamesExtra[index].names)
	{
		delete[] materialNamesExtra[index].names;
		materialNamesExtra[index].names = NULL;
	}
	materialNamesExtra[index].count = count;
	materialNamesExtra[index].names = new std::string[count];
}

std::string Gfx_SkinMesh::GetMaterialExtraName(DWORD index, DWORD name) const
{
	return materialNamesExtra[index].names[name];
}

void Gfx_SkinMesh::SetMaterial(DWORD index, DWORD style, const std::string& materialName,
	DWORD blend, const D3DMATERIAL8& values, DWORD textures, const std::string* names,
	const DWORD* handles, BYTE flagB9, BYTE flagBA)
{
	if (index >= materialCount) return;
	materials[index].name = materialName;
	{
		Gfx_Material& material = materials[index];
		material.style1 = style;
		material.style2 = style;
	}
	materials[index].SetValues(values);
	materials[index].flagB9 = flagB9;
	materials[index].flagBA = flagBA;
	{
		Gfx_Material& material = materials[index];
		material.mode24 = material.mode28 = blend;
	}
	if (textures > 4) textures = 4;
	for (DWORD texture = 0; texture < textures; ++texture)
	{
		if (texture < 4)
		{
			materials[index].textureNames[texture] = names[texture];
			materials[index].textureHandles[texture] = handles[texture];
		}
	}
}

void Gfx_SkinMesh::RebuildGroups(const Gfx_Material* values, std::list<DWORD>* output)
{
	for (DWORD pass = 0; pass < 6; ++pass) output[pass].clear();
	for (DWORD index = 0; index < combinationCount; ++index)
		output[values[combinations[index].AttribId].mode28].push_back(index);
}

DWORD Gfx_SkinMesh::GetPrimaryTexture(DWORD index)
{
	DWORD result = 0;
	if (index < materialCount) result = materials[index].textureHandles[0];
	return result;
}

void Gfx_SkinMesh::Register()
{
	handle = g_SkinMeshManager->Register(this);
}

HRESULT Gfx_SkinMesh::CreateSkinMeshFVF(DWORD fvf, DWORD vertexCount,
	const void* vertexData, DWORD groupCount, const std::string* names,
	const DWORD* indexCounts, const DWORD* primitiveCounts, const void* const* indices,
	DWORD inputBoneCount, Gfx_ModelData526620* model,
	const Gfx_G3D_BoneWeight* const* weights, DWORD /*unused*/)
{
	materialNames = new std::string[groupCount];
	maximumBlendMatrices = g_GfxSystem->MaximumVertexBlendMatrices();
	DWORD faceCount = 0;
	for (DWORD group = 0; group < groupCount; ++group) faceCount += primitiveCounts[group];
	faceMaterials = new DWORD[faceCount];
	DWORD options = D3DXMESH_MANAGED | (g_D3D.currentDevice->guardBand ? D3DXMESH_DONOTCLIP : 0);
	ID3DXMesh* source = NULL;
	HRESULT result = D3DXCreateMeshFVF(faceCount, vertexCount, options, fvf, g_D3D.device, &source);
	if (FAILED(result)) return result;
	BYTE* data = NULL;
	if (SUCCEEDED(source->LockVertexBuffer(0, &data)))
	{
		memcpy(data, vertexData, vertexCount * D3DXGetFVFVertexSize(fvf));
		source->UnlockVertexBuffer();
	}
	if (SUCCEEDED(source->LockIndexBuffer(0, &data)))
	{
		DWORD cursor = 0;
		DWORD stride = g_GfxSystem->ModeIndex();
		for (DWORD group = 0; group < groupCount; ++group)
		{
			DWORD bytes = stride * indexCounts[group];
			memcpy(data + cursor, indices[group], bytes);
			cursor += bytes;
		}
		source->UnlockIndexBuffer();
	}
	DWORD* attributes = NULL;
	if (SUCCEEDED(source->LockAttributeBuffer(0, &attributes)))
	{
		DWORD cursor = 0;
		for (DWORD group = 0; group < groupCount; ++group)
		{
			for (DWORD face = 0; face < primitiveCounts[group]; ++face)
			{
				attributes[cursor] = group;
				faceMaterials[cursor++] = group;
			}
			if (names) materialNames[group] = names[group];
		}
		source->UnlockAttributeBuffer();
	}
	DWORD* inputAdjacency = new DWORD[3 * faceCount];
	source->GenerateAdjacency(0.0f, inputAdjacency);
	typedef std::list<std::pair<DWORD, float> > VertexWeights;
	std::map<int, VertexWeights> influenceMap;
	const bool optimizeMesh = optimize != 0;
	ID3DXBuffer* vertexRemap;
	const DWORD* remappedVertices = NULL;
	DWORD influenceVertexCount = vertexCount;
	bool optimized = false;
	if (optimizeMesh)
	{
		adjacency = new DWORD[3 * faceCount];
		optimized = SUCCEEDED(source->OptimizeInplace(D3DXMESHOPT_COMPACT |
			D3DXMESHOPT_ATTRSORT | D3DXMESHOPT_VERTEXCACHE,
			inputAdjacency, adjacency, NULL, &vertexRemap));
		influenceVertexCount = 0;
		if (optimized)
		{
			influenceVertexCount = vertexRemap->GetBufferSize() >> 2;
			remappedVertices = static_cast<const DWORD*>(vertexRemap->GetBufferPointer());
		}
	}
	else
	{
		adjacency = inputAdjacency;
	}
	for (DWORD vertex = 0; vertex < influenceVertexCount; ++vertex)
	{
		const DWORD oldVertex = optimizeMesh ? remappedVertices[vertex] : vertex;
		float sum = 0.0f;
		for (DWORD channel = 0; channel < 2; ++channel)
		{
			const Gfx_G3D_BoneWeight& weight = weights[channel][oldVertex];
			if (weight.bone == -1) continue;
			influenceMap[weight.bone].push_back(std::make_pair(vertex, weight.weight));
			sum += weight.weight;
		}
		if (fabs(1.0 - sum) > 0.001)
		{
			char warning[260];
			sprintf(warning, "Gfx_SkinMesh::CreateSkinMeshFVF - [WARNING] Vertex #%d's weights do NOT add up to 1.0 (%f)!\n", oldVertex, sum);
			MessageBoxA(NULL, warning, "WARNING - Incorrect vertex weight", MB_ICONWARNING);
			JE::LogDebugString("%s", warning);
		}
	}
	if (optimizeMesh)
	{
		if (optimized && vertexRemap) vertexRemap->Release();
		delete[] inputAdjacency;
	}
	boneCount = inputBoneCount;
	boneInfluences = new BoneInfluences[inputBoneCount];
	semanticBones = new DWORD[inputBoneCount];
	BYTE* allocation = static_cast<BYTE*>(::operator new((inputBoneCount << 6) + 16));
	BYTE* aligned = NULL;
	if (allocation)
	{
		const BYTE offset = static_cast<BYTE>(16 - (reinterpret_cast<DWORD>(allocation) & 15));
		aligned = allocation + offset;
		aligned[-1] = offset;
	}
	boneMatrices = reinterpret_cast<D3DXMATRIX*>(aligned);
	for (DWORD bone = 0; bone < inputBoneCount; ++bone)
	{
		semanticBones[bone] = 212;
		std::map<int, VertexWeights>::const_iterator found = influenceMap.find(bone);
		if (found != influenceMap.end())
		{
			const DWORD count = static_cast<DWORD>(found->second.size());
			boneInfluences[bone].bone = found->first;
			boneInfluences[bone].count = count;
			boneInfluences[bone].vertices = new DWORD[count];
			boneInfluences[bone].weights = new float[count];
			const std::string boneName(model->bones[boneInfluences[bone].bone].name.c_str());
			semanticBones[bone] = Gfx_FindSemanticBone(boneName.c_str());
			if (semanticBones[bone] == 212)
			{
				JE::FatalError("Gfx_SkinMesh::CreateSkinMeshFVF - CANNOT FIND BONE: %s\n",
					boneName.c_str());
			}
			DWORD* vertices = boneInfluences[bone].vertices;
			float* values = boneInfluences[bone].weights;
			DWORD index = 0;
			for (VertexWeights::const_iterator weight = found->second.begin();
				weight != found->second.end(); ++weight, ++index)
			{
				vertices[index] = weight->first;
				values[index] = weight->second;
			}
		}
		else
		{
			boneInfluences[bone].bone = bone;
			boneInfluences[bone].count = 0;
			boneInfluences[bone].vertices = NULL;
			boneInfluences[bone].weights = NULL;
		}
	}
	D3DXCreateSkinMeshFromMesh(source, boneCount, &skin);
	source->Release();
	for (DWORD bone = 0; bone < boneCount; ++bone)
	{
		BoneInfluences& influence = boneInfluences[bone];
		skin->SetBoneInfluence(influence.bone, influence.count,
			influence.vertices, influence.weights);
	}
	DWORD conversionOptions = options | D3DXMESHOPT_COMPACT | D3DXMESHOPT_ATTRSORT | D3DXMESHOPT_VERTEXCACHE;
	if (maximumBlendMatrices < 4)
	{
		conversionOptions |= D3DXMESH_SOFTWAREPROCESSING;
		softwareBlends = 1;
	}
	if (SUCCEEDED(skin->ConvertToBlendedMesh(conversionOptions,
		adjacency, NULL, &combinationCount,
		&combinationBuffer, NULL, NULL, &blended)))
	{
		blended->GetFVF();
		combinationBuffer->GetBufferSize();
		combinations = static_cast<D3DXBONECOMBINATION*>(combinationBuffer->GetBufferPointer());
		blendCounts = new DWORD[combinationCount];
		for (DWORD group = 0; group < combinationCount; ++group)
		{
			DWORD count = 0;
			for (DWORD bone = 0; bone < 4; ++bone)
				if (combinations[group].BoneId[bone] != DWORD(-1)) ++count;
			if (count) --count;
			blendCounts[group] = count;
		}
	}
	result = skin->GenerateSkinnedMesh(D3DXMESH_SYSTEMMEM |
		(g_D3D.currentDevice->guardBand ? D3DXMESH_DONOTCLIP : 0), 0.0f,
		adjacency, NULL, NULL, NULL, &shadowMesh);
	if (SUCCEEDED(result)) result = BuildShadowGeometry();
	return result;
}

HRESULT Gfx_SkinMesh::BuildShadowGeometry()
{
	shadowMesh->GetAttributeTable(NULL, &shadowAttributeCount);
	shadowAttributes = new D3DXATTRIBUTERANGE[shadowAttributeCount];
	shadowMesh->GetAttributeTable(shadowAttributes, &shadowAttributeCount);
	shadowFaceCount = shadowMesh->GetNumFaces();
	faces = new Gfx_SkinFace[shadowFaceCount];
	BYTE* vertices = NULL;
	HRESULT result = shadowMesh->LockVertexBuffer(D3DLOCK_READONLY, &vertices);
	if (FAILED(result)) return result;
	shadowVertexStride = D3DXGetFVFVertexSize(shadowMesh->GetFVF());
	BYTE* indices = NULL;
	result = shadowMesh->LockIndexBuffer(D3DLOCK_READONLY, &indices);
	if (SUCCEEDED(result))
	{
		if (g_GfxSystem->ModeIndex() == 2)
		{
			const WORD* input = reinterpret_cast<const WORD*>(indices);
			for (DWORD index = 0; index < shadowFaceCount; ++index)
			{
				Gfx_SkinFace& face = faces[index];
				const Gfx_Material& material = materials[faceMaterials[index]];
				face.flagB9 = material.flagB9;
				face.flagBA = material.flagBA;
				for (DWORD edge = 0; edge < 3; ++edge)
				{
					face.edges[edge].vertices[0] = input[3 * index + edge];
					face.edges[edge].vertices[1] = input[3 * index + (edge + 1) % 3];
					face.edges[edge].adjacentFace = -1;
					face.edges[edge].ownerFace = DWORD(-1);
				}
				const D3DXVECTOR3& a = *reinterpret_cast<const D3DXVECTOR3*>(vertices + shadowVertexStride * input[3 * index]);
				const D3DXVECTOR3& b = *reinterpret_cast<const D3DXVECTOR3*>(vertices + shadowVertexStride * input[3 * index + 1]);
				const D3DXVECTOR3& c = *reinterpret_cast<const D3DXVECTOR3*>(vertices + shadowVertexStride * input[3 * index + 2]);
				D3DXVECTOR3 ab = b - a, ac = c - a;
				D3DXVec3Cross(&face.normal, &ab, &ac);
				D3DXVec3Normalize(&face.normal, &face.normal);
			}
		}
		shadowMesh->UnlockIndexBuffer();
	}
	shadowMesh->UnlockVertexBuffer();
	//50B1E2 continues even when index-lock failure or index width left faces unwritten.
	for (DWORD index = 0; index < shadowFaceCount; ++index)
	{
		Gfx_SkinFace& face = faces[index];
		for (DWORD edge = 0; edge < 3; ++edge)
		{
			Gfx_SkinFace::Edge& current = face.edges[edge];
			if (current.adjacentFace != -1) continue;
			current.ownerFace = index;
			if (!face.flagBA) continue;
			bool found = false;
			for (DWORD other = 0; other < shadowFaceCount && !found; ++other)
			{
				if (other == index) continue;
				for (DWORD candidate = 0; candidate < 3; ++candidate)
				{
					Gfx_SkinFace::Edge& test = faces[other].edges[candidate];
					if ((current.vertices[0] == test.vertices[0] && current.vertices[1] == test.vertices[1])
						|| (current.vertices[0] == test.vertices[1] && current.vertices[1] == test.vertices[0]))
					{
						current.adjacentFace = other;
						test.adjacentFace = index;
						test.ownerFace = other;
						found = true;
						break;
					}
				}
			}
		}
	}
	Gfx_SkinEdge* scratch = new Gfx_SkinEdge[3 * shadowFaceCount];
	edgeCount = 0;
	for (DWORD index = 0; index < shadowFaceCount; ++index)
	{
		for (DWORD edge = 0; edge < 3; ++edge)
		{
			const Gfx_SkinFace::Edge& input = faces[index].edges[edge];
			bool found = false;
			if (input.adjacentFace != -1)
			{
				for (DWORD prior = 0; prior < edgeCount; ++prior)
				{
					const Gfx_SkinEdge& test = scratch[prior];
					if ((input.vertices[0] == test.vertices[0] && input.vertices[1] == test.vertices[1])
						|| (input.vertices[0] == test.vertices[1] && input.vertices[1] == test.vertices[0]))
					{ found = true; break; }
				}
			}
			if (!found)
			{
				memcpy(scratch + edgeCount, &input, sizeof(Gfx_SkinEdge));
				++edgeCount;
			}
		}
	}
	edges = new Gfx_SkinEdge[edgeCount];
	for (DWORD edge = 0; edge < edgeCount; ++edge) edges[edge] = scratch[edge];
	if (scratch) delete[] scratch;
	return result; //50B4A9 retains the index-lock status; unlock calls do not replace it.
}
