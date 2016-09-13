#include "StdAfx.h"
#include "waveAnalyzer.h"
#include "log.h"
#include <numeric>

waveAnalyzer::waveAnalyzer(void)
: bInPulse (false)
, totalSampleCount (0)
, pulseSampleIndex (0)
, minThreshold(0x7fff)
, maxThreshold(0)
, isThresholdValid(false)
, nbBytesPerSample(2)
{
}

waveAnalyzer::waveAnalyzer(const char *dumpFileName)
: bInPulse (false)
, totalSampleCount (0)
, pulseSampleIndex (0)
, minThreshold(0x7fff)
, maxThreshold(0)
, isThresholdValid(false)
, nbBytesPerSample(2)
{
	std::string file = dumpFileName;
	file.insert(0, "c:/");
	file.insert(file.size(), "abs.pcm");
	dumpfilter.open(file.c_str(), std::ios::binary);

	file.clear();
	file = dumpFileName;
	file.insert(0, "c:/");
	file.insert(file.size(), "2value.pcm");
	dump2Value.open(file.c_str(), std::ios::binary);
}

waveAnalyzer::~waveAnalyzer(void)
{
	if(dumpfilter.is_open())
		dumpfilter.close();

	if(dump2Value.is_open())
		dump2Value.close();
}

int32_t waveAnalyzer::absFilter(std::string &channelData)
{
	int16_t *data = NULL;
	char *filter_data = (char *)channelData.c_str();

	data = (int16_t *)filter_data;
	for(size_t i=0; i<channelData.size()/getBytesPerSample(); i++){
		*(data+i) = abs(*(data+i));
	}

	return 0;
}

int32_t waveAnalyzer::updateThreshold(std::string &channelData)
{
	int32_t sum = 0;
	int16_t *data = (int16_t *)channelData.c_str();

	for(size_t i=0; i<channelData.size()/getBytesPerSample(); i++){
		minThreshold = min(*data, minThreshold);
		maxThreshold = max(*data, maxThreshold);
		data ++;
	}

	if(maxThreshold - minThreshold > 10000){
		isThresholdValid = true;
		inter_log(Debug, "threshold is %d, min %d, max %d at sample %f", getThreshold(), minThreshold, maxThreshold, (totalSampleCount+channelData.size()/getBytesPerSample())*1.0/44100);
	}

	return 0;
}

int32_t waveAnalyzer::getThreshold()
{
	return ((maxThreshold+minThreshold)/4);
}

int32_t waveAnalyzer::findPulse(std::string &channelData, uint32_t &start, uint32_t &end)
{
	int16_t *data = (int16_t *)channelData.c_str();
	int32_t sum = 0;
	size_t count = 0;
	int16_t setValue = 0;
	uint32_t endSampleIndex = 0;
	int32_t threshold = getThreshold();
	int32_t nbFilterWorkSamples = 0;

	nbFilterWorkSamples = channelData.size()/getBytesPerSample()/10; // 10ms

	while(count < channelData.size()){
		sum = 0;
		for(int32_t i = 0; i<nbFilterWorkSamples; i++){
			sum += *(data+i);
		}
		sum /= nbFilterWorkSamples;

		inter_log(Pulse, "sum = %d, threshold %d", sum, threshold);

		if(sum > threshold){
			if(!bInPulse){
				bInPulse = true;
				pulseSampleIndex = totalSampleCount + count/getBytesPerSample();
			}
		}else{
			if(bInPulse){
				bInPulse = false;
				endSampleIndex = totalSampleCount + count/getBytesPerSample();
				start = pulseSampleIndex;
				end = endSampleIndex;
			}
		}

		if(bInPulse){
			setValue = 30000;
		}else{
			setValue = 0;
		}

		for(int32_t i = 0; i<nbFilterWorkSamples; i++){
			*(data+i) = setValue;
		}

		data += nbFilterWorkSamples;
		count += nbFilterWorkSamples*getBytesPerSample();
	}
		
	return 0;
}

retType waveAnalyzer::analyzer(std::string &channelData, uint32_t &start, uint32_t &end)
{
	absFilter(channelData);

	if(dumpfilter.is_open())
		dumpfilter.write(channelData.c_str(), channelData.size());

	if(!ifThresholdValid()){
		updateThreshold(channelData);
	}else{
		findPulse(channelData, start, end);
	}

	if(dump2Value.is_open())
		dump2Value.write(channelData.c_str(), channelData.size());

	totalSampleCount += channelData.size()/getBytesPerSample();

	if(start && end){
		return RET_FIND_START;
	}else{
		return RET_OK;
	}	
}