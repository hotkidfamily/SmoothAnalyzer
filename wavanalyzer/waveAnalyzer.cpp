#include "StdAfx.h"
#include "waveAnalyzer.h"
#include "log.h"

WaveAnalyzer::WaveAnalyzer(void)
: bInPulse (false)
, totalSampleCount (0)
, pulseSampleIndex (0)
, minThreshold(0x7fff)
, maxThreshold(0)
, isThresholdValid(false)
{
}

WaveAnalyzer::WaveAnalyzer(CHANNELID id, std::string &filePath)
: bInPulse (false)
, totalSampleCount (0)
, pulseSampleIndex (0)
, minThreshold(0x7fff)
, maxThreshold(0)
, isThresholdValid(false)
{
	mFilters[FILTER_UPDOWN]	 = new UDFilter(filePath + "." + chanenlIDNameList[id]);
	mFilters[FILTER_SMOOTH] = new SmoothFilter(filePath + "." + chanenlIDNameList[id]);
}

WaveAnalyzer::~WaveAnalyzer(void)
{
	delete static_cast<UDFilter*>(mFilters[FILTER_UPDOWN]);
	delete static_cast<SmoothFilter*>(mFilters[FILTER_SMOOTH]);
}

int32_t WaveAnalyzer::UpdateThreshold(std::string &channelData)
{
	int32_t sum = 0;
	int16_t *data = (int16_t *)channelData.c_str();

	for(size_t i=0; i<channelData.size()/GetBytesPerSample(); i++){
		minThreshold = min(*data, minThreshold);
		maxThreshold = max(*data, maxThreshold);
		data ++;
	}

	if(totalSampleCount/mWavFormat.nSamplerate > 10){
		isThresholdValid = true;
		inter_log(Info, "threshold is %d, min %d, max %d", GetThreshold(), minThreshold, maxThreshold);
	}

	return 0;
}

int32_t WaveAnalyzer::GetThreshold()
{
	return ((maxThreshold+minThreshold)/4);
}

void WaveAnalyzer::ReplaceValue(const int16_t *buffer, uint32_t nb_samples, bool bPulse)
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

void WaveAnalyzer::FindPulse(const int16_t *data, uint32_t nb_samples, uint32_t &start, uint32_t &end, uint32_t count)
{
	int32_t sum = 0;
	uint32_t startSamples = 0;
	uint32_t endSamples = 0;
	int32_t threshold = GetThreshold();
	uint32_t endSampleIndex = 0;

	sum = 0;
	for(uint32_t i = 0; i<nb_samples; i++){
		sum += *(data+i);
	}
	sum /= nb_samples;

	//inter_log(Pulse, "sum = %d, threshold %d", sum, threshold);

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

int32_t WaveAnalyzer::SplitDataAndFindPulse(std::string &channelData, uint32_t &start, uint32_t &end)
{
	int16_t *data = (int16_t *)channelData.c_str();
	size_t processedSamplesCount = 0;
	uint32_t nbSampleSplitStep = 0;
	uint32_t nbProcessSamples = 0;

	size_t nbTotalSamples = channelData.size()/GetBytesPerSample();

	if(nbTotalSamples > 100){
		nbSampleSplitStep = nbTotalSamples/10;
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

		FindPulse(data, nbProcessSamples, start, end, processedSamplesCount);
		if(start && end){
			return 0;
		}

		ReplaceValue(data, nbProcessSamples, bInPulse);

		data += nbProcessSamples;
		processedSamplesCount += nbProcessSamples;

	}while(processedSamplesCount < nbTotalSamples);

	return 0;
}

int32_t WaveAnalyzer::SplitDataAndFindPulse(std::string &channelData, std::list<pulseIndex> &pulses)
{
	int16_t *data = (int16_t *)channelData.c_str();
	size_t processedSamplesCount = 0;
	uint32_t nbSampleSplitStep = 0;
	uint32_t nbProcessSamples = 0;
	uint32_t start = 0, end = 0;

	size_t nbTotalSamples = channelData.size()/GetBytesPerSample();

	if(nbTotalSamples > 100){
		nbSampleSplitStep = nbTotalSamples/10;
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

		FindPulse(data, nbProcessSamples, start, end, processedSamplesCount);
		if(start && end){
			pulseIndex index(start, end);
			pulses.push_back(index);
			//return 0;
		}

		ReplaceValue(data, nbProcessSamples, bInPulse);

		data += nbProcessSamples;
		processedSamplesCount += nbProcessSamples;

	}while(processedSamplesCount < nbTotalSamples);

	return 0;
}

retType WaveAnalyzer::Analyzer(std::string &channelData, uint32_t &start, uint32_t &end)
{
	mFilters[FILTER_UPDOWN]->process(channelData, GetBytesPerSample());
	mFilters[FILTER_SMOOTH]->process(channelData, GetBytesPerSample());

	if(!IfThresholdValid()){
		UpdateThreshold(channelData);
	}else{
		SplitDataAndFindPulse(channelData, start, end);
	}

	totalSampleCount += channelData.size()/GetBytesPerSample();

	if(start && end){
		return RET_FIND_PULSE;
	}else{
		return RET_OK;
	}	
}
