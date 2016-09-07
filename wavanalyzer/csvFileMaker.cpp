#include "StdAfx.h"
#include "csvFileMaker.h"
#include <stdarg.h>

csvOutput::csvOutput(void)
{
}

csvOutput::~csvOutput(void)
{
}

csvOutput::csvOutput(const char* filename)
{
	csvFilePath.assign(filename, strlen(filename));
}

void csvOutput::recordTimestamp(int32_t channelID, int32_t start, int32_t end)
{
	syncTimestamp time;
	time.channelID = channelID;
	time.start = start;
	time.end = end;

	m_dataList.push_back(time);
}

void csvOutput::outputResult()
{
	if(m_dataList.size()){
		csvFile.open(csvFilePath.c_str());
		if(csvFile.is_open()){
			while(m_dataList.size()){
				syncTimestamp &time = m_dataList.front();
				writeCsvFile("%s, %d, %d", time.channelID, time.start, time.end);
				m_dataList.pop_front();
			}
			csvFile.close();
		}
	}
}

void csvOutput::writeCsvFile(const char* format, ...)
{    
	char csvLine[1024] = "";
	va_list args;
	va_start(args, format);
	vsprintf_s(csvLine, format, args);
	va_end(args);

	csvFile << csvLine;
}