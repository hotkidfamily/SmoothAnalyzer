#pragma once

#include "stdafx.h"

class CSVFile
{
public:
	~CSVFile(void);
	CSVFile(std::string &filename);

	void WriteCsvLine(const char* format,  ...);
protected:
	void flush();

private:
	std::ofstream csvFile;
	std::string csvFileName;
	std::list<std::string> csvLines;
};
