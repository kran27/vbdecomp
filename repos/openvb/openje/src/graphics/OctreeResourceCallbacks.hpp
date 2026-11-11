#pragma once

#include <windows.h>
#include <cstddef>
#include <cstdio>

namespace JE { namespace Resources { struct ResourceFileHandle; } }

//5AD940 owns three writable callback slots, initially the actual CRT functions.
//FILE* is the callback transport type; Graphics providers decode a resource
//handle at their boundary instead of accessing it as a CRT FILE object.
typedef size_t (__cdecl *OctreeResourceReadCallback)(void*, size_t, size_t, FILE*);
typedef int (__cdecl *OctreeResourceSeekCallback)(FILE*, LONG, int);
typedef LONG (__cdecl *OctreeResourceTellCallback)(FILE*);

extern OctreeResourceReadCallback g_OctreeReadCallback; //6FF708, initially fread619089.
extern OctreeResourceSeekCallback g_OctreeSeekCallback; //6FF70C, initially fseek618F57.
extern OctreeResourceTellCallback g_OctreeTellCallback; //6FF710, initially ftell61A243.

OctreeResourceTellCallback SetOctreeResourceCallbacks(OctreeResourceReadCallback read,
	OctreeResourceSeekCallback seek, OctreeResourceTellCallback tell);
size_t ReadOctreeResourceInput(void* output, size_t size, size_t count,
	JE::Resources::ResourceFileHandle* handle);
int SeekOctreeResourceInput(JE::Resources::ResourceFileHandle* handle, LONG offset, int origin);
LONG TellOctreeResourceInput(JE::Resources::ResourceFileHandle* handle);
