#include <stdio.h>
#include <conio.h>
#include <windows.h>
#include "Compiler.h"

int main(int argc, char *argv[])
{
	Compiler compiler;
	compiler.setTypeOfFile(argv[1]);
	int i = compiler.getResult();
	SYSTEMTIME lt;

	while (_kbhit() == 0)
	{
		GetLocalTime(&lt);
		printf("%02d:%02d:%02d\r", lt.wHour, lt.wMinute, lt.wSecond);
		Sleep(50);
	}
	return i;
}