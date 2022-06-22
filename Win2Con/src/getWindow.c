#include "win2con.h"

HWND getWindow(void)
{
	long long val;
	fputs(">", stdout);
	scanf("%llx", &val);
	return (HWND)val;
}