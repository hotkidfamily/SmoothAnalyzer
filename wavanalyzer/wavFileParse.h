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

	int32_t openWavFile(const char* filename);
	int32_t getLRChannelData(std::string &lChannel, std::string &rChannel);
	int32_t closeWavFile();

	double covertSampleToMS(uint32_t sampleIndex);

	int32_t getSampleRate() const {return fmtHeader.sampleRate; }
	int32_t getBytesPerSample() const {return fmtHeader.bitsPerSample/8; }

private:
	int32_t dumpWavFileHeaders();
	int32_t readWavFile(char* buffer, uint32_t data_size);
	int32_t parseWavParameter();

	int32_t separateLRChannel(char *data, uint32_t data_size, std::string &lChannel, std::string &rChannel);
	int32_t findStart(int16_t *data, uint32_t data_size);
	int32_t findEnd(int16_t *data, uint32_t data_size);

	const char* getWavFileFormat(int32_t format_type);
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