#pragma once

#include "stdafx.h"

class FileEnumer
{
public:
	FileEnumer(void);
	~FileEnumer(void);

	int32_t EnumDirectory(STRING path, STRING postFix);
	int32_t IsDirectory(STRING &path);
	
	int32_t GetFile(STRING &file);

private:
	std::list<STRING> m_filesList;
};
