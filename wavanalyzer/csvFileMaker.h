#pragma once
#include "stdafx.h"

#if 0
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
#endif

struct PulseTimestamp{
	PulseTimestamp(){
		reset();
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

	void recordTimestamp(PulseTimestamp::CHANNELID channelID, double start, double end);
	void outputResult();

#if 0
	void recordAVSyncTimestamp(syncTimestamp::CHANNELID channelID, double start, double end);
	void outputAVSyncResult();
#endif

private:
	void writeCsvLine(const char* format,  ...);
// 	void GenerateLowHighDurationList(std::list<PulseTimestamp>& startTimeList, std::list<PulseTimestamp>& EndTimeList, std::list<double>& lowDurationList, std::list<double>& highDurationList);
// 	double CacluMeanValue(std::list<double>& durationList);
// 	double CacluMSE(std::list<double>& lowDurationList, std::list<double>& highDurationList);
// 	bool ReadLChannelListInfo(double &startTime, double &endTime, double &lowDuration, double &highDuration);
// 	bool ReadRChannelListInfo(double &startTime, double &endTime, double &lowDuration, double &highDuration);

private:

#if 0
	std::list<syncTimestamp> m_dataListLChannel;
	std::list<syncTimestamp> m_dataListRChannel;
#endif

	std::list<PulseTimestamp> m_dataListLChannel;
	std::list<PulseTimestamp> m_dataListRChannel;
	
	std::ofstream  csvFile;
	std::string csvFilePath;
};
