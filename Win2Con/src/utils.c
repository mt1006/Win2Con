#include "win2con.h"

#define W2C_MAX_OLD_TITLE_LEN 1024
#define W2C_MAX_NEW_TITLE_LEN 128
#define W2C_WAIT_FOR_SET_TITLE 100

double getTime(void)
{
	return (double)clock() / (double)CLOCKS_PER_SEC;
}

void strToLower(char* str)
{
	for (int i = 0; i < strlen(str); i++)
	{
		str[i] = (char)tolower((int)str[i]);
	}
}

void getConsoleWindow(void)
{
	if (conHWND) { return; }

	RECT clientRect;
	conHWND = GetConsoleWindow();
	GetClientRect(conHWND, &clientRect);

	if (clientRect.right == 0 || clientRect.bottom == 0)
	{
		wchar_t oldConTitle[W2C_MAX_OLD_TITLE_LEN];
		char newConTitle[W2C_MAX_NEW_TITLE_LEN];

		GetConsoleTitleW(oldConTitle, W2C_MAX_OLD_TITLE_LEN);
		sprintf(newConTitle, "Win2Con-(%d/%d)",
			(int)time(NULL), (int)GetCurrentProcessId());
		SetConsoleTitleA(newConTitle);
		Sleep(W2C_WAIT_FOR_SET_TITLE);
		HWND newConHWND = FindWindowA(NULL, newConTitle);
		SetConsoleTitleW(oldConTitle);

		if (newConHWND)
		{
			conHWND = newConHWND;
			wtDragBarHWND = FindWindowExW(conHWND, NULL,
				L"DRAG_BAR_WINDOW_CLASS", NULL);
		}
	}
}

void clearScreen(void)
{
	// https://docs.microsoft.com/en-us/windows/console/clearing-the-screen
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	SMALL_RECT scrollRect;
	COORD scrollTarget;
	CHAR_INFO fill;

	GetConsoleScreenBufferInfo(outputHandle, &csbi);

	scrollRect.Left = 0;
	scrollRect.Top = 0;
	scrollRect.Right = csbi.dwSize.X;
	scrollRect.Bottom = csbi.dwSize.Y;
	
	scrollTarget.X = 0;
	scrollTarget.Y = (SHORT)(0 - csbi.dwSize.Y);

	fill.Char.UnicodeChar = TEXT(' ');
	fill.Attributes = csbi.wAttributes;

	ScrollConsoleScreenBuffer(outputHandle, &scrollRect, NULL, scrollTarget, &fill);

	csbi.dwCursorPosition.X = 0;
	csbi.dwCursorPosition.Y = 0;

	SetConsoleCursorPosition(outputHandle, csbi.dwCursorPosition);
}

void setDefaultColor(void)
{
	if (ansiEnabled)
	{
		fputs("\x1B[0m", stdout);
	}
	else if (settings.setColorMode == SCM_WINAPI)
	{
		#ifdef _WIN32
		SetConsoleTextAttribute(outputHandle,
			FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
		#endif
	}
}

void setConsoleTopMost(int topMost)
{
	static bool isTopMost = false;
	getConsoleWindow();

	if (topMost == -1) { topMost = !isTopMost; }

	if (topMost && !isTopMost)
	{
		SetWindowPos(conHWND, HWND_TOPMOST,
			0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);

		LONG exStyle = GetWindowLongA(conHWND, GWL_EXSTYLE);
		exStyle |= WS_EX_TRANSPARENT;
		SetWindowLongPtrA(conHWND, GWL_EXSTYLE, exStyle);

		isTopMost = true;
	}
	else if (!topMost && isTopMost)
	{
		SetWindowPos(conHWND, HWND_NOTOPMOST,
			0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);

		LONG exStyle = GetWindowLongA(conHWND, GWL_EXSTYLE);
		exStyle &= ~WS_EX_TRANSPARENT;
		SetWindowLongPtrA(conHWND, GWL_EXSTYLE, exStyle);

		isTopMost = true;
	}
}

void setCursorPos(int x, int y)
{
	COORD cursor = { (SHORT)x,(SHORT)y };
	if (!settings.disableCLS) { SetConsoleCursorPosition(outputHandle, cursor); }
}

size_t getOutputArraySize(void)
{
	const int CSTD_16_CODE_LEN = 6;   // "\x1B[??m?"
	const int CSTD_256_CODE_LEN = 12; // "\x1B[38;5;???m?"
	const int CSTD_RGB_CODE_LEN = 20; // "\x1B[38;2;???;???;???m?"

	if (settings.useFakeConsole)
	{
		return imgW * imgH * sizeof(GlConsoleChar);
	}

	switch (settings.colorMode)
	{
	case CM_CSTD_GRAY:
		return (imgW + 1) * imgH * sizeof(char);
	case CM_CSTD_16:
		return ((imgW * imgH * CSTD_16_CODE_LEN) + imgH) * sizeof(char);
	case CM_CSTD_256:
		return ((imgW * imgH * CSTD_256_CODE_LEN) + imgH) * sizeof(char);
	case CM_CSTD_RGB:
		return ((imgW * imgH * CSTD_RGB_CODE_LEN) + imgH) * sizeof(char);
	case CM_WINAPI_GRAY:
	case CM_WINAPI_16:
		return imgW * imgH * sizeof(CHAR_INFO);
	}
}

uint8_t rgbToAnsi256(uint8_t r, uint8_t g, uint8_t b)
{
	// https://stackoverflow.com/questions/15682537/ansi-color-specific-rgb-sequence-bash
	if (r == g && g == b)
	{
		if (r < 8) { return 16; }
		if (r > 248) { return 231; }
		return (uint8_t)round((((double)r - 8.0) / 247.0) * 24.0) + 232;
	}

	return (uint8_t)(16.0
		+ (36.0 * round((double)r / 255.0 * 5.0))
		+ (6.0 * round((double)g / 255.0 * 5.0))
		+ round((double)b / 255.0 * 5.0));
}

void stopMainThread(void)
{
	stopMainThreadVal = 1;
	while (stopMainThreadVal != 2) { Sleep(5); }
}

void w2cExit(int code)
{
	disableMagnifierMode();
	setConsoleTopMost(0);
	setDefaultColor();
	restoreConsoleMode();
	exit(code);
}

void error(const char* description, const char* fileName, int line)
{
	puts("\nSomething went wrong...");
	printf("%s [%s:%d]\n", description, fileName, line);
	w2cExit(-1);
}