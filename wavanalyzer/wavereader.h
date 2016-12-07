#pragma once

#ifndef _WAVE_H 
#define _WAVE_H 
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <string.h>
 
#pragma pack(1)
typedef struct _WaveFormat 
{ 
    unsigned short nFormatag;       // 格式类别，0x01为PCM形式的声音数据 
    unsigned short nChannels;       // 声道数  
    unsigned int nSamplerate;     // 采样率  
    unsigned int nAvgBytesRate;   // 码率  
    unsigned short nblockalign; 
    unsigned short nBitsPerSample;  // 采样深度      
} WaveFormat; 
#pragma pack() 

/******************************************************************************* 
    CWaveReader类定义，用于读取.wav文件中的音频数据  
*******************************************************************************/ 
class CWaveReader 
{ 
public: 
    CWaveReader(); 
    ~CWaveReader(); 
    bool Open(const char* pFileName);
    void Close(); 
    int  ReadData(unsigned char* pData, int nLen); 
    bool GetFormat(WaveFormat* pWaveFormat); 
    FILE* Handle();
private:
    bool ReadHeader(); 
private: 
    FILE* m_pFile; 
    int m_nDataLen; 
    WaveFormat m_WaveFormat; 
};  
 
#endif 
