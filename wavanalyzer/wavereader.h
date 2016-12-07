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
    unsigned short nFormatag;       // ��ʽ���0x01ΪPCM��ʽ���������� 
    unsigned short nChannels;       // ������  
    unsigned int nSamplerate;     // ������  
    unsigned int nAvgBytesRate;   // ����  
    unsigned short nblockalign; 
    unsigned short nBitsPerSample;  // �������      
} WaveFormat; 
#pragma pack() 

/******************************************************************************* 
    CWaveReader�ඨ�壬���ڶ�ȡ.wav�ļ��е���Ƶ����  
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
