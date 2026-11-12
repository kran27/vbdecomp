#pragma once

#include <windows.h>

namespace JE
{
	class Client;
	class DebugWindow;
	namespace WorldContainerServices
	{
		DebugWindow* CaptureInspectionDebug(); //406DBD, before popup callbacks.
		void WriteInspectionDebug(DebugWindow* window); //406E34, captured actual window.
		void GetScreenRectangle(LONG& x, LONG& y, LONG& width, LONG& height);
		Client& GetClient(); //Borrowed actual Network.client.
	}
}
