#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>
#include <conio.h>
#include <iostream>
#include <vector>

#define MAX_COUNT_OF_PROCESS 5

using namespace std;

vector<HANDLE> exit_events;
vector<HANDLE> print_events;
PROCESS_INFORMATION process_info[MAX_COUNT_OF_PROCESS];
char EventID[30];
int current_procces = 0;
bool flag = false;

int start();
void add_new_process();
void remove_process();
int remove_all_process();
void print_last_process();
int getch_noblock();

int main()
{
	int i = start();
	if (i == EXIT_FAILURE) cout << "Exit error";
	return 0;
}

int start()
{
	char temp;
	while (true)
	{
		temp = getch_noblock();
		if (temp == '+') add_new_process();
		else if (temp == '-') remove_process();
		else if (temp == 'q') return remove_all_process();

		print_last_process();
	}
	return EXIT_FAILURE;
}

void add_new_process()
{
	if (exit_events.size() < MAX_COUNT_OF_PROCESS)
	{
		STARTUPINFO si;
		PROCESS_INFORMATION pi;
		ZeroMemory(&si, sizeof(si));
		si.cb = sizeof(si);
		ZeroMemory(&pi, sizeof(pi));

		char strWin[256];
		sprintf_s(EventID, " %d", exit_events.size() + 1);
		exit_events.push_back(CreateEvent(
			NULL, //атрибуты защиты 
			FALSE, //автосброс
			FALSE, //несигнальное начальное состояние
			EventID)); //уникаьное имя-идентификатор события

		sprintf_s(EventID, " %dc", print_events.size() + 1);
		print_events.push_back(CreateEvent(NULL, TRUE, FALSE, EventID));

		sprintf_s(strWin, "parent.exe %s", EventID);

		if (!CreateProcess(NULL,
			strWin,
			NULL,
			NULL,
			FALSE,
			NULL,
			NULL,
			NULL,
			&si,
			&pi)
			)
		{
			printf("CreateProcess failed (%lu)\n", GetLastError());
		}
		process_info[exit_events.size() - 1] = pi;
	}
}

void remove_process()
{
	if (exit_events.size() > 0)
	{
		SetEvent(exit_events.back());
		WaitForSingleObject(process_info[exit_events.size() - 1].hProcess, INFINITE);

		if (!CloseHandle(exit_events.back()))
			printf("Close Handle failed (%d)\n", GetLastError());
		if (!CloseHandle(print_events.back()))
			printf("Close Handle failed (%d)\n", GetLastError());

		exit_events.pop_back();
		print_events.pop_back();
		if (current_procces >= exit_events.size())
		{
			current_procces = 0;
			flag = true;
		}
	}
}

int remove_all_process()
{
	if (exit_events.size() > 0)
	{
		while (exit_events.size() > 0)
		{
			SetEvent(exit_events.back());
			WaitForSingleObject(process_info[exit_events.size() - 1].hProcess, INFINITE);

			if (!CloseHandle(exit_events.back()) || !CloseHandle(print_events.back()) || !CloseHandle(process_info[exit_events.size() - 1].hProcess)
				|| !CloseHandle(process_info[exit_events.size() - 1].hThread))
				printf("Close Handle failed (%d)\n", GetLastError());

			exit_events.pop_back();
			print_events.pop_back();
		}
		current_procces = 0;
	}
	printf("\n\n");
	system("pause");
	return EXIT_SUCCESS;
}

void print_last_process()
{
	if (print_events.size() > 0 && WaitForSingleObject(print_events[current_procces], 0) == WAIT_TIMEOUT)
	{
		if (current_procces >= (int)print_events.size() - 1) current_procces = 0;
		else if (!flag) current_procces++;
		flag = false;

		SetEvent(print_events[current_procces]);
	}
}

int getch_noblock()
{
	if (_kbhit())
		return _getch();
	else
		return -1;
}
