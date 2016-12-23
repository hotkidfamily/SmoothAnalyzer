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

class PulseAnalyzer
{
public:
	PulseAnalyzer(std::string &filename);
	~PulseAnalyzer(void);

	void SetAnalyzerData(std::list<PulseDesc>*);
	void SetOffset(double offset) { mChannelOffset = offset; };
	void OutputResult();

protected:
	BOOL DetectPulseWidth(double &);
	inline int32_t GetPulseType(PULSETYPE ltype, PULSETYPE rtype);

	void ProcessSyncDetail(double);
	void WriteSyncDetail();
	void WriteRawSyncDetail();

	void WriteRawPulseDetail();

	void GetFrameInfoByChannel(const double &);

	void JudgetDropFrame(const int32_t &);
	void HistogramInfo(const int32_t &, const double &);
	void AnalyzerSmoooth(const double &);
	void WriteSmoothDetail();

	void PulseLowFilter(std::list<PulseDesc> &);
	void PulseFilter();
	void MergeOffset();

	void ReportProgress(int32_t progress, int32_t total);

private:
	std::list<PulseDesc>mPulseList[MAX_CHANNEL];
	std::vector<FrameDesc> mFramePulse;
	uint32_t mFrameHistograms[FH_COUNT];
	std::string mSourceFileName;

	double mChannelOffset;

	StdevAlgorithm mStdevpAlgorithm;
};
