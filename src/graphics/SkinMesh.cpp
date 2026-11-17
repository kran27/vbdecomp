#include "SkinRender.hpp"
#include "G3DMeshExport.hpp"
#include "ModelData.hpp"
#include "AnimationObject.hpp"
#include "D3DState.hpp"
#include "SystemInterface.hpp"
#include "../diagnostics/DebugOutput.hpp"
#include <malloc.h>
#include <cstdint>
#include <cstring>
#include <limits>
#include <vector>
#include <map>
#include <math.h>
#include <stdio.h>
#include <string.h>

Gfx_SkinMesh::Gfx_SkinMesh()
    : value24(1), handle(0), skin(NULL), blended(NULL), adjacency(NULL), boneInfluences(NULL), boneCount(0),
      semanticBones(NULL), combinationCount(0), combinationBuffer(NULL), combinations(NULL), blendCounts(NULL),
      maximumBlendMatrices(0), materialCount(0), materials(NULL), faceMaterials(NULL), materialNames(NULL),
      materialNamesExtra(NULL), softwareBlends(0), shadowMesh(NULL), shadowVertexStride(0), shadowFaceCount(0),
      faces(NULL), edgeCount(0), edges(NULL), shadowAttributeCount(0), shadowAttributes(NULL), boneMatrices(NULL),
      sourceMesh(NULL)
{
    flag7D = 1;
    optimize = 0;
    groups = new std::list<DWORD>[6]; // 5089EE; no earlier pointer store or owning wrapper.
}

Gfx_SkinMesh::~Gfx_SkinMesh()
{
    g_SkinMeshManager->Remove(handle);
    if (shadowMesh)
    {
        shadowMesh->Release();
        shadowMesh = NULL;
    }
    if (shadowAttributes)
    {
        delete[] shadowAttributes;
        shadowAttributes = NULL;
    }
    if (boneMatrices)
    {
        BYTE* aligned = reinterpret_cast<BYTE*>(boneMatrices);
        ::operator delete(aligned - aligned[-1]);
        boneMatrices = NULL;
    }
    if (faces)
    {
        delete[] faces;
        faces = NULL;
    }
    if (edges)
    {
        delete[] edges;
        edges = NULL;
    }
    if (groups)
    {
        delete[] groups;
        groups = NULL;
    }
    if (semanticBones)
    {
        delete[] semanticBones;
        semanticBones = NULL;
    }
    if (materialNames)
    {
        delete[] materialNames;
        materialNames = NULL;
    }
    if (faceMaterials)
    {
        delete[] faceMaterials;
        faceMaterials = NULL;
    }
    for (DWORD index = 0; index < materialCount; ++index)
    {
        for (DWORD texture = 0; texture < 4; ++texture)
        {
            const DWORD handle = materials[index].textureHandles[texture];
            if (handle)
            {
                g_GfxSystem->DeleteTexture(handle);
            }
        }
        if (materialNamesExtra && materialNamesExtra[index].names)
        {
            delete[] materialNamesExtra[index].names;
            materialNamesExtra[index].names = NULL;
        }
    }
    if (materialNamesExtra)
    {
        delete[] materialNamesExtra;
        materialNamesExtra = NULL;
    }
    if (materials)
    {
        delete[] materials;
        materials = NULL;
    }
    if (blendCounts)
    {
        delete[] blendCounts;
        blendCounts = NULL;
    }
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
    if (boneInfluences)
    {
        delete[] boneInfluences;
        boneInfluences = NULL;
    }
    if (adjacency)
    {
        delete[] adjacency;
        adjacency = NULL;
    }
    if (combinationBuffer)
    {
        combinationBuffer->Release();
        combinationBuffer = NULL;
    }
    if (blended)
    {
        blended->Release();
        blended = NULL;
    }
    if (sourceMesh)
    {
        sourceMesh->Release();
        sourceMesh = NULL;
    }
    if (skin)
    {
        skin->Release();
        skin = NULL;
    }
    // The real usedGroups member dies next, then the NodeAttribute base. Counts
    // and the borrowed combinations pointer retain the original stale values.
}

void Gfx_SkinMesh::SetMaterialCount(DWORD count)
{
    materialCount = count;
    materials = new Gfx_Material[count]; // 508830 intentionally does not free old storage.
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
    if (materialNamesExtra)
    {
        delete[] materialNamesExtra;
        materialNamesExtra = NULL;
    }
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

void Gfx_SkinMesh::SetMaterial(DWORD index, DWORD style, const std::string& materialName, DWORD blend,
                               const D3DMATERIAL9& values, DWORD textures, const std::string* names,
                               const DWORD* handles, BYTE flagB9, BYTE flagBA)
{
    if (index >= materialCount)
    {
        return;
    }
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
    if (textures > 4)
    {
        textures = 4;
    }
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
    for (DWORD pass = 0; pass < 6; ++pass)
    {
        output[pass].clear();
    }
    for (DWORD index = 0; index < combinationCount; ++index)
    {
        output[values[combinations[index].AttribId].mode28].push_back(index);
    }
}

DWORD Gfx_SkinMesh::GetPrimaryTexture(DWORD index)
{
    DWORD result = 0;
    if (index < materialCount)
    {
        result = materials[index].textureHandles[0];
    }
    return result;
}

void Gfx_SkinMesh::Register()
{
    handle = g_SkinMeshManager->Register(this);
}

HRESULT Gfx_SkinMesh::CreateSkinMeshFVF(DWORD fvf, DWORD vertexCount, const void* vertexData, DWORD groupCount,
                                        const std::string* names, const DWORD* indexCounts,
                                        const DWORD* primitiveCounts, const void* const* indices, DWORD inputBoneCount,
                                        Gfx_ModelData526620* model, const Gfx_G3D_BoneWeight* const* weights,
                                        DWORD /*unused*/)
{
    if (!vertexData || !indexCounts || !primitiveCounts || !indices || !model || !weights || !weights[0] ||
        !weights[1] || vertexCount == 0 || inputBoneCount == 0 || groupCount == 0)
    {
        return E_INVALIDARG;
    }

    const DWORD indexStride = g_GfxSystem->ModeIndex();
    if (indexStride != sizeof(WORD) && indexStride != sizeof(DWORD))
    {
        return E_INVALIDARG;
    }

    DWORD faceCount = 0;
    for (DWORD group = 0; group < groupCount; ++group)
    {
        if (primitiveCounts[group] > ((std::numeric_limits<DWORD>::max)() / 3) ||
            indexCounts[group] != 3 * primitiveCounts[group] ||
            faceCount > (std::numeric_limits<DWORD>::max)() - primitiveCounts[group] ||
            (indexCounts[group] != 0 && !indices[group]))
        {
            return E_INVALIDARG;
        }
        faceCount += primitiveCounts[group];
    }
    if (faceCount == 0 || (indexStride == sizeof(WORD) && vertexCount > 65536))
    {
        return E_INVALIDARG;
    }

    materialNames = new std::string[groupCount];
    faceMaterials = new DWORD[faceCount];
    maximumBlendMatrices = g_GfxSystem->MaximumVertexBlendMatrices();

    DWORD options = D3DXMESH_MANAGED;
    if (g_D3D.currentDevice->guardBand)
    {
        options |= D3DXMESH_DONOTCLIP;
    }
    if (indexStride == sizeof(DWORD))
    {
        options |= D3DXMESH_32BIT;
    }

    // SkinInfo does not own geometry in D3DX9. Keep this mesh for the lifetime
    // of the skin: its vertex order also defines all skin influence indices.
    HRESULT result = D3DXCreateMeshFVF(faceCount, vertexCount, options, fvf, g_D3D.device, &sourceMesh);
    if (FAILED(result))
    {
        return result;
    }

    void* lockedData = nullptr;
    result = sourceMesh->LockVertexBuffer(0, &lockedData);
    if (FAILED(result))
    {
        return result;
    }
    std::memcpy(lockedData, vertexData, size_t(vertexCount) * D3DXGetFVFVertexSize(fvf));
    result = sourceMesh->UnlockVertexBuffer();
    if (FAILED(result))
    {
        return result;
    }

    result = sourceMesh->LockIndexBuffer(0, &lockedData);
    if (FAILED(result))
    {
        return result;
    }
    BYTE* destination = static_cast<BYTE*>(lockedData);
    size_t byteOffset = 0;
    for (DWORD group = 0; group < groupCount; ++group)
    {
        const size_t byteCount = size_t(indexStride) * indexCounts[group];
        if (byteCount != 0)
        {
            std::memcpy(destination + byteOffset, indices[group], byteCount);
        }
        byteOffset += byteCount;
    }
    result = sourceMesh->UnlockIndexBuffer();
    if (FAILED(result))
    {
        return result;
    }

    DWORD* attributes = nullptr;
    result = sourceMesh->LockAttributeBuffer(0, &attributes);
    if (FAILED(result))
    {
        return result;
    }
    DWORD face = 0;
    for (DWORD group = 0; group < groupCount; ++group)
    {
        for (DWORD primitive = 0; primitive < primitiveCounts[group]; ++primitive)
        {
            attributes[face] = group;
            faceMaterials[face] = group;
            ++face;
        }
        if (names)
        {
            materialNames[group] = names[group];
        }
    }
    result = sourceMesh->UnlockAttributeBuffer();
    if (FAILED(result))
    {
        return result;
    }

    DWORD* originalAdjacency = new DWORD[3 * faceCount];
    result = sourceMesh->GenerateAdjacency(0.0f, originalAdjacency);
    if (FAILED(result))
    {
        delete[] originalAdjacency;
        return result;
    }

    ID3DXBuffer* vertexRemap = nullptr;
    if (optimize)
    {
        adjacency = new DWORD[3 * faceCount];
        result = sourceMesh->OptimizeInplace(D3DXMESHOPT_COMPACT | D3DXMESHOPT_ATTRSORT | D3DXMESHOPT_VERTEXCACHE,
                                             originalAdjacency, adjacency, nullptr, &vertexRemap);
        delete[] originalAdjacency;
        if (FAILED(result))
        {
            if (vertexRemap)
            {
                vertexRemap->Release();
            }
            return result;
        }
    }
    else
    {
        adjacency = originalAdjacency;
    }

    // OptimizeInplace can reorder (and sometimes split) vertices. The remap
    // maps new vertex indices to the original mesh's vertex indices.
    const DWORD remappedCount = sourceMesh->GetNumVertices();
    const DWORD* remap = nullptr;
    if (optimize)
    {
        if (!vertexRemap || vertexRemap->GetBufferSize() < size_t(remappedCount) * sizeof(DWORD))
        {
            if (vertexRemap)
            {
                vertexRemap->Release();
            }
            return E_FAIL;
        }
        remap = static_cast<const DWORD*>(vertexRemap->GetBufferPointer());
    }

    typedef std::list<std::pair<DWORD, float>> VertexWeights;
    std::map<int, VertexWeights> influenceMap;
    for (DWORD vertex = 0; vertex < remappedCount; ++vertex)
    {
        const DWORD original = remap ? remap[vertex] : vertex;
        if (original >= vertexCount)
        {
            if (vertexRemap)
            {
                vertexRemap->Release();
            }
            return E_FAIL;
        }

        float sum = 0.0f;
        for (DWORD channel = 0; channel < 2; ++channel)
        {
            const Gfx_G3D_BoneWeight& weight = weights[channel][original];
            if (weight.bone == -1)
            {
                continue;
            }
            if (weight.bone < 0 || static_cast<DWORD>(weight.bone) >= inputBoneCount)
            {
                if (vertexRemap)
                {
                    vertexRemap->Release();
                }
                return E_INVALIDARG;
            }
            influenceMap[weight.bone].push_back(std::make_pair(vertex, weight.weight));
            sum += weight.weight;
        }
        if (fabs(1.0f - sum) > 0.001f)
        {
            JE::LogDebugString("Gfx_SkinMesh::CreateSkinMeshFVF - vertex %u weights sum to %f\n", original, sum);
        }
    }
    if (vertexRemap)
    {
        vertexRemap->Release();
    }

    boneCount = inputBoneCount;
    boneInfluences = new BoneInfluences[boneCount]();
    semanticBones = new DWORD[boneCount];
    BYTE* allocation = static_cast<BYTE*>(::operator new(size_t(boneCount) * sizeof(D3DXMATRIX) + 16));
    const BYTE offset = static_cast<BYTE>(16 - (reinterpret_cast<std::uintptr_t>(allocation) & 15));
    BYTE* aligned = allocation + offset;
    aligned[-1] = offset;
    boneMatrices = reinterpret_cast<D3DXMATRIX*>(aligned);

    result = D3DXCreateSkinInfoFVF(remappedCount, fvf, boneCount, &skin);
    if (FAILED(result))
    {
        return result;
    }

    D3DXMATRIX identity;
    D3DXMatrixIdentity(&identity);
    for (DWORD bone = 0; bone < boneCount; ++bone)
    {
        D3DXMatrixIdentity(boneMatrices + bone);
        BoneInfluences& influence = boneInfluences[bone];
        influence.bone = bone;
        semanticBones[bone] = 212;

        const std::map<int, VertexWeights>::const_iterator found = influenceMap.find(bone);
        if (found != influenceMap.end())
        {
            influence.count = static_cast<DWORD>(found->second.size());
            influence.vertices = new DWORD[influence.count];
            influence.weights = new float[influence.count];
            const std::string boneName(model->bones[bone].name.c_str());
            semanticBones[bone] = Gfx_FindSemanticBone(boneName.c_str());
            if (semanticBones[bone] == 212)
            {
                JE::FatalError("Gfx_SkinMesh::CreateSkinMeshFVF - CANNOT FIND BONE: %s\n", boneName.c_str());
            }

            DWORD i = 0;
            for (const std::pair<DWORD, float>& weight : found->second)
            {
                influence.vertices[i] = weight.first;
                influence.weights[i] = weight.second;
                ++i;
            }
            result = skin->SetBoneInfluence(bone, influence.count, influence.vertices, influence.weights);
            if (FAILED(result))
            {
                return result;
            }
        }
        result = skin->SetBoneOffsetMatrix(bone, &identity);
        if (FAILED(result))
        {
            return result;
        }
    }

    DWORD conversionOptions = options | D3DXMESHOPT_COMPACT | D3DXMESHOPT_ATTRSORT | D3DXMESHOPT_VERTEXCACHE;
    if (maximumBlendMatrices < 4)
    {
        conversionOptions |= D3DXMESH_SOFTWAREPROCESSING;
        softwareBlends = 1;
    }

    DWORD maxFaceInfluences = 0;
    result = skin->ConvertToBlendedMesh(sourceMesh, conversionOptions, adjacency, nullptr, nullptr, nullptr,
                                        &maxFaceInfluences, &combinationCount, &combinationBuffer, &blended);
    if (FAILED(result))
    {
        return result;
    }
    if (!combinationBuffer || !blended)
    {
        return E_FAIL;
    }

    combinations = static_cast<D3DXBONECOMBINATION*>(combinationBuffer->GetBufferPointer());
    blendCounts = new DWORD[combinationCount];
    for (DWORD group = 0; group < combinationCount; ++group)
    {
        DWORD matrices = 0;
        for (DWORD bone = 0; bone < maxFaceInfluences; ++bone)
        {
            if (combinations[group].BoneId && combinations[group].BoneId[bone] != DWORD(-1))
            {
                ++matrices;
            }
        }
        if (matrices == 0 || matrices > 4)
        {
            JE::LogDebugString(
                "Gfx_SkinMesh::CreateSkinMeshFVF - unsupported fixed-function bone combination (%u matrices)\n",
                matrices);
            return E_FAIL;
        }
        blendCounts[group] = matrices - 1;
    }

    // This is the unblended, *optimized* mesh, not the blended render mesh.
    // CPU skinning of its vertices requires SkinInfo and sourceMesh to have
    // matching vertex indices. Cloning also retains face and material order.
    const DWORD shadowOptions = D3DXMESH_SYSTEMMEM | (sourceMesh->GetOptions() & D3DXMESH_32BIT);
    result = sourceMesh->CloneMeshFVF(shadowOptions, sourceMesh->GetFVF(), g_D3D.device, &shadowMesh);
    if (FAILED(result))
    {
        return result;
    }
    return BuildShadowGeometry();
}

HRESULT Gfx_SkinMesh::BuildShadowGeometry()
{
    if (!shadowMesh)
    {
        return E_POINTER;
    }

    shadowFaceCount = shadowMesh->GetNumFaces();
    const DWORD vertexCount = shadowMesh->GetNumVertices();
    shadowVertexStride = shadowMesh->GetNumBytesPerVertex();

    // The legacy silhouette owner uses WORD vertex indices. Preserve that ABI
    // without truncating indices from large DX9 meshes: rendering still works,
    // but this particular shadow-volume path cannot represent their vertices.
    if (vertexCount > 65536)
    {
        JE::LogDebugString("Gfx_SkinMesh::BuildShadowGeometry - skipping 32-bit shadow volume\n");
        shadowMesh->Release();
        shadowMesh = nullptr;
        shadowFaceCount = 0;
        return S_OK;
    }
    if ((shadowMesh->GetFVF() & D3DFVF_POSITION_MASK) != D3DFVF_XYZ || shadowVertexStride < sizeof(D3DXVECTOR3))
    {
        return D3DERR_INVALIDCALL;
    }

    void* vertexBuffer = nullptr;
    HRESULT result = shadowMesh->LockVertexBuffer(D3DLOCK_READONLY, &vertexBuffer);
    if (FAILED(result))
    {
        return result;
    }

    void* indexBuffer = nullptr;
    result = shadowMesh->LockIndexBuffer(D3DLOCK_READONLY, &indexBuffer);
    if (FAILED(result))
    {
        shadowMesh->UnlockVertexBuffer();
        return result;
    }

    DWORD* attributes = nullptr;
    result = shadowMesh->LockAttributeBuffer(D3DLOCK_READONLY, &attributes);
    if (FAILED(result))
    {
        shadowMesh->UnlockIndexBuffer();
        shadowMesh->UnlockVertexBuffer();
        return result;
    }

    faces = new Gfx_SkinFace[shadowFaceCount];
    const BYTE* vertices = static_cast<const BYTE*>(vertexBuffer);
    const bool index32 = (shadowMesh->GetOptions() & D3DXMESH_32BIT) != 0;
    std::vector<D3DXATTRIBUTERANGE> ranges;

    // Build the ranges from actual face attributes, not the pre-optimization
    // material order. CloneMeshFVF may not retain an attribute *table* even
    // when its per-face attribute buffer remains valid.
    result = S_OK;
    for (DWORD faceIndex = 0; faceIndex < shadowFaceCount; ++faceIndex)
    {
        const DWORD material = attributes[faceIndex];
        if (material >= materialCount)
        {
            result = E_FAIL;
            break;
        }
        faceMaterials[faceIndex] = material;

        if (ranges.empty() || ranges.back().AttribId != material)
        {
            D3DXATTRIBUTERANGE range = {};
            range.AttribId = material;
            range.FaceStart = faceIndex;
            range.VertexStart = (std::numeric_limits<DWORD>::max)();
            ranges.push_back(range);
        }
        D3DXATTRIBUTERANGE& range = ranges.back();
        ++range.FaceCount;
        Gfx_SkinFace& face = faces[faceIndex];
        const Gfx_Material& materialInfo = materials[material];
        face.flagB9 = materialInfo.flagB9;
        face.flagBA = materialInfo.flagBA;

        DWORD vertexIndices[3] = {};
        for (DWORD corner = 0; corner < 3; ++corner)
        {
            const size_t position = size_t(faceIndex) * 3 + corner;
            const DWORD vertex = index32 ? static_cast<const DWORD*>(indexBuffer)[position]
                                         : static_cast<const WORD*>(indexBuffer)[position];
            if (vertex >= vertexCount || vertex > (std::numeric_limits<WORD>::max)())
            {
                result = E_FAIL;
                break;
            }
            vertexIndices[corner] = vertex;
            if (range.VertexStart == (std::numeric_limits<DWORD>::max)())
            {
                range.VertexStart = vertex;
                range.VertexCount = 1;
            }
            else
            {
                const DWORD end = range.VertexStart + range.VertexCount - 1;
                const DWORD first = vertex < range.VertexStart ? vertex : range.VertexStart;
                const DWORD last = vertex > end ? vertex : end;
                range.VertexStart = first;
                range.VertexCount = last - first + 1;
            }
        }
        if (FAILED(result))
        {
            break;
        }

        for (DWORD edge = 0; edge < 3; ++edge)
        {
            face.edges[edge].vertices[0] = static_cast<WORD>(vertexIndices[edge]);
            face.edges[edge].vertices[1] = static_cast<WORD>(vertexIndices[(edge + 1) % 3]);
            face.edges[edge].adjacentFace = -1;
            face.edges[edge].ownerFace = DWORD(-1);
        }
        const D3DXVECTOR3& a =
            *reinterpret_cast<const D3DXVECTOR3*>(vertices + size_t(shadowVertexStride) * vertexIndices[0]);
        const D3DXVECTOR3& b =
            *reinterpret_cast<const D3DXVECTOR3*>(vertices + size_t(shadowVertexStride) * vertexIndices[1]);
        const D3DXVECTOR3& c =
            *reinterpret_cast<const D3DXVECTOR3*>(vertices + size_t(shadowVertexStride) * vertexIndices[2]);
        const D3DXVECTOR3 ab = b - a;
        const D3DXVECTOR3 ac = c - a;
        D3DXVec3Cross(&face.normal, &ab, &ac);
        D3DXVec3Normalize(&face.normal, &face.normal);
    }

    const HRESULT attributeUnlock = shadowMesh->UnlockAttributeBuffer();
    const HRESULT indexUnlock = shadowMesh->UnlockIndexBuffer();
    const HRESULT vertexUnlock = shadowMesh->UnlockVertexBuffer();
    if (FAILED(result))
    {
        return result;
    }
    if (FAILED(attributeUnlock))
    {
        return attributeUnlock;
    }
    if (FAILED(indexUnlock))
    {
        return indexUnlock;
    }
    if (FAILED(vertexUnlock))
    {
        return vertexUnlock;
    }

    shadowAttributeCount = static_cast<DWORD>(ranges.size());
    shadowAttributes = new D3DXATTRIBUTERANGE[shadowAttributeCount];
    for (DWORD i = 0; i < shadowAttributeCount; ++i)
    {
        shadowAttributes[i] = ranges[i];
    }

    // Preserve the original silhouette edge pairing and ownership rules.
    for (DWORD faceIndex = 0; faceIndex < shadowFaceCount; ++faceIndex)
    {
        Gfx_SkinFace& face = faces[faceIndex];
        for (DWORD edge = 0; edge < 3; ++edge)
        {
            Gfx_SkinFace::Edge& current = face.edges[edge];
            if (current.adjacentFace != -1)
            {
                continue;
            }
            current.ownerFace = faceIndex;
            if (!face.flagBA)
            {
                continue;
            }
            bool found = false;
            for (DWORD other = 0; other < shadowFaceCount && !found; ++other)
            {
                if (other == faceIndex)
                {
                    continue;
                }
                for (DWORD candidate = 0; candidate < 3; ++candidate)
                {
                    Gfx_SkinFace::Edge& test = faces[other].edges[candidate];
                    if ((current.vertices[0] == test.vertices[0] && current.vertices[1] == test.vertices[1]) ||
                        (current.vertices[0] == test.vertices[1] && current.vertices[1] == test.vertices[0]))
                    {
                        current.adjacentFace = other;
                        test.adjacentFace = faceIndex;
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
    for (DWORD faceIndex = 0; faceIndex < shadowFaceCount; ++faceIndex)
    {
        for (DWORD edge = 0; edge < 3; ++edge)
        {
            const Gfx_SkinFace::Edge& input = faces[faceIndex].edges[edge];
            bool found = false;
            if (input.adjacentFace != -1)
            {
                for (DWORD prior = 0; prior < edgeCount; ++prior)
                {
                    const Gfx_SkinEdge& candidate = scratch[prior];
                    if ((input.vertices[0] == candidate.vertices[0] && input.vertices[1] == candidate.vertices[1]) ||
                        (input.vertices[0] == candidate.vertices[1] && input.vertices[1] == candidate.vertices[0]))
                    {
                        found = true;
                        break;
                    }
                }
            }
            if (!found)
            {
                std::memcpy(scratch + edgeCount, &input, sizeof(Gfx_SkinEdge));
                ++edgeCount;
            }
        }
    }
    edges = new Gfx_SkinEdge[edgeCount];
    for (DWORD edge = 0; edge < edgeCount; ++edge)
    {
        edges[edge] = scratch[edge];
    }
    delete[] scratch;
    return S_OK;
}
