#include "win2con.h"

static HWND hwnd;
static HDC hdc;
static uint8_t* bitmapArray = NULL;

static uint8_t* setBitmap(int w, int h);

void initGetFrame(HWND inputWindow)
{
	hwnd = inputWindow;
	hdc = CreateCompatibleDC(GetDC(NULL));
	refreshWinSize();
}

void refreshWinSize(void)
{
	RECT wndRect;
	GetClientRect(hwnd, &wndRect);

	int newWndW = wndRect.right;
	int newWndH = wndRect.bottom;

	if (wndW != newWndW || wndH != newWndH)
	{
		wndW = newWndW;
		wndH = newWndH;
		bitmapArray = setBitmap(wndW, wndH);
	}
}

void getFrame(Frame* frame)
{
	PrintWindow(hwnd, hdc, PW_CLIENTONLY);
	frame->bitmapArray = bitmapArray;
}

static uint8_t* setBitmap(int w, int h)
{
	if (!w || !h) { return NULL; }

	uint8_t* bmpArray;
	BITMAPINFO bmpInfo;
	RECT tempRect = { 0,0,w,h };

	memset(&bmpInfo, 0, sizeof(BITMAPINFO));
	bmpInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bmpInfo.bmiHeader.biPlanes = 1;
	bmpInfo.bmiHeader.biBitCount = 32;
	bmpInfo.bmiHeader.biCompression = BI_RGB;
	bmpInfo.bmiHeader.biWidth = w;
	bmpInfo.bmiHeader.biHeight = h;
	bmpInfo.bmiHeader.biSizeImage = w * h * 3;

	HBITMAP bitmap = CreateDIBSection(hdc, &bmpInfo,
		DIB_RGB_COLORS, (void**)&bmpArray, 0, 0);
	HGDIOBJ oldObj = SelectObject(hdc, bitmap);
	if (oldObj) { DeleteObject(oldObj); }

	return bmpArray;
}