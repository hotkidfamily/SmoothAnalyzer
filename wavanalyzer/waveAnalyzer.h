#pragma once
#include <string>
#include <fstream>
#include <list>
#include "stdint.h"

typedef enum tagRetType{
	RET_OK,
	RET_NEED_MORE_DATA,
	RET_FIND_START,
}retType;

class waveAnalyzer
{
public:
	waveAnalyzer(void);
	waveAnalyzer(const char *dumpFileName);
	~waveAnalyzer(void);
	retType analyze(std::string &channelData, std::list<int> &startTiming, std::list<int> &endTiming);
	void setWaveSampleRate(int samplerate);

private:
	int absFilter(std::string &channelData);
	int updateThreshold(std::string &channelData);
	int getThreshold();
	bool ifThresholdValid() const { return isThresholdValid; }
	bool findPulseStartEnd(std::string &channelData, std::list<int> &startTiming, std::list<int> &endTiming);
	inline int round(double x);

private:
	int minThreshold;
	int maxThreshold;
	bool isThresholdValid;

	int m_sampleRate;
	
	bool findNewStart;
	bool findNewEnd;
};
