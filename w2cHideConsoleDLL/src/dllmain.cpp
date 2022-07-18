/*
* Source code of "w2cHideConsoleDLL.dll".
* It's injected into "conhost.exe" to make console window
* invisible for BitBlt, when using magnifier mode.
* Check "magnifierMode.c".
* 
* Project from which I took the method of hiding window from BitBlt:
* https://github.com/radiantly/Invisiwind
*/

#include <string.h>
#include <Windows.h>

#define W2C_HCDLL_MAX_STR 64

void excludeWindow()
{
	const char* CONHOST_WINDOW_CLASS = "ConsoleWindowClass";
	const char* WT_WINDOW_CLASS = "CASCADIA_HOSTING_WINDOW_CLASS";

	const char* WINDOW_TITLE_HIDE = "Win2Con-Magnifier/Hide";
	const char* WINDOW_TITLE_SHOW = "Win2Con-Magnifier/Show";

	HWND hwnd = NULL;
	char windowClass[W2C_HCDLL_MAX_STR];
	char windowTitle[W2C_HCDLL_MAX_STR];

	do
	{
		hwnd = FindWindowExA(NULL, hwnd, NULL, NULL);
		GetClassNameA(hwnd, windowClass, W2C_HCDLL_MAX_STR);

		if (!strcmp(windowClass, CONHOST_WINDOW_CLASS) ||
			!strcmp(windowClass, WT_WINDOW_CLASS))
		{
			GetWindowTextA(hwnd, windowTitle, W2C_HCDLL_MAX_STR);

			if (!strcmp(windowTitle, WINDOW_TITLE_HIDE))
			{
				SetWindowDisplayAffinity(hwnd, WDA_EXCLUDEFROMCAPTURE);
			}
			else if (!strcmp(windowTitle, WINDOW_TITLE_SHOW))
			{
				SetWindowDisplayAffinity(hwnd, WDA_NONE);
			}
		}
	} while (hwnd);
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

