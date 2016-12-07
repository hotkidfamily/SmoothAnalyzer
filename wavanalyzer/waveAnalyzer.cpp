#include "StdAfx.h"
#include "waveAnalyzer.h"
#include "log.h"
#include <numeric>

waveAnalyzer::waveAnalyzer(void)
: minThreshold(0)
, maxThreshold(0)
, isThresholdValid(false)
, findNewStart(false)
, findNewEnd(true)
{

}

waveAnalyzer::waveAnalyzer(const char *dumpFileName)
: minThreshold(0)
, maxThreshold(0)
, isThresholdValid(false)
, findNewStart(false)
, findNewEnd(true)
{

}

waveAnalyzer::~waveAnalyzer(void)
{

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

int waveAnalyzer::updateThreshold(std::string &channelData)
{
	int16_t *data = (int16_t *)channelData.c_str();
	uint16_t minValue = 0;
	uint16_t maxValue = 0;

	for(size_t i = 0; i<channelData.size()/2; i++){
		minValue = min(abs(*data), minValue);
		maxValue = max(abs(*data), maxValue);
		data ++;
	}

	if(maxValue - minValue > 10000){
		isThresholdValid = true;
		maxThreshold = maxValue;
		minThreshold = minValue;
		inter_log(Debug, "threshold is %d, min %d, max %d", getThreshold(), minThreshold, maxThreshold);
	}
	else
	{
		isThresholdValid = false;
	}

	return 0;
}

int waveAnalyzer::getThreshold()
{
	return ((maxThreshold+minThreshold)/2);
}

inline int waveAnalyzer::round(double x)
{
	return (x > 0) ? (int)(x + 0.5) : (int)(x - 0.5);
}

void waveAnalyzer::setWaveSampleRate(int samplerate)
{
	m_sampleRate =  samplerate;
}

retType waveAnalyzer::analyze(std::string &channelData, std::list<int> &startTiming, std::list<int> &endTiming)
{
	updateThreshold(channelData);

	if (ifThresholdValid())
	{
		if(findPulseStartEnd(channelData, startTiming, endTiming))
		{
			return RET_FIND_START;
		}
	}
	return RET_OK;
}

bool waveAnalyzer::findPulseStartEnd(std::string &channelData, std::list<int> &startTiming, std::list<int> &endTiming)
{
	bool result = false;
	int oneMSSamples = (int)round(0.001 * m_sampleRate); // 1ms
	
	int16_t* pBuf = (int16_t*)channelData.c_str();
	int iSamples = channelData.size()/sizeof(uint16_t);
	
	int checkSamples = 0;
	int timing = 0; 
	int totalTiming = iSamples / oneMSSamples - 1;
	int threshold = getThreshold();
	
	do
	{	
		uint16_t maxVaule = 0;
		int postiveSamples = 0;
		int negtiveSamples = 0;
		bool isTone = false;

		for(int i = 0; i < oneMSSamples; i++)
		{	
			maxVaule = max(abs(pBuf[i]), maxVaule);
			
			if(pBuf[i] > 0)
			{
				postiveSamples++;
			}
			else
			{
				negtiveSamples++;
			}
		}

		if (postiveSamples == 0 || negtiveSamples == 0)
		{
			isTone = false;
		}
		else
		{
			isTone = true;
		}

		if (maxVaule > threshold && isTone && findNewEnd)
		{
			startTiming.push_back(timing); // find a start timing
			findNewStart = true;
			findNewEnd = false;
			result = true;
		}
		else if ((!isTone || maxVaule < 5000)  && findNewStart)
		{
			endTiming.push_back(timing); // find a end timing			
			findNewStart = false;
			findNewEnd = true;			
			result = true;
		}
			
		checkSamples += oneMSSamples;
		pBuf += oneMSSamples;
		timing += 1;
	}while(checkSamples < iSamples && timing < totalTiming);
	
	return result;
}



