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
	void setBytesPerSample(uint32_t bytesPerSample) { nbBytesPerSample = bytesPerSample; }
	uint32_t getBytesPerSample(void) const { return nbBytesPerSample; }
	void setSampleRate(uint32_t sampleRate);
	uint32_t getSampleRate() const {return nbSampleRate;}

private:
	int32_t absFilter(std::string &channelData);

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
	uint32_t nbBytesPerSample;
	uint32_t nbSampleRate;
};
