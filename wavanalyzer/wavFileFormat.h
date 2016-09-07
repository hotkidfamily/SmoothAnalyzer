#pragma once

#include "stdint.h"


struct WAV_RIFF_HEADER{
	unsigned char chunkID[4];					// RIFF string
	unsigned int chunkSize;					// overall size of file in bytes
	unsigned char format[4];					// WAVE string
};

#pragma pack(push)
#pragma pack(2)
struct WAV_FMT_HEADER{
	unsigned char subchunk1ID[4];		// fmt string with trailing null char
	unsigned int subchunk1Size;				// length of the format data
	uint16_t audioFormat;					// format type. 1-PCM, 3- IEEE float, 6 - 8bit A law, 7 - 8bit mu law
	uint16_t nbChannels;						// no.of channels
	unsigned int sampleRate;				// sampling rate (blocks per second)
	unsigned int byteRate;				// SampleRate * NumChannels * BitsPerSample/8
	uint16_t packageSize;					// NumChannels * BitsPerSample/8
	uint16_t bitsPerSample;				// bits per sample, 8- 8bits, 16- 16 bits etc
	uint16_t extraParamSize;				// ExtraParamSize   if PCM, then doesn't exist
};
#pragma pack(pop)

struct WAV_DATA_HEADER{
	unsigned char subchunk2ID [4];	// DATA string or FLLR string
	unsigned int subchunk2Size;					// NumSamples * NumChannels * BitsPerSample/8 - size of the next chunk that will be read
};

// WAVE file header format
//#pragma pack(push)
struct WAV_HEADER {
	// header
	unsigned char riff[4];					// RIFF string
	unsigned int file_size	;				// overall size of file in bytes
	unsigned char wave[4];					// WAVE string

	// fmt
	unsigned char fmt_chunk_marker[4];		// fmt string with trailing null char
	unsigned int fmt_data_size;				// length of the format data
	uint16_t format_type;					// format type. 1-PCM, 3- IEEE float, 6 - 8bit A law, 7 - 8bit mu law
	uint16_t channels;						// no.of channels
	unsigned int sample_rate;				// sampling rate (blocks per second)
	unsigned int bitrateInByte;				// SampleRate * NumChannels * BitsPerSample/8
	uint16_t block_align;					// NumChannels * BitsPerSample/8
	uint16_t bits_per_sample;				// bits per sample, 8- 8bits, 16- 16 bits etc
	uint16_t extra_param_size;				// ExtraParamSize   if PCM, then doesn't exist

	//data
	unsigned char data_chunk_header [4];	// DATA string or FLLR string
	unsigned int data_size;					// NumSamples * NumChannels * BitsPerSample/8 - size of the next chunk that will be read
};
//#pragma pack(pop)