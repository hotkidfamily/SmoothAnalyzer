#pragma once
#include "stdafx.h"
#include "stdint.h"
#include <list>
#include <fstream>
#include <string>

struct syncTimestamp{
	syncTimestamp(){
		memset(this, 0, sizeof(syncTimestamp));
	}

	int32_t start;
	int32_t end;
	int32_t channelID;
};

class csvOutput
{
public:
	csvOutput(void);
	~csvOutput(void);
	csvOutput(const char* filename);

	void recordTimestamp(int32_t channelID, int32_t start, int32_t end);
	void outputResult();
private:
	void writeCsvFile(const char* format,  ...);

private:
	std::list<syncTimestamp> m_dataList;
	std::ofstream  csvFile;
	std::string csvFilePath;
};
