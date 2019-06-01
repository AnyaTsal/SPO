#include <sys/types.h>
#include <unistd.h>
#include <ncurses.h>
#include <time.h>
#include <iostream>
#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define CPP ".cpp"
#define JAVA ".java"
#define JS ".js"

void showResult()
{
	printw("Running file...");
}

void showError()
{
	printw("Forbidden file extension");
}

void showProbableError()
{
	printw("There may be errors");
}

int getResult(char* type)
{
	if (strcmp(CPP, type) == 0 || strcmp(JAVA, type) == 0) {
		showResult();
		return 2;
	}
	else if (strcmp(JS, type) == 0)
	{
		showProbableError();
		return 1;
	}
	else 
	{
		showError();
		return 0;
	}
}

int main(int argc, char** argv)
{
	time_t ltime;
	char c;

	initscr();
	noecho();
	
	char a[100];
	strcpy(a, argv[0]);
	int i = getResult(a);

	
	while(c != 27)
	{
		time(&ltime);
		move(8,0);
		printw(ctime(&ltime));
		
		halfdelay(2);
		c = getch();
		refresh();
	}
		
	return i;
}  
