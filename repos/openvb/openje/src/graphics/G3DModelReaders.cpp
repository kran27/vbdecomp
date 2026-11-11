#include "G3DModel.hpp"
#include "TextParser.hpp"
#include "../model/CommonException.hpp"
#include <mbstring.h>
#include <string.h>
#include <strstream>

namespace
{
	const char Signature[] = "B3D 1.1 ";

	void ResetReaderScales()
	{
		G3D_GlobalScale = 1.0;
		G3D_CoordinateScale = 1024.0;
		G3D_CoordinateInverse = 0.0009765625;
		G3D_CoordinateScaleDouble = 1024.0;
	}

	void InvalidToken(BYTE token)
	{
		std::strstream text;
		text << "invalid token: " << static_cast<int>(token) << std::endl;
		// str() freezes the real VC7 buffer. Exception borrows that allocation.
		throw Common::Exception(text.str());
	}

	D3DXCOLOR ReadFileColorWithAlpha(FILE* stream) //4C89E0; original internal register ABI unclaimed.
	{
		D3DXCOLOR color = G3D_ReadFileColor(stream);
		float alpha;
		fread(&alpha, 4, 1, stream);
		color.a = alpha;
		return color;
	}

	LONG TextureSlot(const std::string& name) //4CACD0, original17-entry6FB0E0 table.
	{
		static const char* names[] = { "SKIN", "BODY", "SHOULDER", "HEAD", "BACK",
			"HANDS", "FEET", "HAIR_HEAD", "HAIR_BEARD", "HAIR_MUSTACHE", "HAIR_PONYTAIL",
			"EYES", "VANITY", "ONHANDWEAPON", "OFFHANDWEAPON", "ONHANDITEM", "OFFHANDITEM" };
		for (LONG index = 0; index < 17; ++index)
		{
			if (!_mbsicmp(reinterpret_cast<const unsigned char*>(name.c_str()),
				reinterpret_cast<const unsigned char*>(names[index]))) return index;
		}
		return -1;
	}
}

G3D_Transform::G3D_Transform() //4CB190.
{
	rotation[0] = 0.0;
	rotation[1] = 0.0;
	rotation[2] = 0.0;
	rotation[3] = 1.0;
	translation[0] = 0.0;
	translation[1] = 0.0;
	translation[2] = 0.0;
	scale[0] = 1.0;
	scale[1] = 1.0;
	scale[2] = 1.0;
}

G3D_Bone::G3D_Bone() //4CB2E0; real string then embedded Transform construction.
{
}

G3D_Bone::~G3D_Bone() //4CB300; reverse ordinary member lifetimes.
{
}

void G3D_Bone::ReadText() //4D07F0.
{
	G3DText::Expect('{');
	G3DText::Token token;
	token = G3DText::NextToken();
	while (token.kind != '}')
	{
		if (token.kind == 256)
		{
			std::string key = G3DText::TokenString(token.value.identifier);
			if (key == "Name") name.ReadText();
			else if (key == "Transform") transform.ReadText();
			else token.ParseError();
		}
		else token.ParseError();
		token = G3DText::NextToken();
	}
}

void G3D_Bone::Read(FILE* stream) //4D09B0.
{
	name.Read(stream);
	transform.Read(stream);
}

void G3D_Texture::Read(FILE* stream) //4D1050; unchecked FILE results are original policy.
{
	std::string temporary;
	FILE* input = stream;
	// The original reads the flag into the low byte of its parameter slot.
	// Keep the saved stream pointer for every subsequent FILE operation.
	fread(&stream, 1, 1, input);
	BYTE bits = static_cast<BYTE>(reinterpret_cast<ULONG_PTR>(stream));
	flags[0] = bits & 1;
	flags[1] = (bits >> 1) & 1;
	flags[2] = (bits & 4) != 0;
	flags[3] = (bits & 8) != 0;
	cube = (bits & 16) != 0;
	name.Read(input); //4D0670 is the actual member boundary.
	G3D_ReadFileString(temporary, input);
	file = temporary;
	LONG value;
	fread(&value, 4, 1, input); SetWidth(value);
	fread(&value, 4, 1, input); SetHeight(value);
}

void G3D_Material::Read(FILE* stream) //4D1290.
{
	std::string temporary;
	G3D_ReadFileString(temporary, stream);
	technique = temporary;
	index = -1;
	name.Read(stream);
	{
		D3DXCOLOR color = ReadFileColorWithAlpha(stream);
		memcpy(&ambient, &color, sizeof(color));
	}
	{
		D3DXCOLOR color = ReadFileColorWithAlpha(stream);
		memcpy(&diffuse, &color, sizeof(color));
	}
	{
		D3DXCOLOR color = ReadFileColorWithAlpha(stream);
		memcpy(&emissive, &color, sizeof(color));
	}
	{
		D3DXCOLOR color = ReadFileColorWithAlpha(stream);
		memcpy(&specular, &color, sizeof(color));
	}
	float value;
	fread(&value, 4, 1, stream); power = value;
	fread(&value, 4, 1, stream); alpha = value;
	G3D_ReadFileString(blend, stream);
	G3D_ReadFileString(materialType, stream);
	DWORD enabled = 0;
	fread(&enabled, 4, 1, stream);
	// The original reuses these four independent double temporaries across
	// specific branches, including on an ignored short read.
	double uOrRotationX, vOrRotationY, sineFirst, sineSecondOrRotationZOrRate;
	if (enabled & 1)
	{
		fread(&uOrRotationX, 8, 1, stream); scrollUEnabled = 1; scrollU = uOrRotationX;
	}
	if (enabled & 2)
	{
		fread(&vOrRotationY, 8, 1, stream); scrollVEnabled = 1; scrollV = vOrRotationY;
	}
	if (enabled & 4)
	{
		fread(&sineFirst, 8, 1, stream); fread(&sineSecondOrRotationZOrRate, 8, 1, stream);
		SetSinU(1, sineFirst, sineSecondOrRotationZOrRate);
	}
	if (enabled & 8)
	{
		fread(&sineFirst, 8, 1, stream); fread(&sineSecondOrRotationZOrRate, 8, 1, stream);
		SetSinV(1, sineFirst, sineSecondOrRotationZOrRate);
	}
	if (enabled & 16)
	{
		fread(&uOrRotationX, 8, 1, stream); fread(&vOrRotationY, 8, 1, stream);
		fread(&sineSecondOrRotationZOrRate, 8, 1, stream);
		SetRotation(1, uOrRotationX, vOrRotationY, sineSecondOrRotationZOrRate);
	}
	if (enabled & 32)
	{
		LONG a, b, c;
		fread(&sineSecondOrRotationZOrRate, 8, 1, stream);
		fread(&a, 4, 1, stream); fread(&b, 4, 1, stream); fread(&c, 4, 1, stream);
		SetAnimation(1, sineSecondOrRotationZOrRate, a, b, c);
	}
	DWORD bits = 0;
	fread(&bits, 4, 1, stream);
	flat = BYTE(bits & 1); twoSided = (bits & 2) != 0;
	isFloor = (bits & 4) != 0; topStory = (bits & 32) != 0;
	noShadow = (bits & 8) != 0; bottomStory = (bits & 16) != 0;
}

G3D_Model::G3D_Model(const char* filename) : version(1), hasColor(0), valueC0(-1) //4DED60.
{
	root = new G3D_Node;
	root->model = this;
	if (filename) ReadFile(filename);
	// Original has an unidentified, constructor-unwritten interval before root.
	// This normal source class deliberately has no fabricated layout filler.
}

G3D_Model::~G3D_Model() //4F3DE0; node before the reverse container/string lifetimes.
{
	delete root;
}

void G3D_Model::AddMaterial(const G3D_Material& material) //4D8E30.
{
	std::string key = material.GetName();
	std::map<std::string, G3D_Material*>::iterator found = materialsByName.find(key);
	if (key.size() > 0 && found == materialsByName.end())
	{
		materials.push_back(material);
		materialsByName[key] = &materials.back();
	}
}

void G3D_Model::AddTexture(const G3D_Texture& texture) //4D8F10.
{
	std::string key = texture.GetName();
	std::map<std::string, G3D_Texture*>::iterator found = texturesByName.find(key);
	if (key.size() > 0 && found == texturesByName.end())
	{
		textures.push_back(texture);
		texturesByName[key] = &textures.back();
	}
}

void G3D_Model::AddBone(const G3D_Bone& bone) //4D9430.
{
	std::string key = bone.GetName();
	std::map<std::string, G3D_Bone*>::iterator found = bonesByName.find(key);
	if (key.size() > 0 && found == bonesByName.end())
	{
		bones.push_back(bone);
		bonesByName[key] = &bones.back();
	}
}

void G3D_Model::AddWater(const G3D_WaterAttribute& water) //4DDA10.
{
	std::string key = water.GetName();
	std::map<std::string, G3D_WaterAttribute>::iterator found = waterByName.find(key);
	if (found == waterByName.end()) waterByName[key] = water;
}

void G3D_Model::RegisterNode(G3D_Node* node) //4D8FF0.
{
	std::string key = node->GetName();
	if (key.size() > 0) nodesByName[key] = node;
}

void G3D_Model::RegisterAttribute(G3D_BaseAttribute* attribute) //4D9080.
{
	std::string key = attribute->GetName();
	if (key.size() > 0) attributesByName[key] = attribute;
}

void G3D_Model::ReadText(FILE* stream) //4DDAF0; Reset only on the original normal exit.
{
	G3DText::Initialize(stream);
	ResetReaderScales();
	version = 0;
	G3DText::Token token = G3DText::NextToken();
	while (token.kind)
	{
		if (token.kind != 256) token.ParseError();
		std::string key = G3DText::TokenString(token.value.identifier);
		if (key == "Version")
		{
			G3DText::Expect('('); version = static_cast<BYTE>(G3DText::ReadNumber()); G3DText::Expect(')');
		}
		else if (key == "GlobalScaleFactor")
		{
			G3DText::Expect('('); globalScale = G3D_GlobalScale = G3DText::ReadNumber(); G3DText::Expect(')');
		}
		else if (key == "CoordinateScale")
		{
			G3DText::Expect('(');
			coordinateScale = G3D_CoordinateScale = G3DText::ReadNumber();
			G3D_CoordinateScaleDouble = coordinateScale;
			G3D_CoordinateInverse = 1.0 / coordinateScale;
			G3DText::Expect(')');
		}
		else if (key == "CoordinateSystem")
		{
			G3DText::Expect('('); coordinateSystem = G3DText::ReadString(); G3DText::Expect(')');
		}
		else if (key == "AmbientLight")
		{
			D3DXCOLOR value = G3D_ReadTextColor(); memcpy(color, &value, sizeof(value)); hasColor = 1;
		}
		else if (key == "Material")
		{
			G3D_Material material; material.ReadText(); AddMaterial(material);
		}
		else if (key == "Texture")
		{
			G3D_Texture texture; texture.ReadText(); texture.cube = 0; AddTexture(texture);
		}
		else if (key == "Bone")
		{
			G3D_Bone bone; bone.ReadText(); AddBone(bone);
		}
		else if (key == "Lightmap")
		{
			G3D_Texture texture; texture.ReadText(); texture.cube = 1; AddTexture(texture);
		}
		else if (key == "Node") root->ReadText();
		else if (key == "TextureSlot")
		{
			G3DText::Expect('('); std::string value = G3DText::ReadString(); G3DText::Expect(')');
			valueC0 = TextureSlot(value);
		}
		else
		{
			if (key != "Water") token.ParseError();
			G3D_WaterAttribute water; water.ReadText(); AddWater(water);
		}
		token = G3DText::NextToken();
	}
	G3DText::Reset();
}

BYTE G3D_Model::Read(FILE* stream) //4DE120; distinct feof/end-token policies.
{
	ResetReaderScales();
	BYTE signature[8];
	fread(signature, 8, 1, stream);
	if (memcmp(signature, Signature, strlen(Signature))) throw Common::Exception("Not a B3D");
	version = 0;
	BYTE ended = 0, token;
	float value;
	while (!feof(stream))
	{
		if (ended) return ended;
		fread(&token, 1, 1, stream);
		switch (token)
		{
		case 1: ended = 1; break;
		case 3:
			fread(&value, 4, 1, stream); globalScale = value; G3D_GlobalScale = value; break;
		case 4:
			fread(&value, 4, 1, stream); coordinateScale = value;
			G3D_CoordinateScale = value; G3D_CoordinateScaleDouble = value;
			G3D_CoordinateInverse = 1.0 / value; break;
		case 5: G3D_ReadFileString(coordinateSystem, stream); break;
		case 6:
			{ D3DXCOLOR ambient = G3D_ReadFileColor(stream); memcpy(color, &ambient, sizeof(ambient)); hasColor = 1; }
			break;
		case 7:
			{ G3D_Material material; material.Read(stream); AddMaterial(material); } break;
		case 8:
			{ G3D_Texture texture; texture.Read(stream); texture.cube = 0; AddTexture(texture); } break;
		case 9:
			{ G3D_Texture texture; texture.Read(stream); texture.cube = 1; AddTexture(texture); } break;
		case 10: root->Read(stream); break;
		case 14:
			{ G3D_Bone bone; bone.Read(stream); AddBone(bone); } break;
		case 22: fread(&valueC0, 4, 1, stream); break;
		case 23:
			{ G3D_WaterAttribute water; water.Read(stream); AddWater(water); } break;
		case 28: fread(&version, 1, 1, stream); break;
		default: InvalidToken(token);
		}
	}
	if (!ended) throw Common::Exception("EOF token not found");
	return ended;
}

LONG G3D_Model::ReadFile(const char* filename) //4DEB90; actual two typed catch/close paths.
{
	FILE* stream = fopen(filename, "rb");
	if (!stream)
	{
		std::strstream text;
		text << "could not open file " << filename << std::endl;
		throw Common::Exception(text.str());
	}
	BYTE binary = 0;
	BYTE signature[8];
	fread(signature, 8, 1, stream);
	if (!memcmp(signature, Signature, strlen(Signature)))
	{
		binary = 1;
		try
		{
			fseek(stream, 0, SEEK_SET);
			Read(stream);
		}
		catch (Common::Exception error)
		{
			fclose(stream);
			throw Common::Exception(error.message);
		}
	}
	else
	{
		fclose(stream);
		stream = fopen(filename, "rt");
		try { ReadText(stream); }
		catch (Common::Exception error)
		{
			fclose(stream);
			throw Common::Exception(error.message);
		}
	}
	fclose(stream);
	return (binary != 0) + 1;
}

void G3D_Model::Read(const BYTE* data) //4DE680; complete original tag dispatch.
{
	if (memcmp(data, Signature, strlen(Signature))) throw Common::Exception("Not a B3D");
	ResetReaderScales();
	version = 0;
	DWORD offset = 8;
	G3D_Input input(data, offset);
	for (;;)
	{
		BYTE token = input.Read<BYTE>();
		switch (token)
		{
		case 1: return;
		case 3: globalScale = G3D_GlobalScale = input.Read<float>(); break;
		case 4:
			coordinateScale = G3D_CoordinateScale = input.Read<float>();
			G3D_CoordinateScaleDouble = coordinateScale; G3D_CoordinateInverse = 1.0 / coordinateScale; break;
		case 5: G3D_ReadMemoryString(coordinateSystem, data, offset); break;
		case 6:
			{ D3DXCOLOR ambient = G3D_ReadMemoryColor(data, offset); memcpy(color, &ambient, sizeof(ambient)); hasColor = 1; }
			break;
		case 7:
			{ G3D_Material material; material.Read(data, offset); AddMaterial(material); } break;
		case 8:
			{ G3D_Texture texture; texture.Read(data, offset); texture.cube = 0; AddTexture(texture); } break;
		case 9:
			{ G3D_Texture texture; texture.Read(data, offset); texture.cube = 1; AddTexture(texture); } break;
		case 10: root->Read(data, offset); break;
		case 14:
			{ G3D_Bone bone; bone.Read(data, offset); AddBone(bone); } break;
		case 22: valueC0 = input.Read<LONG>(); break;
		case 23:
			{ G3D_WaterAttribute water; water.Read(data, offset); AddWater(water); } break;
		case 28: version = input.Read<BYTE>(); break;
		default: InvalidToken(token);
		}
	}
}
