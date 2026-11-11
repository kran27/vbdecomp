#include "MapFog.hpp"
#include "graphics/SystemInterface.hpp"
#include <stdlib.h>

namespace JE
{
	// These callers retain stack cleanup after 48CC40. Its original declaration
	// did not supply the noreturn attribute used by the older diagnostic adapter.
	void __cdecl FatalError(const char* format, ...);
}

namespace
{
	// Actual internal linkage lets VC7 retain the original private register
	// arguments; these helpers are never exposed as external cdecl services.
	static void ReadSavedMapString(FILE* input, std::string& output) //57D2E0, original internal helper.
	{
		output.assign("");
		WORD length;
		fread(&length, 2, 1, input);
		if (length > 0)
		{
			output.resize(length + 1);
			char* buffer = new char[length + 1];
			fread(buffer, 1, length, input);
			buffer[length] = 0;
			output.assign(buffer);
			delete[] buffer;
		}
	}

	// Evaluate the FILE receiver before constructing the by-value string.
	static void WriteSavedMapString(std::string input, FILE* output) //57C3A0, original by-value string ownership.
	{
		WORD length = static_cast<WORD>(input.length());
		fwrite(&length, 2, 1, output);
		fwrite(input.c_str(), 1, length, output);
	}
}

namespace JE
{
	void AreaMapBits::Read(FILE* input) //5AD7E0.
	{
		if (fread(&size, 4, 1, input) == 1)
		{
			if (size)
			{
				data = static_cast<BYTE*>(realloc(data, size));
				if (fread(data, size, 1, input) == 1) return;
			}
			else if (data)
			{
				free(data);
				data = NULL;
				return;
			}
		}
		FatalError("Error loading bit array\n");
	}

	void AreaMapBits::Write(FILE* output) //5AD850.
	{
		if (fwrite(&size, 4, 1, output) != 1 ||
			(size && fwrite(data, size, 1, output) != 1))
			FatalError("Error saving bit array\n");
	}

	void AreaMapNote::Read(FILE* input) //57D440.
	{
		ReadSavedMapString(input, name);
		fread(&value20, 4, 1, input);
		fread(&value24, 4, 1, input);
		fread(&value28, 4, 1, input);
		fread(&value2C, 4, 1, input);
		ReadSavedMapString(input, text);
		fread(position, 12, 1, input);
		fread(&flag5C, 1, 1, input);
		fread(&flag5D, 1, 1, input);
		if (texture)
		{
			g_GfxSystem->DeleteTexture(texture);
			texture = 0;
		}
		textureHeight = 0;
		textureWidth = 0;
	}

	void AreaMapNote::Write(FILE* output) //57C8D0.
	{
		WriteSavedMapString(name, output);
		fwrite(&value20, 4, 1, output);
		fwrite(&value24, 4, 1, output);
		fwrite(&value28, 4, 1, output);
		fwrite(&value2C, 4, 1, output);
		WriteSavedMapString(text, output);
		fwrite(position, 12, 1, output);
		fwrite(&flag5C, 1, 1, output);
		fwrite(&flag5D, 1, 1, output);
	}

	void AreaMap::Read(FILE* input) //57FF40.
	{
		fread(&minX, 4, 1, input);
		fread(&minZ, 4, 1, input);
		fread(&maxX, 4, 1, input);
		fread(&maxZ, 4, 1, input);
		fread(&width, 4, 1, input);
		fread(&height, 4, 1, input);
		LONG count;
		fread(&count, 4, 1, input);
		notes.clear();
		for (LONG i = 0; i < count; ++i)
		{
			AreaMapNote note;
			note.Read(input);
			notes.push_back(note);
		}
		explored.Read(input);
		visible.Clear();
		ResetHeights();
		changes = 1;
	}

	void AreaMap::Write(FILE* output) //57C140.
	{
		fwrite(&minX, 4, 1, output);
		fwrite(&minZ, 4, 1, output);
		fwrite(&maxX, 4, 1, output);
		fwrite(&maxZ, 4, 1, output);
		fwrite(&width, 4, 1, output);
		fwrite(&height, 4, 1, output);
		DWORD count = static_cast<DWORD>(notes.size());
		fwrite(&count, 4, 1, output);
		for (std::vector<AreaMapNote>::iterator i = notes.begin(); i != notes.end(); ++i)
			i->Write(output);
		explored.Write(output);
	}
	AreaMapBits::~AreaMapBits()
	{
		if (data) free(data);
	}
	AreaMapNote::AreaMapNote()
	{
		position[0] = 0;
		position[1] = 0;
		position[2] = 0;
		flag5C = flag5D = 0;
		value20 = 0;
		value24 = 0;
		value28 = 0;
		value2C = 0;
		texture = 0;
		textureWidth = 0;
		textureHeight = 0;
		value30 = -1;
	}
	AreaMapNote::AreaMapNote(const AreaMapNote& other)
	{
		position[0] = 0;
		position[1] = 0;
		position[2] = 0;
		texture = 0;
		*this = other;
	}
	AreaMapNote::~AreaMapNote()
	{
		ClearTexture();
	}
}
