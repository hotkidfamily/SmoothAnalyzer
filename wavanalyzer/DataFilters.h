#pragma once
#include "stdafx.h"

enum FILTERS_INDEX{
	FITLER_ABS,
	FILTER_SMOOTH,
	FILTER_UPDOWN,
};

class DataFilters
{
public:
	DataFilters(void);
	~DataFilters(void);

	int32_t filter(FILTERS_INDEX, std::string &samples, int32_t bytesPerSample);

protected:
	int32_t AbsFilter(std::string &, int32_t Bps);
	int32_t Updown2Filter(std::string &, int32_t Bps);
	int32_t SmoothFilter(std::string &, int32_t Bps);
};
