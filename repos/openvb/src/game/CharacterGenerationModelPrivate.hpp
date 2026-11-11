#pragma once

#include "game/CharacterGenerationModel.hpp"
#include "model/ModelCache.hpp"
#include "model/ModelInfo.hpp"
#include "model/ModelInstance.hpp"

namespace F3 { namespace CharacterGenerationModel
{
  struct AppearanceChoice { LONG value; DWORD reference; };
  extern AppearanceChoice HairChoices[10];
  extern AppearanceChoice BeardChoices[8];

  static std::string SocketTexture(JE::ModelInstance* model,DWORD slot,DWORD palette,LONG variation)
  {
   std::string output;
   if(model&&palette>0)
   {
    std::string name;
    if(variation>=0)
    {
     std::string modelName=model->GetSocketModelName(slot,DWORD(variation));
     if(!modelName.empty())name=JE::g_pModelCache->GetVariationName(slot,variation);
    }
    if(palette<model->info->GetSocketTextureCount(slot,name.c_str()))
     output=model->GetSocketTextureName(slot,palette,name.c_str());
   }
   return output;
  }
} }
