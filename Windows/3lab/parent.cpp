#define _CRT_SECURE_NO_WARNINGS
#define MAX_AMOUNT_OP_PROCESSES 5
#define MAX_SECTION_SIZE 30

#include <conio.h>
#include <windows.h>
#include <ctime>

#include <vector>
#include <iostream>

using namespace std;

vector<PROCESS_INFORMATION> processes;
vector<HANDLE> closeSemas;
vector<HANDLE> filemapSemas;
char procID[10];
int i = 0;

PROCESS_INFORMATION createNewProcess(char* eventID)
{
	STARTUPINFO si;
	ZeroMemory(&si, sizeof(STARTUPINFO));
	si.cb = sizeof(STARTUPINFO);

	PROCESS_INFORMATION pi;
	char args[30];
	sprintf(args, "child.exe %s", eventID);
	if (!CreateProcess(NULL, args, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
		printf("Create Process failed (%d)\n", GetLastError());
	}
	return pi;
}

void addOneProcess(int id) {
	sprintf(procID, " %d", id);
	processes.push_back(createNewProcess(procID));

	sprintf(procID, " %dс", id);
	closeSemas.push_back(CreateSemaphore(NULL, 0, 1, procID));
	sprintf(procID, " %df", id);
	filemapSemas.push_back(CreateSemaphore(NULL, 0, 1, procID));
}

void removeOneProcess() {
	ReleaseSemaphore(closeSemas.back(), 1, NULL);

	WaitForSingleObject(processes.back().hProcess, INFINITE);
	if (!CloseHandle(closeSemas.back()))
		printf("Close Handle failed (%d)\n", GetLastError());
	if (!CloseHandle(filemapSemas.back()))
		printf("Print Handle failed (%d)\n", GetLastError());
	CloseHandle(processes.back().hProcess);
	CloseHandle(processes.back().hThread);

	closeSemas.pop_back();
	filemapSemas.pop_back();
	processes.pop_back();
}

int switchMenu(char key) {
	int retCode = 0;
	if (key == '+' && processes.size() < MAX_AMOUNT_OP_PROCESSES)
	{
		addOneProcess(processes.size() + 1);
		if (processes.size() == 1)
			ReleaseSemaphore(filemapSemas[i], 1, NULL);
		return retCode;
	}
	if (key == '-' && !processes.empty())
	{
		if (i == processes.size() - 1) {
			i = -1;
			retCode = 1;
		}
		removeOneProcess();
		return retCode;
	}
	if (key == 'q')
	{
		while (!processes.empty()) {
			removeOneProcess();
		}
		retCode = -1;
		return retCode;
	}
	return retCode;
}

int main()
{
	HANDLE print = CreateSemaphore(NULL, 0, 1, "Print");
	bool isPrinting = false;
	HANDLE section;
	char uniString[MAX_SECTION_SIZE];
	LPVOID buff;
	section = CreateFileMapping(INVALID_HANDLE_VALUE,
		NULL,
		PAGE_READWRITE,
		0,
		MAX_SECTION_SIZE,
		"FileMap");
	buff = MapViewOfFile(section,
		FILE_MAP_ALL_ACCESS,
		0,
		0,
		MAX_SECTION_SIZE);

	int switchCallback;
	char key = 0;

	while (true)
	{
		if (!filemapSemas.empty() && WaitForSingleObject(print, 1) == WAIT_OBJECT_0 && isPrinting == false)
		{
			isPrinting = true;
			memset(uniString, '\0', MAX_SECTION_SIZE);
			strncpy(uniString, (char*)buff, MAX_SECTION_SIZE);
			for (int j = 0; j < strlen(uniString); j++)
			{
				if (_kbhit())
				{
					switchCallback = switchMenu(_getch());
					if (switchCallback)
						break;
				}
				printf("%c", uniString[j]);
				Sleep(75);
			}
			if (switchCallback == -1)
				break;
			if (i == processes.size() - 1) {
				i = -1;
			}
			isPrinting = false;
			ReleaseSemaphore(filemapSemas[++i], 1, NULL);
		}

		if (_kbhit())
		{
			switchCallback = switchMenu(_getch());
			if (switchCallback == -1)
				break;
		}
	}

	CloseHandle(section);
	printf("\n");
	system("pause");
	return 0;
}
