#include "win2con.h"

static const char* CHARSET_LONG = " .'`^\",:;Il!i><~+_-?][}{1)(|\\/tfjrxnuvczXYUJCLQ0OZmwqpdbkhao*#MW&8%B$@";
static const char* CHARSET_SHORT = " .-+*?#M&%@";
static const char* CHARSET_2 = " *";
static const char* CHARSET_BLOCKS = " \xB0\xB1\xB2\xDB";
static const char* CHARSET_OUTLINE = " @@@@@@@@@@@@@@@@ ";
static const char* CHARSET_BOLD_OUTLINE = " @@@@@@@@@@@@@@@@.";

typedef struct
{
	char* name;
	char* fullName;
	int (*parserFunction)(int, char**);
	bool isOperation;
} Option;

static void checkSettings(void);
static int opHelp(int argc, char** argv);
static int opInput(int argc, char** argv);
static int opColors(int argc, char** argv);
static int opSize(int argc, char** argv);
static int opScalingMode(int argc, char** argv);
static int opClientArea(int argc, char** argv);
static int opTopMost(int argc, char** argv);
static int opMagnifierMode(int argc, char** argv);
static int opInformation(int argc, char** argv);
static int opVersion(int argc, char** argv);
static int opInterlaced(int argc, char** argv);
static int opColorProc(int argc, char** argv);
static int opSetColor(int argc, char** argv);
static int opCharset(int argc, char** argv);
static int opRand(int argc, char** argv);
static int opFontRatio(int argc, char** argv);
static int opFakeConsole(int argc, char** argv);
static int opDisableCLS(int argc, char** argv);
static int opDisableKeys(int argc, char** argv);
static int opIgnoreDPI(int argc, char** argv);
static int opFullInfo(int argc, char** argv);
static void invalidSyntax(int line);

const Option OPTIONS[] = {
	{"-h","--help",&opHelp,true},
	{"-i","--input",&opInput,false},
	{"-c","--colors",&opColors,false},
	{"-s","--size",&opSize,false},
	{"-sm","--scaling-mode",&opScalingMode,false},
	{"-ca","--client-area",&opClientArea,false},
	{"-tm","--top-most",&opTopMost,false},
	{"-m","--magnifier",&opMagnifierMode,false},
	{"-inf","--information",&opInformation,true},
	{"-v","--version",&opVersion,true},
	{"-int","--interlaced",&opInterlaced,false},
	{"-cp","--color-proc",&opColorProc,false},
	{"-sc","--set-color",&opSetColor,false},
	{"-cs","--charset",&opCharset,false},
	{"-r","--rand",&opRand,false},
	{"-fr","--font-ratio",&opFontRatio,false},
	{"-fc","--fake-console",&opFakeConsole,false},
	{"-dcls","--disable-cls",&opDisableCLS,false},
	{"-dk","--disable-keys",&opDisableKeys,false},
	{"-idpi","--ignore-dpi",&opIgnoreDPI,false},
	{"-fi","--full-info",&opFullInfo,true} };

static int optionCount;
static long long inputVal = 0;
static bool argumentsPassed = false;
static bool isFromGetWindow = false;

long long argumentParser(int argc, char** argv, bool* exitReq, bool fromGetWindow)
{
	static bool firstCall = true;
	isFromGetWindow = fromGetWindow;

	if (firstCall)
	{
		settings.charset = CHARSET_LONG;
		settings.charsetSize = (int)strlen(settings.charset);
		firstCall = false;
	}

	*exitReq = false;
	if (argc == 0) { return 0; }

	if (fromGetWindow && argc > 0)
	{
		inputVal = 0;

		if (!strcmp(argv[0], "m") || !strcmp(argv[0], "M"))
		{
			inputVal = -1;
		}

		if (argc < 2)
		{
			if (!inputVal) { opInput(argc, argv); }
			return inputVal;
		}
	}

	if (argumentsPassed)
	{
		puts("\nArguments already passed!");
		return 0;
	}
	argumentsPassed = true;

	optionCount = sizeof(OPTIONS) / sizeof(Option);
	bool* usedOptions = calloc(optionCount, sizeof(bool));

	char** input = (char**)malloc(argc * sizeof(char*));
	for (int i = 0; i < argc; i++)
	{
		input[i] = malloc((strlen(argv[i]) + 1) * sizeof(char));
		strcpy(input[i], argv[i]);
	}

	for (int i = 0; i < argc; i++)
	{
		if (input[i][0] == '-')
		{
			if (i == 0 && !strcmp(input[0], "-?")) { input[0][1] = 'h'; }

			bool optionFound = false;
			for (int j = 0; j < optionCount; j++)
			{
				if (!strcmp(input[i], OPTIONS[j].name) ||
					!strcmp(input[i], OPTIONS[j].fullName))
				{
					if (usedOptions[j]) { invalidSyntax(__LINE__); }
					if (OPTIONS[j].isOperation && i != 0) { invalidSyntax(__LINE__); }

					i += OPTIONS[j].parserFunction(argc - i - 1, input + i + 1);

					if (OPTIONS[j].isOperation)
					{
						if (i != argc - 1) { invalidSyntax(__LINE__); }
						*exitReq = true;
						return NULL;
					}

					usedOptions[j] = true;
					optionFound = true;
					break;
				}
			}

			if (!optionFound) { invalidSyntax(__LINE__); }
		}
		else if (i == 0)
		{
			if (!strcmp(input[0], "/?"))
			{
				input[0][0] = '-';
				input[0][1] = 'h';
				i--;
			}
			else
			{
				opInput(argc, input);
				usedOptions[1] = true;
			}
		}
		else
		{
			invalidSyntax(__LINE__);
		}
	}

	for (int i = 0; i < argc; i++)
	{
		free(input[i]);
	}
	free(input);
	free(usedOptions);

	checkSettings();

	return inputVal;
}

static void checkSettings(void)
{
	if (settings.setColorMode == SCM_WINAPI &&
		settings.colorMode != CM_WINAPI_GRAY &&
		settings.colorMode != CM_CSTD_GRAY)
	{
		error("\"Set color\" works only with grayscale color mode!", "argParser.c", __LINE__);
	}
	if (settings.setColorMode == SCM_WINAPI &&
		settings.colorMode != CM_WINAPI_GRAY &&
		settings.colorMode != CM_CSTD_GRAY)
	{
		error("WinAPI \"set color\" mode mode works only with grayscale color mode!", "argParser.c", __LINE__);
	}
	if ((settings.setColorMode == SCM_CSTD_256 ||
		settings.setColorMode == SCM_CSTD_RGB) &&
		settings.colorMode != CM_CSTD_GRAY)
	{
		error("C std \"set color\" mode works only with \"cstd-gray\" color mode!", "argParser.c", __LINE__);
	}
	if (settings.colorProcMode == CPM_NONE &&
		(settings.colorMode == CM_WINAPI_GRAY ||
			settings.colorMode == CM_CSTD_GRAY))
	{
		error("Single character mode requires colors!", "argParser.c", __LINE__);
	}

	if (settings.colorProcMode == CPM_NONE && settings.brightnessRand)
	{
		if (settings.brightnessRand < 0) { settings.brightnessRand = -settings.brightnessRand; }
	}
}

static int opInput(int argc, char** argv)
{
	if (argc < 1) { invalidSyntax(__LINE__); }
	if (isFromGetWindow && inputVal) { return 1; }
	
	if (isFromGetWindow) { inputVal = strtoll(argv[0], NULL, 10); }
	else { inputVal = strtoll(argv[0], NULL, 16); }

	if (inputVal < 0) { error("Invalid input!", "argParser.c", __LINE__); }
	return 1;
}

static int opColors(int argc, char** argv)
{
	if (argc < 1 || argv[0][0] == '-') { invalidSyntax(__LINE__); }

	strToLower(argv[0]);
	if (!strcmp(argv[0], "winapi-gray")) { settings.colorMode = CM_WINAPI_GRAY; }
	else if (!strcmp(argv[0], "winapi-16")) { settings.colorMode = CM_WINAPI_16; }
	else if (!strcmp(argv[0], "cstd-gray")) { settings.colorMode = CM_CSTD_GRAY; }
	else if (!strcmp(argv[0], "cstd-16")) { settings.colorMode = CM_CSTD_16; }
	else if (!strcmp(argv[0], "cstd-256")) { settings.colorMode = CM_CSTD_256; }
	else if (!strcmp(argv[0], "cstd-rgb")) { settings.colorMode = CM_CSTD_RGB; }
	else { error("Invalid color mode!", "argParser.c", __LINE__); }

	return 1;
}

static int opSize(int argc, char** argv)
{
	if (argc < 2 || argv[0][0] == '-' || argv[1][0] == '-') { invalidSyntax(__LINE__); }
	settings.argW = atoi(argv[0]);
	settings.argH = atoi(argv[1]);
	if ((settings.argW != 0 || settings.argH != 0) &&
		(settings.argW < 4 || settings.argH < 4))
	{
		error("Invalid size!", "argParser.c", __LINE__);
	}
	return 2;
}

static int opScalingMode(int argc, char** argv)
{
	if (argc < 1 || argv[0][0] == '-') { invalidSyntax(__LINE__); }
	settings.scaleWithRatio = false;

	strToLower(argv[0]);
	if (!strcmp(argv[0], "fill")) { settings.scalingMode = SM_FILL; }
	else if (!strcmp(argv[0], "soft-fill")) { settings.scalingMode = SM_SOFT_FILL; }
	else if (!strcmp(argv[0], "const")) { settings.scalingMode = SM_CONST; }
	else if (!strcmp(argv[0], "no-scaling")) { settings.scalingMode = SM_NO_SCALING; }
	else { error("Invalid scaling mode name!", "argParser.c", __LINE__); }
	
	if (settings.scalingMode == SM_CONST)
	{
		if (argc < 2) { invalidSyntax(__LINE__); }

		int retVal = 2;
		int constScaleX = atoi(argv[1]);
		int constScaleY;

		if (argc > 2 && argv[2][0] != '-')
		{
			constScaleY = atoi(argv[2]);
			retVal = 3;
		}
		else
		{
			constScaleY = constScaleX;
		}

		if (constScaleX == 0 || constScaleY == 0)
		{
			error("Invalid const scaling value!", "argParser.c", __LINE__);
		}

		if (constScaleX > 0) { scaleXMul = constScaleX; }
		else { scaleXDiv = -constScaleX; }
		if (constScaleY > 0) { scaleYMul = constScaleY; }
		else { scaleYDiv = -constScaleY; }

		return retVal;
	}
	else if (argc > 1 && argv[1][0] != '-')
	{
		int scalingModeArgument = atoi(argv[1]);
		if (scalingModeArgument != 0 && scalingModeArgument != 1)
		{
			error("Invalid scaling mode argument value!", "argParser.c", __LINE__);
		}

		settings.scaleWithRatio = scalingModeArgument;
		return 2;
	}

	return 1;
}

static int opClientArea(int argc, char** argv)
{
	settings.printClientArea = true;
	return 0;
}

static int opTopMost(int argc, char** argv)
{
	setConsoleTopMost(1);
	return 0;
}

static int opMagnifierMode(int argc, char** argv)
{
	enableMagnifierMode();
	return 0;
}

static int opInformation(int argc, char** argv)
{
	if (argc > 0) { invalidSyntax(__LINE__); }
	showInfo();
	return 0;
}

static int opVersion(int argc, char** argv)
{
	if (argc > 0) { invalidSyntax(__LINE__); }
	showVersion();
	return 0;
}

static int opHelp(int argc, char** argv)
{
	if (argc == 1 && argv[0][0] != '-')
	{
		strToLower(argv[0]);
		if (!strcmp(argv[0], "basic"))              { showHelp(1, 0, 0, 0); }
		else if (!strcmp(argv[0], "advanced"))      { showHelp(0, 1, 0, 0); }
		else if (!strcmp(argv[0], "modes"))         { showHelp(0, 0, 1, 0); }
		else if (!strcmp(argv[0], "keyboard"))      { showHelp(0, 0, 0, 1); }
		else if (!strcmp(argv[0], "full"))          { showHelp(1, 1, 1, 1); }
		else { error("Invalid help topic!", "argParser.c", __LINE__); }
		return 1;
	}
	else if (argc == 0)
	{
		showHelp(1, 0, 0, 1);
		puts("[To see full help use \"win2con -h full\"]");
	}
	else
	{
		invalidSyntax(__LINE__);
	}
	return 0;
}

static int opInterlaced(int argc, char** argv)
{
	if (argc < 1 || argv[0][0] == '-') { invalidSyntax(__LINE__); }
	if (argc > 1 && argv[1][0] != '-')
	{
		settings.scanlineHeight = atoi(argv[1]);
		if (settings.scanlineHeight < 1) { error("Invalid scanline height!", "argParser.c", __LINE__); }

		settings.scanlineCount = atoi(argv[0]);
		if (settings.scanlineCount < 1) { error("Invalid interlacing!", "argParser.c", __LINE__); }
		return 2;
	}
	else
	{
		settings.scanlineCount = atoi(argv[0]);
		if (settings.scanlineCount < 1) { error("Invalid interlacing!", "argParser.c", __LINE__); }
		return 1;
	}
}

static int opColorProc(int argc, char** argv)
{
	if (argc < 1 || argv[0][0] == '-') { invalidSyntax(__LINE__); }

	strToLower(argv[0]);
	if (!strcmp(argv[0], "none")) { settings.colorProcMode = CPM_NONE; }
	else if (!strcmp(argv[0], "char-only")) { settings.colorProcMode = CPM_CHAR_ONLY; }
	else if (!strcmp(argv[0], "both")) { settings.colorProcMode = CPM_BOTH; }
	else { error("Invalid color processing mode!", "argParser.c", __LINE__); }

	return 1;
}

static int opSetColor(int argc, char** argv)
{
	if (argc < 1 || argv[0][0] == '-') { invalidSyntax(__LINE__); }

	if (argv[0][0] >= '0' && argv[0][0] <= '9')
	{
		settings.setColorMode = SCM_WINAPI;
		settings.setColorVal = atoi(argv[0]);
	}
	else if (argv[0][0] == '@')
	{
		settings.setColorMode = SCM_CSTD_256;
		settings.setColorVal = atoi(argv[0] + 1);
		if (settings.setColorVal < 0 || settings.setColorVal > 255)
		{
			error("Invalid color!", "argParser.c", __LINE__);
		}

		if (argc > 1 && argv[1][0] != '-')
		{
			settings.setColorVal2 = atoi(argv[1]);
			if (settings.setColorVal2 < 0 || settings.setColorVal2 > 255)
			{
				error("Invalid color!", "argParser.c", __LINE__);
			}
			return 2;
		}
	}
	else if (argv[0][0] == '#')
	{
		settings.setColorMode = SCM_CSTD_RGB;
		settings.setColorVal = strtol(argv[0] + 1, NULL, 16);
		if (settings.setColorVal < 0 || settings.setColorVal > 0xFFFFFF)
		{
			error("Invalid color!", "argParser.c", __LINE__);
		}

		if (argc > 1 && argv[1][0] != '-')
		{
			settings.setColorVal2 = strtol(argv[1], NULL, 16);
			if (settings.setColorVal2 < 0 || settings.setColorVal2 > 0xFFFFFF)
			{
				error("Invalid color!", "argParser.c", __LINE__);
			}
			return 2;
		}
	}
	else
	{
		invalidSyntax(__LINE__);
	}

	return 1;
}

static int opCharset(int argc, char** argv)
{ 
	const int CHARSET_MAX_SIZE = 256;

	if (argc < 1 || argv[0][0] == '-') { invalidSyntax(__LINE__); }
	if (argv[0][0] == '#')
	{
		strToLower(argv[0]);
		if (!strcmp(argv[0], "#long")) { settings.charset = CHARSET_LONG; }
		else if (!strcmp(argv[0], "#short")) { settings.charset = CHARSET_SHORT; }
		else if (!strcmp(argv[0], "#2")) { settings.charset = CHARSET_2; }
		else if (!strcmp(argv[0], "#blocks")) { settings.charset = CHARSET_BLOCKS; }
		else if (!strcmp(argv[0], "#outline")) { settings.charset = CHARSET_OUTLINE; }
		else if (!strcmp(argv[0], "#bold-outline")) { settings.charset = CHARSET_BOLD_OUTLINE; }
		else { error("Invalid predefined charset name!", "argParser.c", __LINE__); }
		settings.charsetSize = (int)strlen(settings.charset);
	}
	else
	{
		FILE* charsetFile = fopen(argv[0], "rb");
		if (!charsetFile) { error("Failed to open charset file!", "argParser.c", __LINE__); }
		settings.charset = (char*)malloc(CHARSET_MAX_SIZE * sizeof(char));
		settings.charsetSize = (int)fread(settings.charset, sizeof(char), CHARSET_MAX_SIZE, charsetFile);
		fclose(charsetFile);
	}
	return 1;
}

static int opRand(int argc, char** argv)
{
	if (argc < 1 || argv[0][0] == '-') { invalidSyntax(__LINE__); }
	srand(time(NULL));

	if (argv[0][0] == '@') { settings.brightnessRand = -atoi(argv[0] + 1); }
	else { settings.brightnessRand = atoi(argv[0]); }

	if (settings.brightnessRand < -255 || settings.brightnessRand > 255)
	{
		error("Invalid rand value!", "argParser.c", __LINE__);
	}
	return 1;
}

static int opFontRatio(int argc, char** argv)
{
	if (argc < 1 || argv[0][0] == '-') { invalidSyntax(__LINE__); }
	settings.constFontRatio = atof(argv[0]);
	if (settings.constFontRatio <= 0.0) { error("Invalid font ratio!", "argParser.c", __LINE__); }
	return 1;
}

static int opFakeConsole(int argc, char** argv)
{
	settings.useFakeConsole = true;
	return 0;
}

static int opDisableCLS(int argc, char** argv)
{
	settings.disableCLS = true;
	return 0;
}

static int opDisableKeys(int argc, char** argv)
{
	settings.disableKeyboard = true;
	return 0;
}

static int opIgnoreDPI(int argc, char** argv)
{
	settings.ignoreDPI = true;
	return 0;
}

static int opFullInfo(int argc, char** argv)
{
	if (argc > 0) { invalidSyntax(__LINE__); }
	showFullInfo();
	return 0;
}

static void invalidSyntax(int line)
{
	error("Invalid syntax!", "argParser.c", line);
}