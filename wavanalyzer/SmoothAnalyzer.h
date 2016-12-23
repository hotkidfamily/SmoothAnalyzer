#pragma once

#include "stdafx.h"
#include "pulse.h"
#include "csvFileMaker.h"
#include "StdevAlgorithm.h"

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

	void WriteRawPulseDetail();

	void GetFrameInfoByChannel(const double &);

	void JudgetDropFrame();
	void HistogramInfo(const double &);
	void AnalyzerSmoooth(const double &);
	void WriteSmoothDetail();

	void PulseLowFilter(std::list<PulseDesc> &);
	void PulseFilter();
	void MergeOffset();

	void ReportProgress(int32_t progress, int32_t total);

private:
	std::list<PulseDesc>mPulseList[MAX_CHANNEL];
	std::list<FrameDesc> mFramePulse;
	std::list<FrameDesc> mFrameResult;
	uint32_t mFrameHistograms[SYSTEM_RESOLUTION+2];
	std::string mSourceFileName;

	double mChannelOffset;

	StdevAlgorithm mStdevpAlgorithm;
};
