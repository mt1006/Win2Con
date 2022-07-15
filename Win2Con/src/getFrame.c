#include "win2con.h"

static HDC hdc, screenHDC;
static uint8_t* bitmapArray = NULL;

static uint8_t* setBitmap(int w, int h);

void initGetFrame(void)
{
	if (!ignoreDPI) { SetProcessDPIAware(); }
	screenHDC = GetDC(NULL);
	hdc = CreateCompatibleDC(screenHDC);
	refreshWinSize();
}

void refreshWinSize(void)
{
	int newWndW, newWndH;

	if (magnifierMode)
	{
		RECT wndRect;
		GetClientRect(conHWND, &wndRect);
		newWndW = wndRect.right;
		newWndH = wndRect.bottom;
	}
	else
	{
		RECT wndRect;

		if (pwClientArea)
		{
			GetClientRect(hwnd, &wndRect);
			newWndW = wndRect.right;
			newWndH = wndRect.bottom;
		}
		else
		{
			GetWindowRect(hwnd, &wndRect);
			newWndW = wndRect.right - wndRect.left;
			newWndH = wndRect.bottom - wndRect.top;
		}
	}

	if (wndW != newWndW || wndH != newWndH)
	{
		wndW = newWndW;
		wndH = newWndH;
		bitmapArray = setBitmap(wndW, wndH);
	}
}

void getFrame(Frame* frame)
{
	if (magnifierMode)
	{
		BitBlt(hdc, 0, 0, wndW, wndH, screenHDC, 0, 0, SRCCOPY);
		frame->bitmapArray = bitmapArray;
	}
	else
	{
		UINT pwMode;
		if (pwClientArea) { pwMode = PW_CLIENTONLY; }
		else { pwMode = PW_RENDERFULLCONTENT; }

		PrintWindow(hwnd, hdc, pwMode);
		frame->bitmapArray = bitmapArray;

		if (!IsWindow(hwnd)) { reEnterHWND = 1; }
	}
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