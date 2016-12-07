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
	int32_t getLRChannelData(std::string &lChannel, std::string &rChannel);
	WaveFormat &getWavFormat() { return m_wavFormat; };
	double convertIndexToMS(int32_t index);

protected:
	void reportProgress(double progree);
	int32_t separateLRChannel(char *data, uint32_t dataSize, std::string &lChannel, std::string &rChannel);
	
private:
	uint32_t debugFlag;
	std::ofstream dumpLChannelFile;
	std::ofstream dumpRChannelFile;

	CWaveReader* m_wavReader;
	WaveFormat m_wavFormat;
};
