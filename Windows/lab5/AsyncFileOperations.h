#pragma once

#ifdef ASYNCFILEOPERATIONS_EXPORTS
#define ASYNC_API __declspec(dllexport)
#else
#define ASYNC_API __declspec(dllimport)
#endif // ASYNCFILEOPERATIONS_EXPORTS

#include <Windows.h>
#include <iostream>
#include <string>
#include <vector>
#include "IAsyncFileOperations.h"

using namespace std;

extern "C" ASYNC_API IAsyncFileOperations* __cdecl AsyncFileOperationsImportFactory();

namespace async_file_operations
{
	class AsyncFileOperations : public IAsyncFileOperations {
	public:

		AsyncFileOperations();
		~AsyncFileOperations();

		int Concat(wstring folderPath, wstring filePath);
		void Destruct();

	private:
		AsyncFileOperations(const AsyncFileOperations&) = delete;
		AsyncFileOperations& operator =(const AsyncFileOperations&) = delete;

		struct ThreadParams {
		public:
			AsyncFileOperations* operations;
			HANDLE hFile;
			int number;
			OVERLAPPED asyncInfo;
		};

		struct ReadInformation {
			int offset;
			int size;
			wchar_t* data;
		};

		static DWORD WINAPI ReadAsync(LPVOID threadParam);
		static DWORD WINAPI WriteAsync(LPVOID threadParam);

		void FreeAllocatedMemory();
		vector<wstring> GetFilesFromSource(const wstring& folder);

		vector<wstring> filesNames;
		vector<ThreadParams*> threadParams;
		vector<HANDLE> hReadEvents;
		vector<HANDLE> hReaderThreads;
		vector<ReadInformation*> readers;
		HANDLE hWriterThread;
		wstring outputFile;
		int outputFiles;
	};
}