#include "win2con.h"

#define W2C_MAX_INPUT_LEN 1024
#define W2C_MAX_NAME_LEN 128
#define W2C_MAX_WND_COUNT 2048

static int currentPos;
static bool checkIfAltTab;
static HWND* hwndArray;
static int hwndArrayPos;

static long long menuInput(const char* prompt);
static void printWindowList(bool showFullList, HWND parent);
static int printWindowInfo(HWND hwnd, int pos);
static int isAltTabWindow(HWND hwnd);
static BOOL CALLBACK enumWindowsProc(HWND hwnd, LPARAM lParam);

//https://stackoverflow.com/a/1706733/18214530
static int setArgs(char* args, char** argv);
static char** parsedArgs(char* args, int* argc);
static void freeParsedArgs(char** argv);

HWND getWindow(void)
{
	HWND selectedHWND = NULL;
	bool showFullList = false;
	hwndArray = (HWND*)malloc(W2C_MAX_WND_COUNT * sizeof(HWND));

	while (1)
	{
		clearScreen();
		puts("Select window:\n");
		printWindowList(showFullList, NULL);

		fflush(stdout);
		int selection = (int)menuInput(":");

		if (selection > 0 && selection < currentPos)
		{
			selectedHWND = hwndArray[selection];
			break;
		}
		else if (selection == currentPos)
		{
			showFullList = !showFullList;
			continue;
		}
		else if (selection == currentPos + 1)
		{
			char inputStr[W2C_MAX_INPUT_LEN];
			fputs(">", stdout);
			fgets(inputStr, W2C_MAX_INPUT_LEN, stdin);
			selectedHWND = (HWND)strtoll(inputStr, NULL, 16);
			if (selectedHWND == NULL) { continue; }
			break;
		}
		else if (selection == currentPos + 2 || selection == -1)
		{
			enableMagnifierMode();
			break;
		}
		else if (selection == currentPos + 3)
		{
			continue;
		}
		else if (selection == currentPos + 4)
		{
			w2cExit(0);
		}

		fputs("\nUnknown option...", stdout);
		getchar();
	}

	free(hwndArray);
	clearScreen();
	return selectedHWND;
}

static long long menuInput(const char* prompt)
{
	char inputStr[W2C_MAX_INPUT_LEN];

	fputs(prompt, stdout);
	fgets(inputStr, W2C_MAX_INPUT_LEN, stdin);

	int argc;
	bool exitReq;
	char** argv = parsedArgs(inputStr, &argc);
	long long retVal = argumentParser(argc, argv, &exitReq, true);
	freeParsedArgs(argv);

	if (exitReq) { w2cExit(0); }
	return retVal;
}

static void printWindowList(bool showFullList, HWND parent)
{
	currentPos = 1;
	if (showFullList) { checkIfAltTab = false; }
	else { checkIfAltTab = true; }

	EnumWindows(&enumWindowsProc, NULL);

	if (showFullList) { printf("%d. (Show list of visible)\n", currentPos); }
	else { printf("%d. (Show full list)\n", currentPos); }

	printf("%d. (From handle)\n", currentPos + 1);
	printf("%d. (Magnifier mode) [M]\n", currentPos + 2);
	printf("%d. (Refresh)\n", currentPos + 3);
	printf("%d. (Exit)\n", currentPos + 4);
}

static int printWindowInfo(HWND hwnd, int pos)
{
	char windowName[W2C_MAX_NAME_LEN];
	char className[W2C_MAX_NAME_LEN];
	RECT wndRect, clientRect;

	GetWindowTextA(hwnd, windowName, W2C_MAX_NAME_LEN);
	GetClassNameA(hwnd, className, W2C_MAX_NAME_LEN);

	GetWindowRect(hwnd, &wndRect);
	GetClientRect(hwnd, &clientRect);

	wndRect.right -= wndRect.left;
	wndRect.bottom -= wndRect.top;

	if (wndRect.right == 0 && wndRect.bottom == 0 &&
		clientRect.right == 0 && clientRect.bottom == 0)
	{
		return 0;
	}

	printf("%d. \"%s\" <%dx%d | %dx%d> (%s) [%x]\n\n", pos, windowName,
		wndRect.right, wndRect.bottom,
		clientRect.right, clientRect.bottom,
		className, (int)hwnd);

	return 1;
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

static BOOL CALLBACK enumWindowsProc(HWND hwnd, LPARAM lParam)
{
	if (!checkIfAltTab || isAltTabWindow(hwnd))
	{
		if (currentPos >= W2C_MAX_WND_COUNT) { return FALSE; }
		if (printWindowInfo(hwnd, currentPos))
		{
			hwndArray[currentPos] = hwnd;
			currentPos++;
		}
	}
	return TRUE;
}

//Source for str to argv parser: https://stackoverflow.com/a/1706733/18214530

static int setArgs(char* args, char** argv)
{
	int count = 0;
	while (isspace(*args)) ++args;
	while (*args)
	{
		if (argv) { argv[count] = args; }
		while (*args && !isspace(*args)) { ++args; }
		if (argv && *args) { *args++ = '\0'; }
		while (isspace(*args)) { ++args; }
		count++;
	}
	return count;
}

static char** parsedArgs(char* args, int* argc)
{
	char** argv = NULL;
	int argn = 0;

	if (args && *args &&
		(args = strdup(args)) &&
		(argn = setArgs(args, NULL)) &&
		(argv = malloc((argn + 1) * sizeof(char*))))
	{
		*argv++ = args;
		argn = setArgs(args, argv);
	}

	if (args && !argv) { free(args); }

	*argc = argn;
	return argv;
}

static void freeParsedArgs(char** argv)
{
	if (argv)
	{
		free(argv[-1]);
		free(argv - 1);
	}
}