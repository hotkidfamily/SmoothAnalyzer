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

	void RecordTimestamp(CHANNELID channelID, double start, double end);
	void OutputResult();

protected:
	double CacluAvgValue(std::list<FrameDesc>& );
	double CacluSTDEVP(std::list<FrameDesc>&, double&);
	double CacluSTDEVPInOneSecond(std::list<FrameDesc>&, double &);
	double CacluFps(std::list<FrameDesc> &);
	double CacluFrameRate(std::list<FrameDesc> &);
	BOOL DetectPulseWidth(double &);
	inline int32_t GetPulseType(PULSETYPE ltype, PULSETYPE rtype);
	void WriteSyncDetail();
	void WriteRawPulseDetail();
	void GetFrameInfo(double &);
	void WriteSmoothDetail();

	void ReportProgress(int32_t progress, int32_t total);

private:
	std::list<PulseDesc> mPulseList[MAX_CHANNEL];
	std::list<FrameDesc> mFramePulse;
	std::string mSourceFileName;
	int32_t mFrameId[MAX_CHANNEL];
};
