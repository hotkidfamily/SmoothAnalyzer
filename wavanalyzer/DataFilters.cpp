#include "StdAfx.h"
#include "DataFilters.h"

#define SMOOTH_STEP (22)

SmoothFilter::SmoothFilter(std::string &filePath)
	: iDump(filePath + ".smooth.pcm")
{
}

int32_t SmoothFilter::process(std::string &channelData, int32_t Bps)
{
	int16_t *data = NULL;
	char *filter_data = (char*)channelData.c_str();
	int32_t maxPoint = 0;
	int32_t sum = 0;
	int32_t i = 0;

	data = (int16_t*)filter_data;
	maxPoint = channelData.size()/Bps - SMOOTH_STEP;

	for(i=0; i < maxPoint; i++){
		if(data[i] == 0){
			if(data[i + SMOOTH_STEP] != 0){
				data[i] = 30000;
			}
		}
	}

	DumpData(channelData);

	return 0;
}

UDFilter::UDFilter(std::string &filePath)
	:iDump(filePath + ".RemoveNegative.pcm")
{
}

int32_t UDFilter::process(std::string &channelData, int32_t Bps)
{
	int16_t *data = NULL;
	char *filter_data = (char*)channelData.c_str();

	data = (int16_t*)filter_data;

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
	int16_t *data = NULL;
	char *filter_data = (char *)channelData.c_str();

	data = (int16_t *)filter_data;
	for(size_t i=0; i<channelData.size()/Bps; i++){
		*(data+i) = abs(*(data+i));
	}

	DumpData(channelData);

	return 0;
}
