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