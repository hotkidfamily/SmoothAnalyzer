#pragma once

#include "stdafx.h"
#include "csvFileMaker.h"

enum CHANNELID{
	LCHANNEL,
	RCHANNEL,
	MAX_CHANNEL // support channel count
};

enum PULSETYPE{
	PULSE_LOW,
	PULSE_HIGH,
	PULSE_NONE
};

struct Pulse{
	Pulse(double s, double e)
		: start(s)
		, end(e)
		, duration(e-s)
	{}

	double start;
	double end;
	double duration;
};

struct PulseDesc: public Pulse
{
	PulseDesc(CHANNELID id, double start, double end, PULSETYPE pulseType)
		: Pulse(start, end)
		, channelID(id)
		, type(pulseType)
	{
		this->channelID = id;
		this->type = type;
	}

	PULSETYPE type;
	CHANNELID channelID;
};

struct FrameDesc: public Pulse
{
	FrameDesc(int32_t type, double start, double end, double fps)
		: Pulse(start, end)
		, frameType(type)
		, frameRate(fps)
	{}

	int32_t frameType;
	double frameRate;
};

class PulseAnalyzer
{
public:
	PulseAnalyzer(std::string &filename);
	~PulseAnalyzer(void);

	void RecordTimestamp(CHANNELID channelID, double start, double end);
	void OutputResult();

protected:
	double CacluAvgValue(std::list<FrameDesc>& durationList);
	double CacluMSE(std::list<FrameDesc>& lowDurationList);
	double CacluAvgFps(std::list<FrameDesc> &durationList);
	double CacluFps(std::list<FrameDesc> &durationList);
	BOOL DetectPulseWidth(double &duration);
	inline int32_t GetPulseType(PULSETYPE ltype, PULSETYPE rtype);
	void WriteSyncDetail();
	void GetFrameInfo();

private:
	std::list<PulseDesc> mPulseList[MAX_CHANNEL];
	std::list<FrameDesc> mFramePulse;
	std::string mSourceFileName;
};
