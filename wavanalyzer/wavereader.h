#pragma once

#ifndef _WAVE_H 
#define _WAVE_H 
#include "stdafx.h"

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <string.h>

#include "stdint.h"

#pragma pack(1)
typedef struct _WaveFormat 
{ 
    uint16_t nFormatag;       // 格式类别，0x01为PCM形式的声音数据 
    uint16_t nChannels;       // 声道数  
    uint32_t nSamplerate;     // 采样率  
    uint32_t nAvgBytesRate;   // 码率  
    uint16_t nblockalign; 
    uint16_t nBitsPerSample;  // 采样深度      
} WaveFormat; 
#pragma pack() 

/******************************************************************************* 
    CWaveReader类定义，用于读取.wav文件中的音频数据
	current only support 2 channels wav files.
*******************************************************************************/ 
class CWaveReader 
{ 
public: 
    CWaveReader(); 
    ~CWaveReader(); 
    bool Open(const char* pFileName);
    void Close(); 
    size_t ReadData(uint8_t* pData, int nLen); 
	int32_t CWaveReader::ReadData(std::string &data);
    WaveFormat &GetFormat(); 
    FILE* Handle();
	double Progress() { return m_Progress; };
	double SampeIndexToMS(uint32_t sampleIndex);

private:
    bool ReadHeader(); 
	
private: 
    FILE* m_pFile; 
    int m_nDataLen; 
	double m_Progress;
    WaveFormat m_WaveFormat; 
};  
 
#endif 
