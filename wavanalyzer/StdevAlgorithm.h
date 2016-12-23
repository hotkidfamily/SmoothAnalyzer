#pragma once

#include "pulse.h"

class StdevAlgorithm
{
public:
	StdevAlgorithm(void);
	~StdevAlgorithm(void);

	bool CalcAvgStdAndFps(std::vector<FrameDesc> &frameList, double& avg, double& stdevp, double&fps);
	double CalcFps(std::vector<FrameDesc> &frameList);
	double CalcSTDEVP(std::vector<FrameDesc>& durationList, const double &avg);
	double CalcAvgValue(std::vector<FrameDesc>& durationList);
};
