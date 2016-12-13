#pragma once

#include "pulse.h"
#include "csvFileMaker.h"
#include "wavereader.h"
#include "DataFilters.h"

class DataSeparater
{
public:
	DataSeparater(std::string &name);
	~DataSeparater(void);

	int32_t GetChannelData(int32_t index, std::string &source, std::string &samples);
	bool SetWavFormat(WaveFormat &format){ m_wavFormat = format; return true; };

protected:
	int32_t SeparateChannelData(int32_t index, std::string &originalData, std::string &channelData);
	
private:
	WaveFormat m_wavFormat;
	iDump *mDataDump;
};
