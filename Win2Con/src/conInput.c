#include "win2con.h"

#ifdef _WIN32

#define CALL_CONV __cdecl
typedef uintptr_t ThreadIDType;
typedef void ThreadRetType;

#else

#define CALL_CONV
typedef pthread_t ThreadIDType;
typedef void* ThreadRetType;

#endif

static ThreadIDType conThreadID = 0;

static ThreadRetType CALL_CONV conThread(void* ptr);

void initConInput(void)
{
	return;
	#ifdef _WIN32
	
	if (!disableKeyboard)
	{
		conThreadID = _beginthread(&conThread, 0, NULL);
	}

	#else

	if (!disableKeyboard)
	{
		pthread_create(&conThreadID, NULL, &conThread, NULL);
	}

	#endif
}

static ThreadRetType CALL_CONV conThread(void* ptr)
{
	#ifdef _WIN32
	while (1)
	{
		Sleep(100);
	}
	#endif
}