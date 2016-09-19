#pragma once

#include "stdint.h"

#ifdef WAVE_FORMAT_PCM
	#undef WAVE_FORMAT_PCM
#endif

enum WAVE_DATA_FORMAT {
	WAVE_FORMAT_PCM = 0x0001,
	WAVE_FORMAT_IEEE_FLOAT = 0x0003,
	WAVE_FORMAT_ALAW = 0x0006,
	WAVE_FORMAT_MULAW = 0x0007,
	WAVE_FORMAT_EXTENSIBLE = 0xfffe,
};

struct WAV_RIFF_HEADER{
	uint8_t  chunkID[4];			// RIFF string
	uint32_t chunkSize;			// overall size of file in bytes
	uint8_t  format[4];			// WAVE string
};

/*
ckID	4	Chunk ID: fmt 
cksize	4	Chunk size: 16, 18 or 40
	wFormatTag		2	Format code
	nChannels		2	Number of interleaved channels
	nSamplesPerSec	4	Sampling rate (blocks per second)
	nAvgBytesPerSec	4	Data rate
	nBlockAlign		2	Data block size (bytes)
	wBitsPerSample	2	Bits per sample
	cbSize			2	Size of the extension (0 or 22)
	wValidBitsPerSample	2	Number of valid bits
	dwChannelMask	4	Speaker position mask
	SubFormat		16	GUID, including the data format code

*/

struct WAV_CHUCK_HEADER{
	uint8_t  subchunk1ID[4];		// fmt string with trailing null char
	uint32_t subchunk1Size;			// length of the format data
};

#pragma pack(push)
#pragma pack(2)
struct WAV_FMT_HEADER{
	uint16_t audioFormat;			// format type. 1-PCM, 3- IEEE float, 6 - 8bit A law, 7 - 8bit mu law, 0xfffe - extensible
	uint16_t nbChannels;			// no.of channels
	uint32_t sampleRate;			// sampling rate (blocks per second)
	uint32_t byteRate;				// SampleRate * NumChannels * BitsPerSample/8
	uint16_t packageSize;			// NumChannels * BitsPerSample/8
	uint16_t bitsPerSample;			// bits per sample, 8- 8bits, 16- 16 bits etc
};
#pragma pack(pop)

struct WAV_DATA_HEADER{
	uint8_t  subchunk2ID[4];		// DATA string or FLLR string
	uint32_t subchunk2Size;			// NumSamples * NumChannels * BitsPerSample/8 - size of the next chunk that will be read
};
