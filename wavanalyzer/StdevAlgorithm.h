#pragma once

#include "pulse.h"

class StdevAlgorithm
{
public:
	StdevAlgorithm(void);
	~StdevAlgorithm(void);

	bool CalcAvgStdAndFps(std::list<FrameDesc> &frameList, double& avg, double& stdevp, double&fps);
	double CalcFps(std::list<FrameDesc> &frameList);
	double CalcSTDEVP(std::list<FrameDesc>& durationList, const double &avg);
	double CalcAvgValue(std::list<FrameDesc>& durationList);
};
