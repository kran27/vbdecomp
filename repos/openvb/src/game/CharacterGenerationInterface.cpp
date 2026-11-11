#include <winsock2.h>
#include "game/CharacterGenerationInterface.hpp"
#include "game/CharacterGenerationChoices.hpp"
#include "game/CounterControl.hpp"
#include "game/MessageBoxInterface.hpp"
#include "game/NameAgeSexInterface.hpp"
#include "game/GamePlayer.hpp"
#include "game/OptionsInterface.hpp"
#include "game/MainMenuServices.hpp"
#include "startup/NetworkComponent.hpp"
#include "network/EngineClient.hpp"
#include "startup/GameClient.hpp"
#include "graphics/CameraControl.hpp"
#include "graphics/Viewport.hpp"
#include "graphics/Light.hpp"
#include "graphics/RenderManager.hpp"
#include "model/ModelInstance.hpp"
#include "gui/Button.hpp"
#include "gui/Label.hpp"
#include "gui/Manager.hpp"
#include "gui/Event.hpp"
#include <mmsystem.h>
#include <cstring>
#include <cmath>
#include <cstdlib>
#include <stdexcept>

namespace F3
{
 CharacterGenerationTrait CharacterGenerationTraits[16] =
 {
  {"btnTrait11","lblTrait11",0},
  {"btnTrait12","lblTrait12",1},
  {"btnTrait13","lblTrait13",2},
  {"btnTrait14","lblTrait14",3},
  {"btnTrait15","lblTrait15",4},
  {"btnTrait16","lblTrait16",5},
  {"btnTrait17","lblTrait17",6},
  {"btnTrait18","lblTrait18",7},
  {"btnTrait21","lblTrait21",8},
  {"btnTrait22","lblTrait22",9},
  {"btnTrait23","lblTrait23",10},
  {"btnTrait24","lblTrait24",11},
  {"btnTrait25","lblTrait25",12},
  {"btnTrait26","lblTrait26",13},
  {"btnTrait27","lblTrait27",14},
  {"btnTrait28","lblTrait28",15},
 };
 namespace
 {
  static LONG SkillButtonIndex(const char* name) //424950, actual private linkage.
  {
   if (std::strlen(name)>8 && !std::strncmp(name,"btnSkill",8))
   {
    const char* number=name+8;
    if (std::atol(number)>=1) return std::atol(number)-1;
   }
   return -1;
  }
  static LONG TraitButtonIndex(const char* name) //4249A0, actual private linkage.
  {
   for (LONG index=0;index<16;++index)
    if (!_stricmp(CharacterGenerationTraits[index].buttonName,name)) return CharacterGenerationTraits[index].id;
   return -1;
  }

  const DWORD BodyRefs[]={219,220,221,1455,222,223,224,225,226,227,228,229,230,231,232,233,234,235,236,237};
  const DWORD HeadRefs[]={1458,1459,1460};
 }
 DWORD CharacterGenerationInterface::BodyReference(LONG variation) const
 {
  if (variation < 0 || variation >= 20) return DWORD(-1);
  return BodyRefs[variation];
 }
 DWORD CharacterGenerationInterface::HeadReference(LONG variation) const
 {
  if (variation < 0 || variation >= 3) return 7;
  return HeadRefs[variation];
 }
 void CharacterGenerationInterface::ResetAppearance() //42F710.
 {
  SetRace(0, 0);
  BuildBodyAppearanceList();
  BuildHeadAppearanceList();
  BuildHairAppearanceList();
  BuildBeardAppearanceList();
 }
 void CharacterGenerationInterface::ApplyNameAgeSex() //426F80.
 {
  std::string text;
  LONG years, gender;
  nameAgeSex->GetValues(text, years, gender);
  character->characterName = text;
  character->baseValues[4] = years;
  character->baseValues[5] = gender;
 }
 void NameAgeSexInterface::RefreshAge()
 {ageCounter->SetValue(age,2);}
 namespace
 {
  float FloatBits(DWORD value) { float result; std::memcpy(&result,&value,4); return result; }
  ::GUI::Label* Label(CharacterGenerationInterface& owner,const char* name)
  { return static_cast< ::GUI::Label*>(owner.FindWindow(name)); }
  const char* const PassiveLabels[] = {"lblST",
"lblPE",
"lblEN",
"lblCH",
"lblIN",
"lblAG",
"lblLK",
"lblSkillsHeading1",
"lblSkillsHeading1a",
"lblSkillsHeading1b",
"lblSkillsHeading2",
"lblSkillsHeading3",
"lblSkillsHeading4",
"lblSkill1a",
"lblSkill1b",
"lblSkill1c",
"lblSkill2a",
"lblSkill2b",
"lblSkill2c",
"lblSkill3a",
"lblSkill3b",
"lblSkill3c",
"lblSkill4a",
"lblSkill4b",
"lblSkill4c",
"lblSkill5a",
"lblSkill5b",
"lblSkill5c",
"lblSkill6a",
"lblSkill6b",
"lblSkill6c",
"lblSkill7a",
"lblSkill7b",
"lblSkill7c",
"lblSkill8a",
"lblSkill8b",
"lblSkill8c",
"lblSkill9a",
"lblSkill9b",
"lblSkill9c",
"lblSkill10a",
"lblSkill10b",
"lblSkill10c",
"lblSkill11a",
"lblSkill11b",
"lblSkill11c",
"lblSkill12a",
"lblSkill12b",
"lblSkill12c",
"lblSkill13a",
"lblSkill13b",
"lblSkill13c",
"lblSkill14a",
"lblSkill14b",
"lblSkill14c",
"lblTrait11",
"lblTrait12",
"lblTrait13",
"lblTrait14",
"lblTrait15",
"lblTrait16",
"lblTrait17",
"lblTrait18",
"lblTrait21",
"lblTrait22",
"lblTrait23",
"lblTrait24",
"lblTrait25",
"lblTrait26",
"lblTrait27",
"lblTrait28",
"lblDerivedStats",
"lblResistances",
"lblDerivedStat1a",
"lblDerivedStat2a",
"lblDerivedStat3a",
"lblDerivedStat4a",
"lblDerivedStat5a",
"lblDerivedStat6a",
"lblDerivedStat7a",
"lblDerivedStat8a",
"lblDerivedStat9a",
"lblDerivedStat10a",
"lblDerivedStat11a",
"lblDerivedStat12a",
"lblDerivedStat13a",
"lblDerivedStat14a",
"lblDerivedStat1b",
"lblDerivedStat2b",
"lblDerivedStat3b",
"lblDerivedStat4b",
"lblDerivedStat5b",
"lblDerivedStat6b",
"lblDerivedStat7b",
"lblDerivedStat8b",
"lblDerivedStat9b",
"lblDerivedStat10b",
"lblDerivedStat11b",
"lblDerivedStat12b",
"lblDerivedStat13b",
"lblDerivedStat14b",
"lblAppearanceHeading1",
"lblAppearanceHeading2",
"lblAppearanceHeading3",
"lblAppearanceHeading4",
"lblAppearanceHeading5",
"lblAppearanceHeading6",
"lblAppearanceHeading7",
"lblAppearance11",
"lblAppearance12",
"lblAppearance13",
"lblAppearance14",
"lblAppearance15",
"lblAppearance16",
"lblAppearance21",
"lblAppearance22",
"lblAppearance23",
"lblAppearance24",
"lblAppearance25",
"lblAppearance26",
"lblAppearance31",
"lblAppearance32",
"lblAppearance33",
"lblAppearance34",
"lblAppearance35",
"lblAppearance36",
"lblAppearance41",
"lblAppearance42",
"lblAppearance43",
"lblAppearance44",
"lblAppearance45",
"lblAppearance46",
"lblAppearance51",
"lblAppearance52",
"lblAppearance53",
"lblAppearance54",
"lblAppearance55",
"lblAppearance56",
"lblAppearance61",
"lblAppearance62",
"lblAppearance63",
"lblAppearance64",
"lblAppearance65",
"lblAppearance66",
"lblAppearance71",
"lblAppearance72",
"lblAppearance73",
"lblAppearance74",
"lblAppearance75",
"lblAppearance76",
"lblStats",
"lblDamageThresholds",
"lblDTHead",
"lblHeadBallistic",
"lblHeadBiological",
"lblHeadElectrical",
"lblHeadEMP",
"lblHeadHeat",
"lblHeadNormal",
"lblHeadBallisticValue",
"lblHeadBiologicalValue",
"lblHeadElectricalValue",
"lblHeadEMPValue",
"lblHeadHeatValue",
"lblHeadNormalValue",
"lblDTBody",
"lblBodyBallistic",
"lblBodyBiological",
"lblBodyElectrical",
"lblBodyEMP",
"lblBodyHeat",
"lblBodyNormal",
"lblBodyBallisticValue",
"lblBodyBiologicalValue",
"lblBodyElectricalValue",
"lblBodyEMPValue",
"lblBodyHeatValue",
"lblBodyNormalValue",
"lblHP1",
"lblHP2"};
 }
 CharacterGenerationInterface::CharacterGenerationInterface() : JE::EngineInterface("CharacterGeneration"), GameInterface("CharacterGeneration"),
  character(NULL), attributePoints(NULL), tagSkillPoints(NULL), field6C(0,0,0), cameraTarget(0,0,0),
  alternateCameraTarget(0,0,0), selectedResource(NULL), modelViewport(NULL),modelCamera(NULL),
  modelAzimuth(FloatBits(1078530011)),modelElevation(10),modelDistance(26),modelTime(timeGetTime()),
  modelVisible(1),modelLight(NULL),modelDragging(0),mouseX(0),mouseY(0)
 {
  for (LONG i=0;i<7;++i) attributes[i]=NULL;
  LoadResource("CharacterGeneration.INT");
  messageBox=new MessageBoxInterface;
  nameAgeSex=new NameAgeSexInterface;
  const char* const abbreviations[]={"ST","PE","EN","CH","IN","AG","LK"};
  for (LONG i=0;i<7;++i)
  {
   CounterControl* counter=attributes[i]=new CounterControl;
   counter->SetDigits(2);counter->SetMinimum(1);counter->SetMaximum(10);
   std::string name=std::string("win")+abbreviations[i];
   counter->BindDigit(0,FindWindow(name+"1"));counter->BindDigit(1,FindWindow(name+"10"));
   name=std::string("btn")+abbreviations[i];
   counter->BindButtons(FindWindow(name+"+"),FindWindow(name+"-"));counter->animationStep=0.1f;
  }
  attributePoints=new CounterControl;attributePoints->SetDigits(2);
  attributePoints->BindDigit(0,FindWindow("winCharPoints1"));attributePoints->BindDigit(1,FindWindow("winCharPoints10"));attributePoints->animationStep=0.1f;
  tagSkillPoints=new CounterControl;tagSkillPoints->SetDigits(2);
  tagSkillPoints->BindDigit(0,FindWindow("winTagSkills1"));tagSkillPoints->BindDigit(1,FindWindow("winTagSkills10"));tagSkillPoints->animationStep=0.1f;
  for(LONG i=0;i<7;++i) choices[i]=new CharacterGenerationChoices;
  ::GUI::Window* window=FindWindow("winModel");
  if(window)
  {
   modelViewport=new Gfx_Viewport;modelCamera=new Gfx_CamCtrl_Orbit;
   LONG width,height,x,y;window->GetSize(width,height);window->GetAbsolutePosition(x,y);
   float scaleX,scaleY;::GUI::g_Manager.GetScale(scaleX,scaleY);
   D3DVIEWPORT8 view={static_cast<DWORD>(x*scaleX),static_cast<DWORD>(y*scaleY),static_cast<DWORD>(width*scaleX),static_cast<DWORD>(height*scaleY),0,1};
   modelViewport->SetViewport(view);modelViewport->clearColor=0;
   for(LONG i=0;i<4;++i) modelViewport->vertices[i].diffuse=0;
   modelViewport->SetCameraController(modelCamera);
  }
  for(size_t i=0;i<sizeof(PassiveLabels)/sizeof(*PassiveLabels);++i)
  { window=FindWindow(PassiveLabels[i]);if(window)window->SetValue3B(0); }
  field6C=D3DXVECTOR3(FloatBits(1132572180),0,FloatBits(1132959826));field78=FloatBits(1124204544);
  cameraTarget=D3DXVECTOR3(FloatBits(1119213322),FloatBits(1096349123),FloatBits(1138141102));
  cameraAzimuth=FloatBits(1134336410);cameraElevation=FloatBits(0xC0900000);cameraDistance=FloatBits(1131741184);cameraFov=FloatBits(1108082688);
  alternateCameraTarget=D3DXVECTOR3(FloatBits(1125240013),FloatBits(1107296256),FloatBits(1136535142));
  alternateAzimuth=FloatBits(1134336410);alternateElevation=FloatBits(0xC0900000);alternateDistance=FloatBits(1131741184);alternateFov=FloatBits(1108082688);
  D3DLIGHT8 lightDescriptor;
  Gfx_InitLightDescriptor(&lightDescriptor,D3DLIGHT_POINT,
   1.0f,1.0f,1.0f,0.0f,0.0f,1.0f,0.0f,0.0f,0.0f,
   FloatBits(1153138688),1.0f,1.0f,FloatBits(1008981770),0.0f,0.0f,0.0f);
  modelLight=new Gfx_Light(lightDescriptor,0);
  modelLight->flag48=1;modelLight->transform._41=0;modelLight->transform._42=100;modelLight->transform._43=0;
  modelLight->light.Position=D3DXVECTOR3(0,100,0);
  optionsOverlay=new ::GUI::Window(0);optionsOverlay->Resize(1024,768);optionsOverlay->ApplySkin("DefaultInterface.ini");
  optionsOverlay->SetColor(0xFF000000);optionsOverlay->SetFrameVisible(0);optionsOverlay->SetHidden(1);optionsOverlay->SetValue3B(1);
  optionsOverlay->SetAbsolutePosition(0,0);optionsOverlay->SetAlpha(200);optionsOverlay->SetValue60(1);optionsOverlay->SetMaterialModes(1);optionsOverlay->SetValue68(1);AddWindow(optionsOverlay);
 }
 CharacterGenerationInterface::~CharacterGenerationInterface()
 {
  for(LONG i=0;i<7;++i)delete attributes[i];delete attributePoints;delete tagSkillPoints;
  delete modelViewport;delete modelCamera;delete modelLight;modelLight=NULL;
  delete static_cast<JE::Entity*>(character);
  for(LONG i=0;i<7;++i)delete choices[i];
  //The original destructor deliberately omits the separately registered MessageBox/NAS helpers.
 }
 void CharacterGenerationInterface::Open()
 {
  if (character)
  {
   delete static_cast<GameCreature*>(character);
   character = NULL;
  }
  selectedResource = NULL;
  modelAzimuth = 3.1415927f;
  modelElevation = 10;
  modelDistance = 26;
  modelTime = timeGetTime();
  modelDragging = 0;
  mouseX = 0;
  mouseY = 0;
  appearancePage = 0;
  skinTexture = 12;
  bodyIndex = 1;
  headIndex = 0;
  hairIndex = 0;
  beardIndex = 0;
  paletteIndex = 0;

  ::GUI::Window* thresholds = FindWindow("winThresholds");
  thresholds->SetHidden(1);
  ::GUI::Window* stats = FindWindow("winStats");
  stats->SetHidden(0);
  ::GUI::Window* model = FindWindow("winModel");
  model->SetHidden(0);
  ::GUI::Window* statsButton = FindWindow("btnStats");
  statsButton->BringToFront();
  ::GUI::Window* thresholdsButton = FindWindow("btnThresholds");
  thresholdsButton->BringToFront();
  modelVisible = 1;
  SetRace(0,1);
  Refresh();
  ShowTooltip(NULL);
  SetHidden(0);
  ApplyCamera();
 }
 void CharacterGenerationInterface::Close()
 {
  SetHidden(1);
  CounterControl* counters[9] = {attributes[0], attributes[1], attributes[2], attributes[3],
   attributes[4], attributes[5], attributes[6], attributePoints, tagSkillPoints};
  for(LONG i=0;i<9;++i)counters[i]->Finish();
  if(character){delete static_cast<GameCreature*>(character);character=NULL;}
  selectedResource=NULL;
 }
 void CharacterGenerationInterface::ApplyCamera()
 {
  JE::Client* client=static_cast<JE::Client*>(g_pNetworkInstance->client);
  client->child0x84->SetTarget(cameraTarget);client->child0x84->SetAzimuth(cameraAzimuth);
  client->child0x84->SetElevation(cameraElevation);client->child0x84->SetDistance(cameraDistance);client->child0x84->SetFieldOfView(cameraFov);
 }
 void CharacterGenerationInterface::BeforeUpdate()
 {
  if(!optionsOverlay->GetHidden())
  {
   OptionsInterface* options=static_cast<GameClient*>(g_pNetworkInstance->client)->Interfaces()->options;
   if(options&&options->GetHidden())optionsOverlay->SetHidden(1);
  }
  CounterControl* counters[9] = {attributes[0], attributes[1], attributes[2], attributes[3],
   attributes[4], attributes[5], attributes[6], attributePoints, tagSkillPoints};
  for(LONG i=0;i<9;++i)counters[i]->Update();
  for(LONG i=0;i<7;++i)if(choices[i])choices[i]->Update();
  OptionsInterface* options=static_cast<GameClient*>(g_pNetworkInstance->client)->Interfaces()->options;
  if(modelVisible&&character&&modelViewport&&(!options||options->GetHidden()))
  {
   ::GUI::g_Manager.AddViewport(modelViewport);D3DXVECTOR3 position=character->position;modelTime=timeGetTime();
   Gfx_CamCtrl_Orbit* camera=modelCamera;
   camera->target=D3DXVECTOR3(position.x,position.y+3.0999999f,position.z);camera->dirty=1;
   modelCamera->SetOrbit(modelAzimuth,modelElevation*0.017453292f,modelDistance);modelCamera->Update();
   Gfx_Viewport* viewport=modelViewport;
   Gfx_RenderManager* renderer=viewport->renderer;
   viewport->flag1C=1;renderer->QueueMeshInstances(&viewport->instance,1);
  }
 }
 void CharacterGenerationInterface::BeforeDraw()
 {
  OptionsInterface* options=static_cast<GameClient*>(g_pNetworkInstance->client)->Interfaces()->options;
  if(modelVisible&&character&&modelViewport&&(!options||options->GetHidden()))
  {
   if(character)character->GetAnimationTime();character->ClearLights();if(modelLight)character->AddLight(modelLight);
   character->model->Submit(modelViewport,0,0.05f,0xFF00FFFF);
  }
 }
 void CharacterGenerationInterface::Refresh()
 {
  if(!character)return;
  SetNameLabel(character->characterName);SetAgeLabel(character->baseValues[4]);SetSexLabel(character->baseValues[5]);
  attributes[0]->SetValue(character->baseSpecial[0],2);
  attributes[1]->SetValue(character->baseSpecial[1],2);
  attributes[2]->SetValue(character->baseSpecial[2],2);
  attributes[3]->SetValue(character->baseSpecial[3],2);
  attributes[4]->SetValue(character->baseSpecial[4],2);
  attributes[5]->SetValue(character->baseSpecial[5],2);
  attributes[6]->SetValue(character->baseSpecial[6],2);
  RefreshAttributePoints(1);RefreshSkills();RefreshTagSkillPoints(1);RefreshTraits();RefreshDerived();RefreshHealth();RefreshArmor();
  DiscoverBodyVariations();
  DiscoverHeadVariations();
  BuildRaceAppearanceList();
  BuildSkinAppearanceList();
  BuildBodyAppearanceList();
  BuildHeadAppearanceList();
  BuildHairAppearanceList();
  BuildBeardAppearanceList();
  BuildPaletteAppearanceList();
  RefreshRaceSelection();
  RefreshSkinSelection();
  RefreshBodySelection();
  RefreshHeadSelection();
  RefreshHairSelection();
  RefreshBeardSelection();
  RefreshPaletteSelection();
 }
 int CharacterGenerationInterface::OnEvent(::GUI::Window* window,::GUI::Event* event)
 {
  OptionsInterface* options=MainMenuServices::GetOptionsInterface();if(options&&!options->GetHidden())return 0;
  if(event->type==4||event->type==12)ShowTooltip(window);
  CounterControl* counters[9];for(LONG i=0;i<7;++i)counters[i]=attributes[i];counters[7]=attributePoints;counters[8]=tagSkillPoints;
  for(LONG i=0;i<9;++i)
  {
   CounterControl* counter=counters[i];
   if((event->type==4||event->type==7||event->type==0)&&window->type==1)
   {
    LONG remaining=RemainingAttributePoints();
    if((window==counter->decrementButton||(window==counter->incrementButton&&remaining>0))&&counter->Process(window,event))
    {
     if(i<7){SetAttribute(i,counter->GetValue());RefreshAttributePoints(0);RefreshSkills();RefreshDerived();RefreshHealth();RefreshArmor();}
     return JE::EngineInterface::OnEvent(window,event);
    }
   }
  }
  if(event->type==6&&window->type==1)
  {
   const char* name=window->GetName();
   if(!_stricmp(name,"btnOptions")){optionsOverlay->SetHidden(0);optionsOverlay->BringToFront();MainMenuServices::QueueCommand("OptionsWindow");MainMenuServices::GetOptionsInterface()->BringToFront();}
   else if(!_stricmp(name,"btnDone"))Commit();else if(!_stricmp(name,"btnCancel"))Cancel();
   else if(!_stricmp(name,"btnName")||!_stricmp(name,"btnAge")||!_stricmp(name,"btnSex")||!_stricmp(name,"btnRace"))
   {if(character)nameAgeSex->Show(this,character,character->characterName,character->baseValues[4],character->baseValues[5]);}
   else if(!_stricmp(name,"btnStats")||!_stricmp(name,"btnThresholds"))
   {
    BYTE thresholds=!_stricmp(name,"btnThresholds");FindWindow("winStats")->SetHidden(thresholds);FindWindow("winThresholds")->SetHidden(!thresholds);FindWindow("winModel")->SetHidden(thresholds);
    FindWindow("btnStats")->BringToFront();FindWindow("btnThresholds")->BringToFront();modelVisible=!thresholds;
   }
   else
   {
    LONG skill=SkillButtonIndex(window->GetName());
    if(skill>=0)ToggleSkill(skill);
    else
    {
     LONG trait=TraitButtonIndex(window->GetName());
     if(trait>=0)ToggleTrait(trait);
     else
     {
      const char* appearanceName=window->GetName();
      if(std::strlen(appearanceName)>13&&!_strnicmp(appearanceName,"btnAppearance",13)&&std::atol(appearanceName+13)>=1)SelectAppearance(std::atol(appearanceName+13));
     }
    }
   }
  }
  else if(event->type==4&&!_stricmp(window->GetName(),"winModel")){modelDragging=1;mouseX=event->x;mouseY=event->y;}
  else if(event->type==5)modelDragging=0;
  else if(event->type==2)
  {
   if(!(event->buttons&1))modelDragging=0;
   if(modelDragging){modelAzimuth+=(event->x-mouseX)*0.02f;if(modelAzimuth>6.2831855f)modelAzimuth=static_cast<float>(std::fmod(modelAzimuth,6.2831855f));modelElevation+=(event->y-mouseY)*0.5f;if(modelElevation>90)modelElevation=90;if(modelElevation<0)modelElevation=0;mouseX=event->x;mouseY=event->y;}
  }
  else if(event->type==17||event->type==19)
  {if(event->x==1)Cancel();else if(event->x==28||event->x==156||event->x==284)Commit();else if(event->x==41)MainMenuServices::QueueCommand("ConsoleWindow");}
  for(LONG i=0;i<7;++i)if(choices[i]&&choices[i]->Process(window,event))break;
  return JE::EngineInterface::OnEvent(window,event);
 }
}
