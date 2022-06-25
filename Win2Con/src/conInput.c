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
static LPARAM getMouseCoords(COORD conCoords);

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
	int pressCounter = 0;

	while (1)
	{
		ReadConsoleInputA(inputHandle, input, 128, &read);
		FlushConsoleInputBuffer(inputHandle);

		for (int i = 0; i < read; i++)
		{
			UINT msg;
			WPARAM wParam;
			LPARAM lParam;

			switch (input[i].EventType)
			{
			case KEY_EVENT:
				if (input[i].Event.KeyEvent.bKeyDown) { msg = WM_KEYDOWN; }
				else { msg = WM_KEYUP; }

				wParam = input[i].Event.KeyEvent.wVirtualKeyCode;

				lParam = input[i].Event.KeyEvent.wRepeatCount & 0xFFFF |
					((input[i].Event.KeyEvent.wVirtualScanCode & 0xFF) << 16);
				if (msg == WM_KEYUP) { lParam |= (unsigned int)0b11 << 30; }

				PostMessageA(hwnd, msg, wParam, lParam);

				break;

			case MOUSE_EVENT:
				wParam = 0;
				if (input[i].Event.MouseEvent.dwControlKeyState == LEFT_CTRL_PRESSED ||
					input[i].Event.MouseEvent.dwControlKeyState == RIGHT_CTRL_PRESSED)
				{
					wParam |= MK_CONTROL;
				}
				if (input[i].Event.MouseEvent.dwControlKeyState == SHIFT_PRESSED)
				{
					wParam |= MK_SHIFT;
				}

				if (input[i].Event.MouseEvent.dwButtonState == FROM_LEFT_1ST_BUTTON_PRESSED)
				{
					wParam |= MK_LBUTTON;
				}
				if (input[i].Event.MouseEvent.dwButtonState == FROM_LEFT_2ND_BUTTON_PRESSED)
				{
					wParam |= MK_MBUTTON;
				}
				if (input[i].Event.MouseEvent.dwButtonState == RIGHTMOST_BUTTON_PRESSED)
				{
					wParam |= MK_RBUTTON;
				}

				if (input[i].Event.MouseEvent.dwEventFlags == 0 ||
					input[i].Event.MouseEvent.dwEventFlags == DOUBLE_CLICK)
				{
					if (input[i].Event.MouseEvent.dwButtonState == FROM_LEFT_1ST_BUTTON_PRESSED)
					{
						if (input[i].Event.MouseEvent.dwEventFlags == DOUBLE_CLICK) { msg = WM_LBUTTONDBLCLK; }
					}
					if (input[i].Event.MouseEvent.dwButtonState == FROM_LEFT_2ND_BUTTON_PRESSED)
					{
						wParam |= MK_MBUTTON;
					}
					if (input[i].Event.MouseEvent.dwButtonState == RIGHTMOST_BUTTON_PRESSED)
					{
						wParam |= MK_RBUTTON;
					}
					pressCounter++;
				}
				else if (input[i].Event.MouseEvent.dwEventFlags == MOUSE_MOVED)
				{

				}

				lParam = getMouseCoords(input[i].Event.MouseEvent.dwMousePosition);

				PostMessageA(hwnd, msg, wParam, lParam);
				break;
			}
		}
	}
	#endif
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

	if (!pwClientArea) { ScreenToClient(hwnd, &point); }

	return (point.x & 0xFFFF) | ((point.y & 0xFFFF) << 16);
}