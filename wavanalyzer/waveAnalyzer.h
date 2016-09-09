#pragma once
#include <string>
#include <fstream>

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

	retType analyzer(std::string &channelData, uint32_t &start, uint32_t &end);

private:
	int absFilter(std::string &channelData);
	int findPulse(std::string &channelData, uint32_t &start, uint32_t &end);
	int updateThreshold(std::string &channelData);
	int getThreshold();
	bool ifThresholdValid() const { return isThresholdValid; }

private:
	std::ofstream dumpfilter;
	std::ofstream dump2Value;
	bool bInPulse;
	uint32_t pulseSampleIndex;
	uint32_t totalSampleCount;
	int16_t minThreshold;
	int16_t maxThreshold;
	bool isThresholdValid;
};
