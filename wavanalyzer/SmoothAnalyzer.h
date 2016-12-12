#pragma once

#include "stdafx.h"
#include "csvFileMaker.h"

enum CHANNELID{
	LCHANNEL,
	RCHANNEL,
	MAX_CHANNEL = 0xFF
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
};

struct FrameDesc: public Pulse
{
	FrameDesc(int32_t type, double start, double end, double fps, double stdmse, int32_t idx)
		: Pulse(start, end)
		, frameType(type)
		, frameRate(fps)
		, MSE(stdmse)
		, index(idx)
	{}

	int32_t index;
	int32_t frameType;
	double frameRate;
	double MSE;
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
	double CacluMSE(std::list<FrameDesc>& );
	double CacluMSEInOneSecond(std::list<FrameDesc>& );
	double CacluFps(std::list<FrameDesc> &);
	double CacluFrameRate(std::list<FrameDesc> &);
	BOOL DetectPulseWidth(double &);
	inline int32_t GetPulseType(PULSETYPE ltype, PULSETYPE rtype);
	void WriteSyncDetail();
	void WriteRawPulseDetail();
	void GetFrameInfo();
	void WriteSmoothDetail();

private:
	std::list<PulseDesc> mPulseList[MAX_CHANNEL];
	std::list<FrameDesc> mFramePulse;
	std::string mSourceFileName;
	int32_t mFrameId[MAX_CHANNEL];
};
