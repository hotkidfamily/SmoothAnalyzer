#include "StdAfx.h"
#include "log.h"
#include "wavFileParse.h"

WAVFileParse::WAVFileParse(uint32_t flag)
: extraParamBuffer(NULL)
, debugFlag(flag)
, readSamples(0)
{
	if(debugFlag & DEBUG_CHANNEL_DATA){
		dumpLChannelFile.open("c:/lChannelOriginal.pcm", std::ios::binary);
		dumpRChannelFile.open("c:/rChannelOriginal.pcm", std::ios::binary);
	}
}

WAVFileParse::WAVFileParse(void)
: extraParamBuffer(NULL)
, debugFlag(0)
, readSamples(0)
{
}

WAVFileParse::~WAVFileParse(void)
{
	if(extraParamBuffer){
		delete extraParamBuffer;
		extraParamBuffer = NULL;
	}
	if(dumpLChannelFile.is_open())
		dumpLChannelFile.close();

	if(dumpRChannelFile.is_open())
		dumpRChannelFile.close();
}

int32_t WAVFileParse::openWavFile(const char* filename)
{
	int32_t ret = -1;
	wavFile.open(filename, std::ios::binary);
	if(wavFile.is_open()){
		ret = 0;
	}

	theWorkingWithFileName.assign(filename, strlen(filename));

	ret = parseWavParameter();

	readSamples = 0;

	return ret;
}

const char* WAVFileParse::getWavFileFormat(int32_t format_type)
{
	const char *type_str = "unknow";
	switch(format_type){
		case WAVE_FORMAT_PCM:
			type_str = "PCM";
			break;
		case WAVE_FORMAT_IEEE_FLOAT:
			type_str = "IEEE float";
			break;
		case WAVE_FORMAT_ALAW:
			type_str = "A-law";
			break;
		case WAVE_FORMAT_MULAW:
			type_str = "Mu-law";
			break;
		case WAVE_FORMAT_EXTENSIBLE:
			type_str = "extensible";
			break;
		default:
			break;
	}
	return type_str;
}

int32_t WAVFileParse::dumpWavFileHeaders()
{
	int32_t durationInSecond = 0;
	int32_t durationLastMs = 0;
	uint32_t nb_samples = dataHeader.subchunk2Size*8/fmtHeader.nbChannels/fmtHeader.bitsPerSample;
	durationInSecond = nb_samples/fmtHeader.sampleRate;
	durationLastMs = nb_samples%fmtHeader.sampleRate;

#define expanse4bytes(x) x[0],x[1],x[2],x[3]

	inter_log(Debug, "Riff: %c%c%c%c", expanse4bytes(wavHeader.chunkID));
	inter_log(Debug, "File size: %d, %d Kb", wavHeader.chunkSize+8, (wavHeader.chunkSize+8)/1024);
	inter_log(Debug, "Wav marker: %c%c%c%c", expanse4bytes(wavHeader.format));
	inter_log(Debug, "");
	inter_log(Debug, "Fmt marker: %c%c%c%c", expanse4bytes(chunkHeader.subchunk1ID));
	inter_log(Debug, "Fmt header Size: %u", chunkHeader.subchunk1Size);
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
	inter_log(Debug, "Duration: %02d:%02d:%02d.%03d", durationInSecond/(60*60), durationInSecond/60, durationInSecond%60, durationLastMs*1000/fmtHeader.sampleRate);

#undef  expanse4bytes

	return 0;
}

int32_t WAVFileParse::readWavFile(char* buffer, uint32_t data_size)
{
	wavFile.read(buffer, data_size);
	if(wavFile){
		inter_log(FileSystem, "Read %d bytes.", data_size);
	}else{
		inter_log(FileSystem, "Only read %d bytes.", wavFile.gcount());
	}
	
	return wavFile.gcount();
}

int32_t WAVFileParse::parseWavParameter()
{
	int32_t ret = -1;
	int32_t readSize = 0;
	char *ptr = (char*)&wavHeader;
	readSize = readWavFile(ptr, sizeof(WAV_RIFF_HEADER));
	if(readSize != sizeof(WAV_RIFF_HEADER)){
		inter_log(Error, "Get WAV_RIFF_HEADER %d need %d", readSize, sizeof(WAV_RIFF_HEADER));
		goto cleanup;
	}

	ptr = (char*)&chunkHeader;
	readSize = readWavFile(ptr, sizeof(WAV_CHUCK_HEADER));
	if(readSize != sizeof(WAV_CHUCK_HEADER)){
		inter_log(Error, "Get WAV_CHUCK_HEADER %d need %d", readSize, sizeof(WAV_CHUCK_HEADER));
		goto cleanup;
	}

	ptr = (char*)&fmtHeader;
	readSize = readWavFile(ptr, chunkHeader.subchunk1Size);
	if(readSize != chunkHeader.subchunk1Size){
		inter_log(Error, "Get WAV_FMT_HEADER %d need %d", readSize, sizeof(WAV_FMT_HEADER));
		goto cleanup;
	}

	if(chunkHeader.subchunk1Size == 16){
		fmtHeader.extraParamSize = 0;
	}

	if(fmtHeader.extraParamSize > 0){
		if(extraParamBuffer){
			delete extraParamBuffer;
			extraParamBuffer = NULL;
		}
		extraParamBuffer = new char[fmtHeader.extraParamSize];
		readSize = readWavFile(extraParamBuffer, fmtHeader.extraParamSize);
		if(readSize != fmtHeader.extraParamSize){
			inter_log(Error, "Get extra params %d need %d", readSize, fmtHeader.extraParamSize);
			goto cleanup;
		}
	}
	ptr = (char*)&dataHeader;
	readSize = readWavFile(ptr, sizeof(WAV_DATA_HEADER));
	if(readSize != sizeof(WAV_DATA_HEADER)){
		inter_log(Error, "Get WAV_DATA_HEADER %d need %d", readSize, sizeof(WAV_DATA_HEADER));
		goto cleanup;
	}

	if((fmtHeader.audioFormat == 1) && (fmtHeader.extraParamSize != 0)){
		inter_log(Error, "WAV file in pcm data should not have extra parameters.");
		goto cleanup;
	}else if(fmtHeader.audioFormat != 1){
		inter_log(Error, "WAV in %s format is not been supported.", getWavFileFormat(fmtHeader.audioFormat));
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

int32_t WAVFileParse::closeWavFile()
{
	if(wavFile.is_open()){
		wavFile.close();
	}

	return 0;
}

int32_t WAVFileParse::separateLRChannel(char *data, uint32_t dataSize, std::string &lChannel, std::string &rChannel)
{
	int16_t *dataPtr = (int16_t*)data;
	int16_t *lchannelData = NULL;
	int16_t *rchannelData = NULL;

	lChannel.resize(dataSize/fmtHeader.nbChannels, 0);
	rChannel.resize(dataSize/fmtHeader.nbChannels, 0);
	lchannelData = (int16_t*)lChannel.c_str();
	rchannelData = (int16_t*)rChannel.c_str();

	for(size_t i = 0; i<rChannel.size()/(fmtHeader.bitsPerSample/8); i++){	
		*lchannelData = *dataPtr;
		*rchannelData = *(dataPtr+1);
		lchannelData ++;
		rchannelData ++;
		dataPtr += fmtHeader.packageSize/fmtHeader.nbChannels;
	}

	return 0;
}

// 100ms data one time
int32_t WAVFileParse::getLRChannelData(std::string &lChannel, std::string &rChannel)
{
	int32_t ret = 0;
	uint32_t nbReadSamples = fmtHeader.sampleRate / 10;
	uint32_t nbSampleDataSize = nbReadSamples * fmtHeader.packageSize ; // 10ms
	uint32_t readDataLength = 0;
	
	std::string tenMSDataBuffer;
	tenMSDataBuffer.resize(nbSampleDataSize, 0);
	char* buffer = (char*)tenMSDataBuffer.c_str();
	
	//inter_log(Debug, "10 ms data size %d, %d samples", dataSizeIn10MS, dataSizeIn10MS/fmtHeader.packageSize);
	
	readDataLength = readWavFile(buffer, tenMSDataBuffer.size());

	if((readSamples*fmtHeader.packageSize + readDataLength) > dataHeader.subchunk2Size){
		readDataLength = dataHeader.subchunk2Size - readSamples*fmtHeader.packageSize;
	}
	separateLRChannel(buffer, readDataLength, lChannel, rChannel);

	if(dumpLChannelFile.is_open())
		dumpLChannelFile.write(lChannel.c_str(), lChannel.size());
	if(dumpRChannelFile.is_open())
		dumpRChannelFile.write(rChannel.c_str(), rChannel.size());

	readSamples += readDataLength/fmtHeader.packageSize;

	reportProgress(readSamples);
	if(readDataLength < nbSampleDataSize){
		ret = -1;
	}

	return ret;
}

double WAVFileParse::covertSampleToMS(uint32_t sampleIndex)
{
	return sampleIndex*1.0/fmtHeader.sampleRate;
}

void WAVFileParse::reportProgress(int32_t nbProcessedSamples)
{
	double nbTotalSamples = 0;
	nbTotalSamples = dataHeader.subchunk2Size*1.0/fmtHeader.nbChannels/(fmtHeader.bitsPerSample/8);
	printf("\tProcess %0.2f%%...\r", nbProcessedSamples*100.0/nbTotalSamples);
}