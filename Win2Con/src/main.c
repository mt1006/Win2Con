#include "win2con.h"

HWND hwnd = NULL;
HANDLE outputHandle = NULL;
HWND conHWND = NULL, wtDragBarHWND = NULL;
int imgW = -1, imgH = -1;
int conW = -1, conH = -1;
int wndW = -1, wndH = -1;
double fontRatio = 1.0;
int conWndX = -1, conWndY = -1, conWndW = -1, conWndH = -1;
int scaleXMul = 1, scaleYMul = 1, scaleXDiv = 1, scaleYDiv = 1;
bool reEnterHWND = false;
bool ansiEnabled = false;
int stopMainThreadVal = 0;

Settings settings =
{
	.argW = -1, .argH = -1,
	.scaleWithRatio = true,
	.printClientArea = false,
	.colorMode = CM_CSTD_256,
	.scalingMode = SM_FILL,
	.scanlineCount = 1, .scanlineHeight = 1,
	.charset = NULL,
	.charsetSize = 0,
	.constFontRatio = 0.0,
	.setColorMode = SCM_DISABLED,
	.setColorVal = 0, .setColorVal2 = -1,
	.colorProcMode = CPM_BOTH,
	.brightnessRand = 0,
	.magnifierMode = false,
	.disableKeyboard = false,
	.disableCLS = false,
	.ignoreDPI = false
};

void init(void)
{
	setDefaultColor();
	outputHandle = GetStdHandle(STD_OUTPUT_HANDLE);

	if (!hwnd && !settings.magnifierMode)
	{
		hwnd = getWindow();
		if (!IsWindow(hwnd) && !settings.magnifierMode) { error("Invalid window!", "main.c", __LINE__); }
	}

	puts("Loading...");

	getConsoleWindow();
	initGetFrame();
	initDrawFrame();
	initConInput();
}

void loop(void)
{
	const double SIZE_REFRESH_PERIOD = 0.2;
	double lastRefresh = 0.0;

	Frame frame = { 0 };

	while (1)
	{
		if (stopMainThreadVal)
		{
			stopMainThreadVal = 2;
			while (1) { Sleep(20); }
		}

		if (reEnterHWND)
		{
			setDefaultColor();
			hwnd = getWindow();
			if (!IsWindow(hwnd)) { error("Invalid window!", "main.c", __LINE__); }
			reEnterHWND = false;
		}

		double curTime = getTime();
		if ((curTime > lastRefresh + SIZE_REFRESH_PERIOD) || settings.magnifierMode)
		{
			getConsoleInfo();
			refreshWinSize();
			refreshConSize();
			refreshBitmapSize();
			refreshScaling();
			lastRefresh = curTime;
		}

		getFrame(&frame);
		processFrame(&frame);
		drawFrame(&frame);
	}
}

int main(int argc, char** argv)
{
	bool exitReq;
	hwnd = (HWND)argumentParser(argc - 1, argv + 1, &exitReq, false);
	if (exitReq) { w2cExit(0); }
	
	init();
	loop();

	w2cExit(0);
	return 0;
}