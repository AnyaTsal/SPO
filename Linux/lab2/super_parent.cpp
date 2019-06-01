#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cstring>
#include <signal.h>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <iterator>
#include <algorithm>
#include <csignal>
#include <sstream>
#include <sys/sem.h>

using namespace std;

#define MAX_COUNT_OF_PROCESS 5

int init_signal_handler();
void add_process();
void delete_last_process();
void print_process();
void delete_all_process();
void can_print(int);
void set_end_flag(int);
void check_input();

pid_t cpid[MAX_COUNT_OF_PROCESS];
int current_process = 0;
int process_count = 0;
int flag = 0;
int print_flag = 0;
int end_flag = 1;

struct sigaction print_signal, end_signal;

int main()
{
	initscr();
	curs_set(0);
	clear();
	noecho();
	refresh();

	init_signal_handler();

	check_input();

	delete_all_process();
	clear();
	endwin();
	return 0;
}

int init_signal_handler()
{

	print_signal.sa_handler = can_print;
	sigaction(SIGUSR1, &print_signal, NULL);

	end_signal.sa_handler = set_end_flag;
	sigaction(SIGUSR2, &end_signal, NULL);
}

void check_input()
{
	char character = 0;

	while (character != 'q')
	{
		character = getchar();
		if (character == '-') delete_last_process();
		if (character == '+') add_process();
		print_process();
	}
}

void add_process()
{
	if (process_count < MAX_COUNT_OF_PROCESS)
	{
		cpid[process_count] = fork();

		process_count++;
		if (cpid[process_count - 1] == 0)
		{
			char str[256];
			sprintf(str, "%d", process_count);
			if (execlp("./parent", str, NULL) == -1)
				cout << "Error.";
		}
	}
}

void delete_last_process()
{
	if (process_count > 0)
	{
		kill(cpid[process_count - 1], SIGUSR2);
		waitpid(cpid[process_count - 1], NULL, 0);

		process_count--;

		if (current_process >= process_count)
		{
			current_process = 0;
			flag = 1;
			end_flag = 1;
		}
	}
}

void print_process()
{
	if (end_flag && process_count > 0)
	{
		end_flag = 0;
		if (current_process >= process_count - 1)
			current_process = 0;
		else if (!flag) current_process++;

		flag = 0;
		kill(cpid[current_process], SIGUSR1);
	}
}

void delete_all_process()
{
	if (cpid[process_count - 1] != 0)
		while (process_count >= 0)
		{
			kill(cpid[process_count - 1], SIGUSR2);
			waitpid(cpid[process_count - 1], NULL, 0);
			process_count--;
		}
}

void can_print(int signo)
{
	print_flag = 1;
}

void set_end_flag(int signo)
{
	end_flag = 1;
}