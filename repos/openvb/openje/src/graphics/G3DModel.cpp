#include "G3DModel.hpp"
#include "G3DSceneAttributes.hpp"
#include "../diagnostics/DebugOutput.hpp"
#include <math.h>
#include <stdio.h>
#include <string.h>

double G3D_CoordinateScaleDouble = 1024.0; //6FB358; actual distinct owner.

double G3D_GlobalScale = 1.0; //6FB350; shared by original model/text-vertex readers.
double G3D_CoordinateScale = 1024.0; //6FB348; shared original numeric scale.
double G3D_CoordinateInverse = 0.0009765625; //708080; original shared numeric scale.

namespace
{
	void ReadMemoryTextureCoordinate(const BYTE* data, double* output, DWORD& offset) //4C93C0.
	{
		float value;
		memcpy(&value, data + offset, sizeof(value));
		offset += sizeof(value);
		output[0] = value;
		memcpy(&value, data + offset, sizeof(value));
		offset += sizeof(value);
		output[1] = value;
	}

	//4C8A30; the original private register convention remains a separate ABI boundary.
	D3DXCOLOR* ReadMemoryColorWithAlpha(D3DXCOLOR* output, const BYTE* data, DWORD& offset)
	{
		D3DXCOLOR color = G3D_ReadMemoryColor(data, offset);
		memcpy(&color.a, data + offset, sizeof(color.a));
		offset += sizeof(color.a);
		memcpy(output, &color, sizeof(color));
		return output;
	}
}

void G3D_Input::Copy(void* output, DWORD count)
{
	memcpy(output, data + offset, count);
	offset += count;
}

void G3D_ReadMemoryString(std::string& output, const BYTE* data, DWORD& offset) //4D0510.
{
	output.assign("");
	WORD size;
	memcpy(&size, data + offset, sizeof(size));
	offset += sizeof(size);
	if (size > 0)
	{
		output.resize(size + 1);
		char* value = new char[size + 1];
		memcpy(value, data + offset, size);
		offset += size;
		value[size] = 0;
		output.assign(value, strlen(value));
		delete[] value;
	}
}

void G3D_Input::ReadString(std::string& output)
{
	G3D_ReadMemoryString(output, data, offset);
}

void G3D_Name::Read(FILE* stream) //4D0670.
{
	G3D_ReadFileString(*this, stream);
}

void G3D_Name::Read(const BYTE* data, DWORD& offset) //4D0690.
{
	G3D_ReadMemoryString(*this, data, offset);
}

void G3D_LineSegment::WriteText(FILE* stream) //4CA4B0.
{
	G3D_WriteIndent(stream);
	fprintf(stream, "Segment");
	G3D_OpenTextBlock(stream);
	for (LONG vertex = 0; vertex < 2; ++vertex) vertices[vertex].WriteText(stream);
	G3D_CloseTextBlock(stream);
}

void G3D_LineSegment::Write(FILE* stream) //4CA560.
{
	for (LONG vertex = 0; vertex < 2; ++vertex) vertices[vertex].Write(stream);
}

double G3D_TextNumber(double value) //4C8730.
{
	return floor(G3D_CoordinateScale * value + 0.5);
}

void G3D_WriteFileTextureCoordinate(FILE* stream, const double* value) //4C94B0.
{
	float wire = static_cast<float>(value[0]);
	fwrite(&wire, 4, 1, stream);
	wire = static_cast<float>(value[1]);
	fwrite(&wire, 4, 1, stream);
}

void G3D_BoneWeight::Write(FILE* stream) //4C8C90.
{
	LONG index = bone;
	fwrite(&index, 4, 1, stream);
	float value = static_cast<float>(weight);
	fwrite(&value, 4, 1, stream);
}

LONG G3D_TextIndentation = 0; //707F38; shared original text-writer state.

void G3D_WriteIndent(FILE* stream) //4C8B00.
{
	for (LONG level = 0; level < G3D_TextIndentation; ++level) fprintf(stream, "  ");
}

//4CB420 writes the low WORD of the length, then that unsigned WORD byte count.
//The by-value string is a real VC7 string; its destructor owns the temporary.
void G3D_WriteFileString(std::string value, FILE* stream)
{
	DWORD length = static_cast<DWORD>(value.size());
	fwrite(&length, 2, 1, stream);
	fwrite(value.c_str(), 1, static_cast<WORD>(length), stream);
}

std::string G3D_Name::Value() const //4CE890; original source receiver and hidden string output.
{
	return static_cast<const std::string&>(*this);
}

std::string G3D_BaseAttribute::GetName() const { return name.Value(); }
std::string G3D_Node::GetName() const { return name.Value(); }
std::string G3D_Bone::GetName() const { return name.Value(); }
std::string G3D_Texture::GetName() const { return name.Value(); }
std::string G3D_Material::GetName() const { return name.Value(); }
std::string G3D_OcclusionZones::GetName(DWORD index) const { return names[index]; }
std::string G3D_Texture::GetFilename() const { return file; }
std::string G3D_Material::GetBlend() const { return blend; }
std::string G3D_Polygon::TextureValue(LONG index) const { return textures[index]; }
std::string G3D_Polygon::MaterialValue() const { return material; }
std::string G3D_Polygon::GetTextureName(LONG index) const { return TextureValue(index); }
std::string G3D_Polygon::GetMaterial() const { return MaterialValue(); }
std::string G3D_Polygon::GetName(LONG index) const { return names[index]; }

G3D_Material* G3D_Model::FindMaterial(const std::string& name)
{
	std::map<std::string, G3D_Material*>::iterator found = materialsByName.find(name);
	if (found != materialsByName.end()) return found->second;
	return NULL;
}

G3D_Texture* G3D_Model::FindTexture(const std::string& name)
{
	std::map<std::string, G3D_Texture*>::iterator found = texturesByName.find(name);
	if (found != texturesByName.end()) return found->second;
	return NULL;
}

void G3D_Texture::Read(const BYTE* data, DWORD& offset) //4D1180.
{
	std::string temporary;
	BYTE bits = data[offset++];
	flags[0] = bits & 1;
	flags[1] = (bits >> 1) & 1;
	flags[2] = (bits & 4) != 0;
	flags[3] = (bits & 8) != 0;
	cube = (bits & 16) != 0;
	name.Read(data, offset);
	G3D_ReadMemoryString(temporary, data, offset);
	file = temporary;
	LONG value;
	memcpy(&value, data + offset, 4); offset += 4; SetWidth(value);
	memcpy(&value, data + offset, 4); offset += 4; SetHeight(value);
}

void G3D_Texture::Read(G3D_Input& input)
{
	Read(input.data, input.offset);
}

void G3D_Bone::Read(const BYTE* data, DWORD& offset) //4D09D0.
{
	name.Read(data, offset);
	transform.Read(data, offset);
}

void G3D_Bone::Read(G3D_Input& input)
{
	Read(input.data, input.offset);
}

G3D_Material::G3D_Material()
	: twoSided(0), flat(0), isFloor(0), noShadow(0),
	bottomStory(0), bottomStoryInterior(0), topStory(0), buildingNumber(0),
	blend("OPAQUE"), materialType("SILENT")
{
	scrollUEnabled = 0; scrollU = 0;
	scrollVEnabled = 0; scrollV = 0;
	sinUEnabled = 0; sinUFirst = 0; sinUSecond = 0;
	sinVEnabled = 0; sinVFirst = 0; sinVSecond = 0;
	rotationEnabled = 0; rotationFirst = 0; rotationSecond = 0; rotationThird = 0;
	animationEnabled = 0; animationRate = 0;
	animationFirst = 0; animationSecond = 0; animationThird = 0;
	index = -1; //4D042F follows the primitive parameter/default writes.
	// Colors/power/alpha are supplied by Read, not fabricated constructor defaults.
}

void G3D_Material::Read(const BYTE* data, DWORD& offset) //4D16A0.
{
	std::string temporary;
	G3D_Input input(data, offset);
	G3D_ReadMemoryString(temporary, data, offset);
	technique = temporary;
	index = -1;
	name.Read(data, offset);
	D3DXCOLOR color;
	D3DXCOLOR* value = ReadMemoryColorWithAlpha(&color, data, offset);
	memcpy(&ambient, value, sizeof(color));
	value = ReadMemoryColorWithAlpha(&color, data, offset);
	memcpy(&diffuse, value, sizeof(color));
	value = ReadMemoryColorWithAlpha(&color, data, offset);
	memcpy(&emissive, value, sizeof(color));
	value = ReadMemoryColorWithAlpha(&color, data, offset);
	memcpy(&specular, value, sizeof(color));
	power = input.Read<float>();
	alpha = input.Read<float>();
	G3D_ReadMemoryString(blend, data, offset);
	G3D_ReadMemoryString(materialType, data, offset);
	DWORD enabled = input.Read<DWORD>();
	if (enabled & 1)
	{
		double value;
		input.Copy(&value, sizeof(value));
		scrollUEnabled = 1;
		memcpy(&scrollU, &value, sizeof(value));
	}
	if (enabled & 2)
	{
		double value;
		input.Copy(&value, sizeof(value));
		scrollVEnabled = 1;
		memcpy(&scrollV, &value, sizeof(value));
	}
	if (enabled & 4)
	{
		double first, second;
		input.Copy(&first, sizeof(first));
		input.Copy(&second, sizeof(second));
		memcpy(&sinUFirst, &first, sizeof(first));
		sinUEnabled = 1;
		memcpy(&sinUSecond, &second, sizeof(second));
	}
	if (enabled & 8)
	{
		double first, second;
		input.Copy(&first, sizeof(first));
		input.Copy(&second, sizeof(second));
		memcpy(&sinVFirst, &first, sizeof(first));
		sinVEnabled = 1;
		memcpy(&sinVSecond, &second, sizeof(second));
	}
	if (enabled & 16)
	{
		double first, second, third;
		input.Copy(&first, sizeof(first));
		input.Copy(&second, sizeof(second));
		input.Copy(&third, sizeof(third));
		memcpy(&rotationFirst, &first, sizeof(first));
		memcpy(&rotationSecond, &second, sizeof(second));
		rotationEnabled = 1;
		memcpy(&rotationThird, &third, sizeof(third));
	}
	if (enabled & 32)
	{
		double rate;
		input.Copy(&rate, sizeof(rate));
		LONG first = input.Read<LONG>();
		LONG second = input.Read<LONG>();
		LONG third = input.Read<LONG>();
		memcpy(&animationRate, &rate, sizeof(rate));
		animationFirst = first;
		animationEnabled = 1;
		animationSecond = second;
		animationThird = third;
	}
	DWORD bits = input.Read<DWORD>();
	flat = BYTE(bits & 1);
	twoSided = (bits & 2) != 0;
	isFloor = (bits & 4) != 0;
	topStory = (bits & 32) != 0;
	noShadow = (bits & 8) != 0;
	bottomStory = (bits & 16) != 0;
}

void G3D_Material::Read(G3D_Input& input)
{
	Read(input.data, input.offset);
}

BYTE G3D_Mesh::GetConvertedData(G3D_ConvertedMeshData* output)
{
	memset(output, 0, sizeof(*output));
	BYTE result = converted;
	if (result)
	{
		output->vertexCount = vertexCount;
		output->vertexStride = vertexStride;
		output->vertices = vertices;
		output->minimum = minimum;
		output->maximum = maximum;
		output->hasBones = hasBones;
		output->boneWeights = boneWeights;
		output->groupCount = groupCount;
		output->materials = materials;
		output->materialNames = materialNames;
		DWORD* capturedIndexCounts = indexCounts;
		output->indexCounts = capturedIndexCounts;
		output->primitiveCounts = vertexCounts;
		//Native converted data has a borrowed pointer table of16-bit index buffers.
		output->indices = reinterpret_cast<void**>(indices);
		result = static_cast<BYTE>(reinterpret_cast<ULONG_PTR>(capturedIndexCounts));
	}
	return result;
}

void G3D_MeshMaterial::Read(G3D_Input& input)
{
	input.ReadString(name);
	input.ReadString(technique);
	for (DWORD colorIndex = 0; colorIndex < 4; ++colorIndex)
	{
		input.Copy(colors + colorIndex * 4, sizeof(float) * 4);
	}
	input.Copy(&power, sizeof(power));
	input.ReadString(blend);
	input.ReadString(materialType);
	for (DWORD flagIndex = 0; flagIndex < 3; ++flagIndex)
	{
		input.Copy(&flags180[flagIndex], sizeof(flags180[flagIndex]));
	}
	input.Copy(&value184, sizeof(value184));
	input.Copy(&flag188, sizeof(flag188));
	input.Copy(&value192, sizeof(value192));
	input.Copy(&flag196, sizeof(flag196));
	input.Copy(&parametersA[0], sizeof(parametersA[0])); // +C8.
	input.Copy(&parametersA[1], sizeof(parametersA[1])); // +CC.
	input.Copy(&flag208, sizeof(flag208));
	input.Copy(&parametersB[0], sizeof(parametersB[0]));
	input.Copy(&parametersB[1], sizeof(parametersB[1]));
	input.Copy(&flag220, sizeof(flag220));
	input.Copy(&parametersC[0], sizeof(parametersC[0]));
	input.Copy(&parametersC[1], sizeof(parametersC[1]));
	input.Copy(&parametersC[2], sizeof(parametersC[2]));
	input.Copy(&flag236, sizeof(flag236));
	for (DWORD index = 0; index < 4; ++index)
	{
		input.Copy(&parametersD[index], sizeof(parametersD[index]));
	}
	// The final two entries are not extra float reads: +100 is texture count.
	input.Copy(&textureCount, sizeof(textureCount));
	if (textureCount)
	{
		input.ReadString(textures[0]);
		if (textureCount > 1) input.ReadString(textures[1]);
	}
	input.Copy(&nameCount, sizeof(nameCount));
	names = NULL;
	if (nameCount)
	{
		names = new std::string[nameCount];
		for (DWORD index = 0; index < nameCount; ++index) input.ReadString(names[index]);
	}
}

G3D_Mesh::G3D_Mesh()
{
	kind = 15;
	converted = 0;
	vertexCount = 0;
	vertexStride = 0;
	vertices = NULL;
	hasBones = 0;
	for (DWORD index = 0; index < 4; ++index) boneWeights[index] = NULL;
	groupCount = 0;
	materials = NULL;
	materialNames = NULL;
	indexCounts = NULL;
	vertexCounts = NULL;
	indices = NULL;
	//Bounds and natural alignment bytes are not initialized by4D7A20.
}

G3D_Mesh::~G3D_Mesh() //4D9130.
{
	Clear();
}

G3D_Vertex::G3D_Vertex() : textureCount(0), boneCount(0)
{
	memset(position, 0, sizeof(position));
	memset(normal, 0, sizeof(normal));
	memset(color, 0, sizeof(color));
	color[3] = 1.0f;
	memset(textureCoordinates, 0, sizeof(textureCoordinates));
}

void G3D_BoneWeight::Read(const BYTE* data, DWORD& offset) //4C8C60.
{
	LONG index;
	memcpy(&index, data + offset, sizeof(index));
	offset += sizeof(index);
	bone = index;
	float value;
	memcpy(&value, data + offset, sizeof(value));
	offset += sizeof(value);
	weight = value;
}

void G3D_Vertex::Read(const BYTE* data, DWORD& offset) //4C9FB0.
{
	double positionValue[3] = {0.0, 0.0, 0.0};
	double normalValue[3] = {0.0, 0.0, 0.0};
	G3D_ReadMemoryVector(data, positionValue, offset);
	memcpy(position, positionValue, sizeof(positionValue));
	G3D_ReadMemoryVector(data, normalValue, offset);
	memcpy(normal, normalValue, sizeof(normalValue));
	D3DXCOLOR colorValue;
	D3DXCOLOR* result = ReadMemoryColorWithAlpha(&colorValue, data, offset);
	memcpy(color, result, sizeof(colorValue));
	LONG count;
	memcpy(&count, data + offset, sizeof(count));
	offset += sizeof(count);
	textureCount = count;
	for (LONG index = 0; index < count; ++index)
	{
		memset(positionValue, 0, sizeof(double) * 2);
		ReadMemoryTextureCoordinate(data, positionValue, offset);
		memcpy(textureCoordinates[index], positionValue, sizeof(double) * 2);
	}
	memcpy(&count, data + offset, sizeof(count));
	offset += sizeof(count);
	boneCount = count;
	for (LONG index = 0; index < count; ++index)
	{
		boneWeights[index].Read(data, offset);
	}
}

void G3D_Vertex::Read(G3D_Input& input)
{
	Read(input.data, input.offset);
}

void G3D_Polygon::ReadMemoryProperties(const BYTE* data, DWORD& offset) //4D9A80.
{
	std::string temporary;
	DWORD value;
	memcpy(&value, data + offset, sizeof(value));
	offset += sizeof(value);
	value00 = value;
	G3D_ReadMemoryString(temporary, data, offset);
	material = temporary;
	LONG count;
	memcpy(&count, data + offset, sizeof(count));
	offset += sizeof(count);
	textureCount = count;
	for (LONG index = 0; index < count; ++index)
	{
		G3D_ReadMemoryString(temporary, data, offset);
		textures[index] = temporary;
	}
	memcpy(&count, data + offset, sizeof(count));
	offset += sizeof(count);
	for (LONG index = 0; index < count; ++index)
	{
		G3D_ReadMemoryString(temporary, data, offset);
		names.push_back(temporary);
	}
}

void G3D_Polygon::Read(const BYTE* data, DWORD& offset) //4D9C90.
{
	ReadMemoryProperties(data, offset);
	const BYTE tag = data[offset];
	++offset;
	primitive = static_cast<LONG>(tag) - 201;
	LONG count;
	memcpy(&count, data + offset, sizeof(count));
	offset += sizeof(count);
	indices.resize(count, 0);
	edgeFlags.resize(count, 0);
	for (LONG index = 0; index < count; ++index)
	{
		LONG value;
		memcpy(&value, data + offset, sizeof(value));
		offset += sizeof(value);
		indices[index] = value;
	}
	for (LONG index = 0; index < count; ++index)
	{
		const BYTE value = data[offset];
		++offset;
		edgeFlags[index] = value != 0;
	}
}

void G3D_Polygon::Read(G3D_Input& input)
{
	Read(input.data, input.offset);
}

void G3D_Mesh::Read(const BYTE* data, DWORD& offset) //4DC330; actual two-argument virtual memory ABI.
{
	Clear();
	G3D_Input input(data, offset);
	ReadName(data, offset); //4D0750 owns its temporary until the name is copied.
	BYTE version = owner->model->version;
	if (version)
	{
		converted = input.Read<BYTE>();
		if (!converted)
			JE::LogDebugString("G3D_Mesh::ReadB3D - Version is %d but G3D_Mesh is not converted!\n", version);
	}
	if (!version || !converted)
	{
		std::list<G3D_Vertex> staged;
		LONG count = input.Read<LONG>();
		for (LONG index = 0; index < count; ++index)
		{
			G3D_Vertex vertex;
			vertex.Read(data, offset);
			staged.push_back(vertex);
		}
		count = input.Read<LONG>();
		for (LONG index = 0; index < count; ++index)
		{
			G3D_Polygon polygon;
			polygon.Read(data, offset);
			AddPolygon(polygon);
		}
		legacyVertices.resize(staged.size(), G3D_Vertex());
		DWORD output = 0;
		for (std::list<G3D_Vertex>::const_iterator vertex = staged.begin(); vertex != staged.end(); ++vertex, ++output)
			legacyVertices[output] = *vertex;
		return;
	}
	vertexCount = input.Read<DWORD>();
	vertexStride = input.Read<DWORD>();
	vertices = new BYTE[vertexCount * vertexStride];
	input.Copy(vertices, vertexCount * vertexStride);
	input.Copy(minimum, sizeof(minimum));
	input.Copy(maximum, sizeof(maximum));
	hasBones = input.Read<BYTE>();
	if (hasBones)
	{
		if (input.Read<DWORD>() != 4)
		{
			JE::FatalError("G3D_Mesh::ReadB3D - Bone counts different - Need to re-export!!!\n");
		}
		for (DWORD bone = 0; bone < 4; ++bone)
		{
			boneWeights[bone] = new Gfx_G3D_BoneWeight[vertexCount];
			input.Copy(boneWeights[bone], 8 * vertexCount);
		}
	}
	groupCount = input.Read<DWORD>();
	materials = new G3D_MeshMaterial[groupCount];
	materialNames = new std::string[groupCount];
	vertexCounts = new DWORD[groupCount];
	indexCounts = new DWORD[groupCount];
	indices = new WORD*[groupCount];
	for (DWORD index = 0; index < groupCount; ++index) materials[index].Read(input);
	for (DWORD index = 0; index < groupCount; ++index) input.ReadString(materialNames[index]);
	input.Copy(indexCounts, 4 * groupCount);
	input.Copy(vertexCounts, 4 * groupCount);
	for (DWORD index = 0; index < groupCount; ++index)
	{
		indices[index] = new WORD[indexCounts[index]];
		input.Copy(indices[index], 2 * indexCounts[index]);
	}
}

void G3D_OcclusionZones::Read(const BYTE* data, DWORD& offset) //4D9830.
{
	std::string zoneName;
	LONG count;
	memcpy(&count, data + offset, sizeof(count));
	offset += sizeof(count);
	for (LONG i = 0; i < count; ++i)
	{
		G3D_ReadMemoryString(zoneName, data, offset);
		names.push_back(zoneName);
	}
}

