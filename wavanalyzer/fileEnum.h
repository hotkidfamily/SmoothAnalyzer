#pragma once

#include "stdafx.h"
#include <windows.h>
#include <string>
#include <list>

#include "log.h"

class fileEnum
{
public:
	fileEnum(void);
	~fileEnum(void);

	int32_t enumDirectory(std::string path, std::string postFix);
	int32_t isDirectory(std::string path);
	
	int32_t getFile(std::string &file);

private:
	std::list<std::string> m_filesList;
};
