#pragma once
#include <string>
#include <fstream>
#include "wavereader.h"
#include "stdint.h"

typedef enum tagRetType{
	RET_OK,
	RET_NEED_MORE_DATA,
	RET_FIND_PULSE,
}retType;

class waveAnalyzer
{
public:
	waveAnalyzer(void);
	waveAnalyzer(const char *dumpFileName);
	~waveAnalyzer(void);

	retType analyzer(std::string &channelData, uint32_t &start, uint32_t &end);
	void setWavFormat(WaveFormat format) { mWavFormat = format; };

private:
	int32_t absFilter(std::string &channelData);
	int32_t getBytesPerSample() { return mWavFormat.nBitsPerSample >> 3; };

	int32_t splitDataAndFindPulse(std::string &channelData, uint32_t &start, uint32_t &end);
	void findPulse(const int16_t *buffer, uint32_t nb_samples, uint32_t &start, uint32_t &end, uint32_t count);
	void replaceValue(const int16_t *buffer, uint32_t nb_samples, bool bInPulse);

	int32_t updateThreshold(std::string &channelData);
	int32_t getThreshold();
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

	WaveFormat mWavFormat;
};
