#include "win2con.h"

static void helpBasicOptions(void);
static void helpAdvancedOptions(void);
static void helpColorModes(void);
static void helpScalingModes(void);
static void helpKeyboard(void);

void showHelp(int basic, int advanced, int colorModes, int scalingModes, int keyboard)
{
	puts("Win2Con - Help\n");
	if (basic) { helpBasicOptions(); }
	if (advanced) { helpAdvancedOptions(); }
	if (colorModes) { helpColorModes(); }
	if (scalingModes) { helpScalingModes(); }
	if (keyboard) { helpKeyboard(); }
}

void showInfo(void)
{
	puts(
		"Win2Con - Information\n"
		"Win2Con " W2C_VERSION " [" W2C_CPU "/" W2C_OS "]\n"
		"Author: https://github.com/mt1006\n"
		"Version: " W2C_VERSION "\n"
		"Architecture: " W2C_CPU "\n"
		"Platform: " W2C_OS);
}

void showFullInfo(void)
{
	puts(
		"Win2Con - Full info\n"
		"Win2Con " W2C_VERSION " [" W2C_CPU "/" W2C_OS "]\n"
		"Author: https://github.com/mt1006\n"
		"Version: " W2C_VERSION "\n"
		"Architecture: " W2C_CPU "\n"
		"Platform: " W2C_OS);
	#if defined(_MSC_VER)
	puts("Compiler: MSC\n"
		"Compiler version: " DEF_TO_STR(_MSC_VER) " [" DEF_TO_STR(_MSC_FULL_VER) "]");
	#elif defined(__GNUC__)
	puts("Compiler: GCC\n"
		"Compiler version: " __VERSION__);
	#else
	puts("Compiler: [unknown]");
	#endif
}

void showVersion(void)
{
	puts("Win2Con " W2C_VERSION " [" W2C_CPU "/" W2C_OS "]");
}

static void helpBasicOptions(void)
{
	puts(
		"Basic options:\n"
		" [none] / -i         Handle to input window (in hexadecimal).\n"
		"  [handle]           When used, it will skip window selection menu.\n"
		"                     Examples:\n"
		"                      win2con 000100F8\n"
		" -c [mode]           Sets color mode.\n"
		"  (--colors)         Default color mode is \"cstd-256\".\n"
		"                     To get list of all avaible color modes use \"win2con -h color-modes\"\n"
		"                     Examples:\n"
		"                      win2con -c winapi-16\n"
		" -s [w] [h]          Sets width and height of the drawn image.\n"
		"  (--size)           By default size of entire window.\n"
		"                     Using \"-s 0 0\" image size will be constant\n"
		"                     (will not change with the console size change).\n"
		"                     Examples:\n"
		"                      win2con -s 120 50\n"
		" -sm [mode] [...]    Sets scaling mode.\n"
		"  (--scaling-mode)   Default scaling mode is \"fill\" with enabled keeping ratio.\n"
		"                     To get list of all avaible scaling modes use \"win2con -h scaling-modes\"\n"
		"                     Examples:\n"
		"                      win2con -sm no-scaling\n"
		" -ca (--client-area) Gets only window client area (without title bar or menu).\n"
		"                     It may not work properly in some cases!\n"
		" -tm (--top-most)    Places console window above all other windows\n"
		"                     and makes it transparent to input.\n"
		"                     Doesn't work with Windows Terminal!\n"
		" -m (--magnifier)    Enables \"magnifier mode\".\n"
		"                     Instead of drawing one window, Win2Con will draw area under console\n."
		" -inf(--information) Information about Win2Con.\n"
		" -v  (--version)     Information about Win2Con version.\n"
		" -h <topic>          Displays help message.\n"
		"  (--help)           Topics: basic, advanced, color-modes, scaling-modes, keyboard, full\n");
}

static void helpAdvancedOptions(void)
{
	puts(
		"Advanced options:\n"
		" -int [divisor]      Uses interlacing to draw frames.\n"
		"      <height>       The larger the divisor, fewer scanlines there are per frame.\n"
		"  (--interlaced)     Height is optional parameter and means scanline height - by default 1.\n"
		"                     When divisor is equal to 1, then interlacing is disabled.\n"
		"                     Note: in winapi mode instead of improve performance, it may degrade it.\n"
		"                     Examples:\n"
		"                      win2con -int 2\n"
		"                      win2con -int 4 3\n"
		" -cs [charset]       Sets character set used for drawing frames.\n"
		"  (--charset)        Takes name of file with charset or name of predefined charset.\n"
		"                     Predefined charsets: #long, #short, #2, #blocks, #outline, #bold-outline.\n"
		"                     Default charset is \"#long\"\n"
		"                     Examples:\n"
		"                      win2con -ch #blocks\n"
		"                      win2con -ch my_charset.txt\n"
		" -sc [value]         Sets constant color in grayscale mode.\n"
		"  (--set-color)      Only number - sets text attribute using WinAPI.\n"
		"                     \"@color\" - sets color from ANSI 256 palette (only cstd-gray).\n"
		"                     \"#RRGGBB\" - sets RGB color (only cstd-gray).\n"
		"                     With \"@\" or \"#\" you can give second argument as background color.\n"
		"                     Examples:\n"
		"                      win2con -c cstd-gray -sc 4\n"
		"                      win2con -c cstd-gray -sc @93\n"
		"                      win2con -c cstd-gray -sc #FF00FF\n"
		"                      win2con -c cstd-gray -sc #FF00FF FF0000\n"
		" -sch                Uses single character to draw image and sets it's color to original,\n"
		"  (--single-char)    instead of recalculated. Requires colors!\n"
		"                     Examples:\n"
		"                      win2con -c cstd-rgb -sch\n"
		" -fr [ratio]         Sets constant font ratio (x/y).\n"
		"  (--font-ratio)     By default font ratio is obtained using WinAPI.\n"
		"                     If it's not posible, it's set to 8/18 (about 0.44).\n"
		"                     Examples:\n"
		"                      win2con -ft 0.5\n"
		" -dcls               Disables clearing screen every new frame.\n"
		"   (--disable-cls)   Uses new line instead. Useful for printing output to file.\n"
		"                     Works properly only in \"cstd\" color mode and it breaks interlacing.\n"
		"                     Examples:\n"
		"                      win2con -c cstd-gray -s 80 60 -fr 1.0 -ds -dcls > output.txt\n"
		" -dk (--disable-keys)Disables keyboard control.\n"
		" -ei (--enable-input)Enables sending input from console to window.\n"
		"                     Doesn't work properly! Use \"-tm\" instead.\n"
		" -idpi (--ignore-dpi)Ignores DPI.\n"
		" -fi (--full-info)   Full info about Win2Con.\n");
}

static void helpColorModes(void)
{
	puts(
		"Color modes:\n"
		" >winapi-gray\n"
		" >winapi-16\n"
		" >cstd-gray\n"
		" >cstd-16\n"
		" >cstd-256 (default)\n"
		" >cstd-rgb\n");
}

static void helpScalingModes(void)
{
	puts(
		"Scaling modes:\n"
		" >fill <keep-ratio> (default with enabled keeping ratio)\n"
		" >int <keep-ratio>\n"
		" >int-fraction\n"
		" >const [x] [y]\n"
		" >no-scaling <keep-ratio>\n");
}

static void helpKeyboard(void)
{
	puts(
		"Keyboard control [Right Alt + ...]:\n"
		" Q - Exit from window\n"
		" X - Exit from Win2Con\n"
		" C - Switch: entire window/client area\n"
		" T - Switch: console window above all other windows\n");
}