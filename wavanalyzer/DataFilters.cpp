#include "StdAfx.h"
#include "DataFilters.h"

#define SMOOTH_STEP (22)

SmoothFilter::SmoothFilter(std::string &filePath)
	: iDump(filePath + ".smooth.pcm")
{
}

int32_t SmoothFilter::process(std::string &channelData, int32_t Bps)
{
	int16_t *data = (int16_t*)channelData.c_str();
	int32_t maxPoint = 0;
	bool bInPulse = false;

	const uint8_t dataFilter[SMOOTH_STEP] = {0};

	maxPoint = channelData.size()/Bps - SMOOTH_STEP;

	for(int32_t i=0; i < maxPoint; i++){
		if(data[i] == 0){
			if(memcmp(dataFilter, &data[i], SMOOTH_STEP)){
				data[i] = 30000;
			}
		}else{
			bInPulse = false;
		}
	}

	if(bInPulse){
		for(int32_t i=maxPoint; i<channelData.size()/Bps; i++){
			data[i] = 30000;
		}
	}

	DumpData(channelData);

	return 0;
}

RmNegativeFilter::RmNegativeFilter(std::string &filePath)
	:iDump(filePath + ".RemoveNegative.pcm")
{
}

int32_t RmNegativeFilter::process(std::string &channelData, int32_t Bps)
{
	int16_t *data = (int16_t*)channelData.c_str();

	for(size_t i=0; i<channelData.size()/Bps; i++){
		if(data[i] < 10000){
			data[i] = 0;
		}else{
			data[i] = 30000;
		}
	}

	DumpData(channelData);

	return 0;
}


ABSFilter::ABSFilter(std::string &filePath)
		:iDump(filePath + ".abs.pcm")
{
}

int32_t ABSFilter::process(std::string &channelData, int32_t Bps)
{
	int16_t *data = (int16_t*)channelData.c_str();

	for(size_t i=0; i<channelData.size()/Bps; i++){
		*(data+i) = abs(*(data+i));
	}

	DumpData(channelData);

	return 0;
}
