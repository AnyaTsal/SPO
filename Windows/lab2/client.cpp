#include <fstream>
#include "Compiler.h"

using namespace std;

int main(int argc, char *argv[])
{
	fstream stream;
	char file_extension[10];
	stream.open("result.txt", ios::out | ios::app );

	Compiler compiler;

	compiler.setTypeOfFile(argv[1]);
	strcpy_s(file_extension, argv[1]);

	if (compiler.getResult() == 2) {
		stream << "Running " << file_extension << " file" << endl;
	}
	else if (compiler.getResult() == 1) {
		stream << "There may be errors in " << file_extension << " file" << endl;
	}
	else stream << "Forbidden file extension" << endl;
	stream.close();

	return 0;
}