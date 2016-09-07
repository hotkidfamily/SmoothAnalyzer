#pragma once

#include <fstream>
#include "wavFileFormat.h"

class wavFileParse
{
public:
	wavFileParse(void);
	~wavFileParse(void);

	int openWavFile(const char* filename);
	int closeWavFile();

private:
	int dumpWavFileHeaders();
	int readWavFile(char* buffer, uint32_t data_size);
	int parseWavParameter();

	const char* getWavFileFormat(int format_type);

private:
	std::ifstream  wavFile;
	WAV_RIFF_HEADER wavHeader;
	WAV_FMT_HEADER fmtHeader;
	char* extraParamBuffer;
	WAV_DATA_HEADER dataHeader;
	std::string theWorkingWithFileName;
};