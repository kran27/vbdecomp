#include "GraphicsScene.hpp"
#include "Graphics.hpp"
#include "CameraController.hpp"
#include "MapMaskManager.hpp"
#include "OctreeRender.hpp"
#include "SystemInterface.hpp"
#include "entity/World.hpp"
#include "entity/Map.hpp"
#include "gui/Manager.hpp"
#include <set>
#include <cstdlib>
#include <stdexcept>

namespace
{
	OCTREE* currentSceneOctree = NULL; //Original707F08.
}

namespace JE
{
	GraphicsSceneState::GraphicsSceneState(::Graphics& owner)
		: cameraControl(owner.cameraControl), renderer(owner.renderer),
		maskManager(owner.maskManager), shadowMode(owner.shadowMode),
		startupMovies(owner.startupMovies), sceneEnabled(owner.sceneEnabled),
		bounds(owner.bounds), debugBounds(owner.debugBounds),
		currentOctree(owner.currentOctree)
	{
	}
	void SetSceneOctree(OCTREE* tree) { currentSceneOctree = tree; }
	OCTREE* GetSceneOctree() { return currentSceneOctree; }

}
