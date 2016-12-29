#pragma once

#include "stdafx.h"
#include "pulse.h"
#include "libxllibrary.h"
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
	ENDSYNC,
	LEFTAHEAD,
	RIGHTAHEAD,
};

enum fixRet{
	ALLGO,
	LEFTGO,
	RIGHTGO,
};

typedef struct tagAnalyzerParams{
	tagAnalyzerParams(){
		ZeroMemory(this, sizeof(struct tagAnalyzerParams));
	}
	double channelOffset;
	double sampleFrameRate;
	STRING mSourceFileName;
}ANALYZER_PARAMS;

class PulseAnalyzer
{
public:
	PulseAnalyzer(STRING &filename);
	~PulseAnalyzer(void);

	void SetAnalyzerData(std::list<PulseDesc>*);
	void SetWorkingParam(ANALYZER_PARAMS &params);
	void OutputResult();

protected:
	BOOL GetPulseWidthByInput(double &);
	BOOL DetectPulseWidth(double &);
	BOOL GetPulseWidth(double &);

	inline int32_t GetPulseType(PULSETYPE ltype, PULSETYPE rtype);

	void CreateFrameInfo(double);
	void WriteSyncDetail();
	void WriteRawSyncDetail();

	inline bool IsPosSync(const double &diff);
	syncRet ifStartSync(PulseList::iterator &, PulseList::iterator &, PulseList::iterator &, PulseList::iterator &);
	fixRet ifFix(PulseList::iterator &, PulseList::iterator &, Pulse &, const double &);
	int32_t ifNeedSplitPulse(PulseDesc *, PulseDesc *);

	void WriteRawPulseDetail();

	void GetFrameInfo(const double &);

	void JudgetDropFrame(const int32_t &, const double &);
	void HistogramInfo(const int32_t &, const double &);
	void AnalyzerSmoooth(const double &);

	void WriteSmoothDetail();
	void WriteFrameDetail();

	void PulseLowFilter(std::list<PulseDesc> &);
	void PulseFilter();
	void MergeOffset();

	inline bool IsOneFrame(const double &, const double &);

private:
	std::list<PulseDesc>mPulseList[MAX_CHANNEL];
	std::vector<FrameDesc> mFramePulse;
	uint32_t mFrameHistograms[FH_COUNT];

	ANALYZER_PARAMS mWorkParams;

	StdevAlgorithm mStdevpAlgorithm;


	xlsOperator *xlsMachine;
};
