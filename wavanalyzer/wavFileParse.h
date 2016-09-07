#pragma once

#include <fstream>
#include "csvFileMaker.h"
#include "wavFileFormat.h"

class wavFileParse
{
public:
	wavFileParse(void);
	~wavFileParse(void);

	int openWavFile(const char* filename);
	int parseLRSync();
	int closeWavFile();

private:
	int dumpWavFileHeaders();
	int readWavFile(char* buffer, uint32_t data_size);
	int parseWavParameter();

	int separateLRChannel(char *data, uint32_t data_size);
	int findStart(int16_t *data, uint32_t data_size);
	int findEnd(int16_t *data, uint32_t data_size);

	const char* getWavFileFormat(int format_type);

	void reportProgress(int32_t durationInMS);
	
private:
	WAV_RIFF_HEADER wavHeader;
	WAV_FMT_HEADER fmtHeader;
	char* extraParamBuffer;
	WAV_DATA_HEADER dataHeader;

	std::ifstream  wavFile;
	std::string theWorkingWithFileName;

	std::string lChannel;
	std::string RChannel;

	csvOutput recorder;

	std::ofstream dumpLChannelFile;
	std::ofstream dumpRChannelFile;
};