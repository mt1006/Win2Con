#include "win2con.h"

static HDC outputHDC, unscaledHDC, screenHDC;
static uint8_t* bitmapArray = NULL;
static int conWndX, conWndY;
static int conWndW, conWndH;

static uint8_t* setBitmap(HDC hdc, int w, int h);

void initGetFrame(void)
{
	if (!ignoreDPI) { SetProcessDPIAware(); }

	screenHDC = GetDC(NULL);
	outputHDC = CreateCompatibleDC(screenHDC);

	if (scalingMode == SM_SOFT_FILL)
	{
		if (!magnifierMode) { unscaledHDC = CreateCompatibleDC(screenHDC); }
		SetStretchBltMode(outputHDC, HALFTONE);
	}

	refreshWinSize();
}

void refreshWinSize(void)
{
	static int oldConW = -1, oldConH = -1;
	int oldWndW = wndW, oldWndH = wndH;

	if (magnifierMode)
	{
		RECT wndRect;
		GetClientRect(conHWND, &wndRect);
		conWndW = wndRect.right;
		conWndH = wndRect.bottom;

		POINT clientAreaPos = { 0,0 };
		ClientToScreen(conHWND, &clientAreaPos);
		conWndX = clientAreaPos.x;
		conWndY = clientAreaPos.y;

		wndW = conWndW;
		wndH = conWndH;
	}
	else
	{
		RECT wndRect;

		if (pwClientArea)
		{
			GetClientRect(hwnd, &wndRect);
			wndW = wndRect.right;
			wndH = wndRect.bottom;
		}
		else
		{
			GetWindowRect(hwnd, &wndRect);
			wndW = wndRect.right - wndRect.left;
			wndH = wndRect.bottom - wndRect.top;
		}
	}

	if (scalingMode == SM_SOFT_FILL)
	{
		if (conW != oldConW || conH != oldConH)
		{
			bitmapArray = setBitmap(outputHDC, conW, conH);
		}
	}
	else
	{
		if (wndW != oldWndW || wndH != oldWndH)
		{
			bitmapArray = setBitmap(outputHDC, wndW, wndH);
		}
	}

	oldConW = conW;
	oldConH = conH;
}

void getFrame(Frame* frame)
{
	if (magnifierMode)
	{
		if (scalingMode == SM_SOFT_FILL)
		{
			StretchBlt(outputHDC, 0, 0, conW, conH, screenHDC, conWndX, conWndY, conWndW, conWndH, SRCCOPY);
		}
		else
		{
			BitBlt(outputHDC, 0, 0, wndW, wndH, screenHDC, conWndX, conWndY, SRCCOPY);
		}
	}
	else
	{
		UINT pwMode;
		if (pwClientArea) { pwMode = PW_CLIENTONLY; }
		else { pwMode = PW_RENDERFULLCONTENT; }

		if (scalingMode == SM_SOFT_FILL)
		{
			PrintWindow(hwnd, unscaledHDC, pwMode);
			StretchBlt(outputHDC, 0, 0, conW, conH, unscaledHDC, 0, 0, wndW, wndH, SRCCOPY);
		}
		else
		{
			PrintWindow(hwnd, outputHDC, pwMode);
		}

		if (!IsWindow(hwnd)) { reEnterHWND = 1; }
	}

	frame->bitmapArray = bitmapArray;
}

static uint8_t* setBitmap(HDC hdc, int w, int h)
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