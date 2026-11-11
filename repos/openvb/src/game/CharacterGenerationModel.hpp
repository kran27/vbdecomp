#pragma once
#include <windows.h>
#include <string>
namespace JE { class ModelInstance; }
namespace F3 { namespace CharacterGenerationModel
{
 std::string Texture(JE::ModelInstance*,DWORD slot,DWORD index,const char* variation); //566FB0/566E90.
 std::string Variation(DWORD slot,LONG variation); //583D60.
 LONG HairVariation(LONG index);
 LONG BeardVariation(LONG index);
} }
