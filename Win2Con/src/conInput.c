#include "win2con.h"

#define CALL_CONV __cdecl
typedef uintptr_t ThreadIDType;
typedef void ThreadRetType;

static ThreadIDType conThreadID = 0;

static ThreadRetType CALL_CONV conThread(void* ptr);
static void conKeyEvent(KEY_EVENT_RECORD keyEvent);
static void keyboardControl(WORD vkCode);
static BOOL WINAPI consoleCtrlHandler(DWORD ctrlType);

void initConInput(void)
{
	SetConsoleCtrlHandler(&consoleCtrlHandler, TRUE);
	if (!disableKeyboard)
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
	switch (vkCode)
	{
	case 'Q':
		if (magnifierMode)
		{
			stopMainThread();
			w2cExit(0);
		}
		else
		{
			hwnd = NULL;
		}
		break;

	case 'X':
		stopMainThread();
		w2cExit(0);
		break;

	case 'C':
		pwClientArea = !pwClientArea;
		break;

	case 'T':
		setConsoleTopMost(-1);
		break;
	}
}

static BOOL WINAPI consoleCtrlHandler(DWORD ctrlType)
{
	switch (ctrlType)
	{
	case CTRL_C_EVENT:
	case CTRL_BREAK_EVENT:
		stopMainThread();
		w2cExit(0);
		return TRUE;
	}

	return FALSE;
}