#pragma once

#include <fstream>
#include "csvFileMaker.h"
#include "wavFileFormat.h"
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
	int getSampleRate() const {return m_wavFormat.nSamplerate; }
	
private:
	std::string theWorkingWithFileName;

	std::ofstream dumpLChannelFile;
	std::ofstream dumpRChannelFile;

	uint32_t debugFlag;
	CWaveReader* m_wavReader;
	WaveFormat m_wavFormat;
};
