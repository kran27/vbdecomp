#include "G3DModel.hpp"
#include "TextParser.hpp"
#include "../diagnostics/DebugOutput.hpp"

//4D8C10. The original legacy-vertex vector retains its allocation; polygon
//children are owned and destroyed from the back before converted storage.
void G3D_Mesh::Clear()
{
	legacyVertices.resize(0);
	while (polygons.size() > 0)
	{
		delete polygons[polygons.size() - 1];
		polygons.pop_back();
	}
	ResetConverted();
}

//4CCC10. The full original loop4CCD70..4CCD94 releases every index group.
//The older truncated upstream pseudocode incorrectly treated free as noreturn.
void G3D_Mesh::ResetConverted()
{
	converted = 0;
	vertexCount = 0;
	vertexStride = 0;
	if (vertices) { delete[] vertices; vertices = NULL; }
	hasBones = 0;
	for (DWORD bone = 0; bone < 4; ++bone)
		if (boneWeights[bone]) { delete[] boneWeights[bone]; boneWeights[bone] = NULL; }
	if (materials)
	{
		for (DWORD group = 0; group < groupCount; ++group)
			if (materials[group].names)
			{
				delete[] materials[group].names;
				materials[group].names = NULL;
			}
		delete[] materials;
		materials = NULL;
	}
	if (materialNames) { delete[] materialNames; materialNames = NULL; }
	if (indexCounts) { delete[] indexCounts; indexCounts = NULL; }
	if (vertexCounts) { delete[] vertexCounts; vertexCounts = NULL; }
	if (indices)
	{
		for (DWORD group = 0; group < groupCount; ++group)
		{
			delete[] indices[group];
			indices[group] = NULL;
		}
		delete[] indices;
		indices = NULL;
	}
	groupCount = 0;
}

//4D9D50 allocates/deep-copies the actual original polygon child.
void G3D_Mesh::AddPolygon(const G3D_Polygon& polygon)
{
	G3D_Polygon* child = new G3D_Polygon(polygon);
	polygons.push_back(child);
}

//4DB040 is a token-input method, not a FILE or memory-domain adapter. The
//persistent polygon carries Material/Texture/Lightmap state between Triangles.
//Unlike the binary reader, it does not clear the mesh on entry.
void G3D_Mesh::ReadText()
{
	std::list<G3D_Vertex> staged;
	G3D_Polygon polygon;
	G3DText::Expect('{');
	G3DText::Token token = G3DText::NextToken();
	while (token.kind != '}')
	{
		if (token.kind != 256) token.ParseError();
		std::string key = G3DText::TokenString(token.value.identifier);
		if (key == "Name")
		{
			G3DText::Expect('(');
			name = G3DText::ReadString();
			G3DText::Expect(')');
		}
		else if (key == "Vertex")
		{
			G3D_Vertex vertex;
			vertex.ReadText();
			staged.push_back(vertex);
		}
		else if (key == "Material")
		{
			G3DText::Expect('(');
			polygon.value00 = static_cast<LONG>(G3DText::ReadNumber());
			polygon.material = G3DText::ReadString();
			G3DText::Expect(')');
		}
		else if (key == "Texture")
		{
			G3DText::Expect('(');
			polygon.textureCount = 1;
			polygon.textures[0] = G3DText::ReadString();
			G3DText::Expect(')');
		}
		else if (key == "Lightmap")
		{
			G3DText::Expect('(');
			polygon.textureCount = 2;
			polygon.textures[1] = G3DText::ReadString();
			G3DText::Expect(')');
		}
		else if (key == "OcclusionZones")
		{
			polygon.ClearNames();
			G3DText::Expect('{');
			G3DText::Token zone = G3DText::NextToken();
			while (zone.kind != '}')
			{
				if (zone.kind != 256) zone.ParseError();
				std::string zoneKey = G3DText::TokenString(zone.value.identifier);
				if (zoneKey != "Zone") zone.ParseError();
				G3DText::Expect('(');
				polygon.names.push_back(G3DText::ReadString());
				G3DText::Expect(')');
				zone = G3DText::NextToken();
			}
		}
		else if (key == "Triangles")
		{
			polygon.primitive = 0;
			polygon.ReadTextIndices();
			AddPolygon(polygon);
			polygon.ClearNames();
		}
		else if (key == "TextureSlot")
		{
			G3DText::Expect('(');
			std::string unused = G3DText::ReadString();
			G3DText::Expect(')');
		}
		else token.ParseError();
		token = G3DText::NextToken();
	}
	legacyVertices.resize(staged.size(), G3D_Vertex());
	DWORD index = 0;
	for (std::list<G3D_Vertex>::const_iterator vertex = staged.begin(); vertex != staged.end(); ++vertex, ++index)
		legacyVertices[index] = *vertex;
}

//4DB810 reads the real CRT FILE stream in its original order. Read results,
//wire counts, and owner/model access remain unchecked. Raw allocations retain
//the original uninitialized bytes when fread supplies only a partial record.
void G3D_Mesh::Read(FILE* stream) //4DB810.
{
	Clear();
	ReadName(stream);
	BYTE version = owner->model->version;
	if (version)
	{
		fread(&converted, 1, 1, stream);
		if (!converted)
			JE::LogDebugString("G3D_Mesh::ReadB3D - Version is %d but G3D_Mesh is not converted!\n", version);
	}
	if (!version || !converted)
	{
		std::list<G3D_Vertex> staged;
		LONG count;
		fread(&count, sizeof(count), 1, stream);
		for (LONG index = 0; index < count; ++index)
		{
			G3D_Vertex vertex;
			vertex.Read(stream);
			staged.push_back(vertex);
		}
		fread(&count, sizeof(count), 1, stream);
		for (LONG polygonIndex = 0; polygonIndex < count; ++polygonIndex)
		{
			G3D_Polygon polygon;
			polygon.Read(stream);
			AddPolygon(polygon);
		}
		legacyVertices.resize(staged.size(), G3D_Vertex());
		DWORD output = 0;
		for (std::list<G3D_Vertex>::const_iterator vertex = staged.begin(); vertex != staged.end(); ++vertex, ++output)
			legacyVertices[output] = *vertex;
		return;
	}
	fread(&vertexCount, sizeof(vertexCount), 1, stream);
	fread(&vertexStride, sizeof(vertexStride), 1, stream);
	vertices = new BYTE[vertexCount * vertexStride];
	fread(vertices, vertexStride, vertexCount, stream);
	fread(minimum, sizeof(float), 3, stream);
	fread(maximum, sizeof(float), 3, stream);
	fread(&hasBones, 1, 1, stream);
	if (hasBones)
	{
		DWORD channels;
		fread(&channels, sizeof(channels), 1, stream);
		if (channels != 4)
			JE::FatalError("G3D_Mesh::ReadB3D - Bone counts different - Need to re-export!!!\n");
		for (DWORD bone = 0; bone < 4; ++bone)
		{
			boneWeights[bone] = new Gfx_G3D_BoneWeight[vertexCount];
			fread(boneWeights[bone], 8, vertexCount, stream);
		}
	}
	fread(&groupCount, sizeof(groupCount), 1, stream);
	materials = new G3D_MeshMaterial[groupCount];
	materialNames = new std::string[groupCount];
	vertexCounts = new DWORD[groupCount];
	indexCounts = new DWORD[groupCount];
	indices = new WORD*[groupCount];
	for (DWORD group = 0; group < groupCount; ++group)
	{
		G3D_MeshMaterial& material = materials[group];
		G3D_ReadFileString(material.name, stream);
		G3D_ReadFileString(material.technique, stream);
		fread(material.colors, sizeof(float), 4, stream);
		fread(material.colors + 4, sizeof(float), 4, stream);
		fread(material.colors + 8, sizeof(float), 4, stream);
		fread(material.colors + 12, sizeof(float), 4, stream);
		fread(&material.power, sizeof(float), 1, stream);
		G3D_ReadFileString(material.blend, stream);
		G3D_ReadFileString(material.materialType, stream);
		fread(&material.flags180[0], 1, 1, stream);
		fread(&material.flags180[1], 1, 1, stream);
		fread(&material.flags180[2], 1, 1, stream);
		fread(&material.value184, sizeof(DWORD), 1, stream);
		fread(&material.flag188, 1, 1, stream);
		fread(&material.value192, sizeof(DWORD), 1, stream);
		fread(&material.flag196, 1, 1, stream);
		fread(&material.parametersA[0], sizeof(float), 1, stream);
		fread(&material.parametersA[1], sizeof(float), 1, stream);
		fread(&material.flag208, 1, 1, stream);
		fread(&material.parametersB[0], sizeof(float), 1, stream);
		fread(&material.parametersB[1], sizeof(float), 1, stream);
		fread(&material.flag220, 1, 1, stream);
		fread(&material.parametersC[0], sizeof(float), 1, stream);
		fread(&material.parametersC[1], sizeof(float), 1, stream);
		fread(&material.parametersC[2], sizeof(float), 1, stream);
		fread(&material.flag236, 1, 1, stream);
		fread(&material.parametersD[0], sizeof(float), 1, stream);
		fread(&material.parametersD[1], sizeof(float), 1, stream);
		fread(&material.parametersD[2], sizeof(float), 1, stream);
		fread(&material.parametersD[3], sizeof(float), 1, stream);
		fread(&material.textureCount, sizeof(DWORD), 1, stream);
		if (material.textureCount)
		{
			G3D_ReadFileString(material.textures[0], stream);
			if (material.textureCount > 1) G3D_ReadFileString(material.textures[1], stream);
		}
		fread(&material.nameCount, sizeof(material.nameCount), 1, stream);
		material.names = NULL;
		if (material.nameCount)
		{
			material.names = new std::string[material.nameCount];
			for (DWORD nameIndex = 0; nameIndex < material.nameCount; ++nameIndex)
				G3D_ReadFileString(material.names[nameIndex], stream);
		}
	}
	for (DWORD materialIndex = 0; materialIndex < groupCount; ++materialIndex)
		G3D_ReadFileString(materialNames[materialIndex], stream);
	fread(indexCounts, sizeof(DWORD), groupCount, stream);
	fread(vertexCounts, sizeof(DWORD), groupCount, stream);
	for (DWORD indexGroup = 0; indexGroup < groupCount; ++indexGroup)
	{
		indices[indexGroup] = new WORD[indexCounts[indexGroup]];
		fread(indices[indexGroup], sizeof(WORD), indexCounts[indexGroup], stream);
	}
}

void G3D_Mesh::WriteText(FILE* stream) //4D2E80.
{
	if (kind == 15)
	{
		G3D_WriteIndent(stream);
		fprintf(stream, "Mesh");
	}
	G3D_OpenTextBlock(stream);
	name.WriteText(stream, 1);
	for (LONG vertex = 0; vertex < static_cast<LONG>(legacyVertices.size()); ++vertex)
		legacyVertices[vertex].WriteText(stream);
	for (LONG polygon = 0; polygon < static_cast<LONG>(polygons.size()); ++polygon)
		polygons[polygon]->WriteText(stream);
	G3D_CloseTextBlock(stream);
}

void G3D_Mesh::Write(FILE* stream) //4D2F40; original binary FILE domain.
{
	fwrite(&kind, 1, 1, stream);
	WriteName(stream);
	if (owner->model->version)
	{
		if (converted)
		{
			fwrite(&converted, 1, 1, stream);
			fwrite(&vertexCount, 4, 1, stream);
			fwrite(&vertexStride, 4, 1, stream);
			fwrite(vertices, vertexStride, vertexCount, stream);
			fwrite(minimum, 4, 3, stream);
			fwrite(maximum, 4, 3, stream);
			fwrite(&hasBones, 1, 1, stream);
			if (hasBones)
			{
				static const DWORD channels = 4;
				fwrite(&channels, 4, 1, stream);
				for (DWORD bone = 0; bone < 4; ++bone)
					fwrite(boneWeights[bone], 8, vertexCount, stream);
			}
			fwrite(&groupCount, 4, 1, stream);
			for (DWORD group = 0; group < groupCount; ++group)
			{
				G3D_MeshMaterial& material = materials[group];
				G3D_WriteFileString(material.name, stream);
				G3D_WriteFileString(material.technique, stream);
				fwrite(material.colors, 4, 4, stream);
				fwrite(material.colors + 4, 4, 4, stream);
				fwrite(material.colors + 8, 4, 4, stream);
				fwrite(material.colors + 12, 4, 4, stream);
				fwrite(&material.power, 4, 1, stream);
				G3D_WriteFileString(material.blend, stream);
				G3D_WriteFileString(material.materialType, stream);
				fwrite(&material.flags180[0], 1, 1, stream);
				fwrite(&material.flags180[1], 1, 1, stream);
				fwrite(&material.flags180[2], 1, 1, stream);
				fwrite(&material.value184, 4, 1, stream);
				fwrite(&material.flag188, 1, 1, stream);
				fwrite(&material.value192, 4, 1, stream);
				fwrite(&material.flag196, 1, 1, stream);
				fwrite(&material.parametersA[0], 4, 1, stream);
				fwrite(&material.parametersA[1], 4, 1, stream);
				fwrite(&material.flag208, 1, 1, stream);
				fwrite(&material.parametersB[0], 4, 1, stream);
				fwrite(&material.parametersB[1], 4, 1, stream);
				fwrite(&material.flag220, 1, 1, stream);
				fwrite(&material.parametersC[0], 4, 1, stream);
				fwrite(&material.parametersC[1], 4, 1, stream);
				fwrite(&material.parametersC[2], 4, 1, stream);
				fwrite(&material.flag236, 1, 1, stream);
				fwrite(&material.parametersD[0], 4, 1, stream);
				fwrite(&material.parametersD[1], 4, 1, stream);
				fwrite(&material.parametersD[2], 4, 1, stream);
				fwrite(&material.parametersD[3], 4, 1, stream);
				fwrite(&material.textureCount, 4, 1, stream);
				if (material.textureCount)
				{
					G3D_WriteFileString(material.textures[0], stream);
					if (material.textureCount > 1) G3D_WriteFileString(material.textures[1], stream);
				}
				fwrite(&material.nameCount, 4, 1, stream);
				for (DWORD name = 0; name < material.nameCount; ++name)
					G3D_WriteFileString(material.names[name], stream);
			}
			for (DWORD material = 0; material < groupCount; ++material)
				G3D_WriteFileString(materialNames[material], stream);
			fwrite(indexCounts, 4, groupCount, stream);
			fwrite(vertexCounts, 4, groupCount, stream);
			for (DWORD group = 0; group < groupCount; ++group)
				fwrite(indices[group], 2, indexCounts[group], stream);
			return;
		}
		BYTE notConverted = 0;
		fwrite(&notConverted, 1, 1, stream);
	}
	LONG count = static_cast<LONG>(legacyVertices.size());
	fwrite(&count, 4, 1, stream);
	for (LONG vertex = 0; vertex < count; ++vertex)
		legacyVertices[vertex].Write(stream);
	count = static_cast<LONG>(polygons.size());
	fwrite(&count, 4, 1, stream);
	for (LONG polygon = 0; polygon < count; ++polygon)
		polygons[polygon]->Write(stream);
}

void G3D_OcclusionZones::ReadText() //4D9590.
{
	G3DText::Expect('{');
	G3DText::Token token = G3DText::NextToken();
	while (token.kind != '}')
	{
		if (token.kind == 256)
		{
			std::string key = G3DText::TokenString(token.value.identifier);
			if (key.compare(0, key.size(), "Zone") == 0)
			{
				G3DText::Expect('(');
				names.push_back(G3DText::ReadString());
				G3DText::Expect(')');
			}
			else token.ParseError();
		}
		else token.ParseError();
		token = G3DText::NextToken();
	}
}

void G3D_OcclusionZones::WriteText(FILE* stream) //4CB480.
{
	G3D_WriteIndent(stream);
	fprintf(stream, "%s{ ", "OcclusionZones");
	for (LONG zone = 0; zone < static_cast<LONG>(names.size()); ++zone)
		fprintf(stream, "%s(\"%s\") ", "Zone", names[zone].c_str());
	fprintf(stream, "}\n");
}

void G3D_OcclusionZones::Read(FILE* stream) //4D9770.
{
	std::string value;
	//VC7 reuses the dead FILE argument slot; short reads leave its bytes untouched.
	LONG count;
	fread(&count, sizeof(count), 1, stream);
	for (LONG zone = 0; zone < count; ++zone)
	{
		G3D_ReadFileString(value, stream);
		names.push_back(value);
	}
}

void G3D_OcclusionZones::Write(FILE* stream) //4D0F60.
{
	BYTE tag = 27;
	fwrite(&tag, 1, 1, stream);
	LONG count = static_cast<LONG>(names.size());
	fwrite(&count, 4, 1, stream);
	for (LONG zone = 0; zone < count; ++zone)
	{
		//Construct the outgoing value directly from c_str(), truncating NULs.
		G3D_WriteFileString(names[zone].c_str(), stream);
	}
}
