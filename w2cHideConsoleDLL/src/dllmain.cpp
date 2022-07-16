/*
* Source code of "w2cHideConsoleDLL.dll".
* It's injected into "conhost.exe" to make console window
* invisible for BitBlt, when using magnifier mode.
* Check "magnifierMode.c".
* 
* Project from which I took the method of hiding window from BitBlt:
* https://github.com/radiantly/Invisiwind
*/

#include <Windows.h>

void excludeWindow()
{
	const char* WINDOW_CLASS_NAME = "ConsoleWindowClass";

	HWND windowHandle = NULL;
	do
	{
		windowHandle = FindWindowExA(NULL, windowHandle, WINDOW_CLASS_NAME, NULL);
		SetWindowDisplayAffinity(windowHandle, WDA_EXCLUDEFROMCAPTURE);
	} while (windowHandle);
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		excludeWindow();
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}

	return TRUE;
}

