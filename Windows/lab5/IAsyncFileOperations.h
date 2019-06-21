#pragma once
#include <string>

class IAsyncFileOperations
{
public:
	virtual int Concat(std::wstring folderPath, std::wstring filePath) = 0;
	virtual void Destruct() = 0;
	virtual ~IAsyncFileOperations();
};
