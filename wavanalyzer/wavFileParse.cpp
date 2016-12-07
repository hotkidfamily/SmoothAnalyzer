#include "StdAfx.h"
#include "log.h"
#include "wavFileParse.h"

WAVFileParse::WAVFileParse(uint32_t flag)
: debugFlag(flag)
, m_wavReader(NULL)
{
	if(debugFlag & DEBUG_CHANNEL_DATA){
		dumpLChannelFile.open("c:/lChannelOriginal.pcm", std::ios::binary);
		dumpRChannelFile.open("c:/rChannelOriginal.pcm", std::ios::binary);
	}
	m_wavReader = new CWaveReader();
}

WAVFileParse::WAVFileParse(void)
: debugFlag(0)
, m_wavReader(NULL)
{
	m_wavReader = new CWaveReader();
}

WAVFileParse::~WAVFileParse(void)
{
	if(dumpLChannelFile.is_open())
		dumpLChannelFile.close();

	if(dumpRChannelFile.is_open())
		dumpRChannelFile.close();
	
	if (m_wavReader){
		delete m_wavReader;
		m_wavReader = NULL;
	}
}

bool WAVFileParse::openWavFile(const char* filename)
{
	bool result = false;
	if (m_wavReader->Open(filename)){
		if(m_wavReader->GetFormat(&m_wavFormat)){
			result = true;
		}
	}
	return result;
}

bool WAVFileParse::closeWavFile()
{
	if(m_wavReader){
		m_wavReader->Close();
	}
	return true;
}

void WAVFileParse::reportProgress(double progress)
{
	printf("\tProcess %0.2f%%...\r", progress);
}

double WAVFileParse::convertIndexToMS(int32_t index)
{
	return m_wavReader->SampeIndexToMS(index);
}

int32_t WAVFileParse::separateLRChannel(char *data, uint32_t dataSize, std::string &lChannel, std::string &rChannel)
{
	int16_t *dataPtr = (int16_t*)data;
	int16_t *lchannelData = NULL;
	int16_t *rchannelData = NULL;

	lChannel.resize(dataSize/m_wavFormat.nChannels, 0);
	rChannel.resize(dataSize/m_wavFormat.nChannels, 0);
	lchannelData = (int16_t*)lChannel.c_str();
	rchannelData = (int16_t*)rChannel.c_str();

	for(size_t i = 0; i<rChannel.size()/(m_wavFormat.nBitsPerSample/8); i++){	
		*lchannelData = *dataPtr;
		*rchannelData = *(dataPtr+1);
		lchannelData ++;
		rchannelData ++;
		dataPtr += m_wavFormat.nblockalign/m_wavFormat.nChannels;
	}

	return 0;
}

// 100ms data one time
int32_t WAVFileParse::getLRChannelData(std::string &lChannel, std::string &rChannel)
{
	int32_t ret = 0;
	uint32_t nbReadSamples = m_wavFormat.nSamplerate / 10;
	uint32_t nbSampleDataSize = nbReadSamples * m_wavFormat.nblockalign; // 100ms
	int32_t readDataLength = 0;

	std::string tenMSDataBuffer;
	tenMSDataBuffer.resize(nbSampleDataSize, 0);
	char* buffer = (char*)tenMSDataBuffer.c_str();

	//inter_log(Debug, "10 ms data size %d, %d samples", dataSizeIn10MS, dataSizeIn10MS/fmtHeader.packageSize);

	readDataLength = m_wavReader->ReadData((unsigned char*)buffer, tenMSDataBuffer.size());
	if(readDataLength <= 0){
		return false;
	}

	reportProgress(m_wavReader->Progress());

	separateLRChannel(buffer, readDataLength, lChannel, rChannel);

	if(dumpLChannelFile.is_open())
		dumpLChannelFile.write(lChannel.c_str(), lChannel.size());
	if(dumpRChannelFile.is_open())
		dumpRChannelFile.write(rChannel.c_str(), rChannel.size());

	if(readDataLength < nbSampleDataSize){
		ret = EOF;
	}

	return ret;
}


