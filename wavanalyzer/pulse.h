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
		, duration(e-s)
	{}

	double start;
	double end;
	double duration;
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