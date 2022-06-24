#include "win2con.h"

#ifdef _WIN32

#define CALL_CONV __cdecl
typedef uintptr_t ThreadIDType;
typedef void ThreadRetType;

#else

#define CALL_CONV
typedef pthread_t ThreadIDType;
typedef void* ThreadRetType;

#endif

static ThreadIDType conThreadID = 0;

static ThreadRetType CALL_CONV conThread(void* ptr);

void initConInput(void)
{
	#ifdef _WIN32
	
	if (!disableKeyboard)
	{
		conThreadID = _beginthread(&conThread, 0, NULL);
	}

	#else

	if (!disableKeyboard)
	{
		pthread_create(&conThreadID, NULL, &conThread, NULL);
	}

	#endif
}

static ThreadRetType CALL_CONV conThread(void* ptr)
{
	#ifdef _WIN32
	INPUT_RECORD input[128];
	DWORD read;
	HANDLE inputHandle = GetStdHandle(STD_INPUT_HANDLE);

	while (1)
	{
		ReadConsoleInputA(inputHandle, input, 128, &read);
		FlushConsoleInputBuffer(inputHandle);

		for (int i = 0; i < read; i++)
		{
			UINT msg;
			switch (input[i].EventType)
			{
			case KEY_EVENT:
				if (input[i].Event.KeyEvent.bKeyDown) { msg = WM_KEYDOWN; }
				else { break; }

				PostMessageA(hwnd, msg, input[i].Event.KeyEvent.wVirtualKeyCode,
					input[i].Event.KeyEvent.wRepeatCount & 0xFFFF |
					((input[i].Event.KeyEvent.wVirtualScanCode & 0xFF) << 16));

				break;

			case MOUSE_EVENT:
				break;
			}
		}
	}
	#endif
}