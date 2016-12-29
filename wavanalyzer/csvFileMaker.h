#pragma once

#include "stdafx.h"

class CSVFile
{
public:
	~CSVFile(void);
	CSVFile(STRING &filename);

	void WriteCsvLine(const char* format,  ...);
protected:
	void flush();

private:

#ifdef UNICODE
	std::wofstream csvFile;
#else
	std::ofstream csvFile;
#endif
	
	STRING csvFileName;
	std::list<STRING> csvLines;
};
