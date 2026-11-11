// OpenJE

#include <windows.h>

#include <ctime>
#include <mbstring.h>
#include "JE.hpp"
#include "F3.hpp"
#include "startup/CommonException.hpp"

// Original 0x56B810 startup policy. Reconstruction remains incomplete.
static int RunOriginalStartup(HINSTANCE hInstance, LPSTR lpCmdLine, int nCmdShow)
{
	_mbsstr(reinterpret_cast<const unsigned char*>(lpCmdLine),
		reinterpret_cast<const unsigned char*>("-leakcheck"));
	char szModulePath[MAX_PATH];
	int nMainResult;

	F3::g_instance = hInstance;
	if (!GetModuleFileNameA(NULL, szModulePath, MAX_PATH))
		JE::FatalError("Unable to get module file name.");

	F3::SetupSaveDirectory(szModulePath);
	F3::SetupConfigFile();
	F3::SetupLogFile();

	timeBeginPeriod(1);
	F3::g_nSystemTimeMS = timeGetTime();

	F3::Display::CreateGameWindow(nCmdShow);
	F3::SetStartupTime(std::time(0));
	try
	{
		// The original passes this pointer to Main, which never reads it.
		GetCommandLineA();
		nMainResult = F3::Main();
	}
	catch (Common::Exception error)
	{
		// 56B8DC passes the borrowed message as the format, without arguments.
		JE::FatalError(error.message);
	}
	F3::Display::DestroyGameWindow();
	timeEndPeriod(1);
	F3::ShutdownLogFile();
	F3::SaveSettingsFile();
	return nMainResult;
}

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	return RunOriginalStartup(hInstance, lpCmdLine, nCmdShow);
}
