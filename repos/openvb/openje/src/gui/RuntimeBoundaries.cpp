//Linkable addressed virtual leaves for the normal GUI graph. These replace
//test-fixture-only departures, never invent successful unsupported behavior.
#include "Window.hpp"
#include "Fragment.hpp"
#include "Interface.hpp"
#include "Manager.hpp"
#include "Event.hpp"
#include "FileString.hpp"
#include "WindowFactory.hpp"
#include "graphics/SystemInterface.hpp"
#include "graphics/Texture.hpp"
#include "resources/ResourceSystem.hpp"
#include <cstring>

namespace GUI
{
	BYTE Window::ReadFile(FILE* file, int version) //4B0E10.
	{
		ClearChildren();
		if (!Base::ReadFile(file, version)
			|| !ReadFileString(name, file)
			|| std::fread(&hidden, 1, 1, file) != 1
			|| std::fread(&frameVisible, 1, 1, file) != 1
			|| std::fread(&centerVisible, 1, 1, file) != 1
			|| !ReadFileString(skin, file)) return 0;
		if (std::fread(&color, 4, 1, file) != 1) return 0;
		if (version < 2 && static_cast<BYTE>(color >> 24) < 255) materialModes = 1;
		if (std::fread(&clientRectangle, 16, 1, file) != 1) return 0;
		if (version >= 2)
		{
			if (std::fread(&value60, 1, 1, file) != 1
				|| std::fread(&materialModes, 4, 1, file) != 1) return 0;
			SetMaterialModes(materialModes);
		}
		if (version >= 3)
		{
			if (!ReadFileString(stringCB8, file)
				|| std::fread(&valueCD4, 4, 1, file) != 1
				|| !ReadFileString(stringCD8, file)
				|| std::fread(&valueCF4, 4, 1, file) != 1
				|| !ReadFileString(stringCF8, file)) return 0;
		}
		for (int i = 0; i < 9; ++i) if (!fragments[i].ReadFile(file, version)) return 0;
		strings.clear();
		DWORD index;
		if (std::fread(&index, 4, 1, file) != 1) return 0;
		while (index != DWORD(-1))
		{
			//Unlike ReadResource, the original FILE reader never grows this
			//cleared vector before indexing it. Preserve that invalid-input path.
			if (!ReadFileString(strings[index], file)
				|| std::fread(&index, 4, 1, file) != 1) return 0;
		}
		owner = NULL;
		parent = NULL;
		DWORD count;
		if (std::fread(&count, 4, 1, file) != 1) return 0;
		for (DWORD i = 0; i < count; ++i)
		{
			DWORD childType;
			if (std::fread(&childType, 4, 1, file) != 1) return 0;
			Window* child = g_Manager.CreateWindowByType(childType);
			if (!child->ReadFile(file, version)) return 0; //Unchecked NULL/failed-child leak.
			AddChild(child);
		}
		children.reverse();
		return 1;
	}
	BYTE Window::WriteFile(FILE* file) //4AFC70; current serialized fields, no version word.
	{
		if (!Base::WriteFile(file)
			|| !WriteFileString(name, file)
			|| std::fwrite(&hidden, 1, 1, file) != 1
			|| std::fwrite(&frameVisible, 1, 1, file) != 1
			|| std::fwrite(&centerVisible, 1, 1, file) != 1
			|| !WriteFileString(skin, file)
			|| std::fwrite(&color, 4, 1, file) != 1
			|| std::fwrite(&clientRectangle, 16, 1, file) != 1
			|| std::fwrite(&value60, 1, 1, file) != 1
			|| std::fwrite(&materialModes, 4, 1, file) != 1
			|| !WriteFileString(stringCB8, file)
			|| std::fwrite(&valueCD4, 4, 1, file) != 1
			|| !WriteFileString(stringCD8, file)
			|| std::fwrite(&valueCF4, 4, 1, file) != 1
			|| !WriteFileString(stringCF8, file)) return 0;
		for (int i = 0; i < 9; ++i) if (!fragments[i].WriteFile(file)) return 0;
		int index;
		for (index = 0; index < static_cast<int>(strings.size()); ++index)
		{
			if (!strings[index].empty()
				&& (std::fwrite(&index, 4, 1, file) != 1 || !WriteFileString(strings[index], file))) return 0;
		}
		index = DWORD(-1);
		if (std::fwrite(&index, 4, 1, file) != 1) return 0;
		DWORD count = static_cast<DWORD>(children.size());
		if (std::fwrite(&count, 4, 1, file) != 1) return 0;
		for (std::list<Window*>::iterator child = children.begin(); child != children.end(); ++child)
		{
			Window* window = *child; //Original retains this receiver across fwrite.
			DWORD childType = window->type;
			if (std::fwrite(&childType, 4, 1, file) != 1 || !window->WriteFile(file)) return 0;
		}
		return 1;
	}
	void Window::SetValue60(BYTE value) { value60 = value; } //4AF260.
	BYTE Window::GetValue60() { return value60; } //4AF270.
	void Window::SetValue68(BYTE value) { value68 = value; } //4AF280.
	BYTE Window::GetValue68() { return value68; } //5C0350.
	void Window::SetValue3C(BYTE value) { value3C = value; } //4AF300, raw byte.
	BYTE Window::GetValue3C() { return value3C; } //4AF310.
	void Window::BringToFront() //4AF480; reload borrowed owner/parent after callbacks.
	{
		if (owner)
		{
			owner->BringWindowToFront(this);
			owner->BringToFront();
		}
		else if (parent)
		{
			parent->BringChildToFront(this);
			parent->BringToFront();
		}
	}
	void Window::SetString(int index, const std::string& value) //4B13F0.
	{
		//The original signed upper-bound comparison has no negative-index guard.
		if (index >= static_cast<int>(strings.size())) strings.resize(index + 1);
		strings[index] = value;
	}
	void Window::GetString(int index, std::string& value) //4B0520, live output object.
	{
		if (index < static_cast<int>(strings.size()) && !strings[index].empty())
			value = strings[index];
		else value.assign("");
	}
	void Window::ClearString(int index) //4B05A0, preserves virtual setter dispatch.
	{
		SetString(index, std::string("")); //Original call temporary, destroyed at statement end.
	}
	void Window::Scale(float x, float y) //4AF9C0; children precede both rectangles.
	{
		for (std::list<Window*>::iterator child = children.begin(); child != children.end(); ++child)
			(*child)->Scale(x, y);
		rectangle.left = static_cast<LONG>(rectangle.left * x);
		rectangle.top = static_cast<LONG>(rectangle.top * y);
		rectangle.right = static_cast<LONG>(rectangle.right * x);
		rectangle.bottom = static_cast<LONG>(rectangle.bottom * y);
		clientRectangle.left = static_cast<LONG>(clientRectangle.left * x);
		clientRectangle.top = static_cast<LONG>(clientRectangle.top * y);
		clientRectangle.right = static_cast<LONG>(clientRectangle.right * x);
		clientRectangle.bottom = static_cast<LONG>(clientRectangle.bottom * y);
		geometryDirty = 1;
	}
	void Window::Center() //4AF4F0, virtual size and absolute-position calls.
	{
		DWORD screenWidth, screenHeight;
		g_Manager.GetResolution(screenWidth, screenHeight);
		LONG width, height;
		GetSize(width, height);
		float scaleX, scaleY;
		g_Manager.GetScale(scaleX, scaleY);
		screenWidth = DWORD(static_cast<LONG>(double(LONG(screenWidth)) / scaleX));
		screenHeight = DWORD(static_cast<LONG>(double(LONG(screenHeight)) / scaleY));
		SetAbsolutePosition(LONG(screenWidth - DWORD(width)) / 2,
			LONG(screenHeight - DWORD(height)) / 2);
	}
	void Window::SetStringCB8(const std::string& value) { stringCB8 = value; } //4B0640.
	void Window::SetStringCD8(const std::string& value) { stringCD8 = value; } //4B0660.
	std::string Window::GetStringCD8() //4B0160; distinct from CB8's reference.
	{
		if (!stringCD8.empty()) return stringCD8;
		std::string resolved;
		if (valueCF4 != DWORD(-1) && g_Manager.ResolveString(valueCF4, resolved)) return resolved;
		return name;
	}
	BYTE Fragment::ReadFile(FILE* file, int version) //4B1E50.
	{
		if (std::fread(&tileMode, 4, 1, file) != 1
			|| std::fread(&frameWidth, 4, 1, file) != 1
			|| std::fread(&frameHeight, 4, 1, file) != 1
			|| std::fread(&frameRate, 4, 1, file) != 1
			|| std::fread(&currentFrame, 4, 1, file) != 1
			|| std::fread(&frameCount, 4, 1, file) != 1) return 0;
		std::string textureName;
		if (!ReadFileString(textureName, file)) return 0;
		DWORD handle = 0;
		if (!g_GfxSystem->AcquireTextureByName(&handle, textureName))
		{
			JE::Resources::ResourceData* resource = JE::Resources::g_pResourceSystem->Demand(textureName.c_str(), 1);
			if (resource)
			{
				if (FAILED(g_GfxSystem->LoadTextureFromBuffer(&handle, textureName,
					static_cast<const BYTE*>(resource->data), resource->size,
					D3DFMT_UNKNOWN, D3DPOOL_MANAGED, 1, 1, 5, 0))) handle = 0;
				JE::Resources::g_pResourceSystem->ReleaseResource(resource);
			}
		}
		material->textureHandles[0] = handle;
		if (std::fread(&vertices[0].z, 4, 1, file) == 1)
		{
			for (int i = 1; i < 4; ++i) std::memcpy(&vertices[i].z, &vertices[0].z, 4);
			if (std::fread(&color, 4, 1, file) == 1)
			{
				mesh.materialSet.SetColors(1, 1, color, 0);
				uvDirty = 1;
				mesh.materialSet.RebuildGroups();
				return Base::ReadFile(file, version);
			}
		}
		return 0;
	}
	BYTE Fragment::WriteFile(FILE* file) //4B2300.
	{
		if (std::fwrite(&tileMode, 4, 1, file) != 1
			|| std::fwrite(&frameWidth, 4, 1, file) != 1
			|| std::fwrite(&frameHeight, 4, 1, file) != 1
			|| std::fwrite(&frameRate, 4, 1, file) != 1
			|| std::fwrite(&currentFrame, 4, 1, file) != 1
			|| std::fwrite(&frameCount, 4, 1, file) != 1) return 0;
		std::string textureName;
		if (material->textureHandles[0])
		{
			Gfx_Texture* texture = g_GfxSystem->LookupTexture(material->textureHandles[0]);
			if (texture) textureName = texture->GetDiagnosticName();
		}
		if (!WriteFileString(textureName, file)
			|| std::fwrite(&vertices[0].z, 4, 1, file) != 1
			|| std::fwrite(&color, 4, 1, file) != 1) return 0;
		return Base::WriteFile(file); //Original preserves AL through string cleanup.
	}
	int Interface::OnEvent(Window* window, Event* event) //4958D0.
	{
		if (event->type == 1)
		{
			if (window) g_Manager.SetTooltip(window->GetStringCB8());
		}
		else if (event->type == 3) g_Manager.SetTooltip(std::string(""));
		return 0;
	}
}
