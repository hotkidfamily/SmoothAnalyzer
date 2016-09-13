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

void waveAnalyzer::replaceValue(const int16_t *buffer, uint32_t nb_samples, bool bPulse)
{
	int16_t *data = (int16_t*)buffer;
	int16_t value = 0;
	if(bPulse){
		value = 30000;
	}else{
		value = 0;
	}

	for(uint32_t i = 0; i<nb_samples; i++){
		*(data+i) = value;
	}
}

void waveAnalyzer::findPulse(const int16_t *data, uint32_t nb_samples, uint32_t &start, uint32_t &end, uint32_t count)
{
	int32_t sum = 0;
	uint32_t startSamples = 0;
	uint32_t endSamples = 0;
	int32_t threshold = getThreshold();
	uint32_t endSampleIndex = 0;

	sum = 0;
	for(uint32_t i = 0; i<nb_samples; i++){
		sum += *(data+i);
	}
	sum /= nb_samples;

	inter_log(Pulse, "sum = %d, threshold %d", sum, threshold);

	if(sum > threshold){
		for(uint32_t i = 0; i<nb_samples; i++){
			if(*(data+i) > threshold){
				startSamples = i;
				break;
			}
		}
		if(!bInPulse){
			bInPulse = true;
			pulseSampleIndex = totalSampleCount + count + startSamples;
		}
	}else{
		if(bInPulse){
			for(uint32_t i = 0; i<nb_samples; i++){
				if(*(data+i) < threshold){
					endSamples = i;
					break;
				}
			}

			bInPulse = false;
			endSampleIndex = totalSampleCount + count + endSamples;
			start = pulseSampleIndex;
			end = endSampleIndex;
		}
	}
}

int32_t waveAnalyzer::splitDataAndFindPulse(std::string &channelData, uint32_t &start, uint32_t &end)
{
	int16_t *data = (int16_t *)channelData.c_str();
	size_t processedSamplesCount = 0;
	uint32_t nbSampleSplitStep = 0;
	uint32_t nbProcessSamples = 0;
	
	size_t nbTotalSamples = channelData.size()/getBytesPerSample();

	if(nbTotalSamples > 100){
		nbSampleSplitStep = nbTotalSamples/100;
	}else{
		nbSampleSplitStep = nbTotalSamples;
	}
	
	inter_log(Pulse, "analyzer size is %d", nbSampleSplitStep);

	do{
		if(nbSampleSplitStep + processedSamplesCount > nbTotalSamples){
			nbProcessSamples = nbTotalSamples - processedSamplesCount;
		}else{
			nbProcessSamples = nbSampleSplitStep;
		}

		findPulse(data, nbProcessSamples, start, end, processedSamplesCount);

		replaceValue(data, nbProcessSamples, bInPulse);

		data += nbProcessSamples;
		processedSamplesCount += nbProcessSamples;

	}while(processedSamplesCount < nbTotalSamples);
		
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
		splitDataAndFindPulse(channelData, start, end);
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