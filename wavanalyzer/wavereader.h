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
    uint16_t nFormaTag;       // ��ʽ���0x01ΪPCM��ʽ���������� 
    uint16_t nChannels;       // ������  
    uint32_t nSampleRate;     // ������  
    uint32_t nAvgBytesRate;   // ����  
    uint16_t nBlockAlign; 
    uint16_t nBitsPerSample;  // �������      

	inline int32_t GetBytesPerSample() { return nBitsPerSample >> 3; };

} WaveFormat; 
#pragma pack() 

/******************************************************************************* 
    CWaveReader�ඨ�壬���ڶ�ȡ.wav�ļ��е���Ƶ����
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
	double SampeIndexToSecond(uint32_t sampleIndex);

private:
    bool ReadHeader(); 
	
private: 
    FILE* m_pFile; 
    int m_nDataLen; 
	double m_Progress;
    WaveFormat m_WaveFormat; 
};  
 
#endif 
