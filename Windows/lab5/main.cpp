#include <iostream>
#include <string>
#include <Windows.h>
#include "IAsyncFileOperations.h"

using namespace std;

typedef IAsyncFileOperations* (*__cdecl AsyncFileFactory)();

int main(int agrc, char** argv)
{
	HMODULE dll = LoadLibrary("Dll.dll");
	AsyncFileFactory Factory = NULL;

	if (dll != NULL) {
		Factory = (AsyncFileFactory)GetProcAddress(
			dll,
			"AsyncFileOperationsImportFactory"
		);

		if (Factory != NULL) {
			IAsyncFileOperations* fileAsync = Factory();
			cout << "Wait while processing" << endl;
			cout << fileAsync->Concat(L"Resource", L"result.txt") << endl;
			fileAsync->Destruct();
			cout << "Success" << endl;
			FreeLibrary(dll);
		}
		else {
			cout << "Error while importing dll class" << endl;
			if (dll != NULL) {
				FreeLibrary(dll);
			}
		}
	}
	else {
		cout << "Error while importing dll" << endl;
	}

	cin.ignore();
	return 0;
}
