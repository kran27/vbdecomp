#include "G3DModel.hpp"
#include "TextParser.hpp"
#include "../diagnostics/DebugOutput.hpp"
#include "../model/CommonException.hpp"
#include <math.h>
#include <string.h>
#include <strstream>

//48F660; observed ordinary sqrt facade, implementation/support credit0.
double G3D_Sqrt(double value);

//4C8720: coordinates and color components use the shared numeric scale;
//bone indices and weights instead call the unscaled52BAF0 directly.
double G3D_ReadScaledNumber()
{
	return G3DText::ReadNumber() * G3D_CoordinateInverse;
}

D3DXCOLOR G3D_ReadFileColor(FILE* stream) //4C88F0.
{
	float value;
	fread(&value, 4, 1, stream);
	float red = value;
	fread(&value, 4, 1, stream);
	float green = value;
	fread(&value, 4, 1, stream);
	return D3DXCOLOR(red, green, value, 1.0f);
}

D3DXCOLOR G3D_ReadMemoryColor(const BYTE* data, DWORD& offset) //4C8970.
{
	float red, green, blue;
	memcpy(&red, data + offset, 4); offset += 4;
	memcpy(&green, data + offset, 4); offset += 4;
	memcpy(&blue, data + offset, 4); offset += 4;
	return D3DXCOLOR(red, green, blue, 1.0f);
}

void G3D_WriteFileColor(FILE* stream, const float* color) //4C8A80.
{
	float value = color[0]; fwrite(&value, 4, 1, stream);
	value = color[1]; fwrite(&value, 4, 1, stream);
	value = color[2]; fwrite(&value, 4, 1, stream);
}

void G3D_WriteTextColor(const float* color, FILE* stream) //4C8810; internal original register ABI is unclaimed.
{
	float red = color[0], green = color[1], blue = color[2];
	fprintf(stream, "(%g %g %g) ", G3D_TextNumber(red), G3D_TextNumber(green), G3D_TextNumber(blue));
}

void G3D_ReadFileVector(FILE* stream, double* output) //4C9400.
{
	float value;
	fread(&value, 4, 1, stream); output[0] = value;
	fread(&value, 4, 1, stream); output[1] = value;
	fread(&value, 4, 1, stream); output[2] = value;
}

void G3D_ReadMemoryVector(const BYTE* data, double* output, DWORD& offset) //4C9460.
{
	float value;
	memcpy(&value, data + offset, 4); offset += 4; output[0] = value;
	memcpy(&value, data + offset, 4); offset += 4; output[1] = value;
	memcpy(&value, data + offset, 4); offset += 4; output[2] = value;
}

void G3D_ReadTextTextureCoordinate(double* output) //4C9280.
{
	G3DText::Expect('(');
	output[0] = G3D_ReadScaledNumber();
	output[1] = G3D_ReadScaledNumber();
	G3DText::Expect(')');
}

void G3D_ReadTextVector(double* output) //4C92B0.
{
	G3DText::Expect('(');
	output[0] = G3D_ReadScaledNumber();
	output[1] = G3D_ReadScaledNumber();
	output[2] = G3D_ReadScaledNumber();
	G3DText::Expect(')');
}

void G3D_WriteFileVector(FILE* stream, const double* value) //4C94F0.
{
	float wire = static_cast<float>(value[0]); fwrite(&wire, 4, 1, stream);
	wire = static_cast<float>(value[1]); fwrite(&wire, 4, 1, stream);
	wire = static_cast<float>(value[2]); fwrite(&wire, 4, 1, stream);
}

void G3D_WriteTextTextureCoordinate(FILE* stream, const double* value) //4C92F0.
{
	fprintf(stream, "(%g %g) ", G3D_TextNumber(value[0]), G3D_TextNumber(value[1]));
}

void G3D_WriteTextVector(FILE* stream, const double* value) //4C9330.
{
	fprintf(stream, "(%.0f %.0f %.0f) ", G3D_TextNumber(value[0]), G3D_TextNumber(value[1]), G3D_TextNumber(value[2]));
}

//48FC80 is a member operation on the original three-double direction value.
void G3D_Vector3d::CheckUnitMagnitude()
{
	double delta = G3D_Sqrt(value[0] * value[0] + value[1] * value[1] + value[2] * value[2]) - 1.0;
	if (delta < 0.0) delta = -delta;
	if (!(delta < 0.00001)) JE::FatalError("UnitVector has invalid magnitude");
}

namespace
{
	void ReadTextVector(double* output, DWORD count)
	{
		G3DText::Expect('(');
		for (DWORD index = 0; index < count; ++index) output[index] = G3D_ReadScaledNumber();
		G3DText::Expect(')');
	}

	//4C9380; the final CRT count is returned without changing the float wire values.
	size_t ReadFileTextureCoordinate(FILE* stream, double* output)
	{
		float value;
		fread(&value, sizeof(value), 1, stream);
		output[0] = value;
		size_t result = fread(&value, sizeof(value), 1, stream);
		output[1] = value;
		return result;
	}

	//4C89E0; ordinary source arguments adapt the original EAX/EBX register API.
	D3DXCOLOR* ReadFileColorWithAlpha(D3DXCOLOR* output, FILE* stream)
	{
		D3DXCOLOR value = G3D_ReadFileColor(stream);
		float alpha;
		fread(&alpha, sizeof(alpha), 1, stream);
		memcpy(&value.a, &alpha, sizeof(alpha));
		memcpy(output, &value, sizeof(value));
		return output;
	}

	//48FDA0/48FC80: the original unit-vector conversion keeps the magnitude
	//postcondition, including its error for a zero or otherwise invalid normal.
	void Normalize(double* output)
	{
		double squared = output[0] * output[0] + output[1] * output[1] + output[2] * output[2];
		double inverse = squared == 0.0 ? 0.0 : 1.0 / sqrt(squared);
		output[0] *= inverse;
		output[1] *= inverse;
		output[2] *= inverse;
		double magnitude = sqrt(output[0] * output[0] + output[1] * output[1] + output[2] * output[2]);
		if (fabs(magnitude - 1.0) >= 0.00001)
			JE::FatalError("UnitVector has invalid magnitude");
	}
}

//4D0450. WORD length and ignored CRT read results are original policies.
//The temporary terminator deliberately makes embedded NUL truncate the string.
void G3D_ReadFileString(std::string& output, FILE* stream)
{
	output.assign("");
	WORD length;
	fread(&length, sizeof(length), 1, stream);
	if (length > 0)
	{
		output.resize(length + 1);
		char* value = new char[length + 1];
		fread(value, 1, length, stream);
		value[length] = 0;
		output.assign(value, strlen(value));
		delete[] value;
	}
}

//4D06B0 uses a temporary string before assigning the attribute's name.
void G3D_BaseAttribute::ReadName(FILE* stream)
{
	std::string value;
	G3D_ReadFileString(value, stream);
	name = value;
}

void G3D_BaseAttribute::ReadName(const BYTE* data, DWORD& offset) //4D0750.
{
	std::string value;
	G3D_ReadMemoryString(value, data, offset);
	name = value;
}

//4C8C20. VC7 reuses the dead FILE argument slot for the raw index;
//unread bytes retain its incoming bits in this pinned emission.
size_t G3D_BoneWeight::Read(FILE* stream)
{
	LONG index;
	fread(&index, sizeof(index), 1, stream);
	bone = index;
	float value;
	size_t result = fread(&value, sizeof(value), 1, stream);
	weight = value;
	return result;
}

//4C8BC0; the index and weight use unscaled numeric tokens.
void G3D_BoneWeight::ReadText()
{
	G3DText::Expect('(');
	bone = static_cast<LONG>(G3DText::ReadNumber());
	weight = G3DText::ReadNumber();
	G3DText::Expect(')');
}

//4C8BF0; preserve the double weight and exact authored trailing space.
void G3D_BoneWeight::WriteText(FILE* stream)
{
	fprintf(stream, "%s(%d %f) ", "Bone", bone, weight);
}

//4C9E00. Separate staged vectors, color/UV values and captured signed counts
//precede publication to the actual record. Fixed UV/bone arrays stay unchecked.
void G3D_Vertex::Read(FILE* stream)
{
	double positionValue[3] = {0.0, 0.0, 0.0};
	double normalValue[3] = {0.0, 0.0, 0.0};
	G3D_ReadFileVector(stream, positionValue);
	memcpy(position, positionValue, sizeof(position));
	G3D_ReadFileVector(stream, normalValue);
	memcpy(normal, normalValue, sizeof(normal));
	D3DXCOLOR components;
	D3DXCOLOR* value = ReadFileColorWithAlpha(&components, stream);
	memcpy(color, value, sizeof(color));
	LONG count;
	fread(&count, sizeof(count), 1, stream);
	textureCount = count;
	for (LONG texture = 0; texture < count; ++texture)
	{
		double coordinates[2] = {0.0, 0.0};
		ReadFileTextureCoordinate(stream, coordinates);
		memcpy(textureCoordinates[texture], coordinates, sizeof(coordinates));
	}
	fread(&count, sizeof(count), 1, stream);
	boneCount = count;
	for (LONG bone = 0; bone < count; ++bone)
		boneWeights[bone].Read(stream);
}

//4CC620. Token+4 is not initialized by the scanner wrapper; diagnostics use
//the parser's actual input line rather than reading that unwritten field.
void G3D_Vertex::ReadText()
{
	G3DText::Expect('{');
	textureCount = 0;
	boneCount = 0;
	G3DText::Token token = G3DText::NextToken();
	while (token.kind != '}')
	{
		if (token.kind != 256) token.ParseError();
		std::string key = G3DText::TokenString(token.value.identifier);
		if (key == "Coordinate")
		{
			double value[3] = {0.0, 0.0, 0.0};
			ReadTextVector(value, 3);
			value[0] *= G3D_GlobalScale;
			value[1] *= G3D_GlobalScale;
			value[2] *= G3D_GlobalScale;
			memcpy(position, value, sizeof(position));
		}
		else if (key == "Normal")
		{
			double value[3] = {0.0, 0.0, 0.0};
			ReadTextVector(value, 3);
			Normalize(value);
			memcpy(normal, value, sizeof(normal));
		}
		else if (key == "Color")
		{
			float value[4];
			G3DText::Expect('(');
			value[0] = static_cast<float>(G3D_ReadScaledNumber());
			value[1] = static_cast<float>(G3D_ReadScaledNumber());
			value[2] = static_cast<float>(G3D_ReadScaledNumber());
			value[3] = static_cast<float>(G3D_ReadScaledNumber());
			G3DText::Expect(')');
			memcpy(color, value, sizeof(color));
		}
		else if (key == "TexCoord" || key == "LMCoord")
		{
			if (textureCount >= 2)
			{
				std::strstream error;
				error << "Exceeded TexCoord count limit " << 2 << std::endl;
				throw Common::Exception(error.str());
			}
			double value[2] = {0.0, 0.0};
			G3D_ReadTextTextureCoordinate(value);
			SetTextureCoordinate(textureCount, value);
			++textureCount;
		}
		else if (key == "Bone")
		{
			if (boneCount >= 2)
			{
				std::strstream error;
				error << "Exceeded Bone Influence count limit " << 2 << std::endl;
				throw Common::Exception(error.str());
			}
			G3D_BoneWeight value;
			value.ReadText();
			SetBoneWeight(boneCount, value);
			++boneCount;
		}
		else token.ParseError();
		token = G3DText::NextToken();
	}
}

//4D46C0 releases the zone names, retaining the other polygon properties.
void G3D_Polygon::ClearNames()
{
	names.clear();
}

//4D9930. Both serialized counts are signed, and existing zone names append.
void G3D_Polygon::ReadFileProperties(FILE* stream)
{
	LONG value;
	std::string text;
	fread(&value, sizeof(value), 1, stream);
	value00 = value;
	G3D_ReadFileString(text, stream);
	material = text;
	fread(&value, sizeof(value), 1, stream);
	textureCount = value;
	for (LONG texture = 0; texture < value; ++texture)
	{
		G3D_ReadFileString(text, stream);
		textures[texture] = text;
	}
	fread(&value, sizeof(value), 1, stream);
	if (value > 0)
		for (LONG zone = 0; zone < value; ++zone)
		{
			G3D_ReadFileString(text, stream);
			names.push_back(text);
		}
}

//4D9BB0. Only the read byte participates in primitiveTag-201.
void G3D_Polygon::Read(FILE* stream)
{
	ReadFileProperties(stream);
	BYTE tag;
	fread(&tag, sizeof(tag), 1, stream);
	primitive = static_cast<LONG>(tag) - 201;
	LONG count;
	fread(&count, sizeof(count), 1, stream);
	const LONG resizeCount = count;
	indices.resize(resizeCount, 0);
	edgeFlags.resize(resizeCount, 0);
	for (LONG index = 0; index < count; ++index)
	{
		LONG value;
		fread(&value, sizeof(value), 1, stream);
		indices[index] = value;
	}
	for (LONG edge = 0; edge < count; ++edge)
	{
		BYTE value;
		fread(&value, sizeof(value), 1, stream);
		edgeFlags[edge] = value != 0;
	}
}

//4D8780 deliberately ignores nonidentifier outer tokens and nonnumeric list
//tokens. Index/Edge each resize both vectors before filling its own channel.
void G3D_Polygon::ReadTextIndices()
{
	G3DText::Expect('{');
	G3DText::Token token = G3DText::NextToken();
	while (token.kind != '}')
	{
		if (token.kind == 256)
		{
			std::string key = G3DText::TokenString(token.value.identifier);
			if (key == "Index")
			{
				G3DText::Expect('(');
				std::list<LONG> values;
				G3DText::Token value = G3DText::NextToken();
				while (value.kind != ')')
				{
					if (value.kind == 257) values.push_back(static_cast<LONG>(value.value.number));
					value = G3DText::NextToken();
				}
				indices.resize(values.size(), 0);
				edgeFlags.resize(values.size(), 0);
				DWORD index = 0;
				for (std::list<LONG>::const_iterator item = values.begin(); item != values.end(); ++item, ++index)
					indices[index] = *item;
			}
			else
			{
				if (key != "Edge") token.ParseError();
				G3DText::Expect('(');
				std::list<LONG> values;
				G3DText::Token value = G3DText::NextToken();
				while (value.kind != ')')
				{
					if (value.kind == 257) values.push_back(static_cast<LONG>(value.value.number));
					value = G3DText::NextToken();
				}
				indices.resize(values.size(), 0);
				edgeFlags.resize(values.size(), 0);
				DWORD index = 0;
				for (std::list<LONG>::const_iterator item = values.begin(); item != values.end(); ++item, ++index)
					edgeFlags[index] = *item != 0;
			}
		}
		token = G3DText::NextToken();
	}
}

void G3D_Name::Write(FILE* stream) const //4CE940.
{
	G3D_WriteFileString(*this, stream);
}

void G3D_BaseAttribute::WriteName(FILE* stream) //4CE980.
{
	G3D_WriteFileString(name.Value(), stream);
}

extern LONG G3D_TextIndentation; //707F38; shared original text-writer state.

void G3D_OpenTextBlock(FILE* stream) //4C8B30.
{
	++G3D_TextIndentation;
	fprintf(stream, "\n");
	G3D_WriteIndent(stream);
	fprintf(stream, "{\n");
}

void G3D_CloseTextBlock(FILE* stream) //4C8B70.
{
	G3D_WriteIndent(stream);
	fprintf(stream, "}\n");
	--G3D_TextIndentation;
}

void G3D_Name::WriteText(FILE* stream, BYTE newline) const //4CA8C0.
{
	if (size() > 0)
	{
		if (newline) G3D_WriteIndent(stream);
		fprintf(stream, "%s(\"%s\")", "Name", c_str());
		if (newline) fprintf(stream, "\n");
		else fprintf(stream, " ");
	}
}

void G3D_Vertex::Write(FILE* stream) //4CA150.
{
	double vector[3];
	memcpy(vector, position, sizeof(vector));
	G3D_WriteFileVector(stream, vector);
	memcpy(vector, normal, sizeof(vector));
	G3D_WriteFileVector(stream, vector);
	for (DWORD component = 0; component < 4; ++component)
	{
		float value;
		memcpy(&value, &color[component], sizeof(value));
		fwrite(&value, 4, 1, stream);
	}
	LONG count = textureCount;
	fwrite(&count, 4, 1, stream);
	for (LONG texture = 0; texture < textureCount; ++texture)
	{
		double coordinate[2];
		memcpy(coordinate, textureCoordinates[texture], sizeof(coordinate));
		G3D_WriteFileTextureCoordinate(stream, coordinate);
	}
	count = boneCount;
	fwrite(&count, 4, 1, stream);
	for (LONG bone = 0; bone < boneCount; ++bone)
		boneWeights[bone].Write(stream);
}

void G3D_Vertex::WriteText(FILE* stream) //4C9CE0.
{
	G3D_WriteIndent(stream);
	fprintf(stream, "%s{ ", "Vertex");
	fprintf(stream, "Coordinate");
	fprintf(stream, "(%.0f %.0f %.0f) ", G3D_TextNumber(position[0]), G3D_TextNumber(position[1]), G3D_TextNumber(position[2]));
	fprintf(stream, "Normal");
	fprintf(stream, "(%.0f %.0f %.0f) ", G3D_TextNumber(normal[0]), G3D_TextNumber(normal[1]), G3D_TextNumber(normal[2]));
	fprintf(stream, "Color");
	fprintf(stream, "(%g %g %g %g) ", G3D_TextNumber(color[0]), G3D_TextNumber(color[1]), G3D_TextNumber(color[2]), G3D_TextNumber(color[3]));
	for (LONG texture = 0; texture < textureCount; ++texture)
	{
		if (texture) fprintf(stream, "LMCoord");
		else fprintf(stream, "TexCoord");
		double coordinate[2];
		memcpy(coordinate, textureCoordinates[texture], sizeof(coordinate));
		G3D_WriteTextTextureCoordinate(stream, coordinate);
	}
	for (LONG bone = 0; bone < boneCount; ++bone)
		boneWeights[bone].WriteText(stream);
	fprintf(stream, "}\n");
}

void G3D_Polygon::WriteFileProperties(FILE* stream) //4D29F0.
{
	LONG count = value00;
	fwrite(&count, 4, 1, stream);
	std::string materialName = MaterialValue();
	if (materialName.size() <= 0) materialName = "NULL_MATERIAL";
	G3D_WriteFileString(materialName, stream);
	count = textureCount;
	fwrite(&count, 4, 1, stream);
	if (count > 0)
	{
		for (LONG texture = 0; texture < count; ++texture)
		{
			std::string textureName = TextureValue(texture);
			if (textureName.size() <= 0) textureName = "NULL_TEXTURE";
			G3D_WriteFileString(textureName, stream);
		}
	}
	count = static_cast<LONG>(names.size());
	fwrite(&count, 4, 1, stream);
	if (count > 0)
	{
		for (LONG zone = 0; zone < count; ++zone)
		{
			std::string zoneName = GetName(zone);
			G3D_WriteFileString(zoneName, stream);
		}
	}
}

void G3D_Polygon::WriteTextProperties(FILE* stream) //4D2770.
{
	LONG materialIndex = value00;
	std::string materialName = MaterialValue();
	if (materialName.size() <= 0) materialName = "NULL_MATERIAL";
	G3D_WriteIndent(stream);
	fprintf(stream, "%s(%d \"%s\")\n", "Material", materialIndex, materialName.c_str());
	if (static_cast<LONG>(textureCount) > 0)
	{
		for (LONG texture = 0; texture < static_cast<LONG>(textureCount); ++texture)
		{
			std::string textureName = TextureValue(texture);
			if (textureName.size() <= 0) textureName = "NULL_TEXTURE";
			G3D_WriteIndent(stream);
			if (texture == 0)
				fprintf(stream, "%s(\"%s\")\n", "Texture", textureName.c_str());
			else
				fprintf(stream, "%s(\"%s\")\n", "Lightmap", textureName.c_str());
		}
	}
	if (static_cast<LONG>(names.size()) > 0)
	{
		G3D_WriteIndent(stream);
		fprintf(stream, "%s{ ", "OcclusionZones");
		for (LONG zone = 0; zone < static_cast<LONG>(names.size()); ++zone)
		{
			std::string zoneName = GetName(zone);
			fprintf(stream, "%s(\"%s\") ", "Zone", zoneName.c_str());
		}
		fprintf(stream, "}\n");
	}
}

void G3D_Polygon::Write(FILE* stream) //4D2DA0.
{
	WriteFileProperties(stream);
	BYTE tag = static_cast<BYTE>(primitive + 201);
	fwrite(&tag, 1, 1, stream);
	LONG count = static_cast<LONG>(indices.size());
	fwrite(&count, 4, 1, stream);
	for (LONG index = 0; index < static_cast<LONG>(indices.size()); ++index)
	{
		count = indices[index];
		fwrite(&count, 4, 1, stream);
	}
	for (LONG edge = 0; edge < static_cast<LONG>(indices.size()); ++edge)
	{
		BYTE value = edgeFlags[edge];
		fwrite(&value, 1, 1, stream);
	}
}

void G3D_Polygon::WriteText(FILE* stream) //4D2C70.
{
	static const char* const primitives[5] = {"Triangles", "TriangleStrip", "TriangleFan", "Polygon", "BezierPatch"};
	WriteTextProperties(stream);
	G3D_WriteIndent(stream);
	fprintf(stream, "%s{%s( ", primitives[primitive], "Index");
	for (LONG index = 0; index < static_cast<LONG>(indices.size()); ++index)
	{
		fprintf(stream, "%d ", indices[index]);
		if (index % 32 == 31)
		{
			fprintf(stream, "\n");
			G3D_WriteIndent(stream);
			fprintf(stream, "  ");
		}
	}
	fprintf(stream, ") %s( ", "Edge");
	for (LONG edge = 0; edge < static_cast<LONG>(indices.size()); ++edge)
	{
		fprintf(stream, "%d ", edgeFlags[edge]);
		if (edge % 32 == 31)
		{
			fprintf(stream, "\n");
			G3D_WriteIndent(stream);
			fprintf(stream, "  ");
		}
	}
	fprintf(stream, ")}\n");
}
void G3D_Vertex::SetTextureCoordinate(LONG index, const double* value) //4C85B0.
{
	memcpy(textureCoordinates[index], value, sizeof(textureCoordinates[index]));
}

void G3D_Vertex::SetBoneWeight(LONG index, const G3D_BoneWeight& value) //4C85E0.
{
	boneWeights[index] = value;
}
