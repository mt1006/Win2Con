#include "win2con.h"

double getTime(void)
{
	#ifdef _WIN32

	return (double)clock() / (double)CLOCKS_PER_SEC;

	#else

	struct timespec timeSpec;
	clock_gettime(CLOCK_REALTIME, &timeSpec);
	return (double)((timeSpec.tv_nsec / 1000000) + (timeSpec.tv_sec * 1000)) / 1000.0;

	#endif
}

void clearScreen(HANDLE outputHandle)
{
	#ifdef _WIN32

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

	#else

	fputs("\x1B[H\x1B[J", stdout);

	#endif
}

void setDefaultColor(void)
{
	if (colorMode == CM_CSTD_16 ||
		colorMode == CM_CSTD_256 ||
		colorMode == CM_CSTD_RGB)
	{
		fputs("\x1B[39m", stdout);
	}
}

void setCursorPos(HANDLE outputHandle, int x, int y)
{
	#ifdef _WIN32

	COORD cursor = { (SHORT)x,(SHORT)y };
	if (!disableCLS) { SetConsoleCursorPosition(outputHandle, cursor); }

	#else

	if (x == 0 && y == 0) { fputs("\x1B[H", stdout); }
	else { printf("\x1B[%d;%dH", x, y); }

	#endif
}

size_t getOutputArraySize(void)
{
	const int CSTD_16_CODE_LEN = 6;   // "\x1B[??m?"
	const int CSTD_256_CODE_LEN = 12; // "\x1B[38;5;???m?"
	const int CSTD_RGB_CODE_LEN = 20; // "\x1B[38;2;???;???;???m?"

	switch (colorMode)
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

ColorMode colorModeFromStr(char* str)
{
	for (int i = 0; i < strlen(str); i++)
	{
		str[i] = (char)tolower((int)str[i]);
	}

	if (!strcmp(str, "winapi-gray")) { return CM_WINAPI_GRAY; }
	else if (!strcmp(str, "winapi-16")) { return CM_WINAPI_16; }
	else if (!strcmp(str, "cstd-gray")) { return CM_CSTD_GRAY; }
	else if (!strcmp(str, "cstd-16")) { return CM_CSTD_16; }
	else if (!strcmp(str, "cstd-256")) { return CM_CSTD_256; }
	else if (!strcmp(str, "cstd-rgb")) { return CM_CSTD_RGB; }

	error("Invalid color mode!", "utils.c", __LINE__);
	return CM_WINAPI_GRAY;
}

void error(const char* description, const char* fileName, int line)
{
	puts("\nSomething went wrong...");
	printf("%s [%s:%d]\n", description, fileName, line);
	exit(-1);
}

#ifndef _WIN32
void Sleep(DWORD ms)
{
	struct timespec timeSpec;
	timeSpec.tv_sec = ms / 1000;
	timeSpec.tv_nsec = (ms % 1000) * 1000000;
	nanosleep(&timeSpec, NULL);
}
#endif