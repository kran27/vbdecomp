#include <winsock2.h>
#include "game/CharacterGenerationInterface.hpp"
#include "game/CharacterGenerationModel.hpp"
#include "game/CharacterGenerationModelPrivate.hpp"
#include "game/GamePlayer.hpp"
#include "game/MessageBoxInterface.hpp"
#include "game/MainMenuServices.hpp"
#include "game/MainMenuInterface.hpp"
#include "startup/GameClient.hpp"
#include "network/CommandProcessor.hpp"
#include "gui/Manager.hpp"
#include "graphics/StringFormat.hpp"
#include "model/ModelCache.hpp"
#include "model/ModelInstance.hpp"
#include "resources/ResourceSystem.hpp"
#include "startup/NetworkComponent.hpp"
#include "startup/SaveDirectory.hpp"
#include "startup/EngineStringSystem.hpp"
#include "network/EngineServer.hpp"
#include "filesystem/Directories.hpp"
#include "filesystem/FileOperations.hpp"
#include <cstdio>
#include <vector>

namespace F3
{
 namespace
 {
  CharacterGenerationInterface* PendingOwner; //Actual707794, static zeroed storage.
  LONG PendingMode; //707790.
 }
 BYTE CharacterGenerationInterface::CanCommit()
 {
  if(RemainingAttributePoints()<=0&&character->TagSkillCount()>=3)return 1;
  PendingOwner=this;PendingMode=0;messageBox->Show(30,1,&MessageBoxCallback);return 0;
 }
 void CharacterGenerationInterface::Commit()
 {
  if(!CanCommit())return;
  character->stringRefs[0]=g_EngineStringSystem.AddString(character->characterName.c_str());
  if(SaveCharacter()){Close();static_cast<JE::Client*>(g_pNetworkInstance->client)->child0x7C->Queue("RequestNewGame");}
 }
 void CharacterGenerationInterface::Cancel()
 {
  if(character){PendingOwner=this;PendingMode=1;messageBox->Show(34,2,&MessageBoxCallback);}
  else
  {
   Close();
   GameClient* client=static_cast<GameClient*>(g_pNetworkInstance->client);
   client->InitializeMainmenuCamera();
   MainMenuInterface* menu=client->Interfaces()->mainMenu;
   menu->SetHidden(0);
   ::GUI::g_Manager.PushModal(menu);
  }
 }
 void CharacterGenerationInterface::MessageBoxCallback(LONG answer)
 {PendingOwner->OnMessageBox(PendingMode,answer);}
 BYTE CharacterGenerationInterface::SaveCharacter()
 {
  if(!character)return 0;
  std::string path=GetSaveDirectoryAbsolute();
  //The original tests the terminator, not the preceding character: a slash is
  //appended even when the supplied save root already ends in one.
  if(path.c_str()[path.size()]!='\\')path+='\\';path+="Characters";
  if(!JE::DirectoryExists(path.c_str())&&!JE::CreateDirectories(path.c_str()))return 0;
  const std::string name=character->characterName;
  path+='\\';path+=name;path+=".CRT";
  if(JE::FileExists(path.c_str()))
  {
   PendingOwner=this;PendingMode=2;
   std::string format;
   std::string text;
   GetString(40,format);
   JE::FormatString(text,format.c_str(),path.c_str());
   messageBox->ShowText(text,2,&MessageBoxCallback);
   outputFilename=path;return 0;
  }
  JE::Resources::ResourceFileHandle* handle=JE::Resources::g_pResourceSystem->OpenStream(path.c_str(),0);
  if(handle){PrepareSave();character->WriteToFile(handle);JE::Resources::g_pResourceSystem->Release(handle,0);}
  if(g_pNetworkInstance->GetMode()&1)
   static_cast<JE::Server*>(g_pNetworkInstance->server)->RegisterCharacter(name.c_str());
  return 1; //Actual AL1 also follows the original NULL-handle path.
 }
 void CharacterGenerationInterface::OnMessageBox(LONG mode,LONG answer)
 {
  if(mode==1)
  {
   if(answer==3)
   {
    Close();
    GameClient* client=static_cast<GameClient*>(g_pNetworkInstance->client);
    client->InitializeMainmenuCamera();
    MainMenuInterface* menu=client->Interfaces()->mainMenu;
    menu->SetHidden(0);
    ::GUI::g_Manager.PushModal(menu);
   }
   return;
  }
  if(mode!=2||answer!=3)return;
  if(!JE::Resources::g_pResourceSystem->FileDelete(outputFilename.c_str())){messageBox->Show(33,0,NULL);return;}
  JE::Resources::ResourceFileHandle* handle=JE::Resources::g_pResourceSystem->OpenStream(outputFilename.c_str(),0);
  if(!handle){messageBox->Show(32,0,NULL);return;}
  PrepareSave();character->WriteToFile(handle);JE::Resources::g_pResourceSystem->Release(handle,0);
  std::string name=character->characterName;
  if(g_pNetworkInstance->GetMode()&1)
   static_cast<JE::Server*>(g_pNetworkInstance->server)->RegisterCharacter(name.c_str());
  Close();
  static_cast<JE::Client*>(g_pNetworkInstance->client)->child0x7C->Queue("RequestNewGame");
 }
 void CharacterGenerationInterface::PrepareSave()
 {
  if(!character)return;
  JE::ModelInstance* model=character->model;
  if(!model)return;
  character->sockets[2].model="";
  character->sockets[2].texture="";
  character->sockets[3].model="";
  character->sockets[3].texture="";
  character->sockets[4].model="";
  character->sockets[4].texture="";
  character->sockets[5].model="";
  character->sockets[5].texture="";
  std::string modelName;
  std::string textureName;
  if(paletteIndex>=0&&paletteIndex<6)
  {
   LONG variation=bodyVariations[bodyIndex];
   if(variation>=0)
   {
    modelName=JE::g_pModelCache->GetAbbreviation(2)+JE::g_pModelCache->GetVariationName(2,variation);
    textureName=CharacterGenerationModel::SocketTexture(model,2,BodyPalette(),variation);
    character->sockets[2].model=modelName;
    character->sockets[2].texture=textureName;
   }
   variation=headVariations[headIndex];
   if(variation>=0)
   {
    modelName=JE::g_pModelCache->GetAbbreviation(3)+JE::g_pModelCache->GetVariationName(3,variation);
    textureName=CharacterGenerationModel::SocketTexture(model,3,HeadPalette(),variation);
    character->sockets[3].model=modelName;
    character->sockets[3].texture=textureName;
   }
  }
  if(!character->baseValues[5]&&hairIndex>=0&&hairIndex<10)
  {
   LONG variation=CharacterGenerationModel::HairChoices[hairIndex].value;
   if(variation>=0)
   {
    modelName=JE::g_pModelCache->GetAbbreviation(4)+JE::g_pModelCache->GetVariationName(4,variation);
    textureName=CharacterGenerationModel::SocketTexture(model,4,HairPalette(),variation);
    character->sockets[4].model=modelName;
    character->sockets[4].texture=textureName;
   }
  }
  if(!character->baseValues[5]&&beardIndex>=0&&beardIndex<8)
  {
   LONG variation=CharacterGenerationModel::BeardChoices[beardIndex].value;
   if(variation>=0)
   {
    modelName=JE::g_pModelCache->GetAbbreviation(5)+JE::g_pModelCache->GetVariationName(5,variation);
    textureName=CharacterGenerationModel::SocketTexture(model,5,BeardPalette(),variation);
    character->sockets[5].model=modelName;
    character->sockets[5].texture=textureName;
   }
  }
 }
}
