#pragma once

#include "EngineClient.hpp"

namespace JE
{
	//External workspace bookkeeping. Every pointer is borrowed; this is not
	//part of the original Client object and never owns a game/UI/Graphics child.
	struct EngineClientBinding
	{
		EngineClientBinding();
		Client::UnconsumedPollingFrame unconsumedPollingFrame;
		void* unconsumedPollingFrameContext;
		Client::UnconsumedInterfaceAction unconsumedInterfaceAction;
		void* unconsumedInterfaceActionContext;
		Client::UnconsumedDialogueUpdate unconsumedDialogueUpdate;
		void* unconsumedDialogueUpdateContext;
		GraphicsLightingState* borrowedLightingState;
		GraphicsSceneState* borrowedGraphicsState;
		ClientInputState* borrowedInputState;
		ClientInputServices* borrowedInputServices;
		GUI::GraphicsInputServices* borrowedGraphicsInput;
	};

	EngineClientBinding& GetEngineClientBinding(const Client& client);
	EngineClientBinding* FindEngineClientBinding(const Client& client); //Does not allocate or publish a binding.
	//The external workspace owner clears bookkeeping at its own entry/exit.
	//Canonical Client construction/destruction never touches host bindings.
	void ClearEngineClientBinding(const Client& client);
}
