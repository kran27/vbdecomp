#include "Map.hpp"
#include "MapEffect.hpp"
#include "MapFog.hpp"
#include "graphics/Octree.hpp"

namespace JE
{
	void Map::InitializeGeometry()
	{
		PublishNotes();
		StitchWaterTiles(10.0f,1.0f);
		AttachPendingEntities();
		for(std::map<std::string,MapEffect*>::iterator i=mapEffects.begin();i!=mapEffects.end();++i)
			i->second->Initialize();
	}
	void Map::PublishNotes()
	{
		if (areaMap)
			for (std::list<AreaMapNote>::const_iterator note=mapNotes.begin();note!=mapNotes.end();++note)
				areaMap->AddNote(note->name,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,
					note->value30,note->position,1);
		mapNotes.clear();
	}
	void Map::AttachPendingEntities()
	{
		while(!entities.empty())
		{
			AttachEntity(entities.front());
			entities.pop_front();
		}
	}
	void Map::GenerateFogHeights()
	{
		if(!areaMap) return;
		areaMap->ResetHeights();
		OCTREE* tree=GetOctree();
		if(!tree) return;
		float bounds[6]; tree->GetLevelBounds(bounds);
		const LONG width=areaMap->width,height=areaMap->height;
		for(LONG row=0;row<height;++row)
		{
			for(LONG first=0;first<width;++first)
			{
				float lowX,highZ,highX,lowZ;
				areaMap->CellToWorld(first,row,&lowX,&highZ,0);
				areaMap->CellToWorld(first+1,row+1,&highX,&lowZ,0);
				float heightValue=bounds[1];
				LONG samples=0,hits=0;
				std::map<LONG,LONG> layers;
				for(float z=highZ;lowZ-0.5f<z;z=float(z-0.5f))
				{
					if(z<lowZ) z=lowZ;
					for(float x=lowX;highX+0.5f>x;x=float(x+0.5f))
					{
						if(x>highX) x=highX;
						float position[3]={x,bounds[4],z};
						float candidate=heightValue;
						++samples;
						if(tree->QueryFloorHeight(position,&candidate,1,0,NULL))
						{
							heightValue = heightValue > candidate ? heightValue : candidate;
							++hits;
						}
						DWORD flags=0;
						if(tree->QueryFloorHeight(position,&candidate,16,1,&flags))
						{
							float layerHeight = bounds[1] > candidate ? bounds[1] : candidate;
							LONG layer=(flags>>5)&15;
							std::map<LONG,LONG>::iterator found=layers.find(layer);
							if (found == layers.end())
							{
								found = layers.insert(std::make_pair(layer,LONG(0))).first;
								found->second = LONG(layerHeight);
							}
							else if (found->second < layerHeight)
								found->second = LONG(layerHeight);
						}
					}
				}
				for(std::map<LONG,LONG>::iterator i=layers.begin();i!=layers.end();++i)
					areaMap->AddLayerHeight(i->first,first,row,float(i->second));
				if(hits<samples)
				{
					for(float z=highZ;lowZ-0.5f<z;z=float(z-0.5f))
					{
						if(z<lowZ) z=lowZ;
						for(float x=lowX;highX+0.5f>x;x=float(x+0.5f))
						{
							if(x>highX) x=highX;
							float position[3]={x,bounds[4],z},candidate=heightValue;
							if(tree->QueryFloorHeight(position,&candidate,17,3,NULL))
								heightValue = heightValue > candidate ? heightValue : candidate;
						}
					}
				}
				areaMap->SetHeight(first,row,heightValue);
			}
		}
	}
}
