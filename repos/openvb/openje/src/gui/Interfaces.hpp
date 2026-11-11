#pragma once

#include <windows.h>

namespace JE
{
	//5967F0. Non-GUI client interface aggregate; no child storage or ownership.
	class Interfaces
	{
	public:
		Interfaces();
		virtual ~Interfaces(); //596830; compiler supplies the deleting wrapper.
		virtual void Initialize(); //404B70.
		virtual void Activate(); //404B70.
		virtual void FinishInitialization(); //404B70.
		virtual void SetLoadingVisible(DWORD visible); //5A9320, actual empty one-argument hook.
		virtual void ShowGameplay(); //404B70.
		virtual void HideGameplay(); //404B70.
		virtual void RefreshSelection(); //404B70.
		virtual void GetScreenRectangle(LONG& x, LONG& y, LONG& width, LONG& height); //596800.
		virtual void OnUnused(); //404B70.
	};
}
