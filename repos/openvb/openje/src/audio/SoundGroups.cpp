#include "SoundGroups.hpp"
#include "SoundInstance.hpp"
#include <cstdlib>
#include <cstdio>
#include <cstring>

namespace JE
{
	SoundGroup::SoundGroup()
	{
		voiceLimit=100;
		fadeIn=0;
		fadeOut=0;
		volume=100;
		pitch=100;
		fullSoundRadius=40;
		noSoundRadius=60;
		minimumVolume=20;
		id=-1;
		matrixIndex=-1;
		presets.clear();
	}
	SoundGroup::~SoundGroup()
	{
		presets.clear();
	}
	void SoundGroup::Parse(const BYTE* input,DWORD size)
	{
		data=input;
		dataSize=size;
		cursor=0;
		char token[32];
		ReadToken(token,32); ReadToken(token,32); voiceLimit=std::atol(token);
		ReadToken(token,32); ReadToken(token,32); fadeIn=std::atol(token);
		ReadToken(token,32); ReadToken(token,32); fadeOut=std::atol(token);
		ReadToken(token,32); ReadToken(token,32); volume=std::atol(token);
		ReadToken(token,32); ReadToken(token,32); pitch=std::atol(token);
		ReadToken(token,32); ReadToken(token,32); fullSoundRadius=std::atol(token);
		ReadToken(token,32); ReadToken(token,32); noSoundRadius=std::atol(token);
		ReadToken(token,32); ReadToken(token,32); minimumVolume=std::atol(token);
		ReadToken(token,32); ReadToken(token,32); ReadToken(token,32);
		while (token[0]!='}')
		{
			presets.push_back(token);
			ReadToken(token,32);
		}
	}
	int SoundGroup::Contains(const char* name) const
	{
		for (std::list<std::string>::const_iterator it=presets.begin();it!=presets.end();++it)
		{
			const std::string candidate=*it; //Original copies each list value before comparing.
			if (!_stricmp(candidate.c_str(),name)) return 1;
		}
		return 0;
	}
	MasterSoundGroups::MasterSoundGroups() : count(0),fallback(new SoundGroup)
	{
		std::memset(mixing,0,sizeof(mixing));
		std::memset(groups,0,sizeof(groups));
	}
	MasterSoundGroups::~MasterSoundGroups()
	{
		Clear();
		delete fallback;
	}
	void MasterSoundGroups::Parse(const BYTE* input,DWORD size)
	{
		data=input;
		dataSize=size;
		cursor=0;
		char token[32],filename[32],warning[256];
		ReadToken(token,32);
		count=std::atol(token);
		if (count>16) count=16;
		for (LONG i=0;i<count;++i)
		{
			ReadToken(token,32);
			std::sprintf(filename,"%s%s","",token); //Original empty prefix at nullbytes.
			SoundGroup* group=new SoundGroup;
			if (group->Read(filename))
			{
				std::sprintf(warning,"Warning - Could not load groupfile, %s.\n",filename);
				OutputDebugStringA(warning);
			}
			group->matrixIndex=i;
			group->id=i;
			groups[i]=group;
		}
		for (LONG heading=0;heading<count;++heading) ReadToken(token,32);
		for (LONG row=0;row<count;++row)
		{
			ReadToken(token,32);
			for (LONG column=0;column<count;++column)
			{
				ReadToken(token,32);
				mixing[row][column]=std::atol(token);
			}
		}
	}
	SoundGroup* MasterSoundGroups::FindGroup(const char* name)
	{
		for (LONG i=0;i<count;++i)
			if (groups[i] && groups[i]->Contains(name)) return groups[i];
		return fallback;
	}
	LONG MasterSoundGroups::MixingAmount(LONG source,LONG target) const
	{
		if (source<0 || source>=count || source>=16 || target<0 || target>=count || target>=16)
			return 0;
		return mixing[source][target];
	}
	void MasterSoundGroups::Clear()
	{
		for (int i=0;i<16;++i)
		{
			if (groups[i])
			{
				delete groups[i];
				groups[i]=NULL;
			}
		}
		count=0;
	}
	int VfxSoundInstance::Random(int minimum,int maximum)
	{
		int result=0;
		if (maximum>=minimum) result=minimum+std::rand()%(maximum-minimum+1);
		return result;
	}

	int __stdcall SoundRandom(int minimum,int maximum)
	{
		int result=0;
		if (maximum>=minimum) result=minimum+std::rand()%(maximum-minimum+1);
		return result;
	}

}
