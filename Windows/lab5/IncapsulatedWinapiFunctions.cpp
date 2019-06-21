#include "pch.h"
#include "IncapsulatedWinapiFunctions.h"


IncapsulatedWinapiFunctions::IncapsulatedWinapiFunctions()
{
}


IncapsulatedWinapiFunctions::~IncapsulatedWinapiFunctions()
{
}

HANDLE WINAPI IncapsulatedWinapiFunctions::CreateThread(LPTHREAD_START_ROUTINE function, LPVOID params) {
	return ::CreateThread(NULL, NULL, function, params, NULL, NULL);
}

HANDLE WINAPI IncapsulatedWinapiFunctions::CreateAutoEvent(BOOL initialState) {
	return ::CreateEventW(NULL, FALSE, initialState, NULL);
}

HANDLE WINAPI IncapsulatedWinapiFunctions::CreateFileUnicode(LPCWSTR fileName, DWORD accessFlag, DWORD creationFlag) {
	return ::CreateFileW(fileName, accessFlag, 0, NULL, creationFlag, FILE_FLAG_OVERLAPPED, NULL);
}