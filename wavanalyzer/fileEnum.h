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

	int enumDirectory(std::string path, std::string postFix);
	int isDirectory(std::string path);
	
	int getFile(std::string &file);

private:
	std::list<std::string> m_filesList;
};
