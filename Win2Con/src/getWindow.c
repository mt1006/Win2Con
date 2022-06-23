#include "win2con.h"

static int isAltTabWindow(HWND hwnd);

HWND getWindow(void)
{
	long long val;
	fputs(">", stdout);
	scanf("%llx", &val);
	return (HWND)val;
}

//https://stackoverflow.com/a/62126899/18214530
static int isAltTabWindow(HWND hwnd)
{
	if (!IsWindowVisible(hwnd)) { return 0; }

	WINDOWINFO wndInfo = { 0 };
	wndInfo.cbSize = sizeof(WINDOWINFO);
	GetWindowInfo(hwnd, &wndInfo);
	if (wndInfo.dwExStyle & WS_EX_TOOLWINDOW) { return 0; }

	int cloaked;
	DwmGetWindowAttribute(hwnd, DWMWA_CLOAKED, &cloaked, sizeof(int));
	if (cloaked) { return 0; }

	return 1;
}