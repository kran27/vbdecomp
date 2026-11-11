// Optional dependency smoke probe, compiled by scripts/Verify-Vc7.ps1.
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <mmsystem.h>
#include <objbase.h>
#include <d3d8.h>
#include <d3dx8.h>
#include <cstdio>
#include <map>
#include <stdexcept>
#include <string>
#include <vector>

int main()
{
	std::vector<std::string> strings(1, "SDK-and-STL-ok");
	std::map<std::string, int> values;
	values[strings[0]] = 42;
	bool caught = false;
	try
	{
		throw std::runtime_error(strings[0]);
	}
	catch (const std::runtime_error& error)
	{
		caught = strings[0] == error.what();
	}
	if (!caught || values[strings[0]] != 42)
		return 1;

	HRSRC resource = FindResourceA(NULL, MAKEINTRESOURCEA(1), RT_RCDATA);
	if (!resource || SizeofResource(NULL, resource) != 4)
		return 2;
	WNDCLASSA windowClass = { 0 };
	windowClass.lpfnWndProc = DefWindowProcA;
	windowClass.hInstance = GetModuleHandleA(NULL);
	windowClass.lpszClassName = "VanBurenVc7SdkProbe";
	if (!RegisterClassA(&windowClass))
		return 3;
	HWND window = CreateWindowExA(0, windowClass.lpszClassName, "SDK probe",
		WS_OVERLAPPEDWINDOW, 0, 0, 320, 240, NULL, NULL, windowClass.hInstance, NULL);
	if (!window)
		return 4;
	HDC context = CreateCompatibleDC(NULL);
	if (!context)
		return 5;
	DeleteDC(context);
	DestroyWindow(window);
	UnregisterClassA(windowClass.lpszClassName, windowClass.hInstance);
	HRESULT comResult = CoInitialize(NULL);
	if (FAILED(comResult))
		return 6;
	CoUninitialize();
	IDirect3D8* direct3D = Direct3DCreate8(D3D_SDK_VERSION);
	if (!direct3D)
		return 7;
	direct3D->Release();
	ID3DXMatrixStack* stack = NULL;
	if (FAILED(D3DXCreateMatrixStack(0, &stack)) || !stack)
		return 8;
	if (FAILED(stack->LoadIdentity()) || stack->GetTop()->_11 != 1.0f)
		return 9;
	stack->Release();
	std::printf("MSC=%d FULL=%d %s value=%d resources=ok window=ok COM=ok D3D8=ok D3DX8=ok timer=%lu\n",
		_MSC_VER, _MSC_FULL_VER, strings[0].c_str(), values[strings[0]], timeGetTime());
	return 0;
}
