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
static void keyboardControl(WORD vkCode);
static void conKeyEvent(KEY_EVENT_RECORD keyEvent);
static void conMouseEvent(MOUSE_EVENT_RECORD mouseEvent);
static LPARAM getMouseCoords(COORD conCoords);

void initConInput(void)
{
	#ifdef _WIN32
	
	if (!disableKeyboard || enableInput)
	{
		conThreadID = _beginthread(&conThread, 0, NULL);
	}

	#else

	if (!disableKeyboard || enableInput)
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
				if ((input[i].Event.KeyEvent.dwControlKeyState &
					RIGHT_ALT_PRESSED) && !disableKeyboard)
				{
					if (input[i].Event.KeyEvent.bKeyDown)
					{
						keyboardControl(input[i].Event.KeyEvent.wVirtualKeyCode);
					}
				}
				else if (enableInput)
				{
					conKeyEvent(input[i].Event.KeyEvent);
				}
				break;

			case MOUSE_EVENT:
				if (enableInput)
				{
					conMouseEvent(input[i].Event.MouseEvent);
				}
				break;
			}
		}
	}
	#endif
}

static void keyboardControl(WORD vkCode)
{
	if (vkCode == 'Q') { hwnd = NULL; }
	else if (vkCode == 'X') { w2cExit(0); }
	else if (vkCode == 'C') { pwClientArea = !pwClientArea; }
	else if (vkCode == 'T') { setConsoleTopMost(-1); }
}

static void conKeyEvent(KEY_EVENT_RECORD keyEvent)
{
	UINT msg;
	if (keyEvent.bKeyDown) { msg = WM_KEYDOWN; }
	else { msg = WM_KEYUP; }

	WPARAM wParam = keyEvent.wVirtualKeyCode;
	LPARAM lParam = keyEvent.wRepeatCount & 0xFFFF |
		((keyEvent.wVirtualScanCode & 0xFF) << 16);
	if (msg == WM_KEYUP) { lParam |= (unsigned int)0b11 << 30; }

	PostMessageA(hwnd, msg, wParam, lParam);
}

static void conMouseEvent(MOUSE_EVENT_RECORD mouseEvent)
{
	static int pressCounterL = 0;
	static int pressCounterM = 0;
	static int pressCounterR = 0;

	UINT msg = 0;
	WPARAM wParam = 0;

	if (mouseEvent.dwControlKeyState & LEFT_CTRL_PRESSED ||
		mouseEvent.dwControlKeyState & RIGHT_CTRL_PRESSED)
	{
		wParam |= MK_CONTROL;
	}
	if (mouseEvent.dwControlKeyState & SHIFT_PRESSED)
	{
		wParam |= MK_SHIFT;
	}

	if (mouseEvent.dwButtonState & FROM_LEFT_1ST_BUTTON_PRESSED)
	{
		wParam |= MK_LBUTTON;
	}
	if (mouseEvent.dwButtonState & FROM_LEFT_2ND_BUTTON_PRESSED)
	{
		wParam |= MK_MBUTTON;
	}
	if (mouseEvent.dwButtonState & RIGHTMOST_BUTTON_PRESSED)
	{
		wParam |= MK_RBUTTON;
	}

	if (mouseEvent.dwEventFlags == 0 ||
		mouseEvent.dwEventFlags == DOUBLE_CLICK)
	{
		if (mouseEvent.dwButtonState & FROM_LEFT_1ST_BUTTON_PRESSED)
		{
			if (mouseEvent.dwEventFlags == DOUBLE_CLICK) { msg = WM_LBUTTONDBLCLK; }
			else if (pressCounterL % 2 == 0) { msg = WM_LBUTTONDOWN; }
			else { msg = WM_LBUTTONUP; }
			pressCounterL++;
		}
		else if (mouseEvent.dwButtonState & FROM_LEFT_2ND_BUTTON_PRESSED)
		{
			if (mouseEvent.dwEventFlags == DOUBLE_CLICK) { msg = WM_MBUTTONDBLCLK; }
			else if (pressCounterM % 2 == 0) { msg = WM_MBUTTONDOWN; }
			else { msg = WM_MBUTTONUP; }
			pressCounterM++;
		}
		else if (mouseEvent.dwButtonState & RIGHTMOST_BUTTON_PRESSED)
		{
			if (mouseEvent.dwEventFlags == DOUBLE_CLICK) { msg = WM_RBUTTONDBLCLK; }
			else if (pressCounterR % 2 == 0) { msg = WM_RBUTTONDOWN; }
			else { msg = WM_RBUTTONUP; }
			pressCounterR++;
		}
	}
	else if (mouseEvent.dwEventFlags == MOUSE_WHEELED)
	{
		wParam |= (mouseEvent.dwButtonState & 0xFFFF0000);
		msg = WM_MOUSEWHEEL;
	}
	else if (mouseEvent.dwEventFlags == MOUSE_HWHEELED)
	{
		wParam |= (mouseEvent.dwButtonState & 0xFFFF0000);
		msg = WM_MOUSEHWHEEL;
	}
	else if (mouseEvent.dwEventFlags == MOUSE_MOVED)
	{
		msg = WM_MOUSEMOVE;
	}

	if (msg == 0) { return; }

	LPARAM lParam = getMouseCoords(mouseEvent.dwMousePosition);

	SendMessageA(hwnd, msg, 1, lParam);
}

static LPARAM getMouseCoords(COORD conCoords)
{
	POINT point = { 0 };

	switch (scalingMode)
	{
	case SM_FILL:
		point.x = (LONG)((double)conCoords.X * ((double)wndW / (double)imgW));
		point.y = (LONG)((double)conCoords.Y * ((double)wndH / (double)imgH));
		break;
	}

	if (!pwClientArea)
	{
		RECT wndRect;
		GetWindowRect(hwnd, &wndRect);
		point.x += wndRect.left;
		point.y += wndRect.top;
		ScreenToClient(hwnd, &point);
	}

	return (point.x & 0xFFFF) | ((point.y & 0xFFFF) << 16);
}