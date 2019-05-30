#include <iostream>
#include "Compiler.h"

using namespace std;

Compiler::Compiler() : type(NULL)
{
}

void Compiler::setTypeOfFile(char type[])
{
	this->type = type;
}

int Compiler::getResult()
{
	if (strcmp(CPP, type) == 0 || strcmp(JAVA, type) == 0) {
		showResult();
		return 2;
	}
	else if (strcmp(JS, type) == 0) {
		showProbableError();
		return 1;
	}
	else {
		showError();
		return 0;
	}
}

void Compiler::showResult()
{
	cout << "Running " << type << " file..." << endl;
}

void Compiler::showError()
{
	cout << "Forbidden file extension" << endl;
}

void Compiler::showProbableError()
{
	cout << "There may be errors" << endl;
}