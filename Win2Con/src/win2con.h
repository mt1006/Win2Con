/*
* Project: Win2Con
* Version: 1.0
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
#include <tlhelp32.h>
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

#define W2C_VERSION "1.0"
#define TO_STR(x) #x
#define DEF_TO_STR(x) TO_STR(x)

#define W2C_DEFAULT_COLOR_MODE CM_CSTD_256
#define W2C_DEFAULT_SCALING_MODE SM_FILL

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
	SM_FILL,
	SM_INT,
	SM_INT_FRACTION,
	SM_CONST,
	SM_NO_SCALING
} ScalingMode;

typedef enum
{
	SCM_DISABLED,
	SCM_WINAPI,
	SCM_CSTD_256,
	SCM_CSTD_RGB
} SetColorMode;

typedef struct
{
	uint8_t* bitmapArray;
	void* output; // char* (video - C std) / CHAR_INFO* (video - WinAPI)
	int* outputLineOffsets;
} Frame;

extern HWND hwnd;
extern HANDLE outputHandle;
extern HWND conHWND, wtDragBarHWND;
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
extern char* charset;
extern int charsetSize;
extern double fontRatio, constFontRatio;
extern int disableKeyboard, disableCLS, ignoreDPI;
extern int enableInput;
extern int reEnterHWND;
extern int ansiEnabled;
extern SetColorMode setColorMode;
extern int setColorVal, setColorVal2;
extern int singleCharMode;
extern int magnifierMode;

//argParser.c
extern long long argumentParser(int argc, char** argv, int* exitReq, int inputNumBase);

//getWindow.c
extern HWND getWindow(void);

//getFrame.c
extern void initGetFrame(void);
extern void refreshWinSize(void);
extern void getFrame(Frame* frame);

//processFrame.c
extern void initProcessFrame(void);
extern void refreshScaling(void);
extern void processFrame(Frame* frame);

//drawFrame.c
extern void initDrawFrame(void);
extern void refreshConSize(void);
extern void drawFrame(Frame* frame);
extern void restoreConsoleMode();

//conInput.c
extern void initConInput(void);

//magnifierMode.c
extern void excludeConsoleFromCapture(void);

//help.c
extern void showHelp(int basic, int advanced, int colorModes, int scalingModes, int keyboard);
extern void showInfo(void);
extern void showFullInfo(void);
extern void showVersion(void);

//utils.c
extern double getTime(void);
extern void strToLower(char* str);
extern void getConsoleWindow(void);
extern void clearScreen(void);
extern void setDefaultColor(void);
extern void setConsoleTopMost(int topMost);
extern void setCursorPos(int x, int y);
extern size_t getOutputArraySize(void);
extern uint8_t rgbToAnsi256(uint8_t r, uint8_t g, uint8_t b);
extern void w2cExit(int code);
extern void error(const char* description, const char* fileName, int line);

#ifndef _WIN32
extern void Sleep(DWORD ms);
#endif