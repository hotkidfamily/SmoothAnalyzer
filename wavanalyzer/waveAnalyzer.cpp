#include "StdAfx.h"
#include "waveAnalyzer.h"
#include "log.h"
#include <numeric>

waveAnalyzer::waveAnalyzer(void)
: bInPulse (false)
, totalSampleCount (0)
, pulseSampleIndex (0)
{
	dumpfilter.open("c:/filter.pcm", std::ios::binary);
	dump2Value.open("c:/2value.pcm", std::ios::binary);
}

waveAnalyzer::waveAnalyzer(const char *dumpFileName)
: bInPulse (false)
, totalSampleCount (0)
, pulseSampleIndex (0)
{
	uint32_t first_size = 0;
	std::string file = dumpFileName;
	file.insert(0, "c:/");
	first_size = file.size();
	file.insert(file.size(), "abs.pcm");
	dumpfilter.open(file.c_str(), std::ios::binary);

	file.resize(0,0);
	file = dumpFileName;
	file.insert(0, "c:/");
	file.insert(file.size(), "2value.pcm");
	dump2Value.open(file.c_str(), std::ios::binary);
}

waveAnalyzer::~waveAnalyzer(void)
{
	dumpfilter.close();
	dump2Value.close();
}

int waveAnalyzer::absFilter(std::string &channelData)
{
	int16_t *data = NULL;
	char *filter_data = (char *)channelData.c_str();

	data = (int16_t *)filter_data;
	for(size_t i=0; i<channelData.size()/2; i++){
		*(data+i) = abs(*(data+i));
	}

	return 0;
}

int waveAnalyzer::findPulse(std::string &channelData, uint32_t &start, uint32_t &end)
{
	int16_t *data = NULL;
	int sum = 0;
	size_t count = 0;
	int16_t setValue = 0;
	uint32_t endSampleIndex = 0;

#define filter_samples (441) // analyzer 10ms one time

	data = (int16_t *)channelData.c_str();
	while(count < channelData.size()){
		sum = 0;
		for(int i = 0; i<filter_samples; i++){
			sum += *(data+i);
		}
		sum /= filter_samples;

		if(sum > 10000){
			if(!bInPulse){
				bInPulse = true;
				pulseSampleIndex = totalSampleCount + count/2;
			}
		}else{
			if(bInPulse){
				bInPulse = false;
				endSampleIndex = totalSampleCount + count/2;
				inter_log(Pulse, "pulse start %d, end %d, length %fms", pulseSampleIndex, endSampleIndex, (endSampleIndex - pulseSampleIndex)*1.0 / 44100);
				start = pulseSampleIndex;
				end = endSampleIndex;
			}
		}

		if(bInPulse){
			setValue = 30000;
		}else{
			setValue = 0;
		}

		for(int i = 0; i<filter_samples; i++){
			*(data+i) = setValue;
		}

		data += filter_samples;
		count += filter_samples*2;
	}

	totalSampleCount += channelData.size()/2;
		
	return 0;
}

retType waveAnalyzer::analyzer(std::string &channelData, uint32_t &start, uint32_t &end)
{
	absFilter(channelData);
	dumpfilter.write(channelData.c_str(), channelData.size());
	
	findPulse(channelData, start, end);
	dump2Value.write(channelData.c_str(), channelData.size());

	if(start && end){
		return RET_FIND_START;
	}
	
	return RET_OK;
}