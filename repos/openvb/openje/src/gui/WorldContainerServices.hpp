#pragma once

#include <windows.h>

namespace JE
{
	class Client;
	class DebugWindow;
	//Explicit workspace requests retained separately from native debug ownership.
	//Actual WorldContainer click sound uses its own embedded CSound.
	class WorldContainerExternalServices
	{
	public:
		virtual ~WorldContainerExternalServices() {}
		virtual void InitializeClickSound() = 0;
		virtual void LoadClickSound(const char* name, BYTE value) = 0;
		virtual void PlayClickSound(DWORD first, DWORD second, DWORD third) = 0;
		virtual void DestroyClickSound() = 0;
		virtual void WriteInspectionDebug() = 0;
	};
	WorldContainerExternalServices* SetWorldContainerExternalServices(WorldContainerExternalServices* services);

	namespace WorldContainerServices
	{
		void InitializeClickSound(); //4C8100 boundary.
		void LoadClickSound(const char* name, BYTE value);
		void PlayClickSound(DWORD first, DWORD second, DWORD third); //4C7EB0 boundary.
		void DestroyClickSound();
		DebugWindow* CaptureInspectionDebug(); //406DBD, before popup callbacks; explicit entry may borrow.
		void WriteInspectionDebug(DebugWindow* window); //406E34, captured actual window or explicit provider.
		void GetScreenRectangle(LONG& x, LONG& y, LONG& width, LONG& height);
		Client& GetClient(); //Borrowed actual Network.client.
	}
}
