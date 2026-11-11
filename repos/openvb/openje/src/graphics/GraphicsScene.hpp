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

	//Field view used by existing Client/World consumers. The owner constructor
	//aliases actual Graphics members; the default constructor retains standalone
	//fixture storage. Neither mode owns any pointed-to graphics resource.
	struct GraphicsSceneState
	{
	private:
		struct StandaloneFields
		{
			Gfx_CamCtrl* cameraControl;
			Octree2Gfx* renderer;
			MapMaskManager* maskManager;
			DWORD shadowMode;
			BYTE startupMovies, sceneEnabled, bounds, debugBounds;
			OCTREE* currentOctree;
		};
		StandaloneFields standalone; //Unused/unread in an actual-owner view.

	public:
		GraphicsSceneState(); //580E20's corresponding scalar/pointer defaults.
		explicit GraphicsSceneState(::Graphics& owner); //Non-owning live field aliases; owner must outlive this view.
		void ReadStartupMovieConfiguration(const Configuration::Settings& settings); //581C30's main-settings+56 factor.
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
	struct GraphicsSceneInputs
	{
		const BYTE* clientFlag10A; //Nonzero suppresses GUI Manager4B37F0.
		const BYTE* clientFlag108; //Nonzero suppresses terrain submission4BC6F0.
		::GUI::Manager* guiManager; //Actual Manager required only when10A is zero.
	};

	void PrepareGraphicsScene(GraphicsSceneState& state, World& world,
		const GraphicsSceneInputs& inputs, BYTE& mapChanged); //5828D0, selects map after GUI Update.
	void SetSceneOctree(OCTREE* tree); //4B81A0, actual global707F08.
	OCTREE* GetSceneOctree(); //4B81B0.
}
