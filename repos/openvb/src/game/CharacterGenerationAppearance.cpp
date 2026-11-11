#include "game/CharacterGenerationInterface.hpp"
#include "game/CharacterGenerationChoices.hpp"
#include "game/CharacterGenerationModel.hpp"
#include "game/CharacterGenerationModelPrivate.hpp"
#include "game/GamePlayer.hpp"
#include "game/EntityTypes.hpp"
#include "model/ModelCache.hpp"
#include "model/ModelInfo.hpp"
#include "model/ModelInstance.hpp"
#include "gui/Label.hpp"
#include <algorithm>
#include <cstdio>
#include <stdexcept>

namespace F3
{
 namespace
 {
  const char MaleTemplate[]="Default_PC.CRT",FemaleTemplate[]="PCFemale.CRT";
  struct AppearanceChoice { LONG value; DWORD reference; };
  const DWORD RaceRefs[]={197,198,199,200};
  const AppearanceChoice SkinChoices[]={{0,201},{1,202},{2,203},{3,204},{4,205},{5,206},{6,207},{7,208},{8,209},{9,210},{10,211},{11,212},{12,213},{13,214},{14,215},{15,216},{16,217},{17,218}};
  const AppearanceChoice PaletteChoices[]={{0,252},{2,250},{4,251},{6,253},{8,254},{1,3077},{3,255},{5,3072},{7,3078},{9,3079}};
  const LONG HeadPaletteValues[]={0,2,4,6,8,1,3,5,7,9};
  const LONG HairPaletteValues[]={0,2,4,6,8,1,3,5,7,9};
  const LONG BeardPaletteValues[]={0,2,4,6,8,1,3,5,7,9};
  void MaterialState(JE::ModelInstance* model)
  {model->SetMaterialFlagBC(0);model->SetAmbientColor(15790320);model->SetAmbientOverride(1);}
 }
 namespace CharacterGenerationModel
 {
  AppearanceChoice HairChoices[10]={{-1,7},{0,241},{1,240},{2,242},{3,243},{4,244},{5,245},{7,247},{8,1843},{9,248}};
  AppearanceChoice BeardChoices[8]={{-1,7},{0,238},{1,241},{2,240},{3,242},{4,243},{5,245},{6,248}};
  std::string Variation(DWORD slot,LONG variation)
  {return JE::g_pModelCache->GetVariationName(slot,variation);}
  std::string Texture(JE::ModelInstance* model,DWORD slot,DWORD index,const char* variation)
  {return model->GetSocketTextureName(slot,index,variation);}
  LONG HairVariation(LONG index){return CharacterGenerationModel::HairChoices[index].value;}
  LONG BeardVariation(LONG index){return CharacterGenerationModel::BeardChoices[index].value;}
 }
 DWORD CharacterGenerationInterface::BodyPalette()const{return PaletteChoices[paletteIndex].value;}
 DWORD CharacterGenerationInterface::HeadPalette()const{return HeadPaletteValues[paletteIndex];}
 DWORD CharacterGenerationInterface::HairPalette()const{return HairPaletteValues[paletteIndex];}
 DWORD CharacterGenerationInterface::BeardPalette()const{return BeardPaletteValues[paletteIndex];}
 void CharacterGenerationInterface::DiscoverBodyVariations()
 {
  JE::ModelInstance* model=character->model;std::vector<LONG>().swap(bodyVariations);bodyIndex=1;
  LONG count=static_cast<LONG>(model->info->models[2].size());
  for(LONG i=0;i<count;++i)if(!model->info->models[2][i].name.empty())
  {CharacterGenerationModel::Variation(2,i);bodyVariations.push_back(i);if(model->variations[2]==DWORD(i))bodyIndex=static_cast<LONG>(bodyVariations.size())-1;}
 }
 void CharacterGenerationInterface::DiscoverHeadVariations()
 {
  JE::ModelInstance* model=character->model;std::vector<LONG>().swap(headVariations);headIndex=0;headVariations.push_back(-1);
  LONG count=static_cast<LONG>(model->info->models[3].size());
  for(LONG i=0;i<count;++i)if(!model->info->models[3][i].name.empty())
  {CharacterGenerationModel::Variation(3,i);headVariations.push_back(i);if(model->variations[3]==DWORD(i))headIndex=static_cast<LONG>(headVariations.size())-1;}
 }
 void CharacterGenerationInterface::SetRace(DWORD race,BYTE resetAppearance)
 {
  std::string name("None");LONG age=21,sex=0,values[7]={5,5,5,5,5,5,5};std::list<DWORD> tags;
  if(character){name=character->characterName;age=character->baseValues[4];sex=character->baseValues[5];for(LONG i=0;i<7;++i)values[i]=character->special[i];tags=character->tagSkills;}
  const char* resource=(race==0||race==2||race==1)?(sex==1?FemaleTemplate:MaleTemplate):NULL;
  if(!resource){delete static_cast<JE::Entity*>(character);character=NULL;selectedResource=NULL;}
  else
  {
   if(selectedResource!=resource)
   {
    selectedResource=resource;delete static_cast<JE::Entity*>(character);character=NULL;
    character=dynamic_cast<GamePlayer*>(CreateGameEntity(10));character->Create(resource,0,0);character->scriptName.clear();
   }
   //Only race0 has a nine-pointer model row before the actual PE table ends.
   if(race!=0)throw std::logic_error("CharacterGeneration42EA2E: nonhuman model table reads beyond authored pointer row6F2834");
   const char* const models[]={"HumMNor","HumMFat","HumMStr","HumMWir","HumFNor","HumFFat","HumFStr","HumFWir"};
   const char* modelName=models[4*(sex==1)+appearancePage];
   if(character->model)
   {
    character->textureOverride=skinTexture<0?std::string():CharacterGenerationModel::Texture(character->model,0,skinTexture,NULL);
    character->SetModel(modelName,1);
   }
   else
   {
    character->SetModel(modelName,1);
    if(character->model){character->textureOverride=skinTexture<0?std::string():CharacterGenerationModel::Texture(character->model,0,skinTexture,NULL);character->model->SetSocketModel(0,character->textureOverride.c_str());}
   }
   character->OnModelLoaded();character->PlayAnimation(-1,-1,0,0,0);character->PlayAnimation(416,0,0,0,0);
   character->characterName=name;character->baseValues[4]=age;character->baseValues[5]=sex;character->baseValues[3]=race;
   for(LONG i=0;i<7;++i)SetAttribute(i,values[i]);
   character->tagSkills.clear();for(std::list<DWORD>::iterator i=tags.begin();i!=tags.end();++i){character->tagSkills.push_back(*i);character->skills[*i]+=20;}
  }
  LONG savedBody=bodyIndex,savedHead=headIndex;DiscoverBodyVariations();DiscoverHeadVariations();bodyIndex=savedBody;headIndex=savedHead;
  if(!character)return;character->baseValues[3]=race;JE::ModelInstance* model=character->model;if(!model)return;
  if(resetAppearance)
  {
   bodyIndex=1;for(size_t i=0;i<bodyVariations.size();++i)if(model->variations[2]==DWORD(bodyVariations[i])){bodyIndex=LONG(i);break;}
   headIndex=0;for(size_t i=0;i<headVariations.size();++i)if(model->variations[3]==DWORD(headVariations[i])){headIndex=LONG(i);break;}
   hairIndex=0;for(LONG i=0;i<10;++i)if(model->variations[4]==DWORD(CharacterGenerationModel::HairChoices[i].value)){hairIndex=i;break;}
   beardIndex=0;for(LONG i=0;i<8;++i)if(model->variations[5]==DWORD(CharacterGenerationModel::BeardChoices[i].value)){beardIndex=i;break;}
  }
  if(bodyIndex>=0&&size_t(bodyIndex)<bodyVariations.size())
  {LONG variation=bodyVariations[bodyIndex];std::string texture=CharacterGenerationModel::SocketTexture(model,2,BodyPalette(),variation);if(resetAppearance)model->SetSocketModel(2,texture.c_str());else model->SetSocket(2,variation,texture.c_str());}
  if(headIndex>=0&&size_t(headIndex)<headVariations.size())
  {LONG variation=headVariations[headIndex];std::string texture=CharacterGenerationModel::SocketTexture(model,3,BodyPalette(),variation);if(resetAppearance)model->SetSocketModel(3,texture.c_str());else model->SetSocket(3,variation,texture.c_str());}
  if(!character->baseValues[5]&&DWORD(hairIndex)<=9)
  {LONG variation=CharacterGenerationModel::HairChoices[hairIndex].value;std::string texture=CharacterGenerationModel::SocketTexture(model,4,HairPalette(),variation);if(resetAppearance)model->SetSocketModel(4,texture.c_str());else model->SetSocket(4,variation,texture.c_str());}
  if(!character->baseValues[5]&&DWORD(beardIndex)<8)
  {LONG variation=CharacterGenerationModel::BeardChoices[beardIndex].value;std::string texture=CharacterGenerationModel::SocketTexture(model,5,BeardPalette(),variation);if(resetAppearance)model->SetSocketModel(5,texture.c_str());else model->SetSocket(5,variation,texture.c_str());}
  MaterialState(model);
 }
 void CharacterGenerationInterface::BuildRaceAppearanceList() //42CDB0.
 {
  CharacterGenerationChoices* list = choices[0];
  list->Clear();
  const char* const widgets[12] = {"btnAppearance11", "lblAppearance11", "btnAppearance12", "lblAppearance12", "btnAppearance13", "lblAppearance13", "btnAppearance14", "lblAppearance14", "btnAppearance15", "lblAppearance15", "btnAppearance16", "lblAppearance16"};
  list->SetRowCount(6);
  for (LONG row = 0; row < 6; ++row)
  {
   ::GUI::Window* button = FindWindow(widgets[2 * row]);
   ::GUI::Label* label = static_cast< ::GUI::Label*>(FindWindow(widgets[2 * row + 1]));
   list->BindRow(row, button, label);
  }
  for (LONG index = 0; index < 4; ++index)
  {
   const DWORD reference = RaceRefs[index];
   list->Add(reference, index, reference, DWORD(-1), "");
  }
  list->BindArrows(FindWindow("btnAppearanceUp1"), FindWindow("btnAppearanceDown1"));
 }
 void CharacterGenerationInterface::BuildSkinAppearanceList() //42D130.
 {
  CharacterGenerationChoices* list = choices[1];
  list->Clear();
  const char* const widgets[12] = {"btnAppearance21", "lblAppearance21", "btnAppearance22", "lblAppearance22", "btnAppearance23", "lblAppearance23", "btnAppearance24", "lblAppearance24", "btnAppearance25", "lblAppearance25", "btnAppearance26", "lblAppearance26"};
  list->SetRowCount(6);
  for (LONG row = 0; row < 6; ++row)
  {
   ::GUI::Window* button = FindWindow(widgets[2 * row]);
   ::GUI::Label* label = static_cast< ::GUI::Label*>(FindWindow(widgets[2 * row + 1]));
   list->BindRow(row, button, label);
  }
  for (LONG index = 0; index < 18; ++index)
  {
   const LONG value = SkinChoices[index].value;
   const DWORD reference = SkinChoices[index].reference;
   list->Add(reference, value, reference, DWORD(-1), "");
  }
  list->BindArrows(FindWindow("btnAppearanceUp2"), FindWindow("btnAppearanceDown2"));
 }
 void CharacterGenerationInterface::BuildBodyAppearanceList() //42D4C0.
 {
  CharacterGenerationChoices* list = choices[2];
  list->Clear();
  const char* const widgets[12] = {"btnAppearance31", "lblAppearance31", "btnAppearance32", "lblAppearance32", "btnAppearance33", "lblAppearance33", "btnAppearance34", "lblAppearance34", "btnAppearance35", "lblAppearance35", "btnAppearance36", "lblAppearance36"};
  list->SetRowCount(6);
  for (LONG row = 0; row < 6; ++row)
  {
   ::GUI::Window* button = FindWindow(widgets[2 * row]);
   ::GUI::Label* label = static_cast< ::GUI::Label*>(FindWindow(widgets[2 * row + 1]));
   list->BindRow(row, button, label);
  }
  const LONG count = static_cast<LONG>(bodyVariations.size());
  for (LONG index = 0; index < count; ++index)
  {
   const LONG variation = bodyVariations[index];
   const DWORD reference = BodyReference(variation);
   list->Add(reference, variation, reference, DWORD(-1), "");
  }
  list->BindArrows(FindWindow("btnAppearanceUp3"), FindWindow("btnAppearanceDown3"));
 }
 void CharacterGenerationInterface::BuildHeadAppearanceList() //42D880.
 {
  CharacterGenerationChoices* list = choices[3];
  list->Clear();
  const char* const widgets[12] = {"btnAppearance41", "lblAppearance41", "btnAppearance42", "lblAppearance42", "btnAppearance43", "lblAppearance43", "btnAppearance44", "lblAppearance44", "btnAppearance45", "lblAppearance45", "btnAppearance46", "lblAppearance46"};
  list->SetRowCount(6);
  for (LONG row = 0; row < 6; ++row)
  {
   ::GUI::Window* button = FindWindow(widgets[2 * row]);
   ::GUI::Label* label = static_cast< ::GUI::Label*>(FindWindow(widgets[2 * row + 1]));
   list->BindRow(row, button, label);
  }
  const LONG count = static_cast<LONG>(headVariations.size());
  for (LONG index = 0; index < count; ++index)
  {
   const LONG variation = headVariations[index];
   const DWORD reference = HeadReference(variation);
   list->Add(reference, variation, reference, DWORD(-1), "");
  }
  list->BindArrows(FindWindow("btnAppearanceUp4"), FindWindow("btnAppearanceDown4"));
 }
 void CharacterGenerationInterface::BuildHairAppearanceList() //42DC40.
 {
  CharacterGenerationChoices* list = choices[4];
  list->Clear();
  ::GUI::Window* heading = FindWindow("lblAppearanceHeading5");
  heading->SetHidden(character->baseValues[5] != 0);
  if (character->baseValues[5]) return;
  const char* const widgets[12] = {"btnAppearance51", "lblAppearance51", "btnAppearance52", "lblAppearance52", "btnAppearance53", "lblAppearance53", "btnAppearance54", "lblAppearance54", "btnAppearance55", "lblAppearance55", "btnAppearance56", "lblAppearance56"};
  list->SetRowCount(6);
  for (LONG row = 0; row < 6; ++row)
  {
   ::GUI::Window* button = FindWindow(widgets[2 * row]);
   ::GUI::Label* label = static_cast< ::GUI::Label*>(FindWindow(widgets[2 * row + 1]));
   list->BindRow(row, button, label);
  }
  for (LONG index = 0; index < 10; ++index)
  {
   const LONG value=CharacterGenerationModel::HairChoices[index].value;
   const DWORD reference=CharacterGenerationModel::HairChoices[index].reference;
   list->Add(reference, value, reference, DWORD(-1), "");
  }
  list->BindArrows(FindWindow("btnAppearanceUp5"), FindWindow("btnAppearanceDown5"));
 }
 void CharacterGenerationInterface::BuildBeardAppearanceList() //42E070.
 {
  CharacterGenerationChoices* list = choices[5];
  list->Clear();
  ::GUI::Window* heading = FindWindow("lblAppearanceHeading6");
  heading->SetHidden(character->baseValues[5] != 0);
  if (character->baseValues[5]) return;
  const char* const widgets[12] = {"btnAppearance61", "lblAppearance61", "btnAppearance62", "lblAppearance62", "btnAppearance63", "lblAppearance63", "btnAppearance64", "lblAppearance64", "btnAppearance65", "lblAppearance65", "btnAppearance66", "lblAppearance66"};
  list->SetRowCount(6);
  for (LONG row = 0; row < 6; ++row)
  {
   ::GUI::Window* button = FindWindow(widgets[2 * row]);
   ::GUI::Label* label = static_cast< ::GUI::Label*>(FindWindow(widgets[2 * row + 1]));
   list->BindRow(row, button, label);
  }
  for (LONG index = 0; index < 8; ++index)
  {
   const LONG value=CharacterGenerationModel::BeardChoices[index].value;
   const DWORD reference=CharacterGenerationModel::BeardChoices[index].reference;
   list->Add(reference, value, reference, DWORD(-1), "");
  }
  list->BindArrows(FindWindow("btnAppearanceUp6"), FindWindow("btnAppearanceDown6"));
 }
 void CharacterGenerationInterface::BuildPaletteAppearanceList() //42E4A0.
 {
  CharacterGenerationChoices* list = choices[6];
  list->Clear();
  const char* const widgets[12] = {"btnAppearance71", "lblAppearance71", "btnAppearance72", "lblAppearance72", "btnAppearance73", "lblAppearance73", "btnAppearance74", "lblAppearance74", "btnAppearance75", "lblAppearance75", "btnAppearance76", "lblAppearance76"};
  list->SetRowCount(6);
  for (LONG row = 0; row < 6; ++row)
  {
   ::GUI::Window* button = FindWindow(widgets[2 * row]);
   ::GUI::Label* label = static_cast< ::GUI::Label*>(FindWindow(widgets[2 * row + 1]));
   list->BindRow(row, button, label);
  }
  for (LONG index = 0; index < 10; ++index)
  {
   const LONG value = PaletteChoices[index].value;
   const DWORD reference = PaletteChoices[index].reference;
   list->Add(reference, value, reference, DWORD(-1), "");
  }
  list->BindArrows(FindWindow("btnAppearanceUp7"), FindWindow("btnAppearanceDown7"));
 }
 void CharacterGenerationInterface::BuildAppearanceList(LONG index)
 {
  if(index==0){BuildRaceAppearanceList();return;}
  if(index==1){BuildSkinAppearanceList();return;}
  if(index==2){BuildBodyAppearanceList();return;}
  if(index==3){BuildHeadAppearanceList();return;}
  if(index==4){BuildHairAppearanceList();return;}
  if(index==5){BuildBeardAppearanceList();return;}
  if(index==6){BuildPaletteAppearanceList();return;}
 }
 void CharacterGenerationInterface::RefreshRaceSelection()
 {
  CharacterGenerationChoices* list=choices[0];
  list->SetSelection(appearancePage);
  list->EnsureSelectionVisible();
 }
 void CharacterGenerationInterface::RefreshSkinSelection()
 {
  CharacterGenerationChoices* list=choices[1];
  list->SetSelection(skinTexture);
  list->EnsureSelectionVisible();
 }
 void CharacterGenerationInterface::RefreshBodySelection()
 {
  CharacterGenerationChoices* list=choices[2];
  LONG selected=bodyIndex;
  if(selected>=0&&selected<LONG(bodyVariations.size()))list->SetSelection(selected);
  else list->SetSelection(-1);
  list->EnsureSelectionVisible();
 }
 void CharacterGenerationInterface::RefreshHeadSelection()
 {
  CharacterGenerationChoices* list=choices[3];
  LONG selected=headIndex;
  if(selected>=0&&selected<LONG(headVariations.size()))list->SetSelection(selected);
  else list->SetSelection(-1);
  list->EnsureSelectionVisible();
 }
 void CharacterGenerationInterface::RefreshHairSelection()
 {
  CharacterGenerationChoices* list=choices[4];
  list->SetSelection(hairIndex);
  list->EnsureSelectionVisible();
 }
 void CharacterGenerationInterface::RefreshBeardSelection()
 {
  CharacterGenerationChoices* list=choices[5];
  list->SetSelection(beardIndex);
  list->EnsureSelectionVisible();
 }
 void CharacterGenerationInterface::RefreshPaletteSelection()
 {
  CharacterGenerationChoices* list=choices[6];
  list->SetSelection(paletteIndex);
  list->EnsureSelectionVisible();
 }
 void CharacterGenerationInterface::SelectAppearance(LONG value)
 {
  LONG group=value/10-1,row=value%10-1;if(group<0||group>=7||row<0||row>=6)return;
  LONG index=choices[group]->GetFirstVisible()+row;
  switch(group)
  {
  case 0:appearancePage=index;RefreshRaceSelection();break;
  case 1:skinTexture=index;RefreshSkinSelection();break;
  case 2:bodyIndex=index;RefreshBodySelection();break;
  case 3:headIndex=index;RefreshHeadSelection();break;
  case 4:hairIndex=index;RefreshHairSelection();break;
  case 5:beardIndex=index;RefreshBeardSelection();break;
  case 6:paletteIndex=index;RefreshPaletteSelection();break;
  }
  if(group==0){SetRace(0,0);return;}
  JE::ModelInstance* model=character->model;
  if(group==1)
  {
   if(!model){character->SetModel(character->modelName.c_str(),1);model=character->model;}
   if(model){character->textureOverride=skinTexture<0?std::string():CharacterGenerationModel::Texture(model,0,skinTexture,NULL);model->SetSocketModel(0,character->textureOverride.c_str());}return;
  }
  if(group>=2&&group<=5)
  {
   if(!character||!model)return;
   LONG variation;
   if(group==2)variation=bodyVariations[bodyIndex];else if(group==3)variation=headVariations[headIndex];else if(group==4){if(DWORD(hairIndex)>9)return;variation=CharacterGenerationModel::HairChoices[hairIndex].value;}else {if(DWORD(beardIndex)>=8)return;variation=CharacterGenerationModel::BeardChoices[beardIndex].value;}
   DWORD palette=group==2?BodyPalette():group==3?HeadPalette():group==4?HairPalette():BeardPalette();
   std::string texture=CharacterGenerationModel::SocketTexture(model,group,palette,variation);model->SetSocket(group,variation,texture.c_str());MaterialState(model);return;
  }
  if(model)
  {
   if(DWORD(paletteIndex)<=9){std::string texture=CharacterGenerationModel::SocketTexture(model,2,BodyPalette(),bodyVariations[bodyIndex]);model->SetSocketModel(2,texture.c_str());texture=CharacterGenerationModel::SocketTexture(model,3,HeadPalette(),headVariations[headIndex]);model->SetSocketModel(3,texture.c_str());}
   if(DWORD(hairIndex)<=9){std::string texture=CharacterGenerationModel::SocketTexture(model,4,HairPalette(),CharacterGenerationModel::HairChoices[hairIndex].value);model->SetSocketModel(4,texture.c_str());}
   if(DWORD(beardIndex)<8){std::string texture=CharacterGenerationModel::SocketTexture(model,5,BeardPalette(),CharacterGenerationModel::BeardChoices[beardIndex].value);model->SetSocketModel(5,texture.c_str());}
  }
 }
}
