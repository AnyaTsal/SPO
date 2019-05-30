#define _CRT_SECURE_NO_WARNINGS
#define MAX_SECTION_SIZE 30

#include <conio.h>
#include <cstdlib>
#include <windows.h>
#include <string>

int main(int argc, char* argv[]) {

	char procID[10];

	sprintf(procID, " %dñ", atoi(argv[1]));
	HANDLE close = OpenSemaphore(SEMAPHORE_ALL_ACCESS, FALSE, procID);

	sprintf(procID, " %df", atoi(argv[1]));
	HANDLE fileMap = OpenSemaphore(SEMAPHORE_ALL_ACCESS, FALSE, procID);

	HANDLE print = OpenSemaphore(SEMAPHORE_ALL_ACCESS, FALSE, "Print");

	HANDLE memSection;
	memSection = OpenFileMapping(FILE_MAP_ALL_ACCESS,
		FALSE,
		"FileMap");
	LPVOID buff = MapViewOfFile(memSection,
		FILE_MAP_ALL_ACCESS,
		0,
		0,
		MAX_SECTION_SIZE);

	char PROCESS_NAMES[5][20] = { "First process\n\r", "Second process\n\r", "Third process\n\r", "Fourth process\n\r", "Fifth process\n\r" };
	char process_name[256];

	int currentProcess;

	currentProcess = atoi(argv[1]);
	sprintf(process_name, "%s", PROCESS_NAMES[currentProcess - 1]);

	char empty[MAX_SECTION_SIZE];
	memset(empty, '\0', MAX_SECTION_SIZE);
	char uniStr[MAX_SECTION_SIZE];
	sprintf(uniStr, process_name);

	while (true)
	{
		if (WaitForSingleObject(fileMap, 1) == WAIT_OBJECT_0)
		{
			CopyMemory((PVOID)buff, empty, sizeof(empty));
			CopyMemory((PVOID)buff, uniStr, strlen(uniStr));
			ReleaseSemaphore(print, 1, NULL);
		}
		if (WaitForSingleObject(close, 1) == WAIT_OBJECT_0)
		{
			CloseHandle(close);
			CloseHandle(fileMap);
			CloseHandle(print);
			CloseHandle(memSection);
			return 0;
		}
	}

	return 0;
}
