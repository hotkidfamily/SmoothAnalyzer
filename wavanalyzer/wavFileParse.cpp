#include "StdAfx.h"
#include "log.h"
#include "wavFileParse.h"

WAVFileParse::WAVFileParse(void)
: m_wavReader(NULL)
{
	m_wavReader = new CWaveReader();
}

WAVFileParse::~WAVFileParse(void)
{
	if (m_wavReader){
		delete m_wavReader;
		m_wavReader = NULL;
	}
}

bool WAVFileParse::OpenWavFile(const char* filename)
{
	bool result = false;
	if (m_wavReader->Open(filename)){
		if(m_wavReader->GetFormat(&m_wavFormat)){
			result = true;
		}
	}
	return result;
}

bool WAVFileParse::CloseWavFile()
{
	if(m_wavReader){
		m_wavReader->Close();
	}
	return true;
}

void WAVFileParse::ReportProgress(double progress)
{
	printf("\tProcess %0.2f%%...\r", progress);
}

double WAVFileParse::ConvertIndexToMS(int32_t index)
{
	return m_wavReader->SampeIndexToMS(index);
}

int32_t WAVFileParse::SeparateChannelData(int32_t index, std::string &originalData, std::string &channelData)
{
	int16_t *dataPtr = (int16_t*)originalData.c_str();
	channelData.resize(originalData.size()/m_wavFormat.nChannels, 0);
	int16_t* lchannelData = (int16_t*)channelData.c_str();
	
	for(size_t i = 0; i<channelData.size()/(m_wavFormat.nBitsPerSample/8); i++){	
		*lchannelData = *(dataPtr+index);
		lchannelData ++;
		dataPtr += m_wavFormat.nblockalign/2/*2 == sizeof(int16_t)*/;
	}

	return 0;
}

int32_t WAVFileParse::GetChannelData(int32_t index, std::string &samples)
{
	int32_t ret = 0;
	uint32_t nbReadSamples = m_wavFormat.nSamplerate / 100;
	uint32_t nbSampleDataSize = nbReadSamples * m_wavFormat.nblockalign; // 100ms
	int32_t readDataLength = 0;

	std::string tenMSDataBuffer;
	tenMSDataBuffer.resize(nbSampleDataSize, 0);
	char* buffer = (char*)tenMSDataBuffer.c_str();

	//inter_log(Debug, "10 ms data size %d, %d samples", dataSizeIn10MS, dataSizeIn10MS/fmtHeader.packageSize);

	readDataLength = m_wavReader->ReadData((unsigned char*)buffer, tenMSDataBuffer.size());
	if(readDataLength < nbSampleDataSize){ 
		if(readDataLength<0){
			goto cleanup;
		}
		ret = EOF;
	}

	ReportProgress(m_wavReader->Progress());

	SeparateChannelData(index, tenMSDataBuffer, samples);

cleanup:
	return ret;
}
