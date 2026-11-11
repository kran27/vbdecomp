#pragma once

#include <windows.h>
#include <cstdio>
#include <cstddef>

// Typed host adaptation of the octree fread/fseek callback pair bound by
// Graphics580E20 through5AD940. FILE and ResourceFileHandle never share storage.
class OctreeInput
{
public:
	virtual ~OctreeInput() {}
	virtual size_t Read(void* output, size_t size, size_t count) = 0;
	virtual int Seek(LONG offset, int origin) = 0;
};

class OctreeFileInput : public OctreeInput
{
public:
	explicit OctreeFileInput(FILE* value) : file(value) {}
	virtual size_t Read(void* output, size_t size, size_t count)
	{
		return fread(output, size, count, file);
	}
	virtual int Seek(LONG offset, int origin) { return fseek(file, offset, origin); }
private:
	FILE* file;
};

inline size_t OctreeRead(void* output, size_t size, size_t count, OctreeInput& input)
{
	return input.Read(output, size, count);
}
inline int OctreeSeek(OctreeInput& input, LONG offset, int origin)
{
	return input.Seek(offset, origin);
}
