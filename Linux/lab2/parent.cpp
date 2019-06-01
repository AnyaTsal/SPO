#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <ncurses.h>
#include <time.h>
#include <cstdio>
#include <string.h>
#include <stdio.h>
#include <iostream>
#include <stdlib.h>

using namespace std;

struct sigaction print_signal, exit_signal;

int print_flag = 0;
int exit_flag = 1;
void canPrint(int);
void setEndFlag(int);

void canPrint(int signo)
{
	print_flag = 1;
}

void setEndFlag(int signo)
{
	exit_flag = 1;
}

int main(int argc, char** argv)
{
	srand(time(NULL));
    char FILE_EXTENSIONS[3][10] = { ".cpp", ".js", "error" };
	int currentExtension = rand() % 3;
	char PROCESS_NAMES[5][20] = { "First process\n\r", "Second process\n\r", "Third process\n\r", "Fourth process\n\r", "Fifth process\n\r" };

	print_signal.sa_handler = canPrint;
	sigaction(SIGUSR1, &print_signal, NULL);

	exit_signal.sa_handler = setEndFlag;
	sigaction(SIGUSR2, &exit_signal, NULL);

	char strWin[256];
	char process_name[256];
	char eventID[30];

	int currentProcess;

	currentProcess = atoi(argv[0]);

	sprintf(process_name, "%s", PROCESS_NAMES[currentProcess-1]);

	initscr();
	curs_set(0);

	int pid = fork();

    char file_extension[10];
    sprintf(file_extension, "%s", FILE_EXTENSIONS[currentExtension]);
	int st;

	switch (pid)
	{
	case -1: {
		printw("Error.");
		break;
	}

	case 0: {
		if (execlp("./child", file_extension, process_name, NULL) == -1)
			printw("Error.");
		break;
	}

	default: {
		if (atoi(argv[0]) == 1) kill(getppid(), SIGUSR2);
		exit_flag = 0;

		while (!exit_flag)
		{
			usleep(100000);

			if (print_flag)
			{
				print_flag = 0;
				for (int i = 0; i < strlen(process_name); i++)
				{
					if (exit_flag)
					{
						print_flag = 0;
						kill(getppid(), SIGUSR2);
						waitpid(pid, &st, 0);
						return 0;
					}
					printw("%c", process_name[i]);
					refresh();
					usleep(100000);
				}
				print_flag = 0;
				refresh();
				waitpid(pid, &st, 0);
				kill(getppid(), SIGUSR2);
			}
		}
		return 0;
	}
	}
	endwin();
	return 0;
}
