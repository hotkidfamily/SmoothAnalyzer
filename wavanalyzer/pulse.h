#pragma once

enum PULSETYPE{
	PULSE_LOW,
	PULSE_HIGH,
	PULSE_NONE
};

struct Pulse{
	Pulse(double s, double e)
		: start(s)
		, end(e)
		, duration((e-s)*1000)
	{
		level = PULSE_LEVEL(duration);
	}

	bool IsPulseInvalid(){
		return (level != 0);
	}

	bool IsLevelInvalid(){
		return ((level <= 0) || (level>SYSTEM_RESOLUTION));
	}

	double start;
	double end;
	double duration;
	int32_t level;
};

enum CHANNELID{
	LCHANNEL,
	RCHANNEL,
	MAX_CHANNEL = 0xFF
};

#define to_str(x) #x

static const char* chanenlIDNameList [] = {
	to_str(LCHANNEL),
	to_str(RCHANNEL),
};

#undef to_str