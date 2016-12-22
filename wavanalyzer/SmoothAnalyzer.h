#pragma once

#include "stdafx.h"
#include "pulse.h"
#include "csvFileMaker.h"

struct PulseDesc: public Pulse
{
	PulseDesc()
		:Pulse(0.0f, 0.0f)
		, channelID(MAX_CHANNEL)
		, type(PULSE_NONE)
		, index(0)
	{
		
	}

	PulseDesc(CHANNELID id, double start, double end, PULSETYPE pulseType, int32_t idx)
		: Pulse(start, end)
		, channelID(id)
		, channelName(id==LCHANNEL?'L':'R')
		, type(pulseType)
		, index(idx)
	{}

	bool IsInvalid()
	{
		return (type == PULSE_NONE) && (channelID == MAX_CHANNEL);
	}

	int32_t index;
	PULSETYPE type;
	CHANNELID channelID;
	char channelName;
};

struct FrameDesc: public Pulse
{
	FrameDesc(int32_t type, double start, double end, double fps, double avg, double stdevp, int32_t idx)
		: Pulse(start, end)
		, frameType(type)
		, frameRate(fps)
		, STDEVP(stdevp)
		, AVG(avg)
		, offset(duration - avg)
		, index(idx)
	{}

	int32_t index;
	int32_t frameType;
	double frameRate;
	double STDEVP;
	double AVG;
	double offset;
};

class PulseAnalyzer
{
public:
	PulseAnalyzer(std::string &filename);
	~PulseAnalyzer(void);

	void RecordPulse(CHANNELID channelID, double start, double end);
	void OutputResult();

protected:
	double CalcAvgValue(std::list<FrameDesc>& );
	double CalcSTDEVP(std::list<FrameDesc>&, const double&);
	double CalcFps(std::list<FrameDesc> &);

	bool CalcAvgStdAndFps(std::list<FrameDesc> &frameList, double&, double&, double&);

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

	void ReportProgress(int32_t progress, int32_t total);

private:
	std::list<PulseDesc> mPulseList[MAX_CHANNEL];
	std::list<FrameDesc> mFramePulse;
	std::list<FrameDesc> mFrameResult;
	uint32_t mFrameHistograms[SYSTEM_RESOLUTION+2];
	std::string mSourceFileName;
	int32_t mFrameId[MAX_CHANNEL];
};
