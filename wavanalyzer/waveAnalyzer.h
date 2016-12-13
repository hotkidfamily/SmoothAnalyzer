#pragma once
#include "stdafx.h"
#include "wavereader.h"
#include "stdint.h"

typedef enum tagRetType{
	RET_OK,
	RET_NEED_MORE_DATA,
	RET_FIND_PULSE,
}retType;

struct pulseIndex{
	pulseIndex(uint32_t start, uint32_t end): startIndex(start), endIndex(end){};

	uint32_t startIndex;
	uint32_t endIndex;
};

class WaveAnalyzer
{
public:
	WaveAnalyzer(void);
	WaveAnalyzer(const char *dumpFileName);
	~WaveAnalyzer(void);

	retType Analyzer(std::string &channelData, uint32_t &start, uint32_t &end);
	void SetWavFormat(WaveFormat format) { mWavFormat = format; };

private:
	int32_t AbsFilter(std::string &channelData);
	int32_t Updown2Filter(std::string &channelData);
	int32_t SmoothFilter(std::string &channelData);

	int32_t SplitDataAndFindPulse(std::string &channelData, std::list<pulseIndex>&pulses);
	int32_t SplitDataAndFindPulse(std::string &channelData, uint32_t &start, uint32_t &end);
	void FindPulse(const int16_t *buffer, uint32_t nb_samples, uint32_t &start, uint32_t &end, uint32_t count);
	void ReplaceValue(const int16_t *buffer, uint32_t nb_samples, bool bInPulse);

	int32_t UpdateThreshold(std::string &channelData);
	int32_t GetThreshold();
	bool IfThresholdValid() const { return isThresholdValid; }

	inline int32_t GetBytesPerSample() { return mWavFormat.nBitsPerSample >> 3; };

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
