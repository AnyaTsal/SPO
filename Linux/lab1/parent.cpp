#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <iostream>
#include <string.h>

using namespace std;

int main()
{
	char type[10];
	cout << "Choose type of file (.cpp, .java, (.js))" << endl;
	cin.clear();
	rewind(stdin);
	cin >> type;

	initscr();
	curs_set(0);

	time_t ltime;
	int pid = fork();

	int st;

	switch (pid)
	{
	case -1: {
		printw("Error.");
		break;
	}

	case 0: {
		if (execlp("./child", type, NULL) == -1)
			printw("Error.");
		break;
	}

	default: {
		while(1)
		{
			refresh();
			time(&ltime);
			move(9,0);
			printw(ctime( &ltime ));

			if(waitpid(pid,&st,WNOHANG)>0)
				break;
			napms(50);
                                
		}
		move(20,0);
		
		if (WIFEXITED(st))
		{
			move(21, 0);
			printw("return value:");
			printw("%d", (int)WEXITSTATUS(st));
		}
	}
	}
	getch();
	endwin();
	return 0;
} 


