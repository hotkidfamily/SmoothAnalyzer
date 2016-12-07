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

bool WAVFileParse::getLRChannelDataSeperately(std::string &lChannel, std::string &rChannel, int timesMS)
{
	if(m_wavFormat.nChannels != 2){
		return false;
	}
	
	int iDataBufferSize = timesMS * m_wavFormat.nSamplerate * m_wavFormat.nChannels * m_wavFormat.nBitsPerSample / 8 / 1000;
	std::string MSDataBuffer;
	MSDataBuffer.resize(iDataBufferSize, 0);
	char* pBuf = (char*)MSDataBuffer.c_str();

	int readSize = m_wavReader->ReadData((unsigned char*)pBuf, iDataBufferSize);
	if(readSize <= 0){
		return false;
	}

	int nBytesPerSample = m_wavFormat.nBitsPerSample / 8;
	for(int i = 0; i < readSize/2/nBytesPerSample; i++){
		lChannel.append(pBuf, nBytesPerSample);
		rChannel.append(pBuf + nBytesPerSample, nBytesPerSample);
		pBuf += nBytesPerSample*2;
	}

	if(dumpLChannelFile.is_open())
		dumpLChannelFile.write(lChannel.c_str(), lChannel.size());
	if(dumpRChannelFile.is_open())
		dumpRChannelFile.write(rChannel.c_str(), rChannel.size());
	
	return true;
}

