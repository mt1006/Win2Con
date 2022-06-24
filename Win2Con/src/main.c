#include "win2con.h"

int imgW = -1, imgH = -1;
int conW = -1, conH = -1;
int wndW = -1, wndH = -1;
int argW = -1, argH = -1;
int scaleXMul = 1, scaleYMul = 1;
int scaleXDiv = 1, scaleYDiv = 1;
int scaleWithRatio = 1;
int pwClientArea = 0;
ColorMode colorMode = W2C_DEFAULT_COLOR_MODE;
ScalingMode scalingMode = W2C_DEFAULT_SCALING_MODE;
int scanlineCount = 1, scanlineHeight = 1;
double fps;
char* charset = NULL;
int charsetSize = 0;
double fontRatio = 1.0, constFontRatio = 0.0;
int disableKeyboard = 0, disableCLS = 0, ignoreDPI = 0;

void init(HWND inputWindow)
{
	initConInput();
	if (!inputWindow)
	{
		setDefaultColor();
		inputWindow = getWindow();
		if (!inputWindow) { return; }
	}

	puts("Loading...");

	initGetFrame(inputWindow);
	initProcessFrame();
	initDrawFrame();
}

void loop(void)
{
	const double SIZE_REFRESH_PERIOD = 0.2;
	double lastRefresh = 0.0;

	Frame frame = { 0 };

	while (1)
	{
		double curTime = getTime();
		if (curTime > lastRefresh + SIZE_REFRESH_PERIOD)
		{
			refreshConSize();
			refreshWinSize();
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
	int exitReq = 0;
	HWND inputWindow = (HWND)argumentParser(argc - 1, argv + 1, &exitReq, 16);
	if (exitReq) { exit(0); }

	init(inputWindow);
	loop();

	setDefaultColor();
	return 0;
}