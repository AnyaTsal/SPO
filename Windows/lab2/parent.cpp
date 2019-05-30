#include <conio.h>
#include <windows.h>
#include <string.h>
#include <stdio.h>
#include <iostream>
#include <time.h>

using namespace std;

void printString(int eventID, char* string)
{
	char tempEventID[30];
	sprintf_s(tempEventID, " %d", eventID);
	HANDLE exit_event = OpenEvent(EVENT_ALL_ACCESS, FALSE, tempEventID);

	sprintf_s(tempEventID, " %dc", eventID);
	HANDLE print_event = OpenEvent(EVENT_ALL_ACCESS, FALSE, tempEventID);

	while (true)
	{
		if (WaitForSingleObject(print_event, 1) == WAIT_OBJECT_0) 
		{
			for (int i = 0; i < strlen(string) - 1; i++)
			{
				if (WaitForSingleObject(exit_event, 0) == WAIT_OBJECT_0)
				{
					CloseHandle(exit_event);
					CloseHandle(print_event);
					return;
				}
				cout << string[i];
				Sleep(100);
			}
			ResetEvent(print_event);
		}

		if (WaitForSingleObject(exit_event, 0) == WAIT_OBJECT_0)
		{
			CloseHandle(exit_event); 
			CloseHandle(print_event);
			return;
		}
	}
	return;
}

int main(int argc, char** argv)
{
	srand(time(NULL));
	char FILE_EXTENSIONS[3][10] = { ".cpp", ".js", "error" };
	int currentExtension = rand() % 3;
	char PROCESS_NAMES[5][20] = { "First process\n\r", "Second process\n\r", "Third process\n\r", "Fourth process\n\r", "Fifth process\n\r" };

	char strWin[256];
	char process_name[256];
	char eventID[30];

	int currentProcess;

	strcpy(eventID, argv[1]);
	currentProcess = atoi(argv[1]);

	sprintf(process_name, "%s ", PROCESS_NAMES[currentProcess-1]);

	sprintf(strWin, "child.exe %s", FILE_EXTENSIONS[currentExtension]);

	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	ZeroMemory(&pi, sizeof(pi));

	if (!CreateProcess(NULL,	// No module name (use command line)
		TEXT(strWin),			// Command line
		NULL,					// Process handle not inheritable
		NULL,					// Thread handle not inheritable
		FALSE,					// Set handle inheritance to FALSE
		CREATE_NEW_CONSOLE,		// 
		NULL,					// Use parent's environment block
		NULL,					// Use parent's starting directory 
		&si,					// Pointer to STARTUPINFO structure
		&pi)					// Pointer to PROCESS_INFORMATION structure
		)
	{
		printf("CreateProcess failed (%lu)\n", GetLastError());
		return -1;
	}

	printString(atoi(eventID), process_name);

	CloseHandle(pi.hThread);

	return 0;
}