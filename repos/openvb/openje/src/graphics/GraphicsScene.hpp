#pragma once

#include <windows.h>
#include "configuration/ConfigurationTypes.hpp"

class OCTREE;
class Octree2Gfx;
class Gfx_CamCtrl;
class MapMaskManager;
struct Graphics;
namespace GUI { class Manager; }

namespace JE
{
	class World;

	//Non-owning aliases of the live Graphics fields.
	struct GraphicsSceneState
	{
	public:
		explicit GraphicsSceneState(::Graphics& owner); //Non-owning live field aliases; owner must outlive this view.
		Gfx_CamCtrl*& cameraControl; //Graphics+0C, actual three-slot interface.
		Octree2Gfx*& renderer; //+10.
		MapMaskManager*& maskManager; //+14.
		DWORD& shadowMode; //+50.
		BYTE& startupMovies; //Frontend Graphics+56; distinct from backend stencil-shadow BYTE56.
		BYTE& sceneEnabled;
		BYTE& bounds;
		BYTE& debugBounds; //+5C/+5D/+5E.
		OCTREE*& currentOctree; //+60, published only after the map-change prefix.

	private:
		GraphicsSceneState(const GraphicsSceneState&);
		GraphicsSceneState& operator=(const GraphicsSceneState&);
	};
	void SetSceneOctree(OCTREE* tree); //4B81A0, actual global707F08.
	OCTREE* GetSceneOctree(); //4B81B0.
}
