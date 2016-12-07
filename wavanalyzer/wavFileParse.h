#pragma once

#include "csvFileMaker.h"
#include "wavereader.h"

enum debug_flags{
	DEBUG_CHANNEL_DATA = 1<<0,
	DEBUG_SOURCE_DATA = 1<<1,
};

class WAVFileParse
{
public:
	WAVFileParse(void);
	~WAVFileParse(void);
	WAVFileParse(uint32_t flag);

	bool openWavFile(const char* filename);
	bool closeWavFile();
	bool getLRChannelDataSeperately(std::string &lChannel, std::string &rChannel, int timesMS);	
	WaveFormat &getWavFormat() { return m_wavFormat; };
	
private:
	uint32_t debugFlag;
	std::ofstream dumpLChannelFile;
	std::ofstream dumpRChannelFile;

	CWaveReader* m_wavReader;
	WaveFormat m_wavFormat;
};
