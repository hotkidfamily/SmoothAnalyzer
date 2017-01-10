#pragma once
#include "stdafx.h"
#include "wavereader.h"
#include "wavFileParse.h"
#include "DataFilters.h"
#include "stdint.h"
#include "pulse.h"

typedef enum tagRetType{
	RET_OK,
	RET_NEED_MORE_DATA,
	RET_FIND_PULSE,
}retType;

struct SamplePos{
	SamplePos(uint32_t start, uint32_t end): startIndex(start), endIndex(end){};

	uint32_t startIndex;
	uint32_t endIndex;
};

struct 	analyzerContext{
	analyzerContext(){
		reset();
	}

	void reset(){
		bInPulse = false;
		pulseSampleIndex = 0;
		totalSampleCount = 0;
		minThreshold = 0;
		maxThreshold = 0;
		isThresholdValid = false;
	}

	bool IfThresholdValid() const { return isThresholdValid; }
	int32_t GetThreshold() { return ((maxThreshold+minThreshold)/4); };

	bool bInPulse;
	uint32_t pulseSampleIndex;
	uint32_t totalSampleCount;

	int16_t minThreshold;
	int16_t maxThreshold;
	bool isThresholdValid;
};

class WaveAnalyzer
{
public:
	WaveAnalyzer(void);
	WaveAnalyzer(STRING &filepath);
	~WaveAnalyzer(void);

	bool AnalyzeFilePulse();
	std::list<PulseDesc>* GetPulseData() { return mPulseList; };
	void SetWavFormat(WaveFormat &format) { mWavFormat = format; };

protected:
	int32_t SplitDataAndFindPulse(analyzerContext *ctx, std::string &channelData, std::list<SamplePos>&pulses);
	int32_t SplitDataAndFindPulse(analyzerContext *ctx, std::string &channelData, uint32_t &start, uint32_t &end);
	void FindPulse(analyzerContext *, const int16_t *buffer, uint32_t nb_samples, uint32_t &start, uint32_t &end, uint32_t count);

	int32_t UpdateThreshold(std::string &channelData, analyzerContext *ctx);

	retType Analyzer(analyzerContext *ctx, std::string &channelData, std::list<SamplePos> &);
	bool AnalyzeFileByChannel(CHANNELID);
	void RecordPulse(CHANNELID channelID, double start, double end);

private:
	WaveFormat mWavFormat;
	int32_t mFrameId[MAX_CHANNEL];
	IFilter *mFilters[FILTER_COUNT];
	std::list<PulseDesc> mPulseList[MAX_CHANNEL];
	STRING mAnalyzerFile;
};
