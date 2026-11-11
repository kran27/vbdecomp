#include "G3DSceneAttributes.hpp"
#include "TextParser.hpp"

G3D_WaterAttribute::G3D_WaterAttribute() //4D8120.
{
	// The original repeats these defaults after the actual string/list lifetime.
	dimensions.columns = 0.0;
	dimensions.rows = 0.0;
	cellSize = 0.0f;
	tilesX = 0;
	tilesZ = 0;
}

std::string G3D_WaterAttribute::GetName() const //4CE730.
{
	return name.Value();
}

void G3D_WaterAttribute::ReadText() //4D24A0.
{
	G3DText::Expect('(');
	name.assign(G3DText::ReadString());
	textureName = G3DText::ReadString();
	secondaryTextureName = G3DText::ReadString();
	G3D_WaterDimensions value;
	value.columns = G3DText::ReadNumber();
	value.rows = G3DText::ReadNumber();
	dimensions = value;
	cellSize = static_cast<float>(G3DText::ReadNumber() * G3D_CoordinateInverse * G3D_GlobalScale);
	LONG z = static_cast<LONG>(G3DText::ReadNumber());
	tilesX = static_cast<LONG>(G3DText::ReadNumber());
	tilesZ = z;
	G3DText::Expect(')');
}

void G3D_WaterAttribute::WriteText(FILE* stream) //4CEF90.
{
	G3D_WriteIndent(stream);
	const LONG savedTilesZ = tilesZ;
	const LONG savedTilesX = tilesX;
	const std::string& savedName = GetName();
	fprintf(stream, "%s( \"%s\" \"%s\" \"%s\" %d %d %f %d %d )\n", "Water",
		savedName.c_str(), textureName.c_str(), secondaryTextureName.c_str(),
		static_cast<LONG>(G3D_WaterDimensions(dimensions).columns),
		static_cast<LONG>(G3D_WaterDimensions(dimensions).rows),
		cellSize * G3D_CoordinateScale, savedTilesX, savedTilesZ);
}

void G3D_WaterAttribute::Read(FILE* stream) //4D2620.
{
	G3D_WaterDimensions value;
	ReadName(stream);
	G3D_ReadFileString(textureName, stream);
	G3D_ReadFileString(secondaryTextureName, stream);
	fread(&value, 16, 1, stream);
	dimensions = value;
	fread(&cellSize, 4, 1, stream);
	fread(&tilesX, 4, 1, stream);
	fread(&tilesZ, 4, 1, stream);
}

void G3D_WaterAttribute::Read(const BYTE* data, DWORD& offset) //4D26D0.
{
	ReadName(data, offset);
	G3D_ReadMemoryString(textureName, data, offset);
	G3D_ReadMemoryString(secondaryTextureName, data, offset);
	DWORD value[4];
	memcpy(value, data + offset, sizeof(value));
	offset += sizeof(value);
	memcpy(&dimensions, value, sizeof(value));
	memcpy(&cellSize, data + offset, sizeof(cellSize));
	offset += sizeof(cellSize);
	memcpy(&tilesX, data + offset, sizeof(tilesX));
	offset += sizeof(tilesX);
	memcpy(&tilesZ, data + offset, sizeof(tilesZ));
	offset += sizeof(tilesZ);
}

void G3D_WaterAttribute::Write(FILE* stream) //4CF090, deliberately no leading23 token.
{
	G3D_WaterDimensions value;
	WriteName(stream);
	G3D_WriteFileString(textureName, stream);
	G3D_WriteFileString(secondaryTextureName, stream);
	value = dimensions;
	fwrite(&value, 16, 1, stream);
	fwrite(&cellSize, 4, 1, stream);
	fwrite(&tilesX, 4, 1, stream);
	fwrite(&tilesZ, 4, 1, stream);
}

G3D_LineSegment::G3D_LineSegment() //4CA480, the actual two-Vertex member array.
{
}

void G3D_LineSegment::ReadText() //4D76B0.
{
	std::list<G3D_Vertex> staged;
	G3DText::Expect('{');
	G3DText::Token token = G3DText::NextToken();
	while (token.kind != '}')
	{
		if (token.kind == 256)
		{
			std::string key = G3DText::TokenString(token.value.identifier);
			if (key == "Vertex")
			{
				G3D_Vertex vertex;
				vertex.ReadText();
				staged.push_back(vertex);
			}
			else token.ParseError();
		}
		// Original ignores non-identifier tokens at this level.
		token = G3DText::NextToken();
	}
	G3D_Vertex* output = vertices;
	for (std::list<G3D_Vertex>::const_iterator vertex = staged.begin(); vertex != staged.end(); ++vertex)
		*output++ = *vertex; //Original unchecked input domain, including more than two vertices.
}

void G3D_LineSegment::Read(FILE* stream) //4CA500.
{
	for (LONG vertex = 0; vertex < 2; ++vertex) vertices[vertex].Read(stream);
}

void G3D_LineSegment::Read(const BYTE* data, DWORD& offset) //4CA530.
{
	for (LONG vertex = 0; vertex < 2; ++vertex) vertices[vertex].Read(data, offset);
}

G3D_LineSystem::G3D_LineSystem() //4D78F0.
{
}

G3D_LineSystem::~G3D_LineSystem() //4D7960, actual vector/base lifetime.
{
}

void G3D_LineSystem::ReadText() //4D8270.
{
	std::list<G3D_LineSegment> staged;
	G3DText::Expect('{');
	G3DText::Token token = G3DText::NextToken();
	while (token.kind != '}')
	{
		if (token.kind == 256)
		{
			std::string key = G3DText::TokenString(token.value.identifier);
			if (key.compare(0, key.size(), "Segment") == 0)
			{
				G3D_LineSegment segment;
				segment.ReadText();
				staged.push_back(segment);
			}
			else token.ParseError();
		}
		// Original ignores non-identifier tokens between Segment records.
		token = G3DText::NextToken();
	}
	segments.resize(staged.size(), G3D_LineSegment());
	DWORD output = 0;
	for (std::list<G3D_LineSegment>::const_iterator segment = staged.begin(); segment != staged.end(); ++segment)
		segments[output++] = *segment;
}

void G3D_LineSystem::WriteText(FILE* stream) //4CBC50.
{
	G3D_WriteIndent(stream);
	fprintf(stream, "LineSystem");
	G3D_OpenTextBlock(stream);
	for (LONG segment = 0; segment < static_cast<LONG>(segments.size()); ++segment)
		segments[segment].WriteText(stream);
	G3D_CloseTextBlock(stream);
}

void G3D_LineSystem::Read(FILE* stream) //4D84E0.
{
	std::list<G3D_LineSegment> staged;
	LONG count;
	fread(&count, 4, 1, stream);
	for (LONG index = 0; index < count; ++index)
	{
		G3D_LineSegment segment;
		segment.Read(stream);
		staged.push_back(segment);
	}
	segments.resize(staged.size(), G3D_LineSegment());
	DWORD output = 0;
	for (std::list<G3D_LineSegment>::const_iterator segment = staged.begin(); segment != staged.end(); ++segment)
		segments[output++] = *segment;
}

void G3D_LineSystem::Read(const BYTE* data, DWORD& offset) //4D8630.
{
	std::list<G3D_LineSegment> staged;
	LONG count;
	memcpy(&count, data + offset, sizeof(count));
	offset += sizeof(count);
	for (LONG index = 0; index < count; ++index)
	{
		G3D_LineSegment segment;
		segment.Read(data, offset);
		staged.push_back(segment);
	}
	segments.resize(staged.size(), G3D_LineSegment());
	DWORD output = 0;
	for (std::list<G3D_LineSegment>::const_iterator segment = staged.begin(); segment != staged.end(); ++segment)
		segments[output++] = *segment;
}

void G3D_LineSystem::Write(FILE* stream) //4CBCC0.
{
	BYTE token = 24;
	fwrite(&token, 1, 1, stream);
	LONG count = static_cast<LONG>(segments.size());
	fwrite(&count, 4, 1, stream);
	for (LONG segment = 0; segment < count; ++segment) segments[segment].Write(stream);
}

G3D_ParticleEmitter::G3D_ParticleEmitter() : scale(G3D_GlobalScale) //4CBB90.
{
}

G3D_ParticleEmitter::~G3D_ParticleEmitter() //4CBBF0, actual two-string/base lifetime.
{
}

void G3D_ParticleEmitter::ReadText() //4D2260.
{
	G3DText::Expect('(');
	G3DText::Token token = G3DText::NextToken();
	std::string value = G3DText::TokenString(token.value.identifier);
	effectName = value; //Original does not validate the token kind.
	G3DText::Expect(')');
	scale = G3D_GlobalScale;
}

void G3D_ParticleEmitter::WriteText(FILE* stream) //4CAC90.
{
	G3D_WriteIndent(stream);
	fprintf(stream, "%s(\"%s\")\n", "ParticleEmitter", effectName.c_str());
}

void G3D_ParticleEmitter::Read(FILE* stream) //4D2330.
{
	std::string value;
	G3D_ReadFileString(value, stream);
	effectName = value;
	scale = G3D_GlobalScale;
}

void G3D_ParticleEmitter::Read(const BYTE* data, DWORD& offset) //4D23E0.
{
	std::string value;
	G3D_ReadMemoryString(value, data, offset);
	effectName = value;
	scale = G3D_GlobalScale;
}

void G3D_ParticleEmitter::Write(FILE* stream) //4CEF30.
{
	BYTE token = 20;
	fwrite(&token, 1, 1, stream);
	G3D_WriteFileString(effectName, stream);
}

G3D_LightmapSurfaceAttribute::G3D_LightmapSurfaceAttribute() //4CB350.
{
}

G3D_LightmapSurfaceAttribute::~G3D_LightmapSurfaceAttribute() //4CB3A0, actual three-string/base lifetime.
{
}

void G3D_LightmapSurfaceAttribute::ReadText() //4D0C80.
{
	G3DText::Expect('{');
	G3DText::Token token = G3DText::NextToken();
	while (token.kind != '}')
	{
		if (token.kind == 256)
		{
			std::string key = G3DText::TokenString(token.value.identifier);
			if (key == "Material")
			{
				G3DText::Expect('(');
				material = G3DText::ReadString();
				G3DText::Expect(')');
			}
			else if (key == "Lightmap")
			{
				G3DText::Expect('(');
				lightmap = G3DText::ReadString();
				G3DText::Expect(')');
			}
			else token.ParseError();
		}
		else token.ParseError();
		token = G3DText::NextToken();
	}
}

void G3D_LightmapSurfaceAttribute::WriteText(FILE* stream) //4CAC10.
{
	G3D_WriteIndent(stream);
	fprintf(stream, "%s{ ", "LightmapSurface");
	fprintf(stream, "%s(\"%s\") ", "Material", material.c_str());
	fprintf(stream, "%s(\"%s\") ", "Lightmap", lightmap.c_str());
	fprintf(stream, "}\n");
}

void G3D_LightmapSurfaceAttribute::Read(FILE* stream) //4D0F00.
{
	G3D_ReadFileString(material, stream);
	G3D_ReadFileString(lightmap, stream);
}

void G3D_LightmapSurfaceAttribute::Read(const BYTE* data, DWORD& offset) //4D0F30.
{
	G3D_ReadMemoryString(material, data, offset);
	G3D_ReadMemoryString(lightmap, data, offset);
}

void G3D_LightmapSurfaceAttribute::Write(FILE* stream) //4CEBB0.
{
	BYTE token = 21;
	fwrite(&token, 1, 1, stream);
	G3D_WriteFileString(material, stream);
	G3D_WriteFileString(lightmap, stream);
}
