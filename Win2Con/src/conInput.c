#include "win2con.h"

#define CALL_CONV __cdecl
typedef uintptr_t ThreadIDType;
typedef void ThreadRetType;

static ThreadIDType conThreadID = 0;

static ThreadRetType CALL_CONV conThread(void* ptr);
static void conKeyEvent(KEY_EVENT_RECORD keyEvent);
static void keyboardControl(WORD vkCode);

void initConInput(void)
{
	if (!disableKeyboard || magnifierMode)
	{
		conThreadID = _beginthread(&conThread, 0, NULL);
	}
}

static ThreadRetType CALL_CONV conThread(void* ptr)
{
	INPUT_RECORD input[128];
	DWORD read;
	HANDLE inputHandle = GetStdHandle(STD_INPUT_HANDLE);
	int pressCounter = 0;

	while (1)
	{
		ReadConsoleInputA(inputHandle, input, 128, &read);
		FlushConsoleInputBuffer(inputHandle);

		for (int i = 0; i < read; i++)
		{
			switch (input[i].EventType)
			{
			case KEY_EVENT:
				conKeyEvent(input[i].Event.KeyEvent);
				break;
			}
		}
	}
}

static void conKeyEvent(KEY_EVENT_RECORD keyEvent)
{
	if ((keyEvent.dwControlKeyState & RIGHT_ALT_PRESSED) &&
		!disableKeyboard)
	{
		if (keyEvent.bKeyDown)
		{
			keyboardControl(keyEvent.wVirtualKeyCode);
		}
	}
	return;
}

static void keyboardControl(WORD vkCode)
{
	if (vkCode == 'Q') { hwnd = NULL; }
	else if (vkCode == 'X') { w2cExit(0); }
	else if (vkCode == 'C') { pwClientArea = !pwClientArea; }
	else if (vkCode == 'T') { setConsoleTopMost(-1); }
}