#include "SoundFile.hpp"
#include "Music.hpp"
#include "SoundSystem.hpp"
#include "resources/ResourceSystem.hpp"

namespace JE
{
	const char* CJumplist::Jump(int index) const
	{
		return index < 0 || index >= jumpCount ? NULL : jumps + 9 * index;
	}

	const char* CJumplist::Segment(int row, int column) const
	{
		if (row < 0 || row >= segmentCount || column < 0) return NULL;
		const int columns = jumpCount * 2 + 2;
		return column >= columns ? NULL : segments + 9 * (row * columns + column);
	}

	SoundFile::SoundFile() : resourceName()
	{
		data = NULL;
		dataSize = 0;
		cursor = 0;
		resourceName = "";
	}

	SoundFile::~SoundFile()
	{
	}

	int SoundFile::Read(const char* name)
	{
		Resources::ResourceSystem& resources = *AudioBackend::g_System->resources;
		resourceName.assign(name);
		Resources::ResourceData* resource = resources.Demand(name, 1);
		if (!resource)
			return 6;
		Parse(static_cast<const BYTE*>(resource->data), resource->size);
		resources.ReleaseResource(resource);
		return 0;
	}

	void SoundFile::ReadToken(char* output, int capacity)
	{
		*output = 0;
		const LONG size = dataSize;
		if (cursor >= size) return;
		do
		{
			if (*data == '\t')
			{
				if (cursor >= size) return;
				++data;
				++cursor;
			}
			if (*data == ' ')
			{
				if (cursor >= size) return;
				++data;
				++cursor;
			}
			if (*data == '\n' || *data == '\r')
			{
				if (cursor >= size) return;
				++data;
				++cursor;
			}
			if (*data == '/')
				for (;;)
				{
					if (cursor >= size) return;
					++data;
					++cursor;
					if (*data == '\n') break;
				}
		}
		while (*data == '\t' || *data == ' ' || *data == '\n' || *data == '\r' || *data == '/');
		int index = 0;
		--capacity;
		for (; index < capacity; ++index)
		{
			if (cursor >= dataSize) return;
			output[index] = *data;
			++data;
			++cursor;
			// Native looks ahead at EOF and leaves capacity-limited words unterminated.
			if (*data == '\t' || *data == ' ' || *data == '\n' || *data == '\r')
			{
				output[index + 1] = 0;
				if (cursor < dataSize)
				{
					++data;
					++cursor;
				}
				return;
			}
		}
	}
}
