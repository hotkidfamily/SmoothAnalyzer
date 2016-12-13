#pragma once

#include "pulse.h"
#include "csvFileMaker.h"
#include "wavereader.h"

class WAVFileParse
{
public:
	WAVFileParse(void);
	~WAVFileParse(void);

	bool OpenWavFile(const char* filename);
	bool CloseWavFile();
	int32_t GetChannelData(int32_t index, std::string &samples);
	WaveFormat &GetWavFormat() { return m_wavFormat; };
	double ConvertIndexToMS(int32_t index);

protected:
	void ReportProgress(double progree);
	int32_t SeparateChannelData(int32_t index, std::string &originalData, std::string &channelData);
	
private:
	CWaveReader* m_wavReader;
	WaveFormat m_wavFormat;
};
