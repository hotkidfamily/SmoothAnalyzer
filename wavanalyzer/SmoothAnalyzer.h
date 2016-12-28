#pragma once

#include "stdafx.h"
#include "pulse.h"
#include "csvFileMaker.h"
#include "StdevAlgorithm.h"

enum FrameHistograms{
	FH_TOTAL,
	FH_LEVEL1,
	FH_LEVEL2,
	FH_LEVEL3,
	FH_RESOLUTION = SYSTEM_RESOLUTION,
	FH_BAD,
	FH_NORMAL,
	FH_DROP,
	FH_COUNT
};

enum syncRet{
	ALLSYNC,
	LEFTAHEAD,
	RIGHTAHEAD,
};

typedef struct tagAnalyzerParams{
	tagAnalyzerParams(){
		ZeroMemory(this, sizeof(struct tagAnalyzerParams));
	}
	double channelOffset;
	double sampleFrameRate;
	std::string mSourceFileName;
}ANALYZER_PARAMS;

class PulseAnalyzer
{
public:
	PulseAnalyzer(std::string &filename);
	~PulseAnalyzer(void);

	void SetAnalyzerData(std::list<PulseDesc>*);
	void SetWorkingParam(ANALYZER_PARAMS &params);
	void OutputResult();

protected:
	BOOL GetPulseWidthByInput(double &);
	BOOL DetectPulseWidth(double &);
	BOOL GetPulseWidth(double &);

	inline int32_t GetPulseType(PULSETYPE ltype, PULSETYPE rtype);

	void SyncChannelsAndMakeNewList(double);
	void WriteSyncDetail();
	void WriteRawSyncDetail();

	syncRet ifSync(PulseList::iterator &, PulseList::iterator &, PulseList::iterator &, PulseList::iterator &);

	void WriteRawPulseDetail();

	void GetFrameInfo(const double &);

	void JudgetDropFrame(const int32_t &, const double &);
	void HistogramInfo(const int32_t &, const double &);
	void AnalyzerSmoooth(const double &);

	void WriteSmoothDetail();

	void PulseLowFilter(std::list<PulseDesc> &);
	void PulseFilter();
	void MergeOffset();
	void MergeChannelToFrame(const double &);

	void ReportProgress(int32_t progress, int32_t total);

	inline bool IsInvalidFrameDuration(const double &, const double &);

private:
	std::list<PulseDesc>mPulseList[MAX_CHANNEL];
	std::vector<FrameDesc> mFramePulse;
	uint32_t mFrameHistograms[FH_COUNT];

	ANALYZER_PARAMS mWorkParams;

	StdevAlgorithm mStdevpAlgorithm;
};
