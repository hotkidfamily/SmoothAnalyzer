#include "StdAfx.h"
#include "log.h"
#include "wavFileParse.h"

DataSeparater::DataSeparater(std::string &name)
{
	mDataDump = new iDump(name + ".original.pcm");
}

DataSeparater::~DataSeparater(void)
{
}

int32_t DataSeparater::SeparateChannelData(int32_t index, std::string &originalData, std::string &channelData)
{
	int16_t *dataPtr = (int16_t*)originalData.c_str();
	channelData.resize(originalData.size()/m_wavFormat.nChannels, 0);
	int16_t* lchannelData = (int16_t*)channelData.c_str();
	
	for(size_t i = 0; i<channelData.size()/(m_wavFormat.nBitsPerSample/8); i++){	
		*lchannelData = *(dataPtr+index);
		lchannelData ++;
		dataPtr += m_wavFormat.nBlockAlign/2/*2 == sizeof(int16_t)*/;
	}

	mDataDump->DumpData(channelData);

	return 0;
}

int32_t DataSeparater::GetChannelData(int32_t index, std::string &source, std::string &samples)
{
	int32_t ret = 0;

	SeparateChannelData(index, source, samples);

	return ret;
}
