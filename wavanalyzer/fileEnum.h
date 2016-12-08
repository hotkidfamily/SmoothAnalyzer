#pragma once

#include "stdafx.h"
#include <windows.h>
#include <string>
#include <list>

#include "log.h"

class FileEnumer
{
public:
	FileEnumer(void);
	~FileEnumer(void);

	int32_t EnumDirectory(std::string path, std::string postFix);
	int32_t IsDirectory(std::string &path);
	
	int32_t GetFile(std::string &file);

private:
	std::list<std::string> m_filesList;
};
