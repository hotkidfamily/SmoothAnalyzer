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

	bool OpenWavFile(const char* filename);
	bool CloseWavFile();
	int32_t GetLRChannelData(std::string &lChannel, std::string &rChannel);
	WaveFormat &GetWavFormat() { return m_wavFormat; };
	double ConvertIndexToMS(int32_t index);

protected:
	void ReportProgress(double progree);
	int32_t SeparateLRChannel(char *data, uint32_t dataSize, std::string &lChannel, std::string &rChannel);
	
private:
	uint32_t debugFlag;
	std::ofstream dumpLChannelFile;
	std::ofstream dumpRChannelFile;

	CWaveReader* m_wavReader;
	WaveFormat m_wavFormat;
};
