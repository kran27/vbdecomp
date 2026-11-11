#include "G3DModel.hpp"
#include "TextParser.hpp"
#include <d3dx8math.h>
#include <stddef.h>
#include <string.h>

#if defined(_MSC_VER) && _MSC_VER == 1300 && defined(_M_IX86)
typedef char G3DMaterialExtent[sizeof(G3D_Material) == 0x160 ? 1 : -1];
#define G3D_MATERIAL_OFFSET(member, value) typedef char G3DMaterial_##member[offsetof(G3D_Material, member) == value ? 1 : -1]
G3D_MATERIAL_OFFSET(technique, 0x00); G3D_MATERIAL_OFFSET(index, 0x1C);
G3D_MATERIAL_OFFSET(name, 0x20); G3D_MATERIAL_OFFSET(ambient, 0x3C); G3D_MATERIAL_OFFSET(diffuse, 0x4C); G3D_MATERIAL_OFFSET(emissive, 0x5C); G3D_MATERIAL_OFFSET(specular, 0x6C);
G3D_MATERIAL_OFFSET(power, 0x80); G3D_MATERIAL_OFFSET(alpha, 0x88);
G3D_MATERIAL_OFFSET(twoSided, 0x90); G3D_MATERIAL_OFFSET(flat, 0x91); G3D_MATERIAL_OFFSET(isFloor, 0x92); G3D_MATERIAL_OFFSET(noShadow, 0x93); G3D_MATERIAL_OFFSET(bottomStory, 0x94); G3D_MATERIAL_OFFSET(bottomStoryInterior, 0x95); G3D_MATERIAL_OFFSET(topStory, 0x96); G3D_MATERIAL_OFFSET(buildingNumber, 0x98);
G3D_MATERIAL_OFFSET(blend, 0x9C); G3D_MATERIAL_OFFSET(materialType, 0xB8);
G3D_MATERIAL_OFFSET(scrollUEnabled, 0xD4); G3D_MATERIAL_OFFSET(scrollU, 0xD8);
G3D_MATERIAL_OFFSET(scrollVEnabled, 0xE0); G3D_MATERIAL_OFFSET(scrollV, 0xE8);
G3D_MATERIAL_OFFSET(sinUEnabled, 0xF0); G3D_MATERIAL_OFFSET(sinUFirst, 0xF8); G3D_MATERIAL_OFFSET(sinUSecond, 0x100);
G3D_MATERIAL_OFFSET(sinVEnabled, 0x108); G3D_MATERIAL_OFFSET(sinVFirst, 0x110); G3D_MATERIAL_OFFSET(sinVSecond, 0x118);
G3D_MATERIAL_OFFSET(rotationEnabled, 0x120); G3D_MATERIAL_OFFSET(rotationFirst, 0x128); G3D_MATERIAL_OFFSET(rotationSecond, 0x130); G3D_MATERIAL_OFFSET(rotationThird, 0x138);
G3D_MATERIAL_OFFSET(animationEnabled, 0x140); G3D_MATERIAL_OFFSET(animationRate, 0x148);
G3D_MATERIAL_OFFSET(animationFirst, 0x150); G3D_MATERIAL_OFFSET(animationSecond, 0x154); G3D_MATERIAL_OFFSET(animationThird, 0x158);
#undef G3D_MATERIAL_OFFSET
#endif

D3DXCOLOR G3D_ReadTextColor() //4C8750; output writes follow the closing delimiter.
{
	G3DText::Expect('(');
	float red = static_cast<float>(G3D_ReadScaledNumber());
	float green = static_cast<float>(G3D_ReadScaledNumber());
	float blue = static_cast<float>(G3D_ReadScaledNumber());
	G3DText::Expect(')');
	return D3DXCOLOR(red, green, blue, 1.0f);
}

void G3D_Name::ReadText() //4D05E0.
{
	G3DText::Expect('(');
	assign(G3DText::ReadString());
	G3DText::Expect(')');
}

void G3D_Material::SetSinU(BYTE enabled, double first, double second) //4C8610.
{
	sinUEnabled = enabled; sinUFirst = first; sinUSecond = second;
}
void G3D_Material::SetSinV(BYTE enabled, double first, double second) //4C8650.
{
	sinVEnabled = enabled; sinVFirst = first; sinVSecond = second;
}
void G3D_Material::SetRotation(BYTE enabled, double first, double second, double third) //4C8690.
{
	rotationEnabled = enabled; rotationFirst = first; rotationSecond = second; rotationThird = third;
}
void G3D_Material::SetAnimation(BYTE enabled, double rate, LONG first, LONG second, LONG third) //4C86E0.
{
	animationEnabled = enabled; animationRate = rate;
	animationFirst = first; animationSecond = second; animationThird = third;
}

void G3D_Texture::ReadText() //4D4BA0.
{
	G3DText::Expect('{');
	flags[0] = 0; flags[1] = 0; flags[2] = 0; flags[3] = 0;
	name = std::string("NULL_TEXTURE");
	G3DText::Token token = G3DText::NextToken();
	while (token.kind != '}')
	{
		if (token.kind != 256) token.ParseError();
		std::string key = G3DText::TokenString(token.value.identifier);
		if (key == "Name") name.ReadText();
		else if (key == "File")
		{
			G3DText::Expect('('); file = G3DText::ReadString(); G3DText::Expect(')');
		}
		else if (key == "Width")
		{
			G3DText::Expect('('); SetWidth(static_cast<LONG>(G3DText::ReadNumber())); G3DText::Expect(')');
		}
		else if (key == "Height")
		{
			G3DText::Expect('('); SetHeight(static_cast<LONG>(G3DText::ReadNumber())); G3DText::Expect(')');
		}
		else
		{
			if (key != "Flags") token.ParseError();
			G3DText::Expect('(');
			G3DText::Token flag = G3DText::NextToken();
			while (flag.kind != ')')
			{
				if (flag.kind != 256) flag.ParseError();
				std::string value = G3DText::TokenString(flag.value.identifier);
				if (value == "WRAP_S") flags[0] = 1;
				else if (value == "WRAP_T") flags[1] = 1;
				else if (value == "CLAMP_S") flags[0] = 0;
				else if (value == "CLAMP_T") flags[1] = 0;
				else if (value == "FILTERING") flags[2] = 1;
				else if (value == "ENVIRONMENT_MAPPING") flags[3] = 1;
				else flag.ParseError();
				flag = G3DText::NextToken();
			}
		}
		token = G3DText::NextToken();
	}
}

void G3D_Material::ReadText() //4D5100.
{
	G3DText::Expect('{');
	twoSided = 0; flat = 0; isFloor = 0; noShadow = 0;
	animationEnabled = 0; rotationEnabled = 0; sinVEnabled = 0;
	sinUEnabled = 0; scrollVEnabled = 0; scrollUEnabled = 0;
	topStory = 0; bottomStoryInterior = 0; bottomStory = 0;
	name = std::string("NULL_MATERIAL");
	G3DText::Token token = G3DText::NextToken();
	while (token.kind != '}')
	{
		if (token.kind != 256) token.ParseError();
		std::string key = G3DText::TokenString(token.value.identifier);
		if (key == "Mtl_ID")
		{
			G3DText::Expect('(');
			std::string value = G3DText::ReadString(); technique = value; index = -1;
			G3DText::Expect(')');
		}
		else if (key == "Name") name.ReadText();
		else if (key == "Ambient") { D3DXCOLOR value = G3D_ReadTextColor(); memcpy(&ambient, &value, 16); }
		else if (key == "Diffuse") { D3DXCOLOR value = G3D_ReadTextColor(); memcpy(&diffuse, &value, 16); }
		else if (key == "Emissive") { D3DXCOLOR value = G3D_ReadTextColor(); memcpy(&emissive, &value, 16); }
		else if (key == "Specular") { D3DXCOLOR value = G3D_ReadTextColor(); memcpy(&specular, &value, 16); }
		else if (key == "Shininess")
		{
			G3DText::Expect('('); power = G3D_ReadScaledNumber(); G3DText::Expect(')');
		}
		else if (key == "Alpha")
		{
			G3DText::Expect('('); alpha = G3D_ReadScaledNumber(); G3DText::Expect(')');
		}
		else if (key == "BLEND_STATE")
		{
			G3DText::Expect('(');
			G3DText::Token value = G3DText::NextToken();
			if (value.kind == 256) blend = G3DText::TokenString(value.value.identifier);
			G3DText::Expect(')');
		}
		else if (key == "MATERIAL_TYPE")
		{
			G3DText::Expect('(');
			G3DText::Token value = G3DText::NextToken();
			if (value.kind == 256) materialType = G3DText::TokenString(value.value.identifier);
			G3DText::Expect(')');
		}
		else if (key == "SCROLL_U")
		{
			G3DText::Expect('('); scrollU = G3DText::ReadNumber(); scrollUEnabled = 1; G3DText::Expect(')');
		}
		else if (key == "SCROLL_V")
		{
			G3DText::Expect('('); scrollV = G3DText::ReadNumber(); scrollVEnabled = 1; G3DText::Expect(')');
		}
		else if (key == "SIN_U")
		{
			G3DText::Expect('('); double first = G3DText::ReadNumber(); double second = G3DText::ReadNumber();
			G3DText::Expect(')'); SetSinU(1, first, second);
		}
		else if (key == "SIN_V")
		{
			G3DText::Expect('('); double first = G3DText::ReadNumber(); double second = G3DText::ReadNumber();
			G3DText::Expect(')'); SetSinV(1, first, second);
		}
		else if (key == "ROTATE")
		{
			G3DText::Expect('('); double first = G3DText::ReadNumber(); double second = G3DText::ReadNumber();
			float third = static_cast<float>(G3DText::ReadNumber());
			G3DText::Expect(')'); SetRotation(1, first, second, third);
		}
		else if (key == "ANIMATE")
		{
			G3DText::Expect('('); double rate = G3DText::ReadNumber();
			LONG first = static_cast<LONG>(G3DText::ReadNumber());
			LONG second = static_cast<LONG>(G3DText::ReadNumber());
			LONG third = static_cast<LONG>(G3DText::ReadNumber());
			G3DText::Expect(')'); SetAnimation(1, rate, first, second, third);
		}
		else
		{
			if (key != "Flags") token.ParseError();
			G3DText::Expect('(');
			G3DText::Token flag = G3DText::NextToken();
			while (flag.kind != ')')
			{
				if (flag.kind != 256) flag.ParseError();
				std::string value = G3DText::TokenString(flag.value.identifier);
				if (value == "FLAT") flat = 1;
				else if (value == "TWO_SIDED") twoSided = 1;
				else if (value == "FLOOR") isFloor = 1;
				else if (value == "NO_SHADOW") noShadow = 1;
				else if (value == "BTM_STORY") bottomStory = 1;
				else if (value == "BTM_STORY_INT") bottomStoryInterior = 1;
				else if (value == "TOP_STORY") topStory = 1;
				else if (value == "BLD_NUM01") buildingNumber = 1;
				else if (value == "BLD_NUM02") buildingNumber = 2;
				else if (value == "BLD_NUM03") buildingNumber = 3;
				else if (value == "BLD_NUM04") buildingNumber = 4;
				else if (value == "BLD_NUM05") buildingNumber = 5;
				else if (value == "BLD_NUM06") buildingNumber = 6;
				else if (value == "BLD_NUM07") buildingNumber = 7;
				else if (value == "BLD_NUM08") buildingNumber = 8;
				else if (value == "BLD_NUM09") buildingNumber = 9;
				else if (value == "BLD_NUM10") buildingNumber = 10;
				else if (value == "BLD_NUM11") buildingNumber = 11;
				else if (value == "BLD_NUM12") buildingNumber = 12;
				else if (value == "BLD_NUM13") buildingNumber = 13;
				else if (value == "BLD_NUM14") buildingNumber = 14;
				else if (value == "BLD_NUM15") buildingNumber = 15;
				else flag.ParseError();
				flag = G3DText::NextToken();
			}
		}
		token = G3DText::NextToken();
	}
}
