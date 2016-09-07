#include "StdAfx.h"
#include "log.h"
#include "endianCovert.h"
#include "wavFileParse.h"

wavFileParse::wavFileParse(void)
: extraParamBuffer(NULL)
{
}

wavFileParse::~wavFileParse(void)
{
	if(extraParamBuffer){
		delete extraParamBuffer;
		extraParamBuffer = NULL;
	}
}

int wavFileParse::openWavFile(const char* filename)
{
	int ret = -1;
	wavFile.open(filename, std::ios::binary);
	if(wavFile.is_open()){
		ret = 0;
	}

	theWorkingWithFileName.assign(filename, strlen(filename));

	parseWavParameter();

	return ret;
}

const char* wavFileParse::getWavFileFormat(int format_type)
{
	const char *type_str = "Error";
	switch(format_type){
		case 1:
			type_str = "PCM";
			break;
		case 6:
			type_str = "A-law";
			break;
		case 7:
			type_str = "Mu-law";
			break;
		default:
			break;
	}
	return type_str;
}

int wavFileParse::dumpWavFileHeaders()
{
	int durationInSecond = 0;
	durationInSecond = dataHeader.subchunk2Size*8/fmtHeader.nbChannels/fmtHeader.bitsPerSample/fmtHeader.sampleRate;
#define expanse4bytes(x) x[0],x[1],x[2],x[3]

	inter_log(Debug, "Riff: %c%c%c%c", expanse4bytes(wavHeader.chunkID));
	inter_log(Debug, "File size: %d, %d Kb", wavHeader.chunkSize+8, (wavHeader.chunkSize+8)/1024);
	inter_log(Debug, "Wav marker: %c%c%c%c", expanse4bytes(wavHeader.format));
	inter_log(Debug, "");
	inter_log(Debug, "Fmt marker: %c%c%c%c", expanse4bytes(fmtHeader.subchunk1ID));
	inter_log(Debug, "Fmt header Size: %u", fmtHeader.subchunk1Size);
	inter_log(Debug, "Format type: %s", getWavFileFormat(fmtHeader.audioFormat));
	inter_log(Debug, "Channels: %u", fmtHeader.nbChannels);
	inter_log(Debug, "Sample rate: %u", fmtHeader.sampleRate);
	inter_log(Debug, "Bit rate: %u Kbps", fmtHeader.byteRate*8/1000);
	inter_log(Debug, "Align: %u", fmtHeader.packageSize);
	inter_log(Debug, "Bits per sample: %u", fmtHeader.bitsPerSample);
	inter_log(Debug, "Extra params size: %u", fmtHeader.extraParamSize);
	inter_log(Debug, "");
	inter_log(Debug, "Data maker: %c%c%c%c", expanse4bytes(dataHeader.subchunk2ID));
	inter_log(Debug, "Data size: %d", dataHeader.subchunk2Size);
	inter_log(Debug, "Duration: %02d:%02d:%02d", durationInSecond/(60*60), durationInSecond/60, durationInSecond%60);

#undef  expanse4bytes

	return 0;
}

int wavFileParse::readWavFile(char* buffer, uint32_t data_size)
{
	wavFile.read(buffer, data_size);
	if(wavFile){
		inter_log(FileSystem, "Read %d bytes.", data_size);
	}else{
		inter_log(FileSystem, "Only read %d bytes.", wavFile.gcount());
	}
	
	return wavFile.gcount();
}

int wavFileParse::parseWavParameter()
{
	int ret = -1;
	char *ptr = (char*)&wavHeader;
	ret = readWavFile(ptr, sizeof(WAV_RIFF_HEADER));
	if(ret != sizeof(WAV_RIFF_HEADER)){
		inter_log(Error, "Get WAV_RIFF_HEADER %d need %d", ret, sizeof(WAV_RIFF_HEADER));
		goto cleanup;
	}

	ptr = (char*)&fmtHeader;
	ret = readWavFile(ptr, sizeof(WAV_FMT_HEADER));
	if(ret != sizeof(WAV_FMT_HEADER)){
		inter_log(Error, "Get WAV_FMT_HEADER %d need %d", ret, sizeof(WAV_FMT_HEADER));
		goto cleanup;
	}

	if(fmtHeader.extraParamSize > 0){
		if(extraParamBuffer){
			delete extraParamBuffer;
			extraParamBuffer = NULL;
		}
		extraParamBuffer = new char[fmtHeader.extraParamSize];
		ret = readWavFile(extraParamBuffer, fmtHeader.extraParamSize);
		if(ret != fmtHeader.extraParamSize){
			inter_log(Error, "Get extra params %d need %d", ret, fmtHeader.extraParamSize);
			goto cleanup;
		}
	}
	ptr = (char*)&dataHeader;
	ret = readWavFile(ptr, sizeof(WAV_DATA_HEADER));
	if(ret != sizeof(WAV_DATA_HEADER)){
		inter_log(Error, "Get WAV_DATA_HEADER %d need %d", ret, sizeof(WAV_DATA_HEADER));
		goto cleanup;
	}

	if((fmtHeader.audioFormat == 1) && (fmtHeader.extraParamSize != 0)){
		inter_log(Error, "WAV file contain pcm data should not have extra parameters.");
		goto cleanup;
	}

	ret = 0;

	dumpWavFileHeaders();

cleanup:
	if(ret){
		inter_log(Error, "read file %s", theWorkingWithFileName.c_str());
	}
	return ret;
}

int wavFileParse::closeWavFile()
{
	if(wavFile.is_open()){
		wavFile.close();
	}

	return 0;
}

int wavFileParse::separateLRChannel(char *data, uint32_t data_size)
{
	int16_t *dataPtr = (int16_t*)data;
	int16_t *lchannelData = NULL;
	int16_t *rchannelData = NULL;
	lChannel.resize(data_size/fmtHeader.nbChannels, 0);
	RChannel.resize(data_size/fmtHeader.nbChannels, 0);

	lchannelData = (int16_t*)lChannel.c_str();
	rchannelData = (int16_t*)RChannel.c_str();

	for(size_t i = 0; i<lChannel.size(); i++){	
		*lchannelData = *dataPtr;
		*rchannelData = *(dataPtr+1);
		lchannelData ++;
		rchannelData ++;
		dataPtr += fmtHeader.packageSize/fmtHeader.nbChannels;
	}

	return 0;
}

int wavFileParse::parseLRSync()
{
	uint32_t dataSizeIn10MS = fmtHeader.sampleRate * fmtHeader.packageSize / 100 ; // 10ms
	std::string tenMSDataBuffer;
	tenMSDataBuffer.resize(dataSizeIn10MS, 0);
	char* buffer = (char*)tenMSDataBuffer.c_str();
	int32_t duration  = 0;
	inter_log(Info, "10 ms data size %d, %d samples", dataSizeIn10MS, dataSizeIn10MS/fmtHeader.packageSize);
	
	dumpLChannelFile.open("c:/lchannel.pcm", std::ios::binary);
	dumpRChannelFile.open("c:/rchannel.pcm", std::ios::binary);

	while(readWavFile(buffer, tenMSDataBuffer.size()) == tenMSDataBuffer.size()){
		separateLRChannel(buffer, tenMSDataBuffer.size());
		dumpLChannelFile.write(lChannel.c_str(), lChannel.size());
		dumpRChannelFile.write(RChannel.c_str(), RChannel.size());

		reportProgress(duration);
		duration += 10;
	}

	dumpLChannelFile.close();
	dumpRChannelFile.close();

	return 0;
}

void wavFileParse::reportProgress(int32_t durationInMS)
{
	int durationInSecond = 0;
	durationInSecond = dataHeader.subchunk2Size*8/fmtHeader.nbChannels/fmtHeader.bitsPerSample/fmtHeader.sampleRate;
	printf("\tProcess %0.2f%%...\r", durationInMS/durationInSecond);
}