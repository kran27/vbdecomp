#include "G3DLightAttributes.hpp"
#include "TextParser.hpp"
#include <stddef.h>
#include <string.h>

#if defined(_MSC_VER) && _MSC_VER == 1300 && defined(_M_IX86)
#define G3D_LIGHT_OFFSET(type, member, value) typedef char type##_##member##_Offset[offsetof(type, member) == value ? 1 : -1]
#define G3D_LIGHT_SIZE(type, value) typedef char type##_Extent[sizeof(type) == value ? 1 : -1]
G3D_LIGHT_SIZE(G3D_BaseLight, 0x60);
G3D_LIGHT_OFFSET(G3D_BaseLight, ambient, 0x24);
G3D_LIGHT_OFFSET(G3D_BaseLight, diffuse, 0x34);
G3D_LIGHT_OFFSET(G3D_BaseLight, specular, 0x44);
G3D_LIGHT_OFFSET(G3D_BaseLight, intensity, 0x58);
G3D_LIGHT_SIZE(G3D_DirectionalLight, 0x78);
G3D_LIGHT_OFFSET(G3D_DirectionalLight, direction, 0x60);
G3D_LIGHT_SIZE(G3D_PointLight, 0x88);
G3D_LIGHT_OFFSET(G3D_PointLight, attenuation, 0x60);
G3D_LIGHT_OFFSET(G3D_PointLight, radius, 0x78);
G3D_LIGHT_OFFSET(G3D_PointLight, falloff, 0x80);
G3D_LIGHT_SIZE(G3D_SpotLight, 0xC0);
G3D_LIGHT_OFFSET(G3D_SpotLight, direction, 0x88);
G3D_LIGHT_OFFSET(G3D_SpotLight, spotAngle, 0xA0);
G3D_LIGHT_OFFSET(G3D_SpotLight, spotSoftAngle, 0xA8);
G3D_LIGHT_OFFSET(G3D_SpotLight, valueB0, 0xB0);
G3D_LIGHT_OFFSET(G3D_SpotLight, valueB8, 0xB8);
G3D_LIGHT_SIZE(G3D_Camera, 0x40);
G3D_LIGHT_OFFSET(G3D_Camera, nearDistance, 0x28);
G3D_LIGHT_OFFSET(G3D_Camera, farDistance, 0x30);
G3D_LIGHT_OFFSET(G3D_Camera, fieldOfView, 0x38);
#undef G3D_LIGHT_OFFSET
#undef G3D_LIGHT_SIZE
#endif

G3D_BaseLight::G3D_BaseLight() //4CB520.
{
	ambient = D3DXCOLOR(0.0f, 0.0f, 0.0f, 1.0f);
	diffuse = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);
	specular = D3DXCOLOR(0.0f, 0.0f, 0.0f, 1.0f);
	intensity = 0.0;
}

void G3D_BaseLight::ReadText() //404B70, original empty base hook.
{
}

void G3D_BaseLight::WriteText(FILE* stream) //5A9320, original empty base hook.
{
}

G3D_DirectionalLight::G3D_DirectionalLight() //4CB5E0.
{
	G3D_Vector3d& ownedDirection = direction;
	ownedDirection.value[0] = 0.0;
	ownedDirection.value[1] = 0.0;
	ownedDirection.value[2] = 0.0;
	G3D_Vector3d value = {0.0, 0.0, -1.0};
	value.CheckUnitMagnitude();
	ownedDirection = value;
}

G3D_PointLight::G3D_PointLight() : radius(1.0) //4CB740.
{
	attenuation[0] = 1.0;
	memset(&attenuation[1], 0, 2 * sizeof(attenuation[1]));
	falloff = 0;
}

G3D_SpotLight::G3D_SpotLight() //4CB890.
{
	G3D_Vector3d& ownedDirection = direction;
	ownedDirection.value[2] = 0.0;
	ownedDirection.value[1] = 0.0;
	ownedDirection.value[0] = 0.0;
	G3D_Vector3d value = {0.0, 0.0, -1.0};
	value.CheckUnitMagnitude();
	ownedDirection = value;
	valueB0 = 45.0;
	valueB8 = 0.0;
}

G3D_Camera::G3D_Camera() : nearDistance(-1.0), farDistance(-1.0), fieldOfView(-1.0) //4CBAB0.
{
}

void G3D_DirectionalLight::ReadText() //4D5EA0.
{
	G3DText::Expect('{');
	for (G3DText::Token token = G3DText::NextToken(); token.kind != '}'; token = G3DText::NextToken())
	{
		if (token.kind == 256)
		{
			std::string key = G3DText::TokenString(token.value.identifier);
			if (key.compare(0, key.size(), "Name") == 0) name.ReadText();
			else if (key == "Ambient") ambient = G3D_ReadTextColor();
			else if (key == "Diffuse") diffuse = G3D_ReadTextColor();
			else if (key == "Specular") specular = G3D_ReadTextColor();
			else if (key == "Intensity")
			{
				G3DText::Expect('('); intensity = G3DText::ReadNumber(); G3DText::Expect(')');
			}
			else token.ParseError();
		}
		else token.ParseError();
	}
}

void G3D_PointLight::ReadText() //4D6150.
{
	G3DText::Expect('{');
	for (G3DText::Token token = G3DText::NextToken(); token.kind != '}'; token = G3DText::NextToken())
	{
		if (token.kind == 256)
		{
			std::string key = G3DText::TokenString(token.value.identifier);
			if (key == "Name") name.ReadText();
			else if (key == "Ambient") ambient = G3D_ReadTextColor();
			else if (key == "Diffuse") diffuse = G3D_ReadTextColor();
			else if (key == "Specular") specular = G3D_ReadTextColor();
			else if (key == "Intensity")
			{
				G3DText::Expect('('); intensity = G3DText::ReadNumber(); G3DText::Expect(')');
			}
			else if (key == "Radius")
			{
				G3DText::Expect('('); radius = G3D_ReadScaledNumber(); G3DText::Expect(')');
			}
			else if (key == "Falloff")
			{
				G3DText::Expect('('); falloff = static_cast<LONG>(G3DText::ReadNumber()); G3DText::Expect(')');
			}
			else if (key == "Attenuation")
			{
				G3DText::Expect('(');
				attenuation[0] = G3DText::ReadNumber();
				attenuation[1] = G3DText::ReadNumber();
				attenuation[2] = G3DText::ReadNumber();
				G3DText::Expect(')');
			}
			else token.ParseError();
		}
		else token.ParseError();
	}
}

void G3D_SpotLight::ReadText() //4D64E0.
{
	G3DText::Expect('{');
	for (G3DText::Token token = G3DText::NextToken(); token.kind != '}'; token = G3DText::NextToken())
	{
		if (token.kind == 256)
		{
			std::string key = G3DText::TokenString(token.value.identifier);
			if (key == "Name") name.ReadText();
			else if (key == "Ambient") ambient = G3D_ReadTextColor();
			else if (key == "Diffuse") diffuse = G3D_ReadTextColor();
			else if (key == "Specular") specular = G3D_ReadTextColor();
			else if (key == "Intensity")
			{
				G3DText::Expect('('); intensity = G3DText::ReadNumber(); G3DText::Expect(')');
			}
			else if (key == "Radius")
			{
				G3DText::Expect('('); radius = G3D_ReadScaledNumber(); G3DText::Expect(')');
			}
			else if (key == "Falloff")
			{
				G3DText::Expect('('); falloff = static_cast<LONG>(G3DText::ReadNumber()); G3DText::Expect(')');
			}
			else if (key == "Attenuation")
			{
				G3DText::Expect('(');
				attenuation[0] = G3DText::ReadNumber();
				attenuation[1] = G3DText::ReadNumber();
				attenuation[2] = G3DText::ReadNumber();
				G3DText::Expect(')');
			}
			else if (key == "SpotDir")
			{
				G3D_Vector3d value = {0.0, 0.0, 0.0};
				G3D_ReadTextVector(value.value);
				direction = value;
			}
			else if (key == "SpotAngle")
			{
				G3DText::Expect('('); spotAngle = G3DText::ReadNumber(); G3DText::Expect(')');
			}
			else if (key == "SpotSoftAngle")
			{
				G3DText::Expect('('); spotSoftAngle = G3DText::ReadNumber(); G3DText::Expect(')');
			}
			else token.ParseError();
		}
		else token.ParseError();
	}
}

void G3D_Camera::ReadText() //4D6940.
{
	G3DText::Expect('{');
	G3DText::Token token;
	for (token = G3DText::NextToken(); token.kind != '}'; token = G3DText::NextToken())
	{
		if (token.kind == 256)
		{
			std::string key = G3DText::TokenString(token.value.identifier);
			if (key.compare(0, key.size(), "Name") == 0) name.ReadText();
			else if (key == "Fov")
			{
				G3DText::Expect('('); fieldOfView = G3DText::ReadNumber(); G3DText::Expect(')');
			}
			else if (key == "Near")
			{
				G3DText::Expect('('); nearDistance = G3D_ReadScaledNumber(); G3DText::Expect(')');
			}
			else if (key == "Far")
			{
				G3DText::Expect('('); farDistance = G3D_ReadScaledNumber(); G3DText::Expect(')');
			}
			else token.ParseError();
		}
		else token.ParseError();
	}
}

void G3D_DirectionalLight::Read(FILE* stream) //4D1AC0.
{
	ReadName(stream);
	ambient = G3D_ReadFileColor(stream);
	diffuse = G3D_ReadFileColor(stream);
	specular = G3D_ReadFileColor(stream);
	float value;
	fread(&value, sizeof(value), 1, stream); intensity = value;
}

void G3D_PointLight::Read(FILE* stream) //4D1C10.
{
	ReadName(stream);
	ambient = G3D_ReadFileColor(stream);
	diffuse = G3D_ReadFileColor(stream);
	specular = G3D_ReadFileColor(stream);
	float value;
	fread(&value, sizeof(value), 1, stream); intensity = value;
	fread(&value, sizeof(value), 1, stream); radius = value;
	fread(&value, sizeof(value), 1, stream); falloff = static_cast<LONG>(value);
	fread(&value, sizeof(value), 1, stream); attenuation[0] = value;
	fread(&value, sizeof(value), 1, stream); attenuation[1] = value;
	fread(&value, sizeof(value), 1, stream); attenuation[2] = value;
}

void G3D_SpotLight::Read(FILE* stream) //4D1E50.
{
	G3D_Vector3d vector = {0.0, 0.0, 0.0};
	ReadName(stream);
	ambient = G3D_ReadFileColor(stream);
	diffuse = G3D_ReadFileColor(stream);
	specular = G3D_ReadFileColor(stream);
	float value;
	fread(&value, sizeof(value), 1, stream); intensity = value;
	fread(&value, sizeof(value), 1, stream); radius = value;
	fread(&value, sizeof(value), 1, stream); falloff = static_cast<LONG>(value);
	fread(&value, sizeof(value), 1, stream); attenuation[0] = value;
	fread(&value, sizeof(value), 1, stream); attenuation[1] = value;
	fread(&value, sizeof(value), 1, stream); attenuation[2] = value;
	G3D_ReadFileVector(stream, vector.value); direction = vector;
	fread(&value, sizeof(value), 1, stream); spotAngle = value;
	fread(&value, sizeof(value), 1, stream); spotSoftAngle = value;
}

void G3D_Camera::Read(FILE* stream) //4D21A0.
{
	ReadName(stream);
	float value;
	fread(&value, sizeof(value), 1, stream); fieldOfView = value;
	fread(&value, sizeof(value), 1, stream); nearDistance = value;
	fread(&value, sizeof(value), 1, stream); farDistance = value;
}

void G3D_DirectionalLight::Read(const BYTE* data, DWORD& offset) //4D1B60.
{
	ReadName(data, offset);
	ambient = G3D_ReadMemoryColor(data, offset);
	diffuse = G3D_ReadMemoryColor(data, offset);
	specular = G3D_ReadMemoryColor(data, offset);
	float value;
	memcpy(&value, data + offset, sizeof(value)); offset += sizeof(value); intensity = value;
}

void G3D_PointLight::Read(const BYTE* data, DWORD& offset) //4D1D30.
{
	ReadName(data, offset);
	ambient = G3D_ReadMemoryColor(data, offset);
	diffuse = G3D_ReadMemoryColor(data, offset);
	specular = G3D_ReadMemoryColor(data, offset);
	float value;
	memcpy(&value, data + offset, sizeof(value)); offset += sizeof(value); intensity = value;
	memcpy(&value, data + offset, sizeof(value)); offset += sizeof(value); radius = value;
	memcpy(&value, data + offset, sizeof(value)); offset += sizeof(value); falloff = static_cast<LONG>(value);
	memcpy(&value, data + offset, sizeof(value)); offset += sizeof(value); attenuation[0] = value;
	memcpy(&value, data + offset, sizeof(value)); offset += sizeof(value); attenuation[1] = value;
	memcpy(&value, data + offset, sizeof(value)); offset += sizeof(value); attenuation[2] = value;
}

void G3D_SpotLight::Read(const BYTE* data, DWORD& offset) //4D1FF0.
{
	G3D_Vector3d vector = {0.0, 0.0, 0.0};
	ReadName(data, offset);
	ambient = G3D_ReadMemoryColor(data, offset);
	diffuse = G3D_ReadMemoryColor(data, offset);
	specular = G3D_ReadMemoryColor(data, offset);
	float value;
	memcpy(&value, data + offset, sizeof(value)); offset += sizeof(value); intensity = value;
	memcpy(&value, data + offset, sizeof(value)); offset += sizeof(value); radius = value;
	memcpy(&value, data + offset, sizeof(value)); offset += sizeof(value); falloff = static_cast<LONG>(value);
	memcpy(&value, data + offset, sizeof(value)); offset += sizeof(value); attenuation[0] = value;
	memcpy(&value, data + offset, sizeof(value)); offset += sizeof(value); attenuation[1] = value;
	memcpy(&value, data + offset, sizeof(value)); offset += sizeof(value); attenuation[2] = value;
	G3D_ReadMemoryVector(data, vector.value, offset); direction = vector;
	memcpy(&value, data + offset, sizeof(value)); offset += sizeof(value); spotAngle = value;
	memcpy(&value, data + offset, sizeof(value)); offset += sizeof(value); spotSoftAngle = value;
}

void G3D_Camera::Read(const BYTE* data, DWORD& offset) //4D2200.
{
	ReadName(data, offset);
	float value;
	memcpy(&value, data + offset, sizeof(value)); offset += sizeof(value); fieldOfView = value;
	memcpy(&value, data + offset, sizeof(value)); offset += sizeof(value); nearDistance = value;
	memcpy(&value, data + offset, sizeof(value)); offset += sizeof(value); farDistance = value;
}

void G3D_DirectionalLight::Write(FILE* stream) //4CEC30.
{
	BYTE type = 18;
	fwrite(&type, sizeof(type), 1, stream);
	WriteName(stream);
	G3D_WriteFileColor(stream, ambient);
	G3D_WriteFileColor(stream, diffuse);
	G3D_WriteFileColor(stream, specular);
	float value = static_cast<float>(intensity); fwrite(&value, sizeof(value), 1, stream);
}

void G3D_PointLight::Write(FILE* stream) //4CECA0.
{
	BYTE type = 16;
	fwrite(&type, sizeof(type), 1, stream);
	WriteName(stream);
	G3D_WriteFileColor(stream, ambient);
	G3D_WriteFileColor(stream, diffuse);
	G3D_WriteFileColor(stream, specular);
	float value = static_cast<float>(intensity); fwrite(&value, sizeof(value), 1, stream);
	value = static_cast<float>(radius); fwrite(&value, sizeof(value), 1, stream);
	value = static_cast<float>(falloff); fwrite(&value, sizeof(value), 1, stream);
	value = static_cast<float>(attenuation[0]); fwrite(&value, sizeof(value), 1, stream);
	value = static_cast<float>(attenuation[1]); fwrite(&value, sizeof(value), 1, stream);
	value = static_cast<float>(attenuation[2]); fwrite(&value, sizeof(value), 1, stream);
}

void G3D_SpotLight::Write(FILE* stream) //4CED80.
{
	BYTE type = 17;
	fwrite(&type, sizeof(type), 1, stream);
	WriteName(stream);
	G3D_WriteFileColor(stream, ambient);
	G3D_WriteFileColor(stream, diffuse);
	G3D_WriteFileColor(stream, specular);
	float value = static_cast<float>(intensity); fwrite(&value, sizeof(value), 1, stream);
	value = static_cast<float>(radius); fwrite(&value, sizeof(value), 1, stream);
	value = static_cast<float>(falloff); fwrite(&value, sizeof(value), 1, stream);
	value = static_cast<float>(attenuation[0]); fwrite(&value, sizeof(value), 1, stream);
	value = static_cast<float>(attenuation[1]); fwrite(&value, sizeof(value), 1, stream);
	value = static_cast<float>(attenuation[2]); fwrite(&value, sizeof(value), 1, stream);
	G3D_Vector3d vector = direction;
	G3D_WriteFileVector(stream, vector.value);
	value = static_cast<float>(spotAngle); fwrite(&value, sizeof(value), 1, stream);
	value = static_cast<float>(spotSoftAngle); fwrite(&value, sizeof(value), 1, stream);
}

void G3D_Camera::Write(FILE* stream) //4CEEB0.
{
	BYTE type = 19;
	fwrite(&type, sizeof(type), 1, stream);
	WriteName(stream);
	float value = static_cast<float>(fieldOfView); fwrite(&value, sizeof(value), 1, stream);
	value = static_cast<float>(nearDistance); fwrite(&value, sizeof(value), 1, stream);
	value = static_cast<float>(farDistance); fwrite(&value, sizeof(value), 1, stream);
}

void G3D_DirectionalLight::WriteText(FILE* stream) //4CB6A0.
{
	G3D_WriteIndent(stream);
	fprintf(stream, "%s{ ", "DirectionalLight");
	name.WriteText(stream, 0);
	fprintf(stream, "Ambient"); G3D_WriteTextColor(ambient, stream);
	fprintf(stream, "Diffuse"); G3D_WriteTextColor(diffuse, stream);
	fprintf(stream, "Specular"); G3D_WriteTextColor(specular, stream);
	fprintf(stream, "%s(%g) ", "Intensity", intensity);
	fprintf(stream, "}\n");
}

void G3D_PointLight::WriteText(FILE* stream) //4CB780.
{
	G3D_WriteIndent(stream);
	fprintf(stream, "%s{ ", "PointLight");
	name.WriteText(stream, 0);
	fprintf(stream, "Ambient"); G3D_WriteTextColor(ambient, stream);
	fprintf(stream, "Diffuse"); G3D_WriteTextColor(diffuse, stream);
	fprintf(stream, "Specular"); G3D_WriteTextColor(specular, stream);
	fprintf(stream, "%s(%g) ", "Intensity", intensity);
	fprintf(stream, "%s(%g) ", "Radius", G3D_TextNumber(radius));
	fprintf(stream, "%s(%d) ", "Falloff", falloff);
	fprintf(stream, "%s(%g %g %g) ", "Attenuation", attenuation[0], attenuation[1], attenuation[2]);
	fprintf(stream, "}\n");
}

void G3D_SpotLight::WriteText(FILE* stream) //4CB950.
{
	G3D_WriteIndent(stream);
	fprintf(stream, "%s{ ", "SpotLight");
	name.WriteText(stream, 0);
	fprintf(stream, "Ambient"); G3D_WriteTextColor(ambient, stream);
	fprintf(stream, "Diffuse"); G3D_WriteTextColor(diffuse, stream);
	fprintf(stream, "Specular"); G3D_WriteTextColor(specular, stream);
	fprintf(stream, "%s(%g) ", "Intensity", intensity);
	fprintf(stream, "%s(%g) ", "Radius", G3D_TextNumber(radius));
	fprintf(stream, "%s(%d) ", "Falloff", falloff);
	fprintf(stream, "%s(%g %g %g) ", "Attenuation", attenuation[0], attenuation[1], attenuation[2]);
	fprintf(stream, "%s", "SpotDir"); G3D_WriteTextVector(stream, direction.value);
	fprintf(stream, "%s(%g) ", "SpotAngle", spotAngle);
	fprintf(stream, "%s(%g) ", "SpotSoftAngle", spotSoftAngle);
	fprintf(stream, "}\n");
}

void G3D_Camera::WriteText(FILE* stream) //4CBAF0.
{
	G3D_WriteIndent(stream);
	fprintf(stream, "%s{ ", "Camera");
	name.WriteText(stream, 0);
	fprintf(stream, "%s(%g) ", "Fov", fieldOfView);
	fprintf(stream, "%s(%g) ", "Near", G3D_TextNumber(nearDistance));
	fprintf(stream, "%s(%g) ", "Far", G3D_TextNumber(farDistance));
	fprintf(stream, "}\n");
}
