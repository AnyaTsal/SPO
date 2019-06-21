#include <Windows.h>

#pragma once
class IncapsulatedWinapiFunctions
{
public:
	IncapsulatedWinapiFunctions();
	~IncapsulatedWinapiFunctions();

	static HANDLE WINAPI CreateThread(LPTHREAD_START_ROUTINE function, LPVOID params);
	static HANDLE WINAPI CreateAutoEvent(BOOL initialState);
	static HANDLE WINAPI CreateFileUnicode(LPCWSTR fileName, DWORD accessFlag, DWORD creationFlag);
};
