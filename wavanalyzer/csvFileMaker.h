#pragma once
#include "stdafx.h"
#include "stdint.h"
#include <list>
#include <fstream>

#include <string>
#include <map>

struct syncTimestamp{
	syncTimestamp(){
		memset(this, 0, sizeof(syncTimestamp));
	}

	syncTimestamp(uint8_t channelId, double start, double end){
		this->channelID = channelId;
		this->start = start;
		this->end = end;
	}

	void reset(){
		start = 0;
		end = 0;
	}

	double start;
	double end;
	uint8_t channelID;
};

class csvOutput
{
public:
	csvOutput(void);
	~csvOutput(void);
	csvOutput(const char* filename);

	void recordTimestamp(uint8_t channelID, double start, double end);
	void outputResult();
private:
	void writeCsvLine(const char* format,  ...);
	void appendToCsvLine(uint32_t lineIndex, const char*format, ...);
	void insertCsvLine(uint32_t lineIndex, const char*format, ...);

	void outputTwoCols(std::list<syncTimestamp> &lChannel, std::list<syncTimestamp> &rChannel);

private:
	std::map<uint8_t, std::list<syncTimestamp>> channelMap;
	std::ofstream  csvFile;
	std::list<std::string> csvContentList;
	std::string csvFilePath;
};
