/*
* Project: Win2Con
* Version: 0.1
* Author: https://github.com/mt1006
*/

#pragma once
#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdint.h>
#include <time.h>

#ifdef _WIN32

#include <process.h>
#include <Windows.h>
#include <dwmapi.h>
#define W2C_OS "Windows"

#else

#include <unistd.h>
#include <pthread.h>
#include <sys/ioctl.h>

#ifdef __linux__
#define W2C_OS "Linux"
#else
#define W2C_OS "[unknown]"
#endif

#endif

#pragma warning(disable : 4996)
#pragma comment(lib, "dwmapi")

#if defined(__x86_64__) || defined(_M_AMD64)
#define W2C_CPU "AMD64"
#elif defined(__i386__) || defined(_M_IX86)
#define W2C_CPU "IA-32"
#else
#define W2C_CPU "[unknown]"
#endif

#define W2C_VERSION "0.1"
#define TO_STR(x) #x
#define DEF_TO_STR(x) TO_STR(x)

#ifdef _WIN32

#define W2C_DEFAULT_COLOR_MODE CM_CSTD_256
#define W2C_DEFAULT_SCALING_MODE SM_INT_FRACTION

#else

#define W2C_DEFAULT_COLOR_MODE CM_CSTD_256
#define W2C_DEFAULT_SCALING_MODE SM_INT_FRACTION

#endif

typedef enum
{
	CM_WINAPI_GRAY,
	CM_WINAPI_16,
	CM_CSTD_GRAY,
	CM_CSTD_16,
	CM_CSTD_256,
	CM_CSTD_RGB
} ColorMode;

typedef enum
{
	SM_INT_FRACTION,
	SM_INT,
	SM_CONST,
	SM_NO_SCALING
} ScalingMode;

typedef struct
{
	uint8_t* bitmapArray;
	void* output; // char* (video - C std) / CHAR_INFO* (video - WinAPI)
	int* outputLineOffsets;
} Frame;

extern int imgW, imgH;
extern int conW, conH;
extern int wndW, wndH;
extern int argW, argH;
extern int scaleXMul, scaleYMul;
extern int scaleXDiv, scaleYDiv;
extern int scaleWithRatio;
extern int pwClientArea;
extern ColorMode colorMode;
extern ScalingMode scalingMode;
extern int scanlineCount, scanlineHeight;
extern double fps;
extern char* charset;
extern int charsetSize;
extern double constFontRatio;
extern int disableKeyboard, disableCLS;

//argParser.c
extern HWND argumentParser(int argc, char** argv, int* exitReq);

//getWindow.c
extern HWND getWindow(void);

//getFrame.c
extern void initGetFrame(HWND inputWindow);
extern void refreshWinSize(void);
extern void getFrame(Frame* frame);

//processFrame.c
extern void initProcessFrame(void);
extern void processFrame(Frame* frame);

//drawFrame.c
extern void initDrawFrame(void);
extern void refreshConSize(void);
extern void drawFrame(Frame* frame);

//conInput.c
extern void initConInput(void);

//help.c
extern void showHelp(int basic, int advanced, int colorModes, int keyboard);
extern void showInfo(void);
extern void showFullInfo(void);
extern void showVersion(void);

//utils.c
extern double getTime(void);
extern void clearScreen(HANDLE outputHandle);
extern void setDefaultColor(void);
extern void setCursorPos(HANDLE outputHandle, int x, int y);
extern size_t getOutputArraySize(void);
extern uint8_t rgbToAnsi256(uint8_t r, uint8_t g, uint8_t b);
extern ColorMode colorModeFromStr(char* str);
extern void error(const char* description, const char* fileName, int line);

#ifndef _WIN32
extern void Sleep(DWORD ms);
#endif