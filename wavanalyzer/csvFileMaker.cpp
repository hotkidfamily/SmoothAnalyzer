#include "StdAfx.h"
#include "csvFileMaker.h"
#include "log.h"

CSVFile::~CSVFile(void)
{
	flush();
}

CSVFile::CSVFile(std::string &filename)
{
	csvFileName.assign(filename);
}

void CSVFile::flush()
{
	if(!csvLines.empty()){
		csvFile.open(csvFileName.c_str());
		if(!csvFile.is_open()){
			inter_log(Info, "Can not create file %s.", csvFileName.c_str());
			return ;
		}

		inter_log(Info, "Create file %s.", csvFileName.c_str());
	}

	while(!csvLines.empty()){
		csvFile << csvLines.front() << '\n';
		csvLines.pop_front();
	}
	
}

void CSVFile::WriteCsvLine(const char* format, ...)
{
	std::string csvLine;
	va_list args;

	csvLine.resize(1024, 0);

	va_start(args, format);
	vsprintf_s((char*)csvLine.c_str(), csvLine.size(), format, args);
	va_end(args);

	//csvFile << csvLine << '\n';
	csvLines.push_back(csvLine);
}
