#include "StdAfx.h"
#include "DataFilters.h"

DataFilters::DataFilters(void)
{
}

DataFilters::~DataFilters(void)
{
}


#define SMOOTH_STEP (22)
#define BYTESPERSAMPLE (2)

int32_t DataFilters::SmoothFilter(std::string &channelData, int32_t Bps)
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

	return 0;
}

int32_t DataFilters::Updown2Filter(std::string &channelData, int32_t Bps)
{
	int16_t *data = NULL;
	char *filter_data = (char*)channelData.c_str();

	data = (int16_t*)filter_data;

	for(size_t i=0; i<channelData.size()/Bps; i++){
		if(data[i] < 0){
			data[i] = 0;
		}else{
			data[i] = 30000;
		}
	}
	return 0;
}

int32_t DataFilters::AbsFilter(std::string &channelData, int32_t Bps)
{
	int16_t *data = NULL;
	char *filter_data = (char *)channelData.c_str();

	data = (int16_t *)filter_data;
	for(size_t i=0; i<channelData.size()/Bps; i++){
		*(data+i) = abs(*(data+i));
	}

	return 0;
}

int32_t DataFilters::filter(FILTERS_INDEX index, std::string &samples, int32_t bytesPerSample)
{
	int32_t ret = -1;
	switch(index){
		case FITLER_ABS:
			ret = AbsFilter(samples, bytesPerSample);
			break;
		case FILTER_SMOOTH:
			ret = SmoothFilter(samples, bytesPerSample);
			break;
		case FILTER_UPDOWN:
			ret = Updown2Filter(samples, bytesPerSample);
			break;
		default:
			break;
	}
	return ret;
}