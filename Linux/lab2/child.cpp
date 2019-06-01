#include <sys/types.h>
#include <unistd.h>
#include <ncurses.h>
#include <time.h>
#include <iostream>
#include <fstream>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define CPP ".cpp"
#define JAVA ".java"
#define JS ".js"

using namespace std;

int main(int argc, char **argv)
{

	fstream stream;
    char file_extension[10];
	stream.open("result.txt", ios::out | ios::app);

	strcpy(file_extension, argv[0]);

	if (strcmp(CPP, file_extension) == 0 || strcmp(JAVA, file_extension) == 0) {
		stream << "Running " << file_extension << " file" << endl;
	}
	else if (strcmp(JS, file_extension) == 0)
	{
		stream << "There may be errors in " << file_extension << " file" << endl;
	}
	else 
	{
		stream << "Forbidden file extension" << endl;
	}
	
	stream.close();
}