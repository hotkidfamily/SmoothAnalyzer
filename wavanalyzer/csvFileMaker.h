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

struct Timestamp{
	Timestamp(){
		memset(this, 0, sizeof(Timestamp));
	}
	void reset(){
		time = 0;
	}

	enum CHANNELID{
		LCHANNEL,
		RCHANNEL
	};

	double time;
	CHANNELID channelID;
};

class csvOutput
{
public:
	csvOutput(void);
	~csvOutput(void);
	csvOutput(const char* filename);

	void recordTimestamp(Timestamp::CHANNELID channelID, std::list<int>& start, std::list<int>& end, int baseTime);
	void outputResult();

	void recordAVSyncTimestamp(syncTimestamp::CHANNELID channelID, double start, double end);
	void outputAVSyncResult();
private:
	void writeCsvFile(const char* format,  ...);
	void GenerateLowHighDurationList(std::list<Timestamp>& startTimeList, std::list<Timestamp>& EndTimeList, std::list<double>& lowDurationList, std::list<double>& highDurationList);
	double CacluMeanValue(std::list<double>& durationList);
	double CacluMSE(std::list<double>& lowDurationList, std::list<double>& highDurationList);
	bool ReadLChannelListInfo(double &startTime, double &endTime, double &lowDuration, double &highDuration);
	bool ReadRChannelListInfo(double &startTime, double &endTime, double &lowDuration, double &highDuration);

private:
	std::list<syncTimestamp> m_dataListLChannel;
	std::list<syncTimestamp> m_dataListRChannel;
	
	std::list<Timestamp> m_startTimeListLChannel;
	std::list<Timestamp> m_endTimeListLChannel;
	std::list<Timestamp> m_startTimeListRChannel;
	std::list<Timestamp> m_endTimeListRChannel;

	std::list<double> m_lowDurationListLChannel;
	std::list<double> m_highDurationListLChannel;
	std::list<double> m_lowDurationListRChannel;
	std::list<double> m_highDurationListRChannel;

	std::ofstream  csvFile;
	std::string csvFilePath;
};
