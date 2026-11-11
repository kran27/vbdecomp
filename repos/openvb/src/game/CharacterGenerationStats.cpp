#include "game/CharacterGenerationInterface.hpp"
#include "game/CharacterGenerationChoices.hpp"
#include "game/CounterControl.hpp"
#include "game/GamePlayer.hpp"
#include "game/MessageBoxInterface.hpp"
#include "startup/EngineStringSystem.hpp"
#include "graphics/StringFormat.hpp"
#include "gui/Label.hpp"
#include "gui/Picture.hpp"
#include <cstdio>
#include <algorithm>
#include <stdexcept>

namespace F3
{
 namespace
 {
  static const char* TraitLabel(LONG id) //4249E0, actual private linkage.
  {
   for (LONG index=0;index<16;++index)
    if (CharacterGenerationTraits[index].id==id) return CharacterGenerationTraits[index].labelName;
   return NULL;
  }

  ::GUI::Label* FindLabel(CharacterGenerationInterface& screen,const char* name)
  {return static_cast< ::GUI::Label*>(screen.FindWindow(name));}
  std::string Number(LONG number,const char* format="%d")
  {char buffer[96];std::sprintf(buffer,format,number);return buffer;}
  const DWORD SelectedColor=DWORD(-11575196),NormalColor=DWORD(-16744448);
 }
 LONG CharacterGenerationInterface::RemainingAttributePoints()const
 {LONG result=40;if(character)for(LONG i=0;i<7;++i)result-=character->baseSpecial[i];return result;}
 void CharacterGenerationInterface::RefreshAttributePoints(BYTE immediate)
 {attributePoints->SetValue(RemainingAttributePoints(),(immediate!=0)+1);}
 void CharacterGenerationInterface::RefreshTagSkillPoints(BYTE immediate)
 {
  LONG count=character->TagSkillCount();
  tagSkillPoints->SetValue(LONG(DWORD(3)-DWORD(count)),(immediate!=0)+1);
 }
 void CharacterGenerationInterface::SetAttribute(LONG index,LONG value)
 {
  character->special[index]=value;
  GamePlayer* target=character;
  const char* const names[]={"lblST","lblPE","lblEN","lblCH","lblIN","lblAG","lblLK"};
  const char* labelName=names[index];
  target->baseSpecial[index]=value;
  const DWORD references[]={15,41,42,43,44,45,46,47,48,49,50};
  ::GUI::Label* label=static_cast< ::GUI::Label*>(FindWindow(labelName));
  label->SetStringReference(references[value]);
 }
 void CharacterGenerationInterface::SetNameLabel(const std::string& name)
 {
  ::GUI::Label* label=static_cast< ::GUI::Label*>(FindWindow("lblName"));
  label->SetText(name);
 }
 void CharacterGenerationInterface::SetAgeLabel(LONG age)
 {
  ::GUI::Label* label=static_cast< ::GUI::Label*>(FindWindow("lblAge"));
  std::string text;
  std::string ageLabel;
  GetString(24,ageLabel);
  JE::FormatString(text,"%s %d",ageLabel.c_str(),age);
  label->SetText(text);
 }
 void CharacterGenerationInterface::SetSexLabel(LONG sex)
 {
  std::string text("??");
  ::GUI::Label* label=static_cast< ::GUI::Label*>(FindWindow("lblSex"));
  const DWORD reference=sex==1?26:25;
  GetString(reference,text);
  label->SetText(text);
 }
 void CharacterGenerationInterface::RefreshSkills(){for(LONG i=0;i<13;++i)RefreshSkill(i);}
 void CharacterGenerationInterface::RefreshSkill(LONG index)
 {
  std::string text;
  JE::FormatString(text,"lblSkill%da",index+1);
  ::GUI::Label* labelA=static_cast< ::GUI::Label*>(FindWindow(text));
  JE::FormatString(text,"lblSkill%db",index+1);
  ::GUI::Label* labelB=static_cast< ::GUI::Label*>(FindWindow(text));
  JE::FormatString(text,"lblSkill%dc",index+1);
  ::GUI::Label* labelC=static_cast< ::GUI::Label*>(FindWindow(text));
  if(character->HasTagSkill(DWORD(index)))
  {
   labelA->SetTextColor(SelectedColor);
   labelB->SetTextColor(SelectedColor);
   labelC->SetTextColor(SelectedColor);
  }
  else
  {
   labelA->SetTextColor(NormalColor);
   labelB->SetTextColor(NormalColor);
   labelC->SetTextColor(NormalColor);
  }
  JE::FormatString(text,"%d",character->GetBaseSkill(index));
  labelB->SetText(text);
  JE::FormatString(text,"%d",character->GetTotalSkill(index));
  labelC->SetText(text);
 }

 void CharacterGenerationInterface::ToggleSkill(LONG index)
 {
  if(character->HasTagSkill(DWORD(index))) character->RemoveTagSkill(DWORD(index));
  else if(character->TagSkillCount()<3) character->AddTagSkill(DWORD(index));
  else messageBox->Show(99,1,NULL);
  RefreshSkill(index);RefreshTagSkillPoints(0);
 }
 void CharacterGenerationInterface::RefreshTrait(LONG index)
 {
  ::GUI::Label* label=static_cast< ::GUI::Label*>(FindWindow(std::string(TraitLabel(index))));
  label->SetTextColor(character->HasTrait(DWORD(index))?SelectedColor:NormalColor);
 }
 void CharacterGenerationInterface::ToggleTrait(LONG index)
 {
  if(character->HasTrait(DWORD(index)))
  {
   character->RemoveTrait(DWORD(index));
   RefreshTrait(index);
   return;
  }
  if(character->TraitCount()>=2)
  {
   messageBox->Show(175,1,NULL);
   RefreshTrait(index);
   return;
  }
  character->AddTrait(DWORD(index));
  RefreshTrait(index);
 }
 void CharacterGenerationInterface::RefreshTraits(){for(LONG i=0;i<16;++i)RefreshTrait(i);}
 void CharacterGenerationInterface::RefreshDerived()
 {
  struct DerivedLabel { const char* title; const char* value; LONG attribute; };
  const DerivedLabel labels[]={
   {"lblDerivedStat1a","lblDerivedStat1b",0},
   {"lblDerivedStat2a","lblDerivedStat2b",1},
   {"lblDerivedStat3a","lblDerivedStat3b",2},
   {"lblDerivedStat4a","lblDerivedStat4b",3},
   {"lblDerivedStat5a","lblDerivedStat5b",4},
   {"lblDerivedStat6a","lblDerivedStat6b",5},
   {"lblDerivedStat7a","lblDerivedStat7b",6},
   {"lblDerivedStat8a","lblDerivedStat8b",7},
   {"lblDerivedStat9a","lblDerivedStat9b",8},
   {"lblDerivedStat10a","lblDerivedStat10b",12},
   {"lblDerivedStat11a","lblDerivedStat11b",13},
   {"lblDerivedStat12a","lblDerivedStat12b",9},
   {"lblDerivedStat13a","lblDerivedStat13b",10},
   {"lblDerivedStat14a","lblDerivedStat14b",11}
  };
  for(LONG i=0;i<14;++i)
  {
   ::GUI::Label* label=static_cast< ::GUI::Label*>(FindWindow(labels[i].value));
   if(label)
   {
    LONG attribute=labels[i].attribute;
    LONG value=character?character->GetDerivedAttribute(attribute):0;
    std::string text;
    switch(attribute)
    {
    case 3:case 9:case 10:case 11:JE::FormatString(text,"%d%%",value);break;
    case 8:JE::FormatString(text,"1/%d",value);break;
    case 12:JE::FormatString(text,"%d/1",value);break;
    case 6:JE::FormatString(text,"+%d",value);break;
    default:JE::FormatString(text,"%d",value);break;
    }
    label->SetText(text);
   }
  }
 }
 void CharacterGenerationInterface::RefreshHealth()
 {
  ::GUI::Label* label=static_cast< ::GUI::Label*>(FindWindow("lblHP2"));
  if(label)
  {
   LONG current=0,maximum=0;
   if(character){current=character->currentHealth;maximum=character->GetMaximumHealth();}
   std::string text;
   JE::FormatString(text,"%d/%d",current,maximum);
   label->SetText(text);
  }
 }
 void CharacterGenerationInterface::RefreshArmor()
 {
  character->RecalculateArmor();
  const char* const names[]={"Ballistic","Biological","Electrical","EMP","Heat","Normal"};
  const LONG indices[]={0,1,2,3,5,4};
  for(LONG i=0;i<6;++i)
  {
   char name[96];std::sprintf(name,"lblHead%sValue",names[i]);
   LONG value=i==5?character->resistances[4]:character->thresholds[indices[i]];
   FindLabel(*this,name)->SetText(Number(value));
   std::sprintf(name,"lblBody%sValue",names[i]);FindLabel(*this,name)->SetText(Number(character->resistances[indices[i]]));
  }
 }
 void CharacterGenerationInterface::ShowTooltip(::GUI::Window* window)
 {
  ::GUI::Label* title=static_cast< ::GUI::Label*>(FindWindow("lblToolTipTitle"));
  ::GUI::Label* text=static_cast< ::GUI::Label*>(FindWindow("lblToolTip"));
  ::GUI::Picture* image=static_cast< ::GUI::Picture*>(FindWindow("picToolTip"));
  if(window)
  {
   title->SetStringReference(window->valueCF4);text->SetStringReference(window->valueCD4);
   if(!window->GetStringCF8().empty())
   {image->LoadTexture(window->GetStringCF8(),0,0,0,-1,-1,1);image->SetHidden(0);}
   else image->SetHidden(1);
   title->SetHidden(0);text->SetHidden(0);
  }
  else{title->SetHidden(1);text->SetHidden(1);image->SetHidden(1);}
 }
}
