#pragma once

enum PULSETYPE{
	PULSE_LOW,
	PULSE_HIGH,
	PULSE_NONE
};

enum CHANNELID{
	LCHANNEL,
	RCHANNEL,
	MAX_CHANNEL = RCHANNEL+1
};

struct Pulse{
	Pulse(double s, double e)
		: start(s)
		, end(e)
		, duration(e-s)
	{
		level = PULSE_LEVEL(duration);
	}

	void SetEnd(double newEnd){
		end = newEnd;
		duration = end - start;
		level = PULSE_LEVEL(duration);
	}
	void SetStart(double newStart){
		start = newStart;
		duration = end - start;
		level = PULSE_LEVEL(duration);
	}

	bool IsPulseValid(){
		return (duration >= VALID_PULSE_DURATION);
	}

	double start;
	double end;
	double duration;
	int32_t level;
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
		, channelName(id==LCHANNEL?_T('L'):_T('R'))
		, type(pulseType)
		, index(idx)
	{}

	bool Empty()
	{
		return (type == PULSE_NONE) && (channelID == MAX_CHANNEL);
	}

	int32_t index;
	PULSETYPE type;
	CHANNELID channelID;
	TCHAR channelName;
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

typedef std::list<PulseDesc> PulseList;
typedef std::vector<PulseDesc> PulseVector;
typedef std::list<FrameDesc> FrameList;
typedef std::vector<FrameDesc> FrameVector;

#define to_str(x) _T(#x)

static const TCHAR* chanenlIDNameList [] = {
	to_str(LCHANNEL),
	to_str(RCHANNEL),
};

#undef to_str