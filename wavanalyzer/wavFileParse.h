#pragma once

#include <fstream>
#include "csvFileMaker.h"
#include "wavFileFormat.h"

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

	int openWavFile(const char* filename);
	int getLRChannelData(std::string &lChannel, std::string &rChannel);
	int closeWavFile();

	double covertSampleToMS(uint32_t sampleIndex);

	int getSampleRate() const {return fmtHeader.sampleRate; }
	int getBytesPerSample() const {return fmtHeader.bitsPerSample/8; }

private:
	int dumpWavFileHeaders();
	int readWavFile(char* buffer, uint32_t data_size);
	int parseWavParameter();

	int separateLRChannel(char *data, uint32_t data_size, std::string &lChannel, std::string &rChannel);
	int findStart(int16_t *data, uint32_t data_size);
	int findEnd(int16_t *data, uint32_t data_size);

	const char* getWavFileFormat(int format_type);
	void reportProgress(int32_t durationInMS);
	
private:
	WAV_RIFF_HEADER wavHeader;
	WAV_CHUCK_HEADER chunkHeader;
	WAV_FMT_HEADER fmtHeader;
	char* extraParamBuffer;
	WAV_DATA_HEADER dataHeader;

	std::ifstream  wavFile;
	std::string theWorkingWithFileName;

	std::ofstream dumpLChannelFile;
	std::ofstream dumpRChannelFile;

	int32_t readSamples;

	uint32_t debugFlag;
};