#include "win2con.h"

static HDC outputHDC, unscaledHDC, screenHDC;
static uint8_t* bitmapArray = NULL;

static uint8_t* setBitmap(HDC hdc, int w, int h);

void initGetFrame(void)
{
	if (!settings.ignoreDPI) { SetProcessDpiAwareness(PROCESS_PER_MONITOR_DPI_AWARE); }

	screenHDC = GetDC(NULL);
	outputHDC = CreateCompatibleDC(screenHDC);

	if (settings.scalingMode == SM_SOFT_FILL)
	{
		if (!settings.magnifierMode) { unscaledHDC = CreateCompatibleDC(screenHDC); }
		SetStretchBltMode(outputHDC, HALFTONE);
	}

	refreshWinSize();
}

void refreshWinSize(void)
{
	if (settings.magnifierMode)
	{
		wndW = conWndW;
		wndH = conWndH;
	}
	else
	{
		RECT wndRect;

		if (settings.printClientArea)
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

}

void refreshBitmapSize(void)
{
	static int oldConW = -1, oldConH = -1;
	static int oldWndW = -1, oldWndH = -1;

	if (settings.scalingMode == SM_SOFT_FILL)
	{
		if (!settings.magnifierMode)
		{
			if (wndW != oldWndW || wndH != oldWndH)
			{
				setBitmap(unscaledHDC, wndW, wndH);
			}
		}

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
	oldWndW = wndW;
	oldWndH = wndH;
}

void getFrame(Frame* frame)
{
	if (settings.magnifierMode)
	{
		if (settings.scalingMode == SM_SOFT_FILL)
		{
			StretchBlt(outputHDC, 0, 0, conW, conH, screenHDC,
				conWndX, conWndY, conWndW, conWndH, SRCCOPY);
		}
		else
		{
			int x = conWndX + (int)round(((double)wndW / (double)conW) / 2.0);
			int y = conWndY + (int)round(((double)wndH / (double)conH) / 2.0);
			BitBlt(outputHDC, 0, 0, wndW, wndH, screenHDC, x, y, SRCCOPY);
		}
	}
	else
	{
		UINT printMode;
		if (settings.printClientArea) { printMode = PW_CLIENTONLY; }
		else { printMode = PW_RENDERFULLCONTENT; }

		if (settings.scalingMode == SM_SOFT_FILL)
		{
			PrintWindow(hwnd, unscaledHDC, printMode);
			StretchBlt(outputHDC, 0, 0, imgW, imgH, unscaledHDC, 0, 0, wndW, wndH, SRCCOPY);
		}
		else
		{
			PrintWindow(hwnd, outputHDC, printMode);
		}

		if (!IsWindow(hwnd)) { reEnterHWND = true; }
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

	HBITMAP bitmap = CreateDIBSection(hdc, &bmpInfo, DIB_RGB_COLORS, (void**)&bmpArray, 0, 0);
	HGDIOBJ oldObj = SelectObject(hdc, bitmap);
	if (oldObj) { DeleteObject(oldObj); }

	return bmpArray;
}