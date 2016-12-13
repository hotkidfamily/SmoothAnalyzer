#include "stdafx.h"
#include "wavereader.h"

// CWaveReader¿‡ µœ÷    
CWaveReader::CWaveReader()   
: m_pFile(NULL)
{   
	memset(&m_WaveFormat, 0, sizeof(m_WaveFormat));   
}    

CWaveReader::~CWaveReader()   
{   
	Close();   
}   

bool CWaveReader::Open(const char* pFileName)
{   
	Close();   
	fopen_s(&m_pFile, pFileName, "rb");
	if( !m_pFile )   
		return false;   

	if( !ReadHeader() )
		return false;

	return true;      
}   

void CWaveReader::Close()   
{   
	if(m_pFile) {
		fclose(m_pFile);   
		m_pFile = 0;   
	}
}   

bool CWaveReader::ReadHeader()   
{   
	if(!m_pFile)   
		return false;   

	int32_t Error = 0;   
	do   
	{   
		char data[5] = { 0 };   

		fread(data, 4, 1, m_pFile);   
		if(strcmp(data, "RIFF") != 0) {   
			Error = 1;   
			break;   
		}   

		fseek(m_pFile, 4, SEEK_CUR);   
		memset(data, 0, sizeof(data));   
		fread(data, 4, 1, m_pFile);   
		if(strcmp(data, "WAVE") != 0) { 
			Error = 1;   
			break;   
		}   

		memset(data, 0, sizeof(data));   
		fread(data, 4, 1, m_pFile);   
		if(strcmp(data, "fmt ") != 0) {   
			Error = 1;   
			break;   
		}   

		memset(data, 0, sizeof(data)); 
		fread(data, 4, 1, m_pFile);   

		int32_t nFmtSize =  data[3] << 24;
		nFmtSize	+=  data[2] << 16;
		nFmtSize    +=  data[1] << 8;
		nFmtSize    +=  data[0];

		if(nFmtSize >= 16) {
			if( fread(&m_WaveFormat, 1, sizeof(m_WaveFormat), m_pFile)    
				!= sizeof(m_WaveFormat) ){   
				Error = 1;   
				break;   
			}

			if(nFmtSize == 18){
				memset(data, 0, sizeof(data)); 
				fread(data, 2, 1, m_pFile);   
				short  cbSize = data[1] << 8;
				cbSize += data[0];
				fseek(m_pFile, cbSize, SEEK_CUR);   
			}
		} else {
			return false;
		}

		memset(data, 0, sizeof(data));   
		bool bFindData = false;
		do {
			fread(data, 4, 1, m_pFile);   
			if(strcmp(data, "data") == 0) {   
				bFindData = true;
				break;   
			} 
		}while(!feof(m_pFile));

		if(bFindData){
			fread(&m_nDataLen, 4, 1, m_pFile);          
		} else {
			Error = 1;   
		}
	}while(0);   


	ftell(m_pFile);
	if(0 == Error)   
		return true;
	else
		fseek(m_pFile, 0, 0);

	return false;   
}

double CWaveReader::SampeIndexToSecond(uint32_t sampleIndex)
{
	return sampleIndex*1.0 / m_WaveFormat.nSamplerate;
}

size_t CWaveReader::ReadData(uint8_t* pData, int32_t nLen)   
{      
	size_t size = 0;
	if(m_pFile)
		size = fread(pData, 1, nLen, m_pFile); 
	if(size > 0){
		long pos = ftell(m_pFile);
		m_Progress = pos*100.0 / m_nDataLen;
	}

	fprintf(stderr, "\t progress %.3f\r", Progress());

	return size;   
} 

int32_t CWaveReader::ReadData(std::string &data)
{      
	int32_t ret = 0;
	uint32_t nbReadSamples = m_WaveFormat.nSamplerate / 1;
	uint32_t nbSampleDataSize = nbReadSamples * m_WaveFormat.nblockalign; // 100ms
	int32_t readDataLength = 0;

	data.resize(nbSampleDataSize, 0);
	char* buffer = (char*)data.c_str();

	readDataLength = ReadData((unsigned char*)buffer, data.size());
	if(readDataLength < nbSampleDataSize){ 
		if(readDataLength<0){
			ret = -2;
			goto cleanup;
		}
		ret = EOF;
	}

cleanup:
	return ret;
} 

WaveFormat &CWaveReader::GetFormat()
{   
	return m_WaveFormat;
}   

FILE* CWaveReader::Handle()
{
	return m_pFile;
}
