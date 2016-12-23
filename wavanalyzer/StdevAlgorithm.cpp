#include "StdAfx.h"
#include "StdevAlgorithm.h"

StdevAlgorithm::StdevAlgorithm(void)
{
}

StdevAlgorithm::~StdevAlgorithm(void)
{
}


double StdevAlgorithm::CalcAvgValue(std::vector<FrameDesc>& durationList)
{
	double sum = 0.0f;
	double avgValue = 0.0f;
	std::vector<FrameDesc>::iterator it;

	if (!durationList.empty()){
		for (it = durationList.begin(); it != durationList.end(); it++){
			sum += it->duration;
		}

		avgValue = sum / durationList.size();
	}

	return avgValue;
}

double StdevAlgorithm::CalcSTDEVP(std::vector<FrameDesc>& durationList, const double &avg)
{
	double Sum = 0.0f;
	double SD = 0.0f;
	std::vector<FrameDesc>::iterator it;

	if (!durationList.empty()){
		for(it = durationList.begin(); it != durationList.end(); it++){
			Sum += pow(fabs(it->duration - avg), 2);
		}

		SD = 100.0 * sqrt(Sum / durationList.size()) / avg;
	}
	return SD;
}

double StdevAlgorithm::CalcFps(std::vector<FrameDesc> &frameList)
{
	double fps = 0.0f;
	double duraionInSecond = 0.0;

	if (!frameList.empty()){
		duraionInSecond = frameList.back().end - frameList.front().start;
		fps = frameList.size() / duraionInSecond;
	}

	return fps;
}

bool StdevAlgorithm::CalcAvgStdAndFps(std::vector<FrameDesc> &frameList, double& avg, double& stdevp, double&fps)
{
	std::vector<FrameDesc>::reverse_iterator rit;
	avg = stdevp = fps = 0.0f;

	for (rit = frameList.rbegin(); rit != frameList.rend(); rit++) {
		if ((frameList.back().end - rit->start) > 1.0) {
			std::vector<FrameDesc> frameListSplit;

			rit++;
			frameListSplit.assign(rit.base(), frameList.end());
			avg = CalcAvgValue(frameListSplit);
			stdevp = CalcSTDEVP(frameListSplit, avg);
			fps = CalcFps(frameListSplit);

			break;
		}
	}

	return true;
}