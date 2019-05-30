#pragma once
#define CPP ".cpp"
#define JAVA ".java"
#define JS ".js"

class Compiler {
private:
	char* type = NULL;

public:
	Compiler();

	void setTypeOfFile(char type[]);
	int getResult();
	void showResult();
	void showError();
	void showProbableError();
};