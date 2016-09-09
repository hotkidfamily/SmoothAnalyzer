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

	void reset(){
		start = 0;
		end = 0;
	}

	enum CHANNELID{
		LCHANNEL,
		RCHANNEL
	};

	double start;
	double end;
	CHANNELID channelID;
};

class csvOutput
{
public:
	csvOutput(void);
	~csvOutput(void);
	csvOutput(const char* filename);

	void recordTimestamp(syncTimestamp::CHANNELID channelID, double start, double end);
	void outputResult();
private:
	void writeCsvFile(const char* format,  ...);

private:
	std::list<syncTimestamp> m_dataListLChannel;
	std::list<syncTimestamp> m_dataListRChannel;
	std::ofstream  csvFile;
	std::string csvFilePath;
};
