#include "win2con.h"

typedef struct 
{
	int conW;
	int conH;
	double fontRatio;
} ConsoleInfo;

static HWND conHWND = NULL;
static HANDLE outputHandle = NULL;
static HANDLE inputHandle = NULL;
static DWORD oldOutputMode, oldInputMode;
static int outputModeChanged = 0, inputModeChanged = 0;

static void drawWithWinAPI(Frame* frame);
static void getConsoleInfo(ConsoleInfo* consoleInfo);

void initDrawFrame(void)
{
	#ifdef _WIN32
	conHWND = GetConsoleWindow();
	outputHandle = GetStdHandle(STD_OUTPUT_HANDLE);
	inputHandle = GetStdHandle(STD_INPUT_HANDLE);

	DWORD mode;
	if (colorMode == CM_CSTD_16 ||
		colorMode == CM_CSTD_256 ||
		colorMode == CM_CSTD_RGB)
	{
		GetConsoleMode(outputHandle, &mode);
		oldOutputMode = mode;
		mode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
		SetConsoleMode(outputHandle, mode);
		outputModeChanged = 1;
	}

	GetConsoleMode(inputHandle, &mode);
	oldInputMode = mode;
	mode &= ~ENABLE_QUICK_EDIT_MODE;
	mode |= ENABLE_MOUSE_INPUT;
	SetConsoleMode(inputHandle, mode);
	inputModeChanged = 1;
	#endif

	refreshConSize();
}

void refreshConSize(void)
{
	static int firstCall = 1;
	static int setNewSize = 0;
	static int lastW = 0, lastH = 0;
	static double lastFR = 0.0;

	ConsoleInfo consoleInfo;
	getConsoleInfo(&consoleInfo);

	int newW = lastW, newH = lastH;
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

		if (scalingMode != SM_FILL || !scaleWithRatio)
		{
			if (firstCall)
			{
				newW = argW;
				newH = argH;
			}
		}
		else if (fontRatio != newFR)
		{
			double wndRatio = (double)wndW / (double)wndH;
			double conRatio = ((double)argW / (double)argH) * fontRatio;

			if (wndRatio > conRatio)
			{
				newW = argW;
				newH = (int)((conRatio / wndRatio) * argH);
			}
			else
			{
				newW = (int)((wndRatio / conRatio) * argW);
				newH = argH;
			}
		}
	}
	else
	{
		if (scalingMode != SM_FILL || !scaleWithRatio)
		{
			if (conW != consoleInfo.conW || conH != consoleInfo.conH)
			{
				conW = consoleInfo.conW;
				conH = consoleInfo.conH;
				newW = consoleInfo.conW;
				newH = consoleInfo.conH;
			}
		}
		else if (conW != consoleInfo.conW || conH != consoleInfo.conH ||
			fontRatio != newFR)
		{
			conW = consoleInfo.conW;
			conH = consoleInfo.conH;

			double wndRatio = (double)wndW / (double)wndH;
			double conRatio = ((double)conW / (double)conH) * fontRatio;
			if (wndRatio > conRatio)
			{
				newW = conW;
				newH = (int)((conRatio / wndRatio) * conH);
			}
			else
			{
				newW = (int)((wndRatio / conRatio) * conW);
				newH = conH;
			}
		}
	}

	if (firstCall)
	{
		imgW = newW;
		imgH = newH;
		fontRatio = newFR;
		lastW = imgW;
		lastH = imgH;
		lastFR = fontRatio;
		firstCall = 0;
	}
	else
	{
		if (newW != lastW ||
			newH != lastH ||
			newFR != lastFR)
		{
			lastW = newW;
			lastH = newH;
			lastFR = newFR;
			setNewSize = 1;
		}
		else if (setNewSize)
		{
			imgW = newW;
			imgH = newH;
			fontRatio = newFR;
			setNewSize = 0;
		}
	}
}

void drawFrame(Frame* frame)
{
	static int scanline = 0;
	static int lastW = 0, lastH = 0;
	if ((lastW != imgW || lastH != imgH) && !disableCLS)
	{
		lastW = imgW;
		lastH = imgH;
		clearScreen(outputHandle);
	}

	if (colorMode == CM_WINAPI_GRAY || colorMode == CM_WINAPI_16)
	{
		drawWithWinAPI(frame);
		return;
	}

	char* output = (char*)frame->output;
	int* lineOffsets = frame->outputLineOffsets;

	if (scanlineCount == 1)
	{
		if (!disableCLS) { setCursorPos(outputHandle, 0, 0); }
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

			if (!disableCLS) { setCursorPos(outputHandle, 0, sy); }
			fwrite(output + lineOffsets[sy], 1, lineOffsets[sy + sh] - lineOffsets[sy], stdout);
		}

		scanline++;
		if (scanline == scanlineCount) { scanline = 0; }
	}
}

static void drawWithWinAPI(Frame* frame)
{
	#ifdef _WIN32
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
	#endif
}

static void getConsoleInfo(ConsoleInfo* consoleInfo)
{
	const double DEFAULT_FONT_RATIO = 8.0 / 18.0;
	const int USE_GET_CURRENT_CONSOLE_FONT = 0;

	int fullConW, fullConH;
	double fontRatio;

	#ifdef _WIN32

	CONSOLE_SCREEN_BUFFER_INFOEX consoleBufferInfo;
	consoleBufferInfo.cbSize = sizeof(CONSOLE_SCREEN_BUFFER_INFOEX);
	GetConsoleScreenBufferInfoEx(outputHandle, &consoleBufferInfo);

	fullConW = consoleBufferInfo.srWindow.Right - consoleBufferInfo.srWindow.Left + 1;
	fullConH = consoleBufferInfo.srWindow.Bottom - consoleBufferInfo.srWindow.Top + 1;

	RECT clientRect = { 0 };
	if (!USE_GET_CURRENT_CONSOLE_FONT) { GetClientRect(conHWND, &clientRect); }

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
		fontRatio = ((double)clientRect.right / (double)fullConW) /
			((double)clientRect.bottom / (double)fullConH);
	}

	#else

	struct winsize winSize;
	ioctl(0, TIOCGWINSZ, &winSize);
	fullConW = winSize.ws_col;
	fullConH = winSize.ws_row;
	fontRatio = DEFAULT_FONT_RATIO;

	#endif

	if (fullConW < 4) { fullConW = 4; }
	if (fullConH < 4) { fullConH = 4; }

	if (colorMode != CM_WINAPI_GRAY && colorMode != CM_WINAPI_16) { fullConW--; }

	consoleInfo->conW = fullConW;
	consoleInfo->conH = fullConH;
	consoleInfo->fontRatio = fontRatio;
}

void restoreConsoleMode()
{
	if (outputModeChanged) { SetConsoleMode(outputHandle, oldOutputMode); }
	if (inputModeChanged) { SetConsoleMode(inputHandle, oldInputMode); }
}