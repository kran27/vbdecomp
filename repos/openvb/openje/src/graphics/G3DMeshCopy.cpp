#include "G3DModel.hpp"
#include <cstring>

void G3D_Mesh::CopyFrom(const G3D_Mesh& source) //4D9DD0.
{
	//No self-copy guard; kind is retained by the actual derived attribute.
	Clear();
	name = source.name;
	owner = source.owner;
	for (DWORD vertex = 0; vertex < source.legacyVertices.size(); ++vertex)
		legacyVertices.push_back(source.legacyVertices[vertex]);
	for (DWORD polygon = 0; polygon < source.polygons.size(); ++polygon)
		AddPolygon(*source.polygons[polygon]);
	ResetConverted();
	converted = source.converted;
	vertexCount = source.vertexCount;
	vertexStride = source.vertexStride;
	std::memcpy(minimum, source.minimum, sizeof(minimum));
	std::memcpy(maximum, source.maximum, sizeof(maximum));
	hasBones = source.hasBones;
	groupCount = source.groupCount;
	if (groupCount)
	{
		materials = new G3D_MeshMaterial[groupCount];
		materialNames = new std::string[groupCount];
		vertexCounts = new DWORD[groupCount];
		indexCounts = new DWORD[groupCount];
		indices = new WORD*[groupCount];
		for (DWORD group = 0; group < groupCount; ++group)
		{
			//The original copies names+140 shallowly. Mesh cleanup still owns it.
			materials[group] = source.materials[group];
			materialNames[group] = source.materialNames[group];
			vertexCounts[group] = source.vertexCounts[group];
			indexCounts[group] = source.indexCounts[group];
			indices[group] = new WORD[indexCounts[group]];
			//4DA0CF/4DA0D5 copy only count BYTES into the 2*count allocation.
			for (DWORD index = 0; index < indexCounts[group]; ++index)
				reinterpret_cast<BYTE*>(indices[group])[index] =
					reinterpret_cast<const BYTE*>(source.indices[group])[index];
		}
	}
	if (vertexStride * vertexCount && source.vertices)
	{
		vertices = new BYTE[vertexStride * vertexCount];
		std::memcpy(vertices, source.vertices, vertexStride * vertexCount);
	}
	if (vertexCount)
		for (DWORD bone = 0; bone < 4; ++bone)
			if (source.boneWeights[bone])
			{
				boneWeights[bone] = new Gfx_G3D_BoneWeight[vertexCount];
				for (DWORD vertex = 0; vertex < vertexCount; ++vertex)
				{
					std::memcpy(&boneWeights[bone][vertex].bone,
						&source.boneWeights[bone][vertex].bone, 4);
					std::memcpy(&boneWeights[bone][vertex].weight,
						&source.boneWeights[bone][vertex].weight, 4);
				}
			}
}
