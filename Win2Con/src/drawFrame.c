#include "win2con.h"

typedef struct 
{
	int conW;
	int conH;
	double fontRatio;
} ConsoleInfo;

static HANDLE inputHandle = NULL;
static DWORD oldOutputMode, oldInputMode;
static int outputModeChanged = 0;
static ConsoleInfo consoleInfo;

static void drawWithWinAPI(Frame* frame);
static void setConstColor(void);

void initDrawFrame(void)
{
	inputHandle = GetStdHandle(STD_INPUT_HANDLE);

	DWORD mode;
	if (colorMode == CM_CSTD_16 ||
		colorMode == CM_CSTD_256 ||
		colorMode == CM_CSTD_RGB ||
		setColorMode == SCM_CSTD_256 ||
		setColorMode == SCM_CSTD_RGB)
	{
		GetConsoleMode(outputHandle, &mode);
		oldOutputMode = mode;
		mode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
		SetConsoleMode(outputHandle, mode);
		ansiEnabled = 1;
		outputModeChanged = 1;
	}

	refreshConSize();
}

void getConsoleInfo(void)
{
	const double DEFAULT_FONT_RATIO = 8.0 / 18.0;

	int fullConW, fullConH;
	double fontRatio;

	CONSOLE_SCREEN_BUFFER_INFOEX consoleBufferInfo;
	consoleBufferInfo.cbSize = sizeof(CONSOLE_SCREEN_BUFFER_INFOEX);
	GetConsoleScreenBufferInfoEx(outputHandle, &consoleBufferInfo);

	fullConW = consoleBufferInfo.srWindow.Right - consoleBufferInfo.srWindow.Left + 1;
	fullConH = consoleBufferInfo.srWindow.Bottom - consoleBufferInfo.srWindow.Top + 1;

	RECT clientRect = { 0 };
	GetClientRect(conHWND, &clientRect);

	if (clientRect.bottom == 0 || fullConW == 0 || fullConH == 0)
	{
		CONSOLE_FONT_INFOEX consoleFontInfo;
		consoleFontInfo.cbSize = sizeof(CONSOLE_FONT_INFOEX);
		GetCurrentConsoleFontEx(outputHandle, FALSE, &consoleFontInfo);

		if (consoleFontInfo.dwFontSize.X == 0 ||
			consoleFontInfo.dwFontSize.Y == 0)
		{
			fontRatio = DEFAULT_FONT_RATIO;
		}
		else
		{
			fontRatio = (double)consoleFontInfo.dwFontSize.X /
				(double)consoleFontInfo.dwFontSize.Y;
		}
	}
	else
	{
		if (magnifierMode)
		{
			POINT clientAreaPos = { 0,0 };
			ClientToScreen(conHWND, &clientAreaPos);
			conWndX = clientAreaPos.x;
			conWndY = clientAreaPos.y;
		}

		if (wtDragBarHWND && IsWindowVisible(wtDragBarHWND))
		{
			RECT wtDragBarRect;
			GetClientRect(wtDragBarHWND, &wtDragBarRect);
			clientRect.bottom -= wtDragBarRect.bottom;
			if (magnifierMode) { conWndY += wtDragBarRect.bottom; }
		}

		fontRatio = ((double)clientRect.right / (double)fullConW) /
			((double)clientRect.bottom / (double)fullConH);
	}

	if (magnifierMode)
	{
		conWndW = clientRect.right;
		conWndH = clientRect.bottom;
	}

	if (fullConW < 4) { fullConW = 4; }
	if (fullConH < 4) { fullConH = 4; }

	if (colorMode != CM_WINAPI_GRAY && colorMode != CM_WINAPI_16) { fullConW--; }

	consoleInfo.conW = fullConW;
	consoleInfo.conH = fullConH;
	consoleInfo.fontRatio = fontRatio;
}

void refreshConSize(void)
{
	static int firstCall = 1;
	static int setNewSize = 0;
	static int lastWndW = 0, lastWndH = 0;

	double newFR;
	if (constFontRatio == 0.0) { newFR = consoleInfo.fontRatio; }
	else { newFR = constFontRatio; }

	if (argW != -1 && argH != -1)
	{
		if (argW == 0 && argH == 0)
		{
			argW = consoleInfo.conW;
			argH = consoleInfo.conH;
		}

		if ((scalingMode != SM_FILL && scalingMode != SM_SOFT_FILL) ||
			!scaleWithRatio)
		{
			fontRatio = newFR;
			if (firstCall)
			{
				imgW = argW;
				imgH = argH;
			}
		}
		else if (lastWndW != wndW || lastWndH != wndH ||
			fontRatio != newFR)
		{
			lastWndW = wndW;
			lastWndH = wndH;
			fontRatio = newFR;

			double wndRatio = (double)wndW / (double)wndH;
			double conRatio = ((double)argW / (double)argH) * fontRatio;

			if (wndRatio > conRatio)
			{
				imgW = argW;
				imgH = (int)((conRatio / wndRatio) * argH);
			}
			else
			{
				imgW = (int)((wndRatio / conRatio) * argW);
				imgH = argH;
			}
		}
	}
	else
	{
		if ((scalingMode != SM_FILL && scalingMode != SM_SOFT_FILL) ||
			!scaleWithRatio)
		{
			fontRatio = newFR;
			if (conW != consoleInfo.conW || conH != consoleInfo.conH)
			{
				conW = consoleInfo.conW;
				conH = consoleInfo.conH;
				imgW = consoleInfo.conW;
				imgH = consoleInfo.conH;
			}
		}
		else if (conW != consoleInfo.conW || conH != consoleInfo.conH ||
			lastWndW != wndW || lastWndH != wndH ||
			fontRatio != newFR)
		{
			lastWndW = wndW;
			lastWndH = wndH;
			fontRatio = newFR;

			conW = consoleInfo.conW;
			conH = consoleInfo.conH;

			double wndRatio = (double)wndW / (double)wndH;
			double conRatio = ((double)conW / (double)conH) * fontRatio;
			if (wndRatio > conRatio)
			{
				imgW = conW;
				imgH = (int)((conRatio / wndRatio) * conH);
			}
			else
			{
				imgW = (int)((wndRatio / conRatio) * conW);
				imgH = conH;
			}
		}
	}

	if (firstCall) { firstCall = 0; }
}

void drawFrame(Frame* frame)
{
	static int scanline = 0;
	static int lastW = 0, lastH = 0;
	if ((lastW != imgW || lastH != imgH) && !disableCLS)
	{
		lastW = imgW;
		lastH = imgH;
		clearScreen();
	}

	if (colorMode == CM_WINAPI_GRAY || colorMode == CM_WINAPI_16)
	{
		drawWithWinAPI(frame);
		return;
	}

	setConstColor();

	char* output = (char*)frame->output;
	int* lineOffsets = frame->outputLineOffsets;

	if (scanlineCount == 1)
	{
		if (!disableCLS) { setCursorPos(0, 0); }
		fwrite(output, 1, lineOffsets[imgH], stdout);
	}
	else
	{
		for (int i = 0; i < imgH; i += scanlineCount * scanlineHeight)
		{
			int sy = i + (scanline * scanlineHeight);
			int sh = scanlineHeight;
			if (sy >= imgH) { break; }
			else if (sy + sh > imgH) { sh = imgH - sy; }

			if (!disableCLS) { setCursorPos(0, sy); }
			fwrite(output + lineOffsets[sy], 1, lineOffsets[sy + sh] - lineOffsets[sy], stdout);
		}

		scanline++;
		if (scanline == scanlineCount) { scanline = 0; }
	}
}

static void drawWithWinAPI(Frame* frame)
{
	static int scanline = 0;

	CHAR_INFO* output = (CHAR_INFO*)frame->output;

	if (scanlineCount == 1)
	{
		COORD charBufSize = { imgW,imgH };
		COORD startCharPos = { 0,0 };
		SMALL_RECT writeRect = { 0,0,imgW,imgH };
		WriteConsoleOutputA(outputHandle, output, charBufSize, startCharPos, &writeRect);
	}
	else
	{
		for (int i = 0; i < imgH; i += scanlineCount * scanlineHeight)
		{
			int sy = i + (scanline * scanlineHeight);
			int sh = scanlineHeight;
			if (sy >= imgH) { break; }
			else if (sy + sh > imgH) { sh = imgH - sy; }


			COORD charBufSize = { imgW,imgH };
			COORD startCharPos = { 0,sy };
			SMALL_RECT writeRect = { 0,sy,imgW,sy + sh };
			WriteConsoleOutputA(outputHandle, output, charBufSize, startCharPos, &writeRect);
		}

		scanline++;
		if (scanline == scanlineCount) { scanline = 0; }
	}
}

void restoreConsoleMode()
{
	if (outputModeChanged) { SetConsoleMode(outputHandle, oldOutputMode); }
}

static void setConstColor(void)
{
	switch (setColorMode)
	{
	case SCM_WINAPI:
		SetConsoleTextAttribute(outputHandle, (WORD)setColorVal);
		break;

	case SCM_CSTD_256:
		printf("\x1B[38;5;%dm", setColorVal);
		if (setColorVal2 != -1)
		{
			printf("\x1B[48;5;%dm", setColorVal2);
		}
		break;

	case SCM_CSTD_RGB:
		printf("\x1B[38;2;%d;%d;%dm",
			(setColorVal & 0xFF0000) >> 16,
			(setColorVal & 0x00FF00) >> 8,
			setColorVal & 0x0000FF);
		if (setColorVal2 != -1)
		{
			printf("\x1B[48;2;%d;%d;%dm",
				(setColorVal2 & 0xFF0000) >> 16,
				(setColorVal2 & 0x00FF00) >> 8,
				setColorVal2 & 0x0000FF);
		}
		break;
	}
}