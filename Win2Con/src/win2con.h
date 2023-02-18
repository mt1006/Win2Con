/*
* Project: Win2Con
* Version: 1.1
* Author: https://github.com/mt1006
*/

#pragma once
#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <time.h>

#include <process.h>
#include <Windows.h>
#include <dwmapi.h>
#include <tlhelp32.h>
#include <Psapi.h>
#include <shellscalingapi.h>
#include <gl/GL.h>
#include <gl/GLU.h>

#pragma warning(disable : 4996)
#pragma comment(lib, "dwmapi")
#pragma comment(lib, "Shcore")
#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "glu32.lib")

#define W2C_OS "Windows"

#if defined(__x86_64__) || defined(_M_AMD64)
#define W2C_CPU "AMD64"
#elif defined(__i386__) || defined(_M_IX86)
#define W2C_CPU "IA-32"
#else
#define W2C_CPU "[unknown]"
#endif

#define W2C_VERSION "1.1"
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
	SM_SOFT_FILL,
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

typedef enum
{
	CPM_NONE,
	CPM_CHAR_ONLY,
	CPM_BOTH
} ColorProcMode;

typedef struct
{
	uint8_t* bitmapArray;
	void* output; // char* (video - C std) / CHAR_INFO* (video - WinAPI)
	int* outputLineOffsets;
} Frame;

typedef struct
{
	uint8_t r, g, b;
	uint8_t ch;
} GlConsoleChar;

typedef struct
{
	int argW, argH;
	bool scaleWithRatio;
	bool printClientArea;
	ColorMode colorMode;
	ScalingMode scalingMode;
	int scanlineCount, scanlineHeight;
	const char* charset;
	int charsetSize;
	double constFontRatio;
	SetColorMode setColorMode;
	int setColorVal, setColorVal2;
	ColorProcMode colorProcMode;
	int brightnessRand;
	bool magnifierMode;
	bool useFakeConsole;
	bool disableKeyboard;
	bool disableCLS;
	bool ignoreDPI;
} Settings;


//main.c
extern HWND hwnd;
extern HANDLE outputHandle;
extern HWND conHWND, wtDragBarHWND;
extern int imgW, imgH;
extern int conW, conH;
extern int wndW, wndH;
extern double fontRatio;
extern int conWndX, conWndY, conWndW, conWndH;
extern int scaleXMul, scaleYMul, scaleXDiv, scaleYDiv;
extern bool reEnterHWND;
extern bool ansiEnabled;
extern int stopMainThreadVal;
extern Settings settings;

//glConsole.c
extern float glCharW, glCharH;


//argParser.c
extern long long argumentParser(int argc, char** argv, bool* exitReq, bool fromGetWindow);

//getWindow.c
extern HWND getWindow(void);

//getFrame.c
extern void initGetFrame(void);
extern void refreshWinSize(void);
extern void refreshBitmapSize(void);
extern void getFrame(Frame* frame);

//processFrame.c
extern void refreshScaling(void);
extern void processFrame(Frame* frame);

//drawFrame.c
extern void initDrawFrame(void);
extern void getConsoleInfo(void);
extern void refreshConSize(void);
extern void drawFrame(Frame* frame);
extern void restoreConsoleMode();

//conInput.c
extern void initConInput(void);

//magnifierMode.c
extern void enableMagnifierMode(void);
extern void disableMagnifierMode(void);

//glConsole.c
extern void initOpenGlConsole(void);
extern void refreshFont(void);
extern void drawWithOpenGL(GlConsoleChar* output, int w, int h);
extern void peekMessages(void);

//help.c
extern void showHelp(bool basic, bool advanced, bool modes, bool keyboard);
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
extern void stopMainThread(void);
extern void w2cExit(int code);
extern void error(const char* description, const char* fileName, int line);