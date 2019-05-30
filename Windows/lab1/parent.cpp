#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <conio.h>
#include <iostream>

using namespace std;

int main()
{
	char type[10];
	cout << "Choose type of file (.cpp, .java, (.js))" << endl;
	cin.clear();
	rewind(stdin);
	cin >> type;

	char str[255];
	sprintf_s(str, "child.exe %s", type);

	STARTUPINFO si;
	PROCESS_INFORMATION pi;

	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	ZeroMemory(&pi, sizeof(pi));

	if (!CreateProcess(NULL,	// No module name (use command line)
		LPSTR(str),				// Command line
		NULL,					// Process handle not inheritable
		NULL,					// Thread handle not inheritable
		FALSE,					// Set handle inheritance to FALSE
		CREATE_NEW_CONSOLE,		
		NULL,					// Use parent's environment block
		NULL,					// Use parent's starting directory 
		&si,					// Pointer to STARTUPINFO structure
		&pi)					// Pointer to PROCESS_INFORMATION structure
		)
	{
		printf("CreateProcess failed (%d)\n", GetLastError());
		return -1;
	}

	SYSTEMTIME lt;

	while (WaitForSingleObject(pi.hProcess, 50))
	{
		GetLocalTime(&lt);
		printf("%02d:%02d:%02d\r", lt.wHour, lt.wMinute, lt.wSecond);
	}
	DWORD a;
	
	if (GetExitCodeProcess(pi.hProcess, &a)) {
		cout << endl << "Child process is completed with code: "<< a << endl;
	}
	else {
		cout << endl << "There are error in child process" << endl;
	}
	CloseHandle(pi.hProcess);
	_getch();
	return 0;
}
