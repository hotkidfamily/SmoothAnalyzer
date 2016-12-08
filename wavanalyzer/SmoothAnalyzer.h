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

	/* common */
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

	/* pulse desc */
	PULSETYPE type;
	CHANNELID channelID;
};

struct FrameDesc: public Pulse
{
	FrameDesc(int32_t type, double start, double end)
		: Pulse(start, end)
		, frameType(type)
	{}

	/* frame desc */
	int32_t frameType;
};

class PulseAnalyzer
{
public:
	PulseAnalyzer(std::string &filename);
	~PulseAnalyzer(void);

	void RecordTimestamp(CHANNELID channelID, double start, double end);
	void OutputResult();

protected:
	double CacluAvgValue(std::list<PulseDesc>& durationList);
	double CacluMSE(std::list<PulseDesc>& lowDurationList);
	BOOL DetectPulseWidth(double &duration);
	inline int32_t GetPulseType(PULSETYPE ltype, PULSETYPE rtype);
	void WriteSyncDetail();
	void GetFrameInfo();

private:
	std::list<PulseDesc> mPulseList[MAX_CHANNEL];
	std::list<PulseDesc> mFramePulse;
	std::string mSourceFileName;
};
