// AsyncFileOperations.cpp: определяет экспортированные функции для приложения DLL.
//

#include "pch.h"
#include "AsyncFileOperations.h"
#include <filesystem>
#include <fstream>
#include <sstream>
#include "IncapsulatedWinapiFunctions.h"


namespace async_file_operations
{
	AsyncFileOperations::AsyncFileOperations() {

	}

	AsyncFileOperations::~AsyncFileOperations() {

	}

	int AsyncFileOperations::Concat(wstring sourceFilePath, wstring resultFilePath) {
		this->outputFile = resultFilePath;
		this->outputFiles = 0;
		this->filesNames = GetFilesFromSource(sourceFilePath);
		int num = 0;
		int offset = 0;

		for (auto& item : this->filesNames) {
			this->threadParams.push_back(new ThreadParams);
			this->threadParams.back()->hFile =
				IncapsulatedWinapiFunctions::CreateFileUnicode(
					item.c_str(),
					GENERIC_READ,
					OPEN_EXISTING
				);
			this->readers.push_back(new ReadInformation);
			this->readers.back()->size = GetFileSize(this->threadParams.back()->hFile, NULL);

			if (this->readers.back()->size == 0) {
				CloseHandle(this->threadParams.back()->hFile);
				delete this->threadParams.back();
				this->threadParams.pop_back();
			}
			else {
				this->hReadEvents.push_back(IncapsulatedWinapiFunctions::CreateAutoEvent(FALSE));
				this->readers.back()->data = new wchar_t[this->readers.back()->size];
				memset(this->readers.back()->data, 0, sizeof(wchar_t));
				this->readers.back()->offset = offset;

				this->threadParams.back()->operations = this;
				this->threadParams.back()->number = num;
				OVERLAPPED async = { 0 };
				this->threadParams.back()->asyncInfo = async;
				this->threadParams.back()->asyncInfo.hEvent = this->hReadEvents.back();


				this->hReaderThreads.push_back(
					IncapsulatedWinapiFunctions::CreateThread(
						ReadAsync,
						reinterpret_cast<LPVOID>(this->threadParams.back())
					)
				);

				offset += this->readers.back()->size;
				++num;
				++this->outputFiles;
			}
		}

		this->hWriterThread = IncapsulatedWinapiFunctions::CreateThread(WriteAsync, static_cast<LPVOID>(this));

		WaitForSingleObject(this->hWriterThread, INFINITE);

		this->FreeAllocatedMemory();

		return offset;
	}

	void AsyncFileOperations::Destruct() {
		delete this;
	}

	DWORD WINAPI AsyncFileOperations::ReadAsync(LPVOID threadParam) {
		ThreadParams* threadParams = reinterpret_cast<ThreadParams*>(threadParam);

		int size = threadParams
			->operations
			->readers[threadParams->number]
			->size;

		wchar_t* buffer = threadParams
			->operations
			->readers[threadParams->number]
			->data;

		ReadFile(
			threadParams->hFile,
			buffer,
			size,
			NULL,
			&threadParams->asyncInfo
		);

		return 0;
	}

	DWORD WINAPI AsyncFileOperations::WriteAsync(LPVOID threadParam) {
		AsyncFileOperations* operations = reinterpret_cast<AsyncFileOperations*>(threadParam);

		vector<HANDLE> hWriteEvents;
		vector<OVERLAPPED*> asyncInfos;
		HANDLE resultFile = IncapsulatedWinapiFunctions::CreateFileUnicode(
			operations->outputFile.c_str(),
			GENERIC_WRITE,
			CREATE_ALWAYS
		);

		while (operations->outputFiles > 0) {
			WaitForMultipleObjects(
				operations->hReadEvents.size(),
				operations->hReadEvents.data(),
				TRUE,
				INFINITE
			);

			for (auto& item : operations->readers) {
				if (item->data[0] != '\0') {
					hWriteEvents.push_back(IncapsulatedWinapiFunctions::CreateAutoEvent(FALSE));
					asyncInfos.push_back(new OVERLAPPED);
					OVERLAPPED async = { 0 };
					*asyncInfos.back() = async;
					asyncInfos.back()->Offset = item->offset;
					asyncInfos.back()->hEvent = hWriteEvents.back();

					WriteFile(
						resultFile,
						item->data,
						item->size,
						NULL,
						asyncInfos.back()
					);

					operations->outputFiles--;
				}
			}
		}

		WaitForMultipleObjects(
			hWriteEvents.size(),
			hWriteEvents.data(),
			TRUE,
			INFINITE
		);

		for (auto& item : hWriteEvents) {
			CloseHandle(item);
		}

		for (auto& item : asyncInfos) {
			delete item;
		}

		CloseHandle(resultFile);

		return 0;
	}

	void AsyncFileOperations::FreeAllocatedMemory() {

		for (auto& item : this->threadParams) {
			CloseHandle(item->hFile);
			delete item;
		}

		this->threadParams.clear();
		this->readers.clear();

		for (auto& item : this->readers) {
			delete item->data;
			delete item;
		}

		this->readers.clear();

		for (auto& item : this->hReadEvents) {
			CloseHandle(item);
		}

		this->hReadEvents.clear();

		for (auto& item : this->hReaderThreads) {
			CloseHandle(item);
		}

		this->hReaderThreads.clear();

		CloseHandle(this->hWriterThread);
	}

	vector<wstring> AsyncFileOperations::GetFilesFromSource(const wstring& folder) {
		vector<wstring> files;
		const wstring path = L".\\" + folder;

		for (auto& item : std::experimental::filesystem::directory_iterator(path)) {
			files.emplace_back(item.path().wstring());
		}

		return files;
	}

}

IAsyncFileOperations* __cdecl AsyncFileOperationsImportFactory() {
	return new async_file_operations::AsyncFileOperations;
}