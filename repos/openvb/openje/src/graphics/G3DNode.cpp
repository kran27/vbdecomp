#include "G3DModel.hpp"
#include "G3DWaterTile.hpp"
#include "G3DSceneAttributes.hpp"
#include "G3DLightAttributes.hpp"
#include "TextParser.hpp"
#include "../model/CommonException.hpp"
#include <stddef.h>
#include <strstream>

G3D_Node::G3D_Node() //4D7600.
{
	parent = NULL;
	model = NULL;
	cachedTransform.SetIdentity();
	written = 0;
}

G3D_Node::~G3D_Node() //4CCEF0;4CEA90 is the compiler delete wrapper.
{
	for (std::vector<G3D_BaseAttribute*>::iterator attribute = attributes.begin(); attribute != attributes.end(); ++attribute)
		delete *attribute;
	for (std::vector<G3D_Node*>::iterator child = children.begin(); child != children.end(); ++child)
		delete *child;
}

void G3D_Node::CacheTransform() //4CAAA0, real double-precision affine composition.
{
	G3D_AffineMatrix inherited(1.0);
	if (parent) inherited = parent->cachedTransform;
	G3D_AffineMatrix local = transform.ToMatrix();
	cachedTransform = inherited * G3D_AffineMatrix(local);
}

void G3D_Node::AddChild(G3D_Node* child) //4D80F0.
{
	child->parent = this;
	child->model = model;
	children.push_back(child);
}

void G3D_Node::AddAttribute(G3D_BaseAttribute* attribute) //4D9400.
{
	model->RegisterAttribute(attribute);
	attribute->owner = this;
	attributes.push_back(attribute);
}

void G3D_Node::Read(G3D_Input& input) //Compatibility adapter, not another original body.
{
	Read(input.data, input.offset);
}

void G3D_Node::ReadText() //4DA230.
{
	G3DText::Expect('{');
	G3DText::Token token = G3DText::NextToken();
	while (token.kind != '}')
	{
		if (token.kind != 256) token.ParseError();
		std::string key = G3DText::TokenString(token.value.identifier);
		if (key == "Node")
		{
			G3D_Node* child = new G3D_Node;
			AddChild(child);
			child->ReadText();
			model->RegisterNode(child);
		}
		else if (key == "Name") name.ReadText();
		else if (key == "Transform")
		{
			transform.ReadText();
			CacheTransform();
		}
		else
		{
			G3D_BaseAttribute* attribute = NULL;
			if (key == "Mesh") attribute = new G3D_Mesh;
			else if (key == "PointLight") attribute = new G3D_PointLight;
			else if (key == "SpotLight") attribute = new G3D_SpotLight;
			else if (key == "DirectionalLight") attribute = new G3D_DirectionalLight;
			else if (key == "Camera") attribute = new G3D_Camera;
			else if (key == "ParticleEmitter") attribute = new G3D_ParticleEmitter;
			else if (key == "Water") attribute = new G3D_WaterAttribute;
			else if (key == "LineSystem") attribute = new G3D_LineSystem;
			else if (key == "LightmapSurface") attribute = new G3D_LightmapSurfaceAttribute;
			else if (key == "OcclusionZones") attribute = new G3D_OcclusionZones;
			else if (key == "WaterTile") attribute = new G3D_WaterTile;
			else token.ParseError();
			if (attribute)
			{
				// Text reads before registration/publication; FILE/raw do the reverse.
				attribute->ReadText();
				AddAttribute(attribute);
			}
		}
		token = G3DText::NextToken();
	}
}

void G3D_Node::Read(FILE* stream) //4DA760.
{
	// The original constructs this real string before the loop and retains its
	// lifetime/EH state, although no reached wire branch writes it.
	std::string text;
	BYTE ended = 0;
	BYTE token;
	while (!feof(stream))
	{
		if (ended) return;
		fread(&token, 1, 1, stream);
		G3D_BaseAttribute* attribute = NULL;
		switch (token)
		{
		case 1: throw Common::Exception("eof reached before end of node");
		case 10:
			{
				G3D_Node* child = new G3D_Node;
				AddChild(child);
				child->Read(stream);
				model->RegisterNode(child);
			}
			continue;
		case 11: ended = 1; continue;
		case 12: name.Read(stream); continue;
		case 13: transform.Read(stream); CacheTransform(); continue;
		case 15: attribute = new G3D_Mesh; break;
		case 16: attribute = new G3D_PointLight; break;
		case 17: attribute = new G3D_SpotLight; break;
		case 18: attribute = new G3D_DirectionalLight; break;
		case 19: attribute = new G3D_Camera; break;
		case 20: attribute = new G3D_ParticleEmitter; break;
		case 21: attribute = new G3D_LightmapSurfaceAttribute; break;
		case 23: attribute = new G3D_WaterAttribute; break;
		case 24: attribute = new G3D_LineSystem; break;
		case 26: attribute = new G3D_WaterTile; break;
		case 27: attribute = new G3D_OcclusionZones; break;
		default:
			{
				std::strstream error;
				error << "invalid token: " << static_cast<int>(token) << std::endl;
				// str() freezes the actual stream buffer. The original exception
				// borrows it through destruction, without adding an owning string.
				throw Common::Exception(error.str());
			}
		}
		if (attribute)
		{
			AddAttribute(attribute);
			attribute->Read(stream);
		}
	}
	if (!ended) throw Common::Exception("end-of-node token not found");
}

void G3D_Node::Read(const BYTE* data, DWORD& offset) //4DABD0, original unbounded buffer domain.
{
	std::string text; //Observed original local string lifetime, as in FILE input.
	for (;;)
	{
		BYTE token = data[offset++];
		G3D_BaseAttribute* attribute = NULL;
		switch (token)
		{
		case 1: throw Common::Exception("eof reached before end of node");
		case 10:
			{
				G3D_Node* child = new G3D_Node;
				AddChild(child);
				child->Read(data, offset);
				model->RegisterNode(child);
			}
			continue;
		case 11: return;
		case 12: name.Read(data, offset); continue;
		case 13: transform.Read(data, offset); CacheTransform(); continue;
		case 15: attribute = new G3D_Mesh; break;
		case 16: attribute = new G3D_PointLight; break;
		case 17: attribute = new G3D_SpotLight; break;
		case 18: attribute = new G3D_DirectionalLight; break;
		case 19: attribute = new G3D_Camera; break;
		case 20: attribute = new G3D_ParticleEmitter; break;
		case 21: attribute = new G3D_LightmapSurfaceAttribute; break;
		case 23: attribute = new G3D_WaterAttribute; break;
		case 24: attribute = new G3D_LineSystem; break;
		case 26: attribute = new G3D_WaterTile; break;
		case 27: attribute = new G3D_OcclusionZones; break;
		default:
			{
				std::strstream error;
				error << "invalid token: " << static_cast<int>(token) << std::endl;
				throw Common::Exception(error.str());
			}
		}
		if (attribute)
		{
			AddAttribute(attribute);
			attribute->Read(data, offset);
		}
	}
}

void G3D_Node::WriteText(FILE* stream) //4CAB30, shared per-node written flag.
{
	if (written) return;
	G3D_WriteIndent(stream);
	fprintf(stream, "Node");
	G3D_OpenTextBlock(stream);
	name.WriteText(stream, 1);
	if (transform.HasTranslation() || transform.HasRotation() || transform.HasScale())
	{
		G3D_WriteIndent(stream);
		transform.WriteText(stream);
		fprintf(stream, "\n");
	}
	for (std::vector<G3D_BaseAttribute*>::iterator attribute = attributes.begin(); attribute != attributes.end(); ++attribute)
		(*attribute)->WriteText(stream);
	for (std::vector<G3D_Node*>::iterator child = children.begin(); child != children.end(); ++child)
		(*child)->WriteText(stream);
	G3D_CloseTextBlock(stream);
	written = 1;
}

void G3D_Node::Write(FILE* stream) //4CEAB0.
{
	if (written) return;
	BYTE token = 10;
	fwrite(&token, 1, 1, stream);
	token = 12;
	fwrite(&token, 1, 1, stream);
	name.Write(stream);
	if (transform.HasTranslation() || transform.HasRotation() || transform.HasScale())
	{
		token = 13;
		fwrite(&token, 1, 1, stream);
		transform.Write(stream);
	}
	for (std::vector<G3D_BaseAttribute*>::iterator attribute = attributes.begin(); attribute != attributes.end(); ++attribute)
		(*attribute)->Write(stream);
	for (std::vector<G3D_Node*>::iterator child = children.begin(); child != children.end(); ++child)
		(*child)->Write(stream);
	written = 1;
	token = 11;
	fwrite(&token, 1, 1, stream);
}

#if defined(_MSC_VER) && _MSC_VER == 1300 && defined(_M_IX86)
#define F3_G3D_NODE_OFFSET(member, value) typedef char F3_Node_##member[(offsetof(G3D_Node, member) == value) ? 1 : -1]
typedef char F3_G3D_Node_Size[(sizeof(G3D_Node) == 0x130) ? 1 : -1];
F3_G3D_NODE_OFFSET(name, 0x08);
F3_G3D_NODE_OFFSET(transform, 0x28);
F3_G3D_NODE_OFFSET(cachedTransform, 0xA0);
F3_G3D_NODE_OFFSET(parent, 0x100);
F3_G3D_NODE_OFFSET(model, 0x104);
F3_G3D_NODE_OFFSET(attributes, 0x108);
F3_G3D_NODE_OFFSET(children, 0x118);
F3_G3D_NODE_OFFSET(written, 0x128);
#undef F3_G3D_NODE_OFFSET
#endif
