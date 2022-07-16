#include "win2con.h"

typedef BOOL(WINAPI* FUNC_ISWOW64PROCESS)(HANDLE, PBOOL);

static DWORD getConhostPID(void);
static void injectDLL(DWORD pid, const char* dllName);
static BOOL IsWOW64();

void excludeConsoleFromCapture(void)
{
	if (IsWOW64()) { error("Win2Con is running under WOW64 - use 64-bit version!", "magnifierMode.c", __LINE__); }

	DWORD conhostPID = getConhostPID();
	if (!conhostPID) { error("Unable to get \"conhost.exe\" process ID!", "magnifierMode.c", __LINE__); }

	injectDLL(conhostPID, "C:\\Users\\Damian\\Desktop\\Win2Con\\Win2Con\\hide.dll");
}

static DWORD getConhostPID(void)
{
	const char* CONHOST_EXE_FILE = "conhost.exe";

	DWORD w2cPID = GetCurrentProcessId();
	DWORD w2cParentPID = 0;

	HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	PROCESSENTRY32 entry = { 0 };
	entry.dwSize = sizeof(PROCESSENTRY32);

	// Loop to get conhost PID if it's child of w2c, and w2c parent PID
	if (Process32First(snapshot, &entry))
	{
		do
		{
			if (entry.th32ParentProcessID == w2cPID &&
				!strcmp(entry.szExeFile, CONHOST_EXE_FILE))
			{
				return entry.th32ProcessID;
			}
			else if (entry.th32ProcessID == w2cPID)
			{
				w2cParentPID = entry.th32ParentProcessID;
			}
		} while (Process32Next(snapshot, &entry));
	}

	// Loop to get conhost PID if it's sibling of w2c
	if (Process32First(snapshot, &entry))
	{
		do
		{
			if (entry.th32ParentProcessID == w2cParentPID &&
				!strcmp(entry.szExeFile, CONHOST_EXE_FILE))
			{
				return entry.th32ProcessID;
			}
		} while (Process32Next(snapshot, &entry));
	}

	return 0;
}

static void injectDLL(DWORD pid, const wchar_t* dllName)
{
	int dllNameLen = wcslen(dllName) * sizeof(wchar_t) + 1;

	HANDLE procHandle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
	if (!procHandle) { error("Unable to open \"conhost.exe\" process!", "magnifierMode.c", __LINE__); }

	HMODULE moduleHandle = GetModuleHandleA("kernel32.dll");
	if (!moduleHandle) { error("Unable to get \"kernel32.dll\" module handle!", "magnifierMode.c", __LINE__); }
	void* loadLibAddress = GetProcAddress(moduleHandle, "LoadLibraryW");

	void* memForStr = VirtualAllocEx(procHandle, NULL, dllNameLen,
		MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
	if (!memForStr) { error("Unable to allocate memory for DLL string!", "magnifierMode.c", __LINE__); }
	WriteProcessMemory(procHandle, memForStr, dllName, dllNameLen, NULL);

	HANDLE threadHandle = CreateRemoteThread(procHandle, NULL, NULL,
		(LPTHREAD_START_ROUTINE)loadLibAddress, memForStr, 0, NULL);
	if (!threadHandle) { error("Unable to create remote thread!", "magnifierMode.c", __LINE__); }
	WaitForSingleObject(threadHandle, INFINITE);
	VirtualFreeEx(procHandle, memForStr, dllNameLen, MEM_RELEASE);

	CloseHandle(threadHandle);
	CloseHandle(procHandle);
}

static BOOL IsWOW64()
{
	BOOL underWOW64 = FALSE;

	HMODULE moduleHandle = GetModuleHandleA("kernel32.dll");
	if (!moduleHandle) { error("Unable to get \"kernel32.dll\" module handle!", "magnifierMode.c", __LINE__); }
	FUNC_ISWOW64PROCESS fnIsWow64Process = (FUNC_ISWOW64PROCESS)GetProcAddress(moduleHandle, "IsWow64Process");

	if (fnIsWow64Process)
	{
		if (!fnIsWow64Process(GetCurrentProcess(), &underWOW64))
		{
			error("Unable to determine if win2con is running under WOW64!", "magnifierMode.c", __LINE__);
		}
	}

	return underWOW64;
}