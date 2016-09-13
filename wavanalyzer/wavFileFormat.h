#pragma once

#include "stdint.h"

struct WAV_RIFF_HEADER{
	uint8_t  chunkID[4];			// RIFF string
	uint32_t chunkSize;			// overall size of file in bytes
	uint8_t  format[4];			// WAVE string
};

#pragma pack(push)
#pragma pack(2)
struct WAV_FMT_HEADER{
	uint8_t  subchunk1ID[4];		// fmt string with trailing null char
	uint32_t subchunk1Size;			// length of the format data
	uint16_t audioFormat;			// format type. 1-PCM, 3- IEEE float, 6 - 8bit A law, 7 - 8bit mu law
	uint16_t nbChannels;			// no.of channels
	uint32_t sampleRate;			// sampling rate (blocks per second)
	uint32_t byteRate;				// SampleRate * NumChannels * BitsPerSample/8
	uint16_t packageSize;			// NumChannels * BitsPerSample/8
	uint16_t bitsPerSample;			// bits per sample, 8- 8bits, 16- 16 bits etc
	uint16_t extraParamSize;		// extra param size, if PCM, then doesn't exist
};
#pragma pack(pop)

struct WAV_DATA_HEADER{
	uint8_t  subchunk2ID[4];		// DATA string or FLLR string
	uint32_t subchunk2Size;			// NumSamples * NumChannels * BitsPerSample/8 - size of the next chunk that will be read
};
